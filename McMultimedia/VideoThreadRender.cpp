#include "VideoThreadRender.h"
#include "renderthread.h"
#include "texturenode.h"
#include <QQuickWindow>
#include <qsize.h>
#include "McVideoFrame.h"

//QList<QThread*> VideoThreadRender::threads;
VideoThreadRender::VideoThreadRender(QQuickItem *parent)
	: m_renderThread(0),
	QQuickItem(parent)
{
	connect(this, &QQuickItem::windowChanged, [this](QQuickWindow* window) {
		connect(window, &QQuickWindow::widthChanged, [this, window]() {
			m_renderThread->setSize(window->size());
		});
	});
	setFlag(ItemHasContents, true); //��ͼ����ʾ������Ϊtrue
	m_renderThread = new RenderThread(QSize(512, 512), nullptr); //ʵ�����̣߳����������Ⱦ�̲߳���ͬһ������Ⱦ�߳���˵��Scene Graphics�������Ⱦ�߳�
}

McVideoFormat::PixelFormat VideoThreadRender::getVideoFormat() noexcept {
	return McVideoFormat::PixelFormat::Format_YUV420P;
}

void VideoThreadRender::setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept {
	this->frame = frame;
}

void VideoThreadRender::rendering() noexcept {
	
}

void VideoThreadRender::ready()
{
	m_renderThread->surface = new QOffscreenSurface(); //ʵ��һ��������Surface���е�����ʾ��Window��ʹ��opengl��contex�ܹ��󶨵�������
	m_renderThread->surface->setFormat(m_renderThread->context->format());
	m_renderThread->surface->create(); //�����ĵ�QOffscreenSurface��createֻ����GUI�̵߳��ã���������������ʵ���ͳ�ʼ����

	m_renderThread->videoSource = m_videoSource;

	m_renderThread->moveToThread(m_renderThread); //�ƶ������߳�ѭ��
	//������ʧЧ�󣬹ر����̵߳���Դ
	connect(window(), &QQuickWindow::sceneGraphInvalidated, m_renderThread, &RenderThread::shutDown, Qt::QueuedConnection);
	//�������߳�
	m_renderThread->start();
	update(); //��updateһ������ʵ��TextureNode����Ϊ����ճ�ʼ��ʱ�����һ�Σ����ڳ�ʼ�����̺߳󣬷����ˣ�����Ҫ����һ��ʵ��TextureNode��
}
//�˺���������Ⱦ�̵߳��õģ�������GUI�߳�
QSGNode *VideoThreadRender::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
	TextureNode *node = static_cast<TextureNode *>(oldNode);

	if (!m_renderThread->context) {
		QOpenGLContext *current = window()->openglContext();
		current->doneCurrent(); //ȡ��opengl�ڵ�ǰ�������еİ󶨣���Ϊ����Ҫ����shareContext������sharedContext�ƶ������߳�

		m_renderThread->context = new QOpenGLContext();
		m_renderThread->context->setFormat(current->format());
		m_renderThread->context->setShareContext(current);
		m_renderThread->context->create();
		m_renderThread->context->moveToThread(m_renderThread); //context���̹߳����ԣ�һ��contextֻ�ܱ����������̵߳���makeCurrent�����ܱ������̵߳���;Ҳֻ����һ����Ӧ��surface
															   //һ���߳���ͬһʱ��Ҳֻ����һ��context
		current->makeCurrent(window()); //�ָ���

		m_renderThread->setFrame(frame.data());
		QMetaObject::invokeMethod(this, "ready"); //���̵߳���
		return 0;
	}

	if (!node) {
		node = new TextureNode(window()); //ʵ�����Զ����������

		//�����������߳���Ⱦ�ú󣬽�����id����С���õ��Զ����QSimpleTextureNode�ṹ��
		connect(m_renderThread, &RenderThread::textureReady, node, &TextureNode::newTexture, Qt::DirectConnection);
		//update�������ú���Ⱦ�̻߳����ʵ���ʱ�򷢳�beforRendering�ź�
		connect(node, &TextureNode::pendingNewTexture, window(), &QQuickWindow::update, Qt::QueuedConnection);
		//�ڿ�ʼ��Ⱦ֮ǰ�������߳���Ⱦ�õ��������õ�QSimpletTextureNode�У��Ա���Ⱦ�߳�ʹ��
		connect(window(), &QQuickWindow::beforeRendering, node, &TextureNode::prepareNode, Qt::DirectConnection);
		//��Ⱦ�õ�����ʹ�ú�֪ͨ���߳���Ⱦ��һ֡
		connect(node, &TextureNode::textureInUse, m_renderThread, &RenderThread::renderNext, Qt::QueuedConnection);

		// Get the production of FBO textures started..
		QMetaObject::invokeMethod(m_renderThread, "renderNext", Qt::QueuedConnection);
	}

	node->setRect(boundingRect()); //������ʾ����Ϊqml�������������
		return node;
}

void VideoThreadRender::setVideoSource(QString s)
{
	if (m_videoSource != s) {
		emit videoSourceChanged();
	}
	m_videoSource = s;
}

QString VideoThreadRender::videoSource()
{
	return m_videoSource;
}