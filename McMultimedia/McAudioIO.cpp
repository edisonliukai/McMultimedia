#include "McAudioIO.h"

#include <qdebug.h>

#include "IMcAudioData.h"
#include "McAudioFrame.h"
#include "McGlobal.h"

struct McAudioIOData {
	IMcAudioData *audio{ nullptr };				// ���ڻ�ȡ��Ƶ����
	QSharedPointer<McAudioFrame> audioFrame;	// ���ڱ�����������Ƶ���ݣ���Ƶ֡�ɱ������ṩ����ֻnewһ�Σ������ظ�new��ɵ�ʱ����ĺ��ڴ���Ƭ�Ĳ���
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

qint64 McAudioIO::readData(char *data, qint64 maxSize) {
	if (maxSize <= 0 || (d->audioFrame->getData() && d->audioFrame->getSize() > maxSize))	// maxSizeС�ڵ���0���߿�д��ռ䲻����д�뵱ǰ֡��ֱ�ӷ���
		return 0;
	qint64 size = 0;
	if (d->audioFrame->getData()) {	// �������δ���ŵ���Ƶ����
		size += mixAudio(data, d->audioFrame);
	}
	while (size < maxSize) {	// �����Ƶ����δд��
		d->audio->getAudioData(d->audioFrame, [&]() {
			if (d->audioFrame->getSize() > maxSize - size)	// ���ʣ��ռ䲻����д�뵱ǰ֡���򱣴�ֱ֡�ӷ���
				return;
			
			size += mixAudio(data + size, d->audioFrame);
		});
		if (d->audioFrame->getData())	// �����ǰ֡���ڣ����ʾʣ��ռ䲻����д�뵱ǰ֡��ֱ���˳�����
			break;
	}
	return size;
}

qint64 McAudioIO::writeData(const char *data, qint64 maxSize) {
	return 0;
}

qint64 McAudioIO::mixAudio(char *buffer, QSharedPointer<McAudioFrame> &frame) noexcept {
	emit signal_clockChanged(frame->getStartClock(), frame->getEndClock());
	qint64 frameSize = frame->getSize();
	memcpy(buffer, frame->getData(), frameSize);
	frame->setData(nullptr);	// ��Ƶ���ݳɹ�д�룬�ÿ�
	return frameSize;
}
