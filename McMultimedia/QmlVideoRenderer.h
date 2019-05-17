#pragma once

#include <QQuickItem>
#include "IMcVideoRenderer.h"

#include "mcmultimedia_global.h"

struct QmlVideoRendererData;

class MCMULTIMEDIA_EXPORT QmlVideoRenderer : public QQuickItem, public IMcVideoRenderer
{
	Q_OBJECT

public:
	QmlVideoRenderer(QQuickItem *parent = 0);
	~QmlVideoRenderer();

	// ָ����Ҫ����Ƶ��ʽ
	McVideoFormat::PixelFormat getVideoFormat() noexcept override;
	// ������Ƶ֡����Ҫ��Ⱦ����Ƶ����ͨ������Ƶ֡����
	void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept override;
	// ��Ⱦ
	void rendering() noexcept override;

protected:
	QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
	bool event(QEvent *e) override;

private:
	QScopedPointer<QmlVideoRendererData> d;
};
