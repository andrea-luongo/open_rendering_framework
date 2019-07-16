#pragma once
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
#include <Qstring>
#include <QJsonObject>
#include <QVector3D>
#include <QJsonArray>
#include "ImageLoader.h"

class Background
{
public:
	Background() {};
	virtual ~Background() {};

	virtual void readJSON(const QJsonObject &json) = 0;
	virtual void writeJSON(QJsonObject &json) const = 0;
	uint getType() { return type; };
	//QJsonObject getParameters() { return parameters; };

protected:
	optix::Context context;
	uint type;
	//QJsonObject parameters;

};


class ConstantBackground : public Background
{
public:
	explicit ConstantBackground(optix::Context context);
	~ConstantBackground();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	QVector3D getBackgroundColor() { return background_color; };
	void setBackgroundColor(QVector3D back_color);

protected:

	QVector3D background_color;
};

class EnvMapBackground : public Background
{
public:
	explicit EnvMapBackground(optix::Context context);
	~EnvMapBackground();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	QString getPath() { return path; };
	void changePath(QString new_path);
	/*QVector3D getBackgroundColor() { return background_color; };
	void setBackgroundColor(QVector3D back_color) { background_color = back_color; };*/

protected:
	QString path;
	optix::TextureSampler envmap;
	QVector3D background_color;
};