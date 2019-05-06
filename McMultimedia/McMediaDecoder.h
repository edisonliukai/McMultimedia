#ifndef _MC_MEDIA_DECODER_H_
#define _MC_MEDIA_DECODER_H_
#pragma execution_character_set("utf-8")

#include <QObject>
#include "IMcControl.h"
#include "IMcMediaDecoder.h"

enum AVMediaType;

struct McMediaDecoderData;

class IMcDecoder;

class McMediaDecoder : public QObject, public IMcControl, public IMcMediaDecoder {
	Q_OBJECT

public:
	enum class DecoderType {
		AUDIO,
		VIDEO,
		SUBTITLE
	};

	explicit McMediaDecoder(QObject *parent = 0);
	~McMediaDecoder();

	// ��ʼ
	bool start() noexcept override;
	// ��ͣ
	void pause() noexcept override;
	// �ָ�
	void resume() noexcept override;
	// ֹͣ
	void stop() noexcept override;
	// �˳�
	void quit() noexcept override;

	// ��ת
	void seek(qint64 pos) noexcept override;

	// ����ý��·��
	void setMediaUrl(const QString &url) noexcept override;
	// ��ӽ�����
	void addDecoder(DecoderType type, IMcDecoder *decoder) noexcept;

signals:
	// ý����ʱ������λ��ms
	void signal_durationChanged(qint64 duration);
	// �������
	void signal_decodeFinished();

private:
	// �ͷ���Դ
	void release() noexcept;
	// ��ʼ��������
	bool initDecoder(const char *url) noexcept;
	// ��̨���룬ʹ��QtConcurrent
	void decodeDetached() noexcept;
	// �ж��Ƿ��������ν���
	bool isContinue() noexcept;
	// �ж��Ƿ�������
	bool isFinished() noexcept;
	// ��������ת
	bool seekTo(qint64 &pos) noexcept;

private:
	QScopedPointer<McMediaDecoderData> d;
};

#endif // !_MC_MEDIA_DECODER_H_