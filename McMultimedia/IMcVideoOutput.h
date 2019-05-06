#ifndef _I_MC_VIDEO_OUTPUT_H_
#define _I_MC_VIDEO_OUTPUT_H_

class IMcVideoRenderer;

class IMcVideoOutput {
public:
	virtual ~IMcVideoOutput() = default;

	/*************************************************
	 <��������>		setRenderer
	 <��    ��>		������Ⱦ��
	 <����˵��>		renderer ��Ⱦ��
	 <����ֵ>
	 <����˵��>		�����������������Ⱦ��
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual void setRenderer(IMcVideoRenderer *renderer) noexcept = 0;
};

#endif // !_I_MC_VIDEO_OUTPUT_H_

