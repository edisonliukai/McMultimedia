#ifndef TEXTURENODE_H
#define TEXTURENODE_H
#include <QSGSimpleTextureNode>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGTexture)
class TextureNode : public QObject, public QSGSimpleTextureNode
{
	Q_OBJECT
public:
	explicit TextureNode(QQuickWindow *window, QObject *parent = nullptr);
	~TextureNode();

signals:
	void pendingNewTexture(); //��һ֡����Ƶ����
	void textureInUse(); //��������ʹ�á�������Ⱦ��һ֡��

public slots:
	void newTexture(int id, const QSize &size); //�����߳���Ⱦ��һ֡ʱ����
	void prepareNode();  //����Ⱦ�߳�׼����ʼ��Ⱦʱ���ᷢ��beforRendering�źţ���ʱ����øú���

private:
	int m_id;
	QSize m_size;
	QSGTexture *m_texture{ nullptr };
	QQuickWindow *m_window{ nullptr };
};

#endif // TEXTURENODE_H