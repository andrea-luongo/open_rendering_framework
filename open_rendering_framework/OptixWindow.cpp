#include "OptixWindow.h"
#include <QtWidgets>
#include <QtGui/QOpenGLContext>
//#include <QtGui/QOpenGLFunctions_4_5_Core>




OptixWindow::OptixWindow(GLuint width, GLuint height, QWidget *parent)
	: QOpenGLWidget(parent)
{
	WIDTH = width;
	HEIGHT = height;
	is_paused = false;
	quit_and_save = false;
	elapsed = 0;
	for (int idx = 0; idx < QCoreApplication::arguments().count(); idx++)
	{
		if (QCoreApplication::arguments().at(idx).compare(QString("-q")) == 0 || QCoreApplication::arguments().at(idx).compare(QString("--quit")) == 0)
		{
			quit_and_save = true;
		}
	}

	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &OptixWindow::animate);
	timer->start(0);
}

OptixWindow::~OptixWindow()
{
	if (optix_scene != NULL)
	{
		delete optix_scene;
		optix_scene == NULL;
	}
}


QSize OptixWindow::minimumSizeHint() const
{
	return QSize(50, 50);
}

QSize OptixWindow::sizeHint() const
{
	return QSize(WIDTH, HEIGHT);
}


void OptixWindow::initializeGL()
{
	initializeOpenGLFunctions();
	glClearColor(0.7, 0.5, 0.3, 1.0);

	buffer_ID = 0;
	glGenBuffers(1, &buffer_ID);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_ID);
	glBufferData(GL_ARRAY_BUFFER, WIDTH*HEIGHT * sizeof(QVector4D), 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	optix_scene = new OptixScene(WIDTH, HEIGHT);
	GLuint w, h;
	optix_scene->initContext(buffer_ID, w, h);
	resizeOnlyGL(w, h);
	optix_scene->loadBuffer();
	resize(WIDTH, HEIGHT);

	previousTime = 0;
	previousFrameCount = 0;
	fps = 0.0;
	frameTimer =  QTime();
	frameTimer.start();
}


void OptixWindow::resizeGL(int width, int height)
{
	WIDTH = width;
	HEIGHT = height;
	optix_scene->resizeScene(WIDTH, HEIGHT);
	if (buffer_ID) {
		optix_scene->unregisterOutputBuffer();
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer_ID);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, WIDTH*HEIGHT * sizeof(QVector4D), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		optix_scene->registerOutputBuffer();
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glViewport(0, 0, WIDTH, HEIGHT);
	resize(WIDTH, HEIGHT);
	emit resized_window(WIDTH, HEIGHT);
}

void OptixWindow::resizeOnlyGL(int width, int height)
{
	WIDTH = width;
	HEIGHT = height;
	if (buffer_ID) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer_ID);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, WIDTH*HEIGHT * sizeof(QVector4D), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glViewport(0, 0, WIDTH, HEIGHT);
	emit resized_window(WIDTH, HEIGHT);
}




void OptixWindow::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!is_paused)
	{
		optix_scene->renderScene();
	}
	if (buffer_ID) {
		paintBuffer();
	}
	else{
		optix_scene->drawOutputBuffer();
	}

	int currentTime = frameTimer.elapsed();
	int currentFrame = optix_scene->getFrameCount();
	double dt = currentTime - previousTime;
	if (dt > fps_update_ms) {

		fps = 1000.0f*(currentFrame - previousFrameCount) / dt;
		previousTime = currentTime;
		previousFrameCount = currentFrame;
		emit updated_fps(fmaxf(fps,0.0f));
	}
	emit updated_frame_count(currentFrame);
	if (currentFrame == optix_scene->getMaxFrame() && quit_and_save)
	{
		emit terminate_application();
	}
}

void OptixWindow::paintBuffer()
{
	static unsigned int gl_tex_id = 0;
	if (!gl_tex_id)
	{
		glGenTextures(1, &gl_tex_id);
		glBindTexture(GL_TEXTURE_2D, gl_tex_id);

		// Change these to GL_LINEAR for super- or sub-sampling
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glBindTexture(GL_TEXTURE_2D, gl_tex_id);

	// send PBO to texture
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer_ID);

	RTsize elmt_size = sizeof(QVector4D);
	if (elmt_size % 8 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
	else if (elmt_size % 4 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	else if (elmt_size % 2 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	else                          glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);
	

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, 1.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}


void OptixWindow::animate()
{
	elapsed = (elapsed + qobject_cast<QTimer*>(sender())->interval()) % 1000;
	update();
}

void OptixWindow::externalResize(int width, int height)
{
	resizeGL(width, height);
}

void OptixWindow::loadScene(QString filename) 
{ 
	//GLuint w, h;
	//optix_scene->loadScene(filename, w, h);
	//resizeGL(w, h);
	delete optix_scene;
	optix_scene = new OptixScene(WIDTH, HEIGHT);
	GLuint w, h;
	optix_scene->initContext(buffer_ID,filename, w, h);
	resizeOnlyGL(w, h);
	optix_scene->loadBuffer();
	resize(WIDTH, HEIGHT);
	previousTime = 0;
	previousFrameCount = 0;
	fps = 0.0;
	frameTimer = QTime();
	frameTimer.start();
}

void OptixWindow::pause(bool status)
{

	is_paused = status;
}