#pragma once
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
#include <Qstring>
#include <QJsonObject>
#include <QVector3D>
#include <QMatrix4x4>
#include "Material.h"

class Geometry 
{
	
public:
	Geometry() {};
	virtual ~Geometry() {};

	virtual void readJSON(const QJsonObject &json) = 0;
	virtual void writeJSON(QJsonObject &json) const = 0;
	virtual QString getName() = 0;
	uint getType() { return type; };
	//QJsonObject getParameters() { return parameters; };
	optix::GeometryGroup& getGeometryGroup() { return geometry_group; };
	optix::Transform& getTransform() { return transform; };
	MyMaterial* getMaterial() { return mtl; };

protected:
	uint type;
	//QJsonObject parameters;
	optix::GeometryGroup geometry_group;
	optix::Transform transform;
	optix::Context context;
	MyMaterial* mtl;

};


class OBJGeometry : public Geometry
{
	
public:
	explicit OBJGeometry(optix::Context context);
	explicit OBJGeometry(optix::Context context, QString filename);
	explicit OBJGeometry(optix::Context context, const QJsonObject &json);
	~OBJGeometry();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	QString getName() { return path; };
	optix::float3 getPosition() { return position; };
	optix::float3 getScale() { return scale; };
	optix::float3 getRotationAxis() { return rotation_axis; };
	float getRotationAngle() { return angle_deg; };
	float getXRotation() { return x_rot; };
	float getYRotation() { return y_rot; };
	float getZRotation() { return z_rot; };
	void loadGeometry();
	void loadMaterial(MyMaterial* mtl);
	void loadMaterial(MaterialType mtl_type);
	void loadMaterialFromJSON(const QJsonObject &json);
	//void loadTexture();
	
protected:
	
	void computeTransformationMatrix();
	QString path;
	optix::float3 position;
	optix::float3 scale;
	optix::Matrix4x4 rotation;
	optix::Matrix4x4 x_rotation;
	optix::Matrix4x4 y_rotation;
	optix::Matrix4x4 z_rotation;
	float x_rot;
	float y_rot;
	float z_rot;
	float angle_deg;
	optix::float3 rotation_axis;
	optix::Matrix4x4 transformationMatrix;
	//uint texture_width;
	//uint texture_height;
	//QJsonObject mtl;

public slots:
	void setPath(QString p);
	void setPosition(QVector3D t);
	void setScale(QVector3D s);
	void setRotation(float angle_degree, QVector3D a);
	void setRotationX(float angle_degree);
	void setRotationY(float angle_degree);
	void setRotationZ(float angle_degree);

signals:
	void updated_translucent();
};