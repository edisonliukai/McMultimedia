#include "McAudioIO.h"

#include <qdebug.h>
#include <qmutex.h>

#include "IMcAudioData.h"
#include "McAudioFrame.h"
#include "McGlobal.h"

struct McAudioIOData {
	IMcAudioData *audio{ nullptr };				// ���ڻ�ȡ��Ƶ����
	QSharedPointer<McAudioFrame> audioFrame;	// ���ڱ�����������Ƶ���ݣ���Ƶ֡�ɱ������ṩ����ֻnewһ�Σ������ظ�new��ɵ�ʱ����ĺ��ڴ���Ƭ�Ĳ���
	qint64 dataIndex{ 0 };						// �������δ���ŵ���Ƶ���ݣ����ʾ�����ݵ���ʼ�±�
	qint64 seekPos{ -1 };						// ��Ҫ��ת����λ�ã�Ĭ��Ϊ-1����ʾ����ת����λ��us
};

McAudioIO::McAudioIO(IMcAudioData *audio, QObject *parent)
	: QIODevice(parent)
	, d(new McAudioIOData())
{
	open(QIODevice::ReadWrite);
	d->audio = audio;
	d->audioFrame.reset(new McAudioFrame());	// ������Ƶ֡
	d->audio->setAudioFrame(d->audioFrame);
}

McAudioIO::~McAudioIO(){
}

void McAudioIO::seekTo(qint64 pos) noexcept {
	d->seekPos = pos * 1000;		// msתus
}

qint64 McAudioIO::readData(char *data, qint64 maxSize) {
	if (maxSize <= 0)	// maxSizeС�ڵ���0��ֱ�ӷ���
		return 0;
	qint64 size = 0;
	// �������δ���ŵ���Ƶ���ݲ������û����ת���򲥷�ʣ������
	if (d->audioFrame->getData() && d->dataIndex < d->audioFrame->getSize() && d->seekPos == -1) {
		size = mixAudio(data, 0, d->audioFrame, d->dataIndex, maxSize);
		d->dataIndex += size;
	}
	while (size < maxSize) {	// �����Ƶ����δд��ʱ
		d->audio->getAudioData([&, this]() {
			/*	�����������ת���򲻲�������תλ��֮ǰ����Ƶ
				������������תʱ����ת������ʵλ�ÿ�����ϣ��λ��֮ǰ������Ҫȥ�����ⲿ����Ƶ
			*/
			if (d->audioFrame->getStartClock() * 1000 < d->seekPos) {
				size = 0;
				return;
			}
			d->seekPos = -1;

			d->dataIndex = mixAudio(data, size, d->audioFrame, 0, maxSize);
			size += d->dataIndex;
		});
		// ������ݲ����ڻ������ݴ�СС�ڵ���0����ʾ����û�����ݿɶ�ȡ����ֱ���˳�����
		if (!d->audioFrame->getData() || d->audioFrame->getSize() <= 0)
			break;
	}
	return size;
}

qint64 McAudioIO::writeData(const char *data, qint64 maxSize) {
	return 0;
}

qint64 McAudioIO::mixAudio(char *buffer, qint64 bufferIndex, const QSharedPointer<McAudioFrame> &frame, qint64 dataIndex, qint64 maxSize) noexcept {
	QMutexLocker locker(&d->audioFrame->getMutex());
	if (!frame->getData() || frame->getSize() <= 0)	// �����Ƶ���ݲ����ڣ�������Ƶ��СС�ڵ���0����д�룬ֱ�ӷ���
		return 0;
	qint64 frameSize = frame->getSize() - dataIndex;	// ��ȡ��ǰ��ʣ�����Ƶ��С
	frameSize = frameSize < 0 ? 0 : frameSize;			// ��Сֵ����С��0
	frameSize = qMin(frameSize, maxSize - bufferIndex);	// ��ȡ��д��Ĵ�С
	emit signal_clockChanged(frame->getStartClock(), dataIndex, frame->getEndClock(), frame->getSize() - dataIndex - frameSize);
	memcpy(buffer + bufferIndex, frame->getData() + dataIndex, frameSize);
	return frameSize;	// ������д����Ƶ��С
}
