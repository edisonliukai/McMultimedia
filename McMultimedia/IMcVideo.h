#ifndef _I_MC_VIDEO_H_
#define _I_MC_VIDEO_H_

#include <functional>
#include <qsharedpointer.h>
#include "McVideoFormat.h"

class McVideoFrame;

class IMcVideo {
public:
	virtual ~IMcVideo() = default;

	/*************************************************
	 <��������>		setVideoFormat
	 <��    ��>		������Ƶ��ʽ���ø�ʽ���������������Ƶ֡�ĸ�ʽ
	 <����˵��>		format	��Ƶ��ʽ
	 <����ֵ>
	 <����˵��>		�����������������Ƶ��ʽ
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual void setVideoFormat(McVideoFormat::PixelFormat format) noexcept = 0;
	/*************************************************
	 <��������>		getVideoData
	 <��    ��>		������Ƶ
	 <����˵��>		frame ��Ƶ֡�����������Ƶ���������Ƶ֡��
					callback �ص��������������ɹ�������ô˺��������򲻵���
	 <����ֵ>
	 <����˵��>		�����������������Ƶ
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual void getVideoData(const QSharedPointer<McVideoFrame> &frame, const std::function<void()> &callback) noexcept = 0;
};

#endif // !_I_MC_VIDEO_H_

