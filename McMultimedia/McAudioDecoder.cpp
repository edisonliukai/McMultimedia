#include "McAudioDecoder.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};

#include <qqueue.h>
#include <qmutex.h>
#include <qdebug.h>
#include <qaudioformat.h>

#include "McAVPacketMan.h"
#include "McAudioFrame.h"

struct McAudioDecoderData {
	AVStream *audioStream{ nullptr };					// ��Ƶ�������ⲿ����
	AVCodecContext *codecContext{ nullptr };			// context
	AVFrame *frame{ nullptr };							// ��Ƶ֡����������ʼ��ʱ��ʼ�������ڽ�����Ƶ��
	SwrContext *audioConvertCtx{ nullptr };				// ������Ƶת��

	QQueue<AVPacket> audioPackets;						// ���ڴ洢�������Ƶ��
	QMutex mtx;											// ��Ƶ����ͬ����
	QAudioFormat audioFormat;							// ��Ƶ��ʽ
	AVSampleFormat sampleFormat{ AV_SAMPLE_FMT_FLT };	// Ŀ���ʽ

	DECLARE_ALIGNED(16, quint8, audioData)[192000];		// ������ʱ��ű����������Ƶ����
	const char *flushStr{ nullptr };
};

McAudioDecoder::McAudioDecoder(QObject *parent)
	: QObject(parent)
	, d(new McAudioDecoderData())
{
}

McAudioDecoder::~McAudioDecoder(){
}

void McAudioDecoder::setFlushStr(const char *str) noexcept {
	d->flushStr = str;
}

bool McAudioDecoder::init(AVStream *stream) noexcept {
	if (!stream) {
		return false;
	}

	AVCodec *pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (pCodec == NULL) {
		qCritical() << "Audio codec not found.";
		return false;
	}
	d->codecContext = avcodec_alloc_context3(pCodec);
	avcodec_parameters_to_context(d->codecContext, stream->codecpar);
	if (avcodec_open2(d->codecContext, pCodec, NULL) < 0) {
		qCritical() << "Could not open Audio codec.";
		return false;
	}

	d->audioFormat.setChannelCount(d->codecContext->channels);
	d->audioFormat.setSampleRate(d->codecContext->sample_rate);
	d->audioFormat.setSampleSize(8 * av_get_bytes_per_sample(d->sampleFormat));
	d->audioFormat.setSampleType(QAudioFormat::Float);
	d->audioFormat.setCodec("audio/pcm");

	d->frame = av_frame_alloc();

	d->audioStream = stream;

	return init_Swr();
}

void McAudioDecoder::addPacket(AVPacket *packet) noexcept {
	if (!packet || !d->audioStream) {
		return;
	}
	if (packet->stream_index != d->audioStream->index) {
		return;
	}
	AVPacket tmpPck;
	av_packet_ref(&tmpPck, packet);
	QMutexLocker locker(&d->mtx);
	d->audioPackets.enqueue(tmpPck);
}

void McAudioDecoder::getAudioData(const QSharedPointer<McAudioFrame> &frame, const std::function<void()> &callback) noexcept {
	if (!d->audioStream) {
		qCritical() << "audio stream not found, please make sure media started decode";
		return;
	}

	QMutexLocker locker(&d->mtx);
	if (d->audioPackets.isEmpty())
		return;
	McAVPacketMan packet(&d->audioPackets.dequeue());
	locker.unlock();

	//�յ�������� ˵���ո�ִ�й���ת ������Ҫ�ѽ����������� ���һ��        
	if (strcmp((char*)packet->data, d->flushStr) == 0) {
		avcodec_flush_buffers(d->codecContext);
		return;
	}

	/* while return -11 means packet have data not resolved,
	 * this packet cannot be unref
	 */
	int ret = avcodec_send_packet(d->codecContext, packet.data());
	if (ret < 0) {
		qCritical() << "Audio send to decoder failed, error code: " << ret;
		return;
	}

	int resampledDataSize;	// ���������Ƶ��С
	double audioClock{ 0.0 };	// ��ǰ��Ƶʱ���
	while ((ret = avcodec_receive_frame(d->codecContext, d->frame)) == 0) {
		if (d->frame->pts == AV_NOPTS_VALUE) {
			continue;
		}

		audioClock = av_q2d(d->audioStream->time_base) * d->frame->pts;	// ��ʼ���ŵ�ʱ��

		frame->setStartClock(audioClock * 1000);	// sתms

		const quint8 **in = (const quint8 **)d->frame->extended_data;
		uint8_t *out[] = { d->audioData };

		int outCount = sizeof(d->audioData) / d->codecContext->channels / av_get_bytes_per_sample(d->sampleFormat);

		int sampleSize = swr_convert(d->audioConvertCtx, out, outCount, in, d->frame->nb_samples);
		if (sampleSize < 0) {
			qCritical() << "swr convert failed";
			continue;
		}

		if (sampleSize == outCount)
			qCritical() << "audio buffer is probably too small";

		resampledDataSize = sampleSize * d->codecContext->channels * av_get_bytes_per_sample(d->sampleFormat);

		// ���Ž�����ʱ��
		audioClock += static_cast<double>(resampledDataSize) /
			(av_get_bytes_per_sample(d->sampleFormat) * d->codecContext->channels * d->codecContext->sample_rate);

		frame->setData(d->audioData);
		frame->setSize(resampledDataSize);
		frame->setEndClock(audioClock * 1000);	// sתms
		callback();
	}
}

QAudioFormat McAudioDecoder::getAudioFormat() noexcept {
	return d->audioFormat;
}

void McAudioDecoder::release() noexcept {
	if (d->audioConvertCtx) 
		swr_free(&d->audioConvertCtx);	// �Զ��ÿ�
	if (d->frame)
		av_frame_free(&d->frame);	// �Զ��ÿ�
	if (d->codecContext) {
		avcodec_close(d->codecContext);
		avcodec_free_context(&d->codecContext);	// �Զ��ÿ�
	}
	d->audioStream = nullptr;
	clearPacket();
}

void McAudioDecoder::clearPacket() noexcept {
	QMutexLocker locker(&d->mtx);
	for (AVPacket &packet : d->audioPackets) {
		av_packet_unref(&packet);
	}
	d->audioPackets.clear();
}

bool McAudioDecoder::init_Swr() noexcept {
	/* get audio channels */
	qint64 inChannelLayout = (d->audioStream->codecpar->channel_layout && d->audioStream->codecpar->channels == av_get_channel_layout_nb_channels(d->audioStream->codecpar->channel_layout)) ?
		d->audioStream->codecpar->channel_layout : av_get_default_channel_layout(d->audioStream->codecpar->channels);

	/* init swr audio convert context */
	qint64 channelLayout = av_get_default_channel_layout(d->codecContext->channels);
	channelLayout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
	/*	�˺����ĵ�һ���������Ϊnullptr���������ڴ沢����ͨ������ֵ���أ�
		�����Ϊnullptr���򲻻����·����ڴ棬��ֱ����ԭָ�������ò���
	*/
	d->audioConvertCtx = swr_alloc_set_opts(d->audioConvertCtx, channelLayout, d->sampleFormat
		, d->audioStream->codecpar->sample_rate,
		inChannelLayout, (AVSampleFormat)d->audioStream->codecpar->format, d->audioStream->codecpar->sample_rate, 0, NULL);
	if (!d->audioConvertCtx || (swr_init(d->audioConvertCtx) < 0)) {
		qCritical() << "cannot init swr!!";
		return false;
	}

	return true;
}
