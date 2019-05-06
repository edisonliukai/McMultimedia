#ifndef _I_MC_MEDIA_DECODER_H_
#define _I_MC_MEDIA_DECODER_H_

#include <qstring.h>

class IMcMediaDecoder {
public:
	virtual ~IMcMediaDecoder() = default;

	/*************************************************
	 <��������>		setMediaUrl
	 <��    ��>		����ý��·��
	 <����˵��>		url	ý��·���������Ǳ���·����Ҳ����������·��
	 <����ֵ>		
	 <����˵��>		���������������ý��·��
	 <��    ��>		mrcao
	 <ʱ    ��>		2019/5/6
	**************************************************/
	virtual void setMediaUrl(const QString &url) noexcept = 0;
};

#endif // !_I_MC_MEDIA_DECODER_H_

