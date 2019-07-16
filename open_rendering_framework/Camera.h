#pragma once
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
#include <Qstring>
#include <QJsonObject>
#include <QVector3D>
#include <QJsonArray>
#include <qobject.h>

class Camera : public QObject
{
	
public:
	Camera() {};
	virtual ~Camera() {};

	virtual void readJSON(const QJsonObject &json, uint& width, uint& height) = 0;
	virtual void writeJSON(QJsonObject &json) const = 0;
	uint getType() { return type; };
	//QJsonObject getParameters() { return parameters; };
	uint getWidth() { return WIDTH; };
	uint getHeight() { return HEIGHT; };
	virtual void resizeCamera(uint width, uint height) = 0;

protected:
	void setWidth(uint w) { WIDTH = w; };
	void setHeight(uint h) { HEIGHT = h; };

	uint type;
	uint WIDTH;
	uint HEIGHT;
	optix::Context context;
};


class PinholeCamera : public Camera
{
	Q_OBJECT
public:
	explicit PinholeCamera(optix::Context c);
	~PinholeCamera();
	void readJSON(const QJsonObject &json, uint& width, uint&  height);
	void writeJSON(QJsonObject &json) const;
	void resizeCamera(uint width, uint height);
	QVector3D getEye() { return eye; };
	QVector3D getLookat() { return lookat; };
	QVector3D getUp() { return up; };
	float getFov() { return vfov; };
	optix::uint2 getPatchOrigin() { return patch_origin; };
	optix::uint2 getPatchDims() { return patch_dims; };

protected:
	void calculateCameraVariables();


	float vfov;
	QVector3D eye;
	QVector3D lookat;
	QVector3D up;
	QVector3D U, V, W;
	optix::uint2 patch_origin;
	optix::uint2 patch_dims;
public slots:
	void setEye(QVector3D e);
	void setLookat(QVector3D l);
	void setUp(QVector3D u);
	void setFov(float f);
	void setPatchOrigin(optix::uint2 origin);
	void setPatchDims(optix::uint2 dims);
};