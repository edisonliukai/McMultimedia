#ifndef _MC_AUDIO_IO_H_
#define _MC_AUDIO_IO_H_

#include <QIODevice>

class IMcAudioData;
class McAudioFrame;

struct McAudioIOData;

class McAudioIO : public QIODevice {
	Q_OBJECT

public:
	explicit McAudioIO(IMcAudioData *audio, QObject *parent = 0);
	~McAudioIO();

	void seekTo(qint64 pos) noexcept;

signals:
	/*************************************************
	 <��������>		signal_clockChanged
	 <��    ��>		�źţ����ͳ���ǰ��Ƶ�Ĺؼ���Ϣ
	 <����˵��>		startClock ��ʼʱ�䣬��ǰ������Ƶ֡�Ŀ�ʼʱ�䣬�����Ҫ��ȡ��ʵʱ�䣬��Ҫ����startIndex��Ӧ��ʱ�� ��λ ms
					startIndex д����Ƶ�豸�ĵ�ǰ��Ƶ֡����ʼλ�ã�Ҳ��ʾ֮ǰд������ݴ�С
					endClock ��ǰ������Ƶ֡�Ľ���ʱ�䣬�����Ҫ��ȡ��ʵʱ�䣬��Ҫ��ȥremainDataSize��Ӧ��ʱ�� ��λ ms
					remainDataSize ��ǰ֡��ʣ��δд�����Ƶ����
	 <����ֵ>
	 <����˵��>		��������������ͳ���ǰ��Ƶ�Ĺؼ���Ϣ
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	void signal_clockChanged(qint64 startClock, qint64 startIndex, qint64 endClock, qint64 remainDataSize);

protected:
	qint64 readData(char * data, qint64 maxSize) override;
	qint64 writeData(const char * data, qint64 maxSize) override;

private:
	/*************************************************
	 <��������>		mixAudio
	 <��    ��>		д����Ƶ����
	 <����˵��>		buffer ��Ƶ������
					bufferIndex ��ʾ��Ҫ����Ƶ�����������λ�ÿ�ʼд�룬Ҳ��ʾ֮ǰ�Ѿ�д��Ĵ�С
					frame ��Ƶ֡
					dataIndex ��ʾ����Ƶ֡�ĵ�ǰλ��д�룬Ҳ��ʾ��ǰ��Ƶ֡�Ѿ�д��Ĵ�С
					maxSize ��Ƶ����������д���С
	 <����ֵ>
	 <����˵��>		�����������д����Ƶ����
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	qint64 mixAudio(char *buffer, qint64 bufferIndex, const QSharedPointer<McAudioFrame> &frame, qint64 dataIndex, qint64 maxSize) noexcept;

private:
	QScopedPointer<McAudioIOData> d;
};

#endif // !_MC_AUDIO_IO_H_