#include "texturenode.h"
#include <QQuickWindow>

TextureNode::TextureNode(QQuickWindow *window, QObject *parent) :
	QObject(parent),
	m_window(window),
	m_id(0),
	m_texture(0),
	m_size(0, 0)
{
	m_texture = m_window->createTextureFromId(m_id, QSize(1, 1)); //��ʼ��ʹ��0������
	setTexture(m_texture);
	setFiltering(QSGTexture::Linear);
}

TextureNode::~TextureNode()
{
	delete m_texture;
}

void TextureNode::newTexture(int id, const QSize &size)
{
	m_id = id;  //ԭ�����ﻹ���������ڷ��źź�ִ������˳��ģ��ҾͰ���ȡ����
	m_size = size;

	emit pendingNewTexture();
}

void TextureNode::prepareNode()
{
	int newId = m_id;
	QSize size = m_size;
	m_id = 0;
	if (newId) {
		delete m_texture;
		m_texture = m_window->createTextureFromId(newId, size);
		setTexture(m_texture);
		markDirty(DirtyMaterial); //֪ͨ������������ؼ��������ʷ����˸ı�
		emit textureInUse();
	}
}