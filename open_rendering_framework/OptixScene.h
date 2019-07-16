#pragma once
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <QOpenGLWidget>
#include <QtGui/QOpenGLFunctions_4_5_Core>
#include "OptixSceneLoader.h"


class OptixScene
{

public:
	explicit OptixScene(GLuint w, GLuint h);
	~OptixScene();
	void resizeScene(GLuint w, GLuint h);
	void initContext(GLuint buffer_id, uint& width, uint& height);
	void initContext(GLuint buffer_id, QString scene_path, uint& width, uint& height);
	void unregisterOutputBuffer();
	void registerOutputBuffer();
	void renderScene();
	void drawOutputBuffer();
	static std::string ptxPath(const std::string& target, const std::string& base);
	GLuint getFrameCount() { return frame; };
	//Camera* getCamera() { return sceneLoader->getCamera(); };
	//Background* getBackground() { return sceneLoader->getBackground(); };
	//QVector<Light*> getLights() { return sceneLoader->getLights(); };
	//LightStruct* getLightStructs() { return sceneLoader->getLightStructs(); };
	//QVector<Geometry*> getGeometries() { return sceneLoader->getGeometries(); };
	//QVector<Integrator*> getIntegrators() { return sceneLoader->getIntegrators(); };
	void restartFrame() { frame = 0; };
	void setMaxFrame(GLuint m) { max_frame = m; if (frame > max_frame && max_frame > 0) restartFrame(); };
	GLuint getMaxFrame() { return max_frame; };
	OptixSceneLoader* getScene() { return sceneLoader; };
	optix::Context getContext() { return optix_context; };
	void saveScene(QString filename);
	void saveScreenshot(QString filename, bool add_frames);
	void loadScene(uint& width, uint& height);
	void loadBuffer();
protected:
	
	void destroyContext();
	optix::Buffer getOutputBuffer();
	optix::Buffer getPositionBuffer();
	optix::Buffer getNormalBuffer();

private:
	optix::Context optix_context;
	GLuint WIDTH;
	GLuint HEIGHT;
	GLuint frame;
	GLuint max_frame;
	OptixSceneLoader* sceneLoader;
	GLuint buffer_id;
	QString buffer_path;
	bool quit_and_save;
	QString scene_path;
	//optix::Buffer ss_samples;
	//GLuint SAMPLES_FRAME;
};