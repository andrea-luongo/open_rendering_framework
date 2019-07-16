#pragma once
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
#include <Qstring>
#include <QJsonObject>
#include <QVector3D>
#include <QJsonArray>

class Integrator
{
public:
	Integrator() {};
	virtual ~Integrator() {};

	virtual void readJSON(const QJsonObject &json) = 0;
	virtual void writeJSON(QJsonObject &json) const = 0;
	uint getType() { return type; };
	//QJsonObject getParameters() { return parameters; };

protected:
	uint type;
	//QJsonObject parameters;
	optix::Context context;
};


class PathTracer : public Integrator
{
public:
	explicit PathTracer(optix::Context c);
	~PathTracer();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	uint getMaxDepth() { return max_depth; };
	void setMaxDepth(uint max_d);
	float getSceneEpsilon() { return scene_epsilon; };
	void setSceneEpsilon(float scene_eps);
	QVector3D getExceptionColor() { return exception_color; };
	void setExceptionColor(QVector3D exc_color);

protected:
	uint max_depth;
	float scene_epsilon;
	QVector3D exception_color;
};

class DepthTracer : public Integrator
{
public:
	explicit DepthTracer(optix::Context c);
	~DepthTracer();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	uint getMaxDepth() { return max_depth; };
	void setMaxDepth(uint max_d);
	float getSceneEpsilon() { return scene_epsilon; };
	void setSceneEpsilon(float scene_eps);
	QVector3D getExceptionColor() { return exception_color; };
	void setExceptionColor(QVector3D exc_color);

protected:
	uint max_depth;
	float scene_epsilon;
	QVector3D exception_color;
};