#ifndef _I_MC_MEDIA_CLOCK_H_
#define _I_MC_MEDIA_CLOCK_H_

#include <qglobal.h>

class IMcMediaClock {
public:
	virtual ~IMcMediaClock() = default;

	/*************************************************
	 <��������>		getMediaClock
	 <��    ��>		��ȡͬ��ʱ�������ʱ�������ͬ������ý�壬����ʹ����Ƶ��ͬ����Ƶ
	 <����˵��>
	 <����ֵ>		��ǰʱ���
	 <����˵��>		�������������ȡͬ��ʱ���
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual qint64 getMediaClock() noexcept = 0;
};

#endif // !_I_MC_MEDIA_CLOCK_H_

