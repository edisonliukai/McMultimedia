#ifndef _I_MC_CONTROL_H_
#define _I_MC_CONTROL_H_

#include <qglobal.h>

class IMcControl {
public:
	virtual ~IMcControl() = default;

	// ��ʼ
	virtual bool start() noexcept = 0;
	// ��ͣ
	virtual void pause() noexcept = 0;
	// �ָ�
	virtual void resume() noexcept = 0;
	// ֹͣ
	virtual void stop() noexcept = 0;
	// �˳����ͷ���Ӧ��Դ
	virtual void quit() noexcept = 0;

	// ��ת
	virtual void seek(qint64 pos) noexcept = 0;
};

#endif // !_I_MC_CONTROL_H_

