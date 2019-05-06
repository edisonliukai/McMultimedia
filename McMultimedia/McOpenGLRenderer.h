#ifndef _MC_OPENGL_RENDERER_H_
#define _MC_OPENGL_RENDERER_H_

#include <QOpenGLWidget>
#include <qopenglfunctions.h>
#include "IMcVideoRenderer.h"

#include "mcmultimedia_global.h"

struct McOpenGLRendererData;

class MCMULTIMEDIA_EXPORT McOpenGLRenderer : public QOpenGLWidget, public IMcVideoRenderer, protected QOpenGLFunctions {
	Q_OBJECT

public:
	explicit McOpenGLRenderer(QWidget *parent = 0);
	virtual ~McOpenGLRenderer();

	// ָ����Ҫ����Ƶ��ʽ
	McVideoFormat::PixelFormat getVideoFormat() noexcept override;
	// ������Ƶ֡����Ҫ��Ⱦ����Ƶ����ͨ������Ƶ֡����
	void setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept override;
	// ��Ⱦ
	void rendering() noexcept override;

protected:
	//��ʼ��gl
	void initializeGL() override;
	//ˢ����ʾ
	void paintGL() override;
	void resizeGL(int width, int height) override;

private:
	QScopedPointer<McOpenGLRendererData> d;
};

#endif // !_MC_OPENGL_RENDERER_H_