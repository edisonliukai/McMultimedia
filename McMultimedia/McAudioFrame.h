#ifndef _MC_AUDIO_FRAME_H_
#define _MC_AUDIO_FRAME_H_

#include "qglobal.h"

// POCO�����ڱ�����������Ƶ֡
class McAudioFrame {
public:
	quint8 *getData() { return m_data; };
	void setData(quint8 *data) { m_data = data; };

	int getSize() { return m_size; };
	void setSize(int size) { m_size = size; };

	qint64 getStartClock() { return m_startClock; };
	void setStartClock(qint64 clock) { m_startClock = clock; };

	qint64 getEndClock() { return m_endClock; };
	void setEndClock(qint64 clock) { m_endClock = clock; };

private:
	quint8 *m_data{ nullptr };
	qint64 m_size{ 0 };
	qint64 m_startClock{ 0 };	// ��ʼʱ�䣬��λms
	qint64 m_endClock{ 0 };		// ����ʱ�䣬��λms
};

#endif // !_MC_AUDIO_FRAME_H_

