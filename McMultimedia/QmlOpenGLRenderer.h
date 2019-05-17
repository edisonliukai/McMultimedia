#pragma once

#include <qopenglfunctions.h>

struct QmlOpenGLRendererData;

class QmlOpenGLRenderer : protected QOpenGLFunctions
{
public:
	QmlOpenGLRenderer();
	virtual ~QmlOpenGLRenderer();

	void clearup() noexcept;
	// ��Ⱦ
	void rendering(uchar *yuv, uint width, uint height) noexcept;
	// ������Ⱦ����С
	void resizeRenderer(int width, int height) noexcept;

private:
	QScopedPointer<QmlOpenGLRendererData> d;
};

