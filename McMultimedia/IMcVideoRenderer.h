#ifndef _I_MC_VIDEO_RENDERER_H_
#define _I_MC_VIDEO_RENDERER_H_

#include "McVideoFormat.h"

class McVideoFrame;

class IMcVideoRenderer {
public:
	virtual ~IMcVideoRenderer() = default;

	/*************************************************
	 <��������>		getVideoFormat
	 <��    ��>		ָ��һ����Ҫ����Ƶ��ʽ
	 <����˵��>		
	 <����ֵ>		��Ƶ��ʽ
	 <����˵��>		�����������ָ��һ����Ҫ����Ƶ��ʽ
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual McVideoFormat::PixelFormat getVideoFormat() noexcept = 0;
	/*************************************************
	 <��������>		setVideoFrame
	 <��    ��>		������Ƶ֡������֮�����Ƶ��Ⱦ
	 <����˵��>		frame ��Ƶ֡
	 <����ֵ>		
	 <����˵��>		�����������������Ƶ֡
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept = 0;
	/*************************************************
	 <��������>		rendering
	 <��    ��>		��Ⱦ��Ƶ
	 <����˵��>		
	 <����ֵ>
	 <����˵��>		�������������Ⱦ��Ƶ
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual void rendering() noexcept = 0;
};

#endif // !_I_MC_VIDEO_RENDERER_H_

