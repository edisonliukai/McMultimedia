#ifndef _QML_MEDIA_PLAYER_H_
#define _QML_MEDIA_PLAYER_H_

#include <QQuickItem>

#include "McMediaPlayer.h"

class QmlMediaPlayer : public QQuickItem {
	Q_OBJECT
	Q_PROPERTY(qreal position READ position NOTIFY signal_positionChanged)
	Q_PROPERTY(qreal duration READ duration NOTIFY signal_durationChanged)
	Q_PROPERTY(QString source MEMBER m_mediaUrl NOTIFY signal_sourceChanged)
	Q_PROPERTY(State state READ state NOTIFY signal_stateChanged)

	Q_ENUMS(State)
public:
	enum State {
		StoppedState = McMediaPlayer::StoppedState,
		PlayingState = McMediaPlayer::PlayingState,
		PausedState = McMediaPlayer::PausedState
	};

	explicit QmlMediaPlayer(QQuickItem *parent = 0);

	qreal position() const noexcept { return m_position; }
	qreal duration() const noexcept { return m_duration; }
	State state() const noexcept { return m_state; }

	Q_INVOKABLE void play() noexcept;
	Q_INVOKABLE void pause() noexcept;
	Q_INVOKABLE void stop() noexcept;
	Q_INVOKABLE void seek(qreal offset) noexcept;

signals:
	void signal_positionChanged();
	void signal_durationChanged();
	void signal_sourceChanged();
	void signal_stateChanged();

private:
	McMediaPlayer m_mediaPlayer;				// ������

	qreal m_position{ 0 };						// ý�嵱ǰ���ŵ�λ�ã���λ��ms
	qreal m_duration{ 0 };						// ý����ʱ������λ ms
	QString m_mediaUrl;							// ��ǰý��

	State m_state{ State::StoppedState };		// ��ǰ״̬
};

#endif // !_QML_MEDIA_PLAYER_H_