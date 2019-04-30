#include "McMediaDecoder.h"

extern "C" {
#include "libavformat/avformat.h"
};

#include <QtConcurrent>
#include <qdebug.h>

#include "IMcDecoder.h"

#define FLUSH "FLUSH"

struct McMediaDecoderData {
	AVFormatContext *formatCtx{ nullptr };
	AVPacket seekPacket;

	QMap<AVMediaType, IMcDecoder *> decoders;		// ������Ž����������������ⲿע�룬ע��ʱ����ָ������������

	QString mediaUrl;
	int initSuccessNum{ 0 };						// ��������ʼ���ɹ�������
	bool isQuit{ false };						// �Ƿ�ֹͣ����
	bool isPause{ false };						// �Ƿ���ͣ����
};

McMediaDecoder::McMediaDecoder(QObject *parent)
	: QObject(parent)
	, d(new McMediaDecoderData())
{
	av_init_packet(&d->seekPacket);
	d->seekPacket.data = (uint8_t *)FLUSH;
}

McMediaDecoder::~McMediaDecoder(){
	av_packet_unref(&d->seekPacket);
}

void McMediaDecoder::setMediaUrl(const QString &url) noexcept {
	d->mediaUrl = url;
}

void McMediaDecoder::start() noexcept {
	if (!initDecoder(d->mediaUrl.toLocal8Bit().data()))
		return;
	// ��̨����
	QtConcurrent::run(this, &McMediaDecoder::decodeDetached);
}

void McMediaDecoder::stop() noexcept {
	d->isQuit = true;
}

void McMediaDecoder::addDecoder(DecoderType type, IMcDecoder *decoder) noexcept {
	decoder->setFlushStr(FLUSH);
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

bool McMediaDecoder::initDecoder(const char *url) {
	// ��ý��
	AVDictionary *dict = NULL;
	av_dict_set(&dict, "protocol_whitelist", "file,crypto,http,tcp", 0);
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
	//emit signal_durationChanged(mediaDuration / 1000L);		// ��λms

	//Output Info-----------------------------
	printf("---------------- File Information ---------------\n");
	av_dump_format(d->formatCtx, 0, url, 0);
	printf("-------------------------------------------------\n");

	return true;
}

void McMediaDecoder::decodeDetached() {
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	
	while (!d->isQuit) {
		if (d->isPause) {
			QThread::msleep(10);
			continue;
		}
		// �����Ƶ֡����ʧ�ܣ�������һ��ʱ��
		if (av_read_frame(d->formatCtx, packet) < 0) {
			QThread::msleep(10);
			continue;
		}
		for (IMcDecoder *decoder : d->decoders) {
			decoder->addPacket(packet);
		}
		av_packet_unref(packet);
	}

	av_freep(&packet);
}
