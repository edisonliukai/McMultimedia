#include "McAudioOutput.h"

#include <qaudiooutput.h>
#include <qdebug.h>

#include "IMcAudio.h"
#include "McAudioIO.h"
#include "McGlobal.h"

struct McAudioOutputData {
	IMcAudio *audio{ nullptr };			// ���ڻ�ȡ��Ƶ����

	QIODevice *io{ nullptr };			// IO
	QAudioOutput *output{ nullptr };	// ��Ƶ����豸

	qint64 audioEndClock{ 0 };			// ��ǰ��д����Ƶ�豸�����ݵĽ���ʱ��
	bool isStop{ false };				// ��Ƶ����Ƿ�ֹͣ
};

McAudioOutput::McAudioOutput(QObject *parent)
	: QObject(parent)
	, d(new McAudioOutputData())
{
}

McAudioOutput::~McAudioOutput(){
	quit();
}

qint64 McAudioOutput::getMediaClock() noexcept {
	if (!d->audio)
		return -1;
	double clock = d->audioEndClock / 1000.0;	// msתs
	int hwBufSize = d->output->bufferSize() - d->output->bytesFree();	// ��δ������ȫ����Ƶ����
	clock -= getPlayTime(hwBufSize);
	return clock * 1000;	// sתms
}

void McAudioOutput::setAudio(IMcAudio *audio) noexcept {
	d->audio = audio;
}

bool McAudioOutput::start() noexcept {
	if (!d->audio)
		return true;
	quit();
	d->isStop = false;	// ��ʼ��Ƶ���
	d->output = new QAudioOutput(d->audio->getAudioFormat(), this);
	McAudioIO *io = new McAudioIO(d->audio, d->output);

	connect(d->output, &QAudioOutput::stateChanged, [this](QAudio::State state) {
		if (state == QAudio::IdleState && !d->isStop) {	// �����Ƶ�豸Ϊ����״̬������Ҫ���¿�ʼ����
			pause();
			resume();
		}
	});
	connect(io, &McAudioIO::signal_clockChanged, [this](qint64 startClock, qint64 startIndex, qint64 endClock, qint64 remainDataSize) {
		startClock += (getPlayTime(startIndex) * 1000);	// sתms
		emit signal_clockChanged(startClock);
		endClock -= (getPlayTime(remainDataSize) * 1000);	// sתms
		d->audioEndClock = endClock;
	});

	d->output->start(io);
	d->io = io;
	return true;
}

void McAudioOutput::pause() noexcept {
	MC_SAFE_CALL(d->output, suspend());
}

void McAudioOutput::resume() noexcept {
	MC_SAFE_CALL(d->output, resume());
}

void McAudioOutput::stop() noexcept {
	pause();	// �˴�������stop������ֻ��ͣ��Ƶ���
	d->isStop = true;	// ֹͣ��Ƶ���
}

void McAudioOutput::quit() noexcept {
	MC_SAFE_CALL(d->output, stop());	// �ͷ���Ƶ��Դ
	release();		// �ͷű�����Դ�����¿�ʼ
}

void McAudioOutput::seek(qint64 pos) noexcept {
	d->audioEndClock = pos;
	MC_SAFE_CALL(d->io, seek(pos));
}

void McAudioOutput::release() noexcept {
	MC_SAFE_CALL(d->output, deleteLater());
	d->output = nullptr;
	d->io = nullptr;
}

double McAudioOutput::getPlayTime(int bufSize) noexcept {
	QAudioFormat format = d->audio->getAudioFormat();
	int bytesPerSec = format.sampleRate() * format.channelCount() * format.sampleSize() / 8;
	return static_cast<double>(bufSize) / bytesPerSec;
}
