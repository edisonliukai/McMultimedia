#ifndef _MC_VIDEO_DECODER_H_
#define _MC_VIDEO_DECODER_H_

#include <QObject>
#include "IMcDecoder.h"
#include "IMcVideo.h"

struct AVStream;
struct AVPacket;
struct AVFrame;

struct McVideoDecoderData;

class McVideoDecoder 
	: public QObject
	, public IMcDecoder
	, public IMcVideo
{
	Q_OBJECT

public:
	explicit McVideoDecoder(QObject *parent = 0);
	virtual ~McVideoDecoder();

	// ����ˢ���ַ���
	void setFlushStr(const char *str) noexcept override;
	// ��ʼ��������
	bool init(AVStream *stream) noexcept override;
	// �����Դ��
	void addPacket(AVPacket *packet) noexcept override;
	// ��ȡʣ�������
	int getPacketNum() noexcept override;
	// ���������Դ��
	void clearPacket() noexcept override;

	// ���ý��������Ƶ֡��ʽ
	void setVideoFormat(McVideoFormat::PixelFormat format) noexcept override;
	// ������Ƶ֡�����������Ƶ���ݶ��������֡�С�
	void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept override;
	// ������Ƶ��
	void getVideoData(const std::function<void()> &callback) noexcept override;

private:
	// �ͷ���Դ
	void release() noexcept;
	double getEndClock(AVFrame *frame, double startClock) noexcept;

private:
	QScopedPointer<McVideoDecoderData> d;
};

#endif // !_MC_VIDEO_DECODER_H_