#ifndef _I_MC_AUDIO_H_
#define _I_MC_AUDIO_H_

#include "IMcAudioData.h"

#include <qaudioformat.h>

class IMcAudio : public IMcAudioData {
public:
	virtual ~IMcAudio() = default;

	/*************************************************
	 <��������>		getAudioFormat
	 <��    ��>		��ȡ��Ƶ��ʽ��
	 <����˵��>
	 <����ֵ>		��Ƶ��ʽ
	 <����˵��>		�������������ȡ��Ƶ��ʽ��
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual QAudioFormat getAudioFormat() noexcept = 0;
};

#endif // !_I_MC_AUDIO_H_