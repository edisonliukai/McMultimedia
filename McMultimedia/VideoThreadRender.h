#ifndef VIDEOTHREADRENDER_H
#define VIDEOTHREADRENDER_H
#include <QQuickItem>
#include "IMcVideoRenderer.h"

#include "mcmultimedia_global.h"

struct VideoThreadRenderData;

QT_FORWARD_DECLARE_CLASS(RenderThread)
class MCMULTIMEDIA_EXPORT VideoThreadRender : public QQuickItem, public IMcVideoRenderer
{
	Q_OBJECT
		Q_PROPERTY(QString videoSource READ videoSource WRITE setVideoSource NOTIFY videoSourceChanged)
public:
	VideoThreadRender(QQuickItem *parent = nullptr);
	static QList<QThread*> threads;

	// ָ����Ҫ����Ƶ��ʽ
	McVideoFormat::PixelFormat getVideoFormat() noexcept override;
	// ������Ƶ֡����Ҫ��Ⱦ����Ƶ����ͨ������Ƶ֡����
	void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept override;
	// ��Ⱦ
	void rendering() noexcept override;

public Q_SLOTS:
	void ready();

signals:
	void videoSourceChanged();

protected:
	QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

private:
	RenderThread *m_renderThread{ nullptr };
	QString m_videoSource;

	void setVideoSource(QString);
	QString videoSource();

	QSharedPointer<McVideoFrame> frame;		// ��Ƶ֡�����ⲿ����
};

#endif // VIDEOTHREADRENDER_H