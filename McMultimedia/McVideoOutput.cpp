#include "McVideoOutput.h"

#include <QtConcurrent>
#include <qpointer.h>

#include "IMcVideo.h"
#include "IMcMediaClock.h"
#include "McVideoFrame.h"
#include "IMcVideoRenderer.h"
#include "McGlobal.h"

struct McVideoOutputData {
	IMcVideo *video{ nullptr };				// ���ڻ�ȡ��Ƶ����
	IMcMediaClock *mediaClock{ nullptr };	// ý��ʱ�ӣ�����ͬ����Ƶ 

	QPointer<QObject> rendererObj;			// ��Ⱦ�����������ж���Ⱦ���Ƿ�ɾ��
	IMcVideoRenderer *renderer{ nullptr };	// ��Ⱦ��
	QSharedPointer<McVideoFrame> videoFrame;	// ����������Ƶ֡����Ƶ֡�ɱ������ṩ��ֻnewһ�Σ������ظ�new��ɵ�ʱ�����ĺ��ڴ���Ƭ

	bool isQuit{ false };						// �Ƿ�ֹͣ����
	bool isPause{ false };						// �Ƿ���ͣ����
	qint64 seekPos{ -1 };						// ��Ҫ��ת����λ�ã�Ĭ��Ϊ-1����ʾ����ת����λ��us

	QFuture<void> playThread;					// �����߳�
};

McVideoOutput::McVideoOutput(QObject *parent)
	: QObject(parent)
	, d(new McVideoOutputData())
{
	d->videoFrame.reset(new McVideoFrame());
}

McVideoOutput::~McVideoOutput(){
	quit();
}

void McVideoOutput::setVideo(IMcVideo *video) noexcept {
	d->video = video;
	if (d->renderer)
		d->video->setVideoFormat(d->renderer->getVideoFormat());
}

void McVideoOutput::setMediaClock(IMcMediaClock *mediaClock) noexcept {
	d->mediaClock = mediaClock;
}

void McVideoOutput::setRenderer(IMcVideoRenderer *renderer) noexcept {
	if (d->renderer == renderer)
		return;
	MC_SAFE_DELETE(d->renderer);	// ����Ѿ�����һ����Ⱦ������ɾ����
	d->rendererObj = dynamic_cast<QObject *>(renderer);
	d->renderer = renderer;
	d->renderer->setVideoFrame(d->videoFrame);
	if(d->video)
		d->video->setVideoFormat(d->renderer->getVideoFormat());
}

bool McVideoOutput::start() noexcept {
	if (!d->video || !d->renderer)
		return true;

	quit();
	d->seekPos = -1;
	resume();
	d->isQuit = false;
	d->playThread = QtConcurrent::run(this, &McVideoOutput::startDetach);
	return true;
}

void McVideoOutput::pause() noexcept {
	d->isPause = true;
}

void McVideoOutput::resume() noexcept {
	d->isPause = false;
}

void McVideoOutput::stop() noexcept {
	d->isQuit = true;
}

void McVideoOutput::quit() noexcept {
	stop();
	while (!d->playThread.isFinished())
		QThread::msleep(10);
	release();		// �����һ������
}

void McVideoOutput::seek(qint64 pos) noexcept {
	d->seekPos = pos * 1000;		// msתus
}

void McVideoOutput::release() noexcept {
	
}

void McVideoOutput::startDetach() noexcept {
	qint64 lastClock = 0;	// ��λus
	while (!d->isQuit) {
		if (d->isPause) {
			QThread::msleep(10);
			continue;
		}
		d->video->getVideoData(d->videoFrame, [this, &lastClock]() {
			qint64 uClock = d->videoFrame->getClock() * 1000;	// ��λus
			/*	�����������ת����Ƶ֡�����˻��ң��򲻲�������תλ��֮ǰ����Ƶ
				������������תʱ����ת������ʵλ�ÿ�����ϣ��λ��֮ǰ������Ҫȥ�����ⲿ����Ƶ
				�򵱷�����תʱ�������������������ȡ������Ƶ֡�����ݱ�֮ǰ��ʱ�仹Ҫ�����ʾ��Ƶ֡�����˻���
			*/
			if (uClock < d->seekPos || (d->seekPos != -1 && d->seekPos <= lastClock && uClock > lastClock))
				return;
			d->seekPos = -1;
			
			if (mediaSync(d->videoFrame->getClock()) && !d->rendererObj.isNull()) {
				d->renderer->rendering();
				lastClock = uClock;
			}
		});
	}
}

bool McVideoOutput::mediaSync(qint64 clock) noexcept {
	while (!d->isQuit && d->seekPos == -1) {
		qint64 syncClock = d->mediaClock->getMediaClock();
		if (clock < syncClock)
			return true;

		int delayTime = clock - syncClock;

		delayTime = delayTime > 5 ? 5 : delayTime;

		// �����Ƶ���ˣ���ȴ�
		QThread::msleep(delayTime);
	}
	return false;
}
