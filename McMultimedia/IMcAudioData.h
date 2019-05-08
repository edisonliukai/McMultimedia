#ifndef _I_MC_AUDIO_DATA_H_
#define _I_MC_AUDIO_DATA_H_

#include <functional>
#include <qsharedpointer.h>

class McAudioFrame;

class IMcAudioData {
public:
	virtual ~IMcAudioData() = default;

	/*************************************************
	 <��������>		setAudioFrame
	 <��    ��>		������Ƶ֡�����������Ƶ���ݶ��������֡�С�
	 <����˵��>		frame ��Ƶ֡�����������Ƶ���������Ƶ֡��
	 <����ֵ>
	 <����˵��>		�����������������Ƶ֡��
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/7
	**************************************************/
	virtual void setAudioFrame(const QSharedPointer<McAudioFrame> &frame) noexcept = 0;
	/*************************************************
	 <��������>		getAudioData
	 <��    ��>		������Ƶ��
	 <����˵��>		callback �ص��������������ɹ�������ô˺��������򲻵���
	 <����ֵ>		
	 <����˵��>		�����������������Ƶ��
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual void getAudioData(const std::function<void()> &callback) noexcept = 0;
};

#endif // !_I_MC_AUDIO_DATA_H_