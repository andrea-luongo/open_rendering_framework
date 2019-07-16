#pragma once
#include <QOpenGLWidget>
#include "OptixScene.h"
#include <QtGui/QOpenGLFunctions>
#include <QtCore/QTime>

class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;

class OptixWindow : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
		
public:
   explicit OptixWindow(GLuint width, GLuint height, QWidget *parent = 0);
   ~OptixWindow();

   void closeEvent(QCloseEvent *event) { QOpenGLWidget::closeEvent(event); emit terminate_application(); };
   void restartFrame(){optix_scene->restartFrame(); };

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void resizeOnlyGL(int width, int height);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	
private: 
	void paintBuffer();
	OptixScene* optix_scene;
	GLuint WIDTH;
	GLuint HEIGHT;
	GLuint buffer_ID;
	int elapsed;
	QTime frameTimer;
	int previousTime;
	int previousFrameCount;
	float fps;
	const float fps_update_ms = 500;
	bool is_paused;
	bool quit_and_save;
public slots:
	void animate();
	void externalResize(int width, int height);
	OptixSceneLoader* getScene() { return optix_scene->getScene(); };
	optix::Context getContext() { return optix_scene->getContext(); };
	int getMaxFrame() { return optix_scene->getMaxFrame(); };
	void setMaxFrame(int frames) { optix_scene->setMaxFrame(frames); };
	void saveScene(QString filename) { optix_scene->saveScene(filename); };
	void saveScreenshot(QString filename, bool add_frames) { optix_scene->saveScreenshot(filename,add_frames); };
	void loadScene(QString filename);
	void pause(bool status);
signals:
	void terminate_application();
	void resized_window(int width, int height);
	void updated_fps(float f);
	void updated_frame_count(int frameCount);
};