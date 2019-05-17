#include "renderthread.h"
#include <QOpenGLFramebufferObject>
#include <QGuiApplication>
#include "nv12render.h"
#include <QDebug>

RenderThread::RenderThread(QSize &size, QObject *parent) :
	QThread(parent),
	m_size(size)
{
	//m_decoder = new DecodeThread; //ʵ���������߳�
}

RenderThread::~RenderThread()
{
}

void RenderThread::renderNext()
{
	context->makeCurrent(surface); //ʹ�����Ļص�������surface

	if (!m_renderFbo) {
		QOpenGLFramebufferObjectFormat format;
		format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
		m_renderFbo = new QOpenGLFramebufferObject(m_size, format); //������windown(��ʾ��)��ʽһ����FBO
		m_displayFbo = new QOpenGLFramebufferObject(m_size, format);
		m_videoRender = new QmlOpenGLRenderer(); //ʵ����������
		//m_videoRender->initialize();   //��ʼ��
		//m_decoder->setUrl(videoSource); //������Ƶ��ַ
		//m_decoder->start(); //��ʼ����
	}

	m_renderFbo->bind(); //����Ⱦ��FBO����ǰ������
	context->functions()->glViewport(0, 0, m_size.width(), m_size.height()); //�ص��ӿ�

//    m_videoRender->render();
	m_videoRender->rendering(m_frame->getData(), m_frame->getWidth(), m_frame->getHeight()); //��Ⱦһ֡��Ƶ
	context->functions()->glFlush(); //ˢ��һ��

	m_renderFbo->bindDefault();  //��Ⱦ��FBO�󶨵�Ĭ�ϵ�������(Ҳ������������surface��Ӧ���Ǹ�������),�����ˣ�����Ҳ����
	qSwap(m_renderFbo, m_displayFbo); //��������FBO������

	emit textureReady(m_displayFbo->texture(), m_size);
}

void RenderThread::shutDown()
{
	context->makeCurrent(surface);
	delete m_renderFbo;
	delete m_displayFbo;
	delete m_videoRender;
	context->doneCurrent();
	delete context;

	surface->deleteLater();

	exit();
	moveToThread(QGuiApplication::instance()->thread());
}