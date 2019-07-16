#pragma once
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
#include <Qstring>
#include <QJsonObject>
#include <QVector3D>
#include <QJsonArray>
#include "structs.h"
#include <qobject.h>

class Light : public QObject
{
public:
	Light() {};
	virtual ~Light() {};

	virtual void readJSON(const QJsonObject &json) = 0;
	virtual void writeJSON(QJsonObject &json) const = 0;
	uint getType() { return type; };
	//QJsonObject getParameters() { return parameters; };

protected:
	uint type;
	//QJsonObject parameters;
	optix::Context context;
};

class PointLight : public Light
{
	Q_OBJECT
public:
	explicit PointLight(optix::Context c);
	~PointLight();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	QVector3D getRadiance() { return radiance; };
	QVector3D getPosition() { return position; };

protected:
	QVector3D position;
	QVector3D radiance;

	public slots:
	void setRadiance(QVector3D r);
	void setPosition(QVector3D p);
};


class DirectionalLight : public Light
{
	Q_OBJECT
public:
	explicit DirectionalLight(optix::Context c);
	~DirectionalLight();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	QVector3D getRadiance() { return radiance; };
	QVector3D getDirection() { return direction; };

protected:
	QVector3D direction;
	QVector3D radiance;

public slots:
	void setRadiance(QVector3D r);
	void setDirection(QVector3D d);
};


class TriangleAreaLight : public Light
{
	Q_OBJECT
public:
	explicit TriangleAreaLight(optix::Context c);
	~TriangleAreaLight();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	QVector3D getRadiance() { return QVector3D(radiance.x,radiance.y,radiance.z); };
	optix::GeometryGroup& getGeometryGroup() { return geometry_group; };
	optix::Buffer& getLightBuffer() { return transformed_light_buffer; };
	optix::Aabb getBoundingBox() { return bounding_box; };
	optix::Transform& getTransform() { return transform; };
	optix::Matrix4x4 getTransformationMatrix() { return transformationMatrix; };
	optix::float3 getTranslation() { return translation; };
	optix::float3 getScale() { return scale; };
	optix::float3 getRotationAxis() { return rotation_axis; };
	float getRotationAngle() { return angle_deg; };
	float getXRotation() { return x_rot; };
	float getYRotation() { return y_rot; };
	float getZRotation() { return z_rot; };
	RTsize getSize() { return size; };

	void loadLightGeometry();
	
protected:
	void setLightMaterial(optix::Context context);
	void applyTransformationMatrix();
	void computeTransformationMatrix();

	QString path;
	optix::float3 radiance;
	optix::float3 translation;
	optix::float3 scale;
	optix::Matrix4x4 rotation;
	optix::Matrix4x4 x_rotation;
	optix::Matrix4x4 y_rotation;
	optix::Matrix4x4 z_rotation;
	optix::Matrix4x4 transformationMatrix;
	optix::Buffer triangle_light_buffer;
	optix::Buffer transformed_light_buffer;
	optix::GeometryGroup geometry_group;
	optix::Transform transform;
	optix::Aabb bounding_box;
	float angle_deg;
	optix::float3 rotation_axis;
	float x_rot;
	float y_rot;
	float z_rot;
	RTsize size;

public slots:
	void setRadiance(QVector3D r);
	void setPath(QString p);
	void setTranslation(QVector3D t);
	void setScale(QVector3D s);
	void setRotation(float angle_degree, QVector3D a);
	void setRotationX(float angle_degree);
	void setRotationY(float angle_degree);
	void setRotationZ(float angle_degree);
};


class DiskAreaLight : public Light
{
	Q_OBJECT
public:
	explicit DiskAreaLight(optix::Context c);
	~DiskAreaLight();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	QVector3D getRadiance() { return QVector3D(radiance.x, radiance.y, radiance.z); };
	QVector3D getPosition() { return QVector3D(position.x, position.y, position.z); };
	float getRadius() { return radius; };
	float getPhi() { return phi; };
	float getTheta() { return theta; };
	QVector3D getNormal() { return QVector3D(normal.x, normal.y, normal.z); };
	optix::GeometryGroup& getGeometryGroup() { return geometry_group; };
	
protected:
	void initGeometry();
	void computeNormal();
	optix::float3 radiance;
	optix::float3 position;
	float radius;
	float theta, phi;
	optix::float3 normal;
	optix::GeometryGroup geometry_group;
	optix::GeometryInstance gi;
	optix::Geometry disk;
public slots:
	void setRadiance(QVector3D r);
	void setPosition(QVector3D p);
	void setRadius(float r);
	void setNormal(float t, float p);
};

class SphericalLight : public Light
{
	Q_OBJECT
public:
	explicit SphericalLight(optix::Context c);
	~SphericalLight();
	void readJSON(const QJsonObject &json);
	void writeJSON(QJsonObject &json) const;
	QVector3D getRadiance() { return QVector3D(radiance.x, radiance.y, radiance.z); };
	QVector3D getPosition() { return QVector3D(position.x, position.y, position.z); };
	float getRadius() { return radius; };
	optix::GeometryGroup& getGeometryGroup() { return geometry_group; };

protected:
	void initGeometry();
	optix::float3 radiance;
	optix::float3 position;
	float radius;
	optix::GeometryGroup geometry_group;
	optix::GeometryInstance gi;
	optix::Geometry sphere;
	public slots:
	void setRadiance(QVector3D r);
	void setPosition(QVector3D p);
	void setRadius(float r);
};