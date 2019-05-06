#ifndef _MC_VIDEO_OUTPUT_H_
#define _MC_VIDEO_OUTPUT_H_

#include <QObject>
#include "IMcControl.h"
#include "IMcVideoOutput.h"

class IMcVideo;
class IMcMediaClock;

struct McVideoOutputData;

class McVideoOutput : public QObject, public IMcControl, public IMcVideoOutput {
	Q_OBJECT

public:
	explicit McVideoOutput(QObject *parent = 0);
	~McVideoOutput();

	// ������ƵԴ
	void setVideo(IMcVideo *video) noexcept;

	// ����ͬ��������Ƶ������ݸ�ʱ���ͬ��
	void setMediaClock(IMcMediaClock *mediaClock) noexcept;

	// ������Ƶ��Ⱦ��
	void setRenderer(IMcVideoRenderer *renderer) noexcept override;

	bool start() noexcept override;
	void pause() noexcept override;
	void resume() noexcept override;
	void stop() noexcept override;
	void quit() noexcept override;

	// ��λ ms
	void seek(qint64 pos) noexcept override;

private:
	void release() noexcept;
	void startDetach() noexcept;
	// ��λ ms
	bool mediaSync(qint64 clock) noexcept;

private:
	QScopedPointer<McVideoOutputData> d;
};

#endif // !_MC_VIDEO_OUTPUT_H_