#ifndef _MC_MEDIA_PLAYER_H_
#define _MC_MEDIA_PLAYER_H_

#include <QObject>

#include "mcmultimedia_global.h"

class IMcVideoRenderer;

struct McMediaPlayerData;

class MCMULTIMEDIA_EXPORT McMediaPlayer : public QObject {
	Q_OBJECT

public:
	enum State {
		StoppedState,
		PlayingState,
		PausedState
	};

	explicit McMediaPlayer(QObject *parent = 0);
	~McMediaPlayer();

	// �����m3u8��ʽ��Ƶ��һ��Ҫ������׺.m3u8
	void setMediaUrl(const QString &url) noexcept;

	McMediaPlayer::State getState() noexcept;
	qint64 getPosition() noexcept;
	void setPosition(qint64 position) noexcept;

	void setVideoRenderer(IMcVideoRenderer *renderer) noexcept;

signals:
	void signal_durationChanged(qint64 duration);
	void signal_positionChanged(qint64 pos);

public slots:
	void play() noexcept;
	void stop() noexcept;
	void pause() noexcept;
	void resume() noexcept;

private:
	void setState(State state) noexcept;
	void setPos(qint64 pos) noexcept;

private:
	QScopedPointer<McMediaPlayerData> d;
};

#endif // !_MC_MEDIA_PLAYER_H_