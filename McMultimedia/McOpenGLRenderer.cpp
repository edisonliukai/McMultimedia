#include "McOpenGLRenderer.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "McVideoFrame.h"
#include "McGlobal.h"

#define VERTEXIN 0
#define TEXTUREIN 1

struct McOpenGLRendererData {
	//shader����
	QOpenGLShaderProgram *program;
	QOpenGLBuffer vbo;
	GLuint textureUniformY, textureUniformU, textureUniformV; //opengl��y��u��v����λ��
	QOpenGLTexture *textureY = nullptr, *textureU = nullptr, *textureV = nullptr;
	GLuint idY, idU, idV; //�Լ��������������ID���������󷵻�0

	QSharedPointer<McVideoFrame> frame;		// ��Ƶ֡�����ⲿ����
};

McOpenGLRenderer::McOpenGLRenderer(QWidget *parent)
	: QOpenGLWidget(parent)
	, d(new McOpenGLRendererData())
{
}

McOpenGLRenderer::~McOpenGLRenderer(){
}

McVideoFormat::PixelFormat McOpenGLRenderer::getVideoFormat() noexcept {
	return McVideoFormat::PixelFormat::Format_YUV420P;
}

void McOpenGLRenderer::setVideoFrame(const QSharedPointer<McVideoFrame> &frame) noexcept {
	d->frame = frame;
}

void McOpenGLRenderer::rendering() noexcept {
	update();
}

//��ʼ��gl
void McOpenGLRenderer::initializeGL() {
	initializeOpenGLFunctions();
	//glEnable(GL_DEPTH_TEST);

	static const GLfloat vertices[]{
		//��������
		-1.0f,-1.0f,
		-1.0f,+1.0f,
		+1.0f,+1.0f,
		+1.0f,-1.0f,
		//��������
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,
		1.0f,1.0f,
	};

	d->vbo.create();
	d->vbo.bind();
	d->vbo.allocate(vertices, sizeof(vertices));

	QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	const char *vsrc =
		"attribute vec4 vertexIn; \
    attribute vec2 textureIn; \
    varying vec2 textureOut;  \
    void main(void)           \
    {                         \
        gl_Position = vertexIn; \
        textureOut = textureIn; \
    }";
	vshader->compileSourceCode(vsrc);

	QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
	const char *fsrc = "varying vec2 textureOut; \
    uniform sampler2D tex_y; \
    uniform sampler2D tex_u; \
    uniform sampler2D tex_v; \
    void main(void) \
    { \
        vec3 yuv; \
        vec3 rgb; \
        yuv.x = texture2D(tex_y, textureOut).r; \
        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
        rgb = mat3( 1,       1,         1, \
                    0,       -0.39465,  2.03211, \
                    1.13983, -0.58060,  0) * yuv; \
        gl_FragColor = vec4(rgb, 1); \
    }";
	fshader->compileSourceCode(fsrc);

	d->program = new QOpenGLShaderProgram(this);
	d->program->addShader(vshader);
	d->program->addShader(fshader);
	d->program->bindAttributeLocation("vertexIn", VERTEXIN);
	d->program->bindAttributeLocation("textureIn", TEXTUREIN);
	d->program->link();
	d->program->bind();
	d->program->enableAttributeArray(VERTEXIN);
	d->program->enableAttributeArray(TEXTUREIN);
	d->program->setAttributeBuffer(VERTEXIN, GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
	d->program->setAttributeBuffer(TEXTUREIN, GL_FLOAT, 8 * sizeof(GLfloat), 2, 2 * sizeof(GLfloat));

	d->textureUniformY = d->program->uniformLocation("tex_y");
	d->textureUniformU = d->program->uniformLocation("tex_u");
	d->textureUniformV = d->program->uniformLocation("tex_v");
	d->textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
	d->textureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
	d->textureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
	d->textureY->create();
	d->textureU->create();
	d->textureV->create();
	d->idY = d->textureY->textureId();
	d->idU = d->textureU->textureId();
	d->idV = d->textureV->textureId();
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

//ˢ����ʾ
void McOpenGLRenderer::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!d->frame || !d->frame->getData()) {
		return;
	}
	uchar *yuv = d->frame->getData();
	uint width = d->frame->getWidth();
	uint height = d->frame->getHeight();

	//    QMatrix4x4 m;
	//    m.perspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f );//͸�Ӿ��������ı仯��ͼ�θ��ű仯����Ļƽ�����ľ����ӵ㣨����ͷ��,��Ҫ��ͼ��������Ļ����һ�����롣
	//    m.ortho(-2,+2,-2,+2,-10,10);//���ü�ƽ����һ������,�������½ǵ���ά�ռ������ǣ�left,bottom,-near��,���Ͻǵ��ǣ�right,top,-near�����Դ˴�Ϊ������ʾz�����Ϊ10��
									//Զ�ü�ƽ��Ҳ��һ������,���½ǵ�ռ������ǣ�left,bottom,-far��,���Ͻǵ��ǣ�right,top,-far�����Դ˴�Ϊ������ʾz����СΪ-10��
									//��ʱ�������Ļ�������Ļˮƽ���м䣬ֻ��ǰ�����ҵľ��������ơ�
	glActiveTexture(GL_TEXTURE0);  //��������ԪGL_TEXTURE0,ϵͳ�����
	glBindTexture(GL_TEXTURE_2D, d->idY); //��y�����������id�����������Ԫ
	//ʹ���ڴ��е����ݴ���������y������������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, yuv);
	//https://blog.csdn.net/xipiaoyouzi/article/details/53584798 �����������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE1); //��������ԪGL_TEXTURE1
	glBindTexture(GL_TEXTURE1, d->idU);
	//ʹ���ڴ��е����ݴ���������u������������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width >> 1, height >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, yuv + width * height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE2); //��������ԪGL_TEXTURE2
	glBindTexture(GL_TEXTURE_2D, d->idV);
	//ʹ���ڴ��е����ݴ���������v������������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width >> 1, height >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, yuv + width * height * 5 / 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//ָ��y����Ҫʹ����ֵ
	glUniform1i(d->textureUniformY, 0);
	//ָ��u����Ҫʹ����ֵ
	glUniform1i(d->textureUniformU, 1);
	//ָ��v����Ҫʹ����ֵ
	glUniform1i(d->textureUniformV, 2);
	//ʹ�ö������鷽ʽ����ͼ��
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glFlush();
}

void McOpenGLRenderer::resizeGL(int width, int height) {
	glViewport(0, 0, width, height);
}
