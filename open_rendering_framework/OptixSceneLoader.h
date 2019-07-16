#pragma once
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <Qstring>
#include <QJsonObject>
#include "Integrator.h"
#include "Background.h"
#include "Camera.h"
#include "Geometry.h"
#include "Light.h"
#include <QVector>

class OptixSceneLoader 
{
	
public:
	explicit OptixSceneLoader(optix::Context c);
	~OptixSceneLoader();

	bool loadJSONScene(const QString scene_path, uint& width, uint& height, QString& buffer_path, unsigned int& frame_count);
	bool saveJSONScene(QString scene_path, uint frame_count);
	void setCamera(Camera* c) { camera = c; };
	void setBackground(Background* b) { background = b; };
	void setIntegrator(Integrator* i) { integrator = i; };
	Camera* getCamera() { return camera; };
	Background* getBackground() { return background; };
	QVector<Light*>* getLights() { return &lights; };
	QVector<LightStruct>* getLightStructs() { return &lightStructData; };
	QVector<Geometry*>* getGeometries() { return &geometries; };
	//QVector<Integrator*>* getIntegrators() { return &integrators; };
	Integrator* getIntegrator() { return integrator; };
	QVector<optix::GeometryInstance> getTranslucentObjects() { return translucentObjects; };
	void resizeCamera(uint width, uint height);
	void updateLights(bool triangleAreaLightChanged);
	void addLight(Light* light);
	void removeLight(unsigned int lightIdx);
	void addGeometry(Geometry* geometry);
	void removeGeometry(unsigned int geometryIdx);
	void updateAcceleration();
	GLuint getSamplesFrame() { return SAMPLES_FRAME; };
	void computeTranslucentGeometries();
	void loadTranslucentGeometry(uint idx);
	
protected:
	void readJSON(const QJsonObject &json, uint& width, uint& height, QString& buffer_path, unsigned int& frame_count);
	void writeJSON(QJsonObject &json);
	Integrator* readIntegrator(const QJsonObject &integratorObject);
	Background* readBackground(const QJsonObject &backgroundObject);
	Camera* readCamera(const QJsonObject &cameraObject, uint& width, uint& height);
	Geometry* readGeometry(const QJsonObject &geometryObject);
	void readLight(const QJsonObject &lightObject, LightStruct* light_data);
	void initLightBuffers();
	void loadTriangleLightBuffer();
	optix::float3 qVector3DtoFloat3(QVector3D vec);
	void initTranslucentContext();

private:
	//QVector<Integrator*> integrators;
	Integrator* integrator;
	Background* background;
	Camera* camera;
	QVector<Geometry*> geometries;
	QVector<Light*> lights;
	QVector<LightStruct> lightStructData;
	QVector<optix::GeometryInstance> translucentObjects;
	optix::Group obj_group;
	optix::Context context;
	optix::Aabb bbox;
	optix::Buffer light_buffer;
	optix::Buffer triangle_light_buffer;
	unsigned int triangle_light_count;
	optix::Buffer ss_samples;
	GLuint SAMPLES_FRAME;

};