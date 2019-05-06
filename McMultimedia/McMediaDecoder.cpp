#include "McMediaDecoder.h"

extern "C" {
#include "libavformat/avformat.h"
};

#include <QtConcurrent>
#include <qdebug.h>

#include "IMcDecoder.h"
#include "McGlobal.h"

#define FLUSH "FLUSH"

struct McMediaDecoderData {
	AVFormatContext *formatCtx{ nullptr };
	AVPacket seekPacket;						// ��ת������Դ��

	QMap<AVMediaType, IMcDecoder *> decoders;	// ������Ž����������������ⲿע�룬ע��ʱ����ָ������������

	QString mediaUrl;							// ý��·��
	int initSuccessNum{ 0 };					// ��������ʼ���ɹ�������
	bool isQuit{ false };						// �Ƿ�ֹͣ����
	bool isPause{ false };						// �Ƿ���ͣ����

	qint64 seekPos{ -1 };						// ��Ҫ��ת����λ�ã�Ĭ��Ϊ-1����ʾ����ת����λ��us

	QFuture<void> decodeThread;					// �����߳�
};

McMediaDecoder::McMediaDecoder(QObject *parent)
	: QObject(parent)
	, d(new McMediaDecoderData())
{
	// ��ʼ����ת��Դ��
	av_new_packet(&d->seekPacket, qstrlen(FLUSH) + 1);
	d->seekPacket.data = (uint8_t *)FLUSH;
}

McMediaDecoder::~McMediaDecoder(){
	quit();
	av_packet_unref(&d->seekPacket);
}

void McMediaDecoder::setMediaUrl(const QString &url) noexcept {
	d->mediaUrl = url;
}

bool McMediaDecoder::start() noexcept {
	quit();	// ��ʼʱ�˳���ǰ�Ľ�����
	if (!initDecoder(d->mediaUrl.toLocal8Bit().data()))	// ��ʼ��������
		return false;
	d->seekPos = -1;
	resume();
	d->isQuit = false;
	// ��̨����
	d->decodeThread = QtConcurrent::run(this, &McMediaDecoder::decodeDetached);
	return true;
}

void McMediaDecoder::pause() noexcept {
	d->isPause = true;
}

void McMediaDecoder::resume() noexcept {
	d->isPause = false;
}

void McMediaDecoder::stop() noexcept {
	d->isQuit = true;
}

void McMediaDecoder::quit() noexcept {
	stop();
	while (!d->decodeThread.isFinished())	// �ȴ�����
		QThread::msleep(10);
	release();		// �����һ������
}

void McMediaDecoder::seek(qint64 pos) noexcept {
	d->seekPos = pos * 1000;		// msתus
}

void McMediaDecoder::addDecoder(DecoderType type, IMcDecoder *decoder) noexcept {
	decoder->setFlushStr(FLUSH);	// ��ӽ�����ʱ����ˢ���ַ���
	AVMediaType avType{ AVMEDIA_TYPE_UNKNOWN };
	switch (type)
	{
	case McMediaDecoder::DecoderType::AUDIO:
		avType = AVMEDIA_TYPE_AUDIO;
		break;
	case McMediaDecoder::DecoderType::VIDEO:
		avType = AVMEDIA_TYPE_VIDEO;
		break;
	case McMediaDecoder::DecoderType::SUBTITLE:
		avType = AVMEDIA_TYPE_SUBTITLE;
		break;
	default:
		break;
	}
	d->decoders.insert(avType, decoder);
}

void McMediaDecoder::release() noexcept {
	if (d->formatCtx)
		avformat_close_input(&d->formatCtx);	// �Զ��ÿ�
}

bool McMediaDecoder::initDecoder(const char *url) noexcept {
	// ��ý��
	AVDictionary *dict = NULL;
	av_dict_set(&dict, "protocol_whitelist", "file,crypto,http,tcp", 0);	// ����Э�鱨����
	av_dict_set(&dict, "allowed_extensions", "ALL", 0);
	if (avformat_open_input(&d->formatCtx, url, NULL, &dict) != 0) {
		qDebug() << "Couldn't open input stream.";
		return false;
	}

	// �����Ƿ����stream
	if (avformat_find_stream_info(d->formatCtx, NULL) < 0) {
		qDebug() << "Couldn't find stream information.";
		return false;
	}

	d->initSuccessNum = 0;		// ��ճ�ʼ���ɹ�������
	for (int i = 0; i < d->formatCtx->nb_streams; ++i) {
		AVStream *stream = d->formatCtx->streams[i];	// ý����
		AVMediaType type = d->formatCtx->streams[i]->codec->codec_type;		// ý��������
		if (d->decoders.contains(type)) {	// ���ע��������͵Ľ����������ʼ��������
			d->initSuccessNum += d->decoders[type]->init(stream) ? 1 : 0;	// �ɹ���һ
		}
	}
	// ���û���κ�һ����������ʼ���ɹ�����ֱ�ӷ��أ���ʾ��Ƶ����ʧ��
	if (d->initSuccessNum == 0) {
		qDebug() << tr("û���κν�������ʼ���ɹ���ý���޷�����");
		return false;
	}

	qint64 mediaDuration = d->formatCtx->duration;	// ��λ��us
	emit signal_durationChanged(mediaDuration / 1000L);		// ��λms

	//Output Info-----------------------------
	printf("---------------- File Information ---------------\n");
	av_dump_format(d->formatCtx, 0, url, 0);
	printf("-------------------------------------------------\n");

	return true;
}

void McMediaDecoder::decodeDetached() noexcept {
	// ����һ��packet�����ڽ��ն��������ݰ�
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	
	bool isRepeat = false;			// �Ƿ���Ҫ�ظ�����
	bool isReadFinished = false;	// �����Ƿ����
	while (!d->isQuit) {
		qint64 seekPos = d->seekPos;	// ��¼��Ҫ��ת����λ��
		d->seekPos = -1;	// ��ת����������Ҫ��ת��λ����Ϊ-1����ʾ����Ҫ��ת
		// ������ŵ���ƵΪm3u8��ʽ��������Ҫ��ת����λ��Ϊ��Ƶ��ʼ�������½��벥��
		if (seekPos != -1 && seekPos < d->formatCtx->start_time && d->mediaUrl.endsWith(".m3u8")) {
			isRepeat = true;
			break;
		}
		// ��ת
		if (seekTo(seekPos)) {
			// �����ת�ɹ�
			MC_LOOP_CALL_P(d->decoders, clearPacket());	// �������������Դ��
			MC_LOOP_CALL_P(d->decoders, addPacket(&d->seekPacket));	// �����ת��Դ��
			continue;
		}
		if (d->isPause) {
			QThread::msleep(10);
			continue;
		}
		if (isContinue()) {
			QThread::msleep(10);
			continue;
		}
		// �����Ƶ֡����ʧ�ܣ�������һ��ʱ��
		if (av_read_frame(d->formatCtx, packet) < 0) {
			if (isFinished() && !isReadFinished) {
				emit signal_decodeFinished();
				isReadFinished = true;
				break;
			}
			QThread::msleep(10);
			continue;
		}
		isReadFinished = false;
		MC_LOOP_CALL_P(d->decoders, addPacket(packet));
		av_packet_unref(packet);
	}

	av_freep(&packet);

	if (isRepeat) {
		QtConcurrent::run([this]() {
			MC_LOOP_CALL_P(d->decoders, clearPacket());
			start();
		});
	}
}

bool McMediaDecoder::isContinue() noexcept {
	int iContinueNum = 0;	// ��ʾ�������еİ�����̫�࣬��Ҫ�����Ľ������ĸ���
	for (IMcDecoder *decoder : d->decoders) {
		// ����������еİ�����̫�࣬�ý����������������˴���������Ϊ100
		iContinueNum += (decoder->getPacketNum() > 100) ? 1 : 0;
	}
	// �����Ҫ�����Ľ����������ڽ���������������ʱ���ٶ�ȡý���
	if (iContinueNum == d->initSuccessNum) {
		return true;
	}
	return false;
}

bool McMediaDecoder::isFinished() noexcept {
	int isFinishedNum = 0;	// ��ʾ�������еİ�����Ϊ0�Ľ���������
	for (IMcDecoder *decoder : d->decoders) {
		isFinishedNum += (decoder->getPacketNum() == 0) ? 1 : 0;
	}
	// ���������Ϊ0�Ľ����������ڽ�����������������ɽ���
	if (isFinishedNum == d->initSuccessNum) {
		return true;
	}
	return false;
}

bool McMediaDecoder::seekTo(qint64 &pos) noexcept {
	// ����Ҫ��ת��λ��Ϊ-1ʱ����ʾ����Ҫ��ת�������ʾ��Ҫ��ת
	if (pos == -1)
		return false;
	// �˴��ڶ�������Ϊý������index���˴�ָ��Ϊ-1����ô������������Ϊ΢��Ƶ�ý��ʱ��
	/*	���ָ��Ϊ����ý����index����ô��Ҫͨ��
			AVRational aVRational = av_get_time_base_q();
			seekPos = av_rescale_q(seekPos, aVRational, pFormatCtx->streams[seekIndex]->time_base);
		�������������������ֵ
	*/
	// AVSEEK_FLAG_BACKWARD��ʾ���0��2Ϊ�ؼ�֡������ת��1֡�Ļ�������0֡��ʼ����
	//if (av_seek_frame(m_pFormatCtx, -1, pos, AVSEEK_FLAG_BACKWARD) < 0) {
	int seekFlag = 0;
	if (d->mediaUrl.endsWith(".m3u8")) {
		//seekFlag |= AVSEEK_FLAG_ANY;
		//seekFlag |= AVSEEK_FLAG_FRAME;
		seekFlag |= AVSEEK_FLAG_BACKWARD;
	}
	if (avformat_seek_file(d->formatCtx, -1, INT64_MIN, pos, INT64_MAX, seekFlag) < 0) {
		if (pos < d->formatCtx->start_time) {
			pos = d->formatCtx->start_time;
			return seekTo(pos);
		}
		qCritical() << "Seek failed.";
		return false;
	}

	return true;
}
