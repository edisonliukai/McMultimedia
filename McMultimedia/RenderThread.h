#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H
#include <QThread>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include "McVideoFrame.h"
#include "QmlOpenGLRenderer.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLFramebufferObject)
QT_FORWARD_DECLARE_CLASS(Nv12Render)
QT_FORWARD_DECLARE_CLASS(DecodeThread)
class RenderThread : public QThread
{
	Q_OBJECT
public:
	RenderThread(QSize &size, QObject *parent = nullptr);
	~RenderThread();
	QOpenGLContext *context{ nullptr };
	QOffscreenSurface *surface{ nullptr }; //��create����ֻ����GUI�߳�(���߳�)���ã�����Ū����ȫ�ֱ������������
	QString videoSource;

	void setSize(QSize size) { m_size = size; }
	void setFrame(McVideoFrame *frame) { m_frame = frame; }

signals:
	void textureReady(int, const QSize); //��Ƶ֡��Ⱦ����

public slots:
	void renderNext(); //��Ⱦ��һ֡

	void shutDown(); //�ر��˳�

private:
	QOpenGLFramebufferObject *m_renderFbo{ 0 }; //������Ⱦ
	QOpenGLFramebufferObject *m_displayFbo{ 0 }; //������ʾ
	QSize m_size;
	//Nv12Render *m_videoRender{ nullptr }; //��Ⱦ��
	QmlOpenGLRenderer *m_videoRender{ nullptr }; //��Ⱦ��
	DecodeThread *m_decoder{ nullptr }; //����
	McVideoFrame *m_frame;
};

#endif // RENDERTHREAD_H