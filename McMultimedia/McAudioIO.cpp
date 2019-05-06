#include "McAudioIO.h"

#include <qdebug.h>

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
}

McAudioIO::~McAudioIO(){
}

bool McAudioIO::seek(qint64 pos) {
	d->seekPos = pos * 1000;		// msתus
	return true;
}

qint64 McAudioIO::readData(char *data, qint64 maxSize) {
	if (maxSize <= 0)	// maxSizeС�ڵ���0��ֱ�ӷ���
		return 0;
	qint64 size = 0;
	if (d->audioFrame->getData() && d->seekPos == -1 && d->dataIndex < d->audioFrame->getSize()) {	// �������δ���ŵ���Ƶ���ݲ���û����ת
		size = mixAudio(data, 0, d->audioFrame, d->dataIndex, maxSize);
		d->dataIndex += size;
	}
	while (size < maxSize) {	// �����Ƶ����δд��
		d->audio->getAudioData(d->audioFrame, [&, this]() {
			/*	�����������ת���򲻲�������תλ��֮ǰ����Ƶ
				������������תʱ����ת������ʵλ�ÿ�����ϣ��λ��֮ǰ������Ҫȥ�����ⲿ����Ƶ
			*/
			if (d->audioFrame->getStartClock() * 1000 < d->seekPos) {
				d->audioFrame->setData(nullptr);
				size = 0;
				return;
			}
			d->seekPos = -1;

			d->dataIndex = mixAudio(data, size, d->audioFrame, 0, maxSize);
			size += d->dataIndex;
			if (d->dataIndex == d->audioFrame->getSize()) {	// ��Ƶȫ��д��
				d->audioFrame->setData(nullptr);	// ��Ƶ���ݳɹ�д�룬�ÿ�
			}
		});
		if (d->audioFrame->getData())	// �����ǰ֡���ڣ����ʾʣ��ռ䲻����д�뵱ǰ֡��ֱ���˳�����
			break;
	}
	return size;
}

qint64 McAudioIO::writeData(const char *data, qint64 maxSize) {
	return 0;
}

qint64 McAudioIO::mixAudio(char *buffer, qint64 bufferIndex, const QSharedPointer<McAudioFrame> &frame, qint64 dataIndex, qint64 maxSize) noexcept {
	qint64 frameSize = frame->getSize() - dataIndex;	// ��ȡ��ǰ��ʣ�����Ƶ��С
	frameSize = qMin(frameSize, maxSize - bufferIndex);	// ��ȡ��д��Ĵ�С
	emit signal_clockChanged(frame->getStartClock(), dataIndex, frame->getEndClock(), frame->getSize() - dataIndex - frameSize);
	memcpy(buffer + bufferIndex, frame->getData() + dataIndex, frameSize);
	return frameSize;	// ������д����Ƶ��С
}
