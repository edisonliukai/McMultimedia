#ifndef _MC_AUDIO_DECODER_H_
#define _MC_AUDIO_DECODER_H_

#include <QObject>
#include "IMcDecoder.h"
#include "IMcAudio.h"

struct McAudioDecoderData;

class McAudioDecoder 
	: public QObject
	, public IMcDecoder
	, public IMcAudio
{
	Q_OBJECT

public:
	explicit McAudioDecoder(QObject *parent = 0);
	virtual ~McAudioDecoder();

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
	// ������Ƶ֡�����������Ƶ���ݶ��������֡�С�
	void setAudioFrame(const QSharedPointer<McAudioFrame> &frame) noexcept override;
	// ������Ƶ��
	void getAudioData(const std::function<void()> &callback) noexcept override;
	// ��ȡ��Ƶ��ʽ
	QAudioFormat getAudioFormat() noexcept override;

private:
	// ��ʼ����Ƶת����
	bool init_Swr() noexcept;
	// �ͷ���Դ
	void release() noexcept;
	// �����Ƶ֡
	void clearAudioFrame() noexcept;

private:
	QScopedPointer<McAudioDecoderData> d;
};

#endif // !_MC_AUDIO_DECODER_H_