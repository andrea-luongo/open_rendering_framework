#pragma once
#include "Light.h"
#include "OptixScene.h"
#include "sampleConfig.h"
#include "ObjLoader.h"


//PointLight
PointLight::PointLight(optix::Context c)
{
	position = QVector3D(0.0, 0.0, 0.0);
	radiance = QVector3D(1.0, 1.0, 1.0);
	type = POINT_LIGHT;
	context = c;
}

PointLight::~PointLight()
{
	
}

void PointLight::readJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}
	
	if (parameters.contains("position") && parameters["position"].isArray()) {
		QJsonArray tmp = parameters["position"].toArray();
		position = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}


	if (parameters.contains("radiance") && parameters["radiance"].isArray()) {
		QJsonArray tmp = parameters["radiance"].toArray();
		radiance = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

}

void PointLight::writeJSON(QJsonObject &json) const
{
	json["type"] = lightNames[type];
	QJsonObject parameters;
	parameters["position"] = QJsonArray{ position.x(), position.y(), position.z() };
	parameters["radiance"] = QJsonArray{ radiance.x(), radiance.y(), radiance.z() };
	json["parameters"] = parameters;
}

void PointLight::setRadiance(QVector3D r) 
{ 
	radiance = r; 
};

void PointLight::setPosition(QVector3D p) 
{ 
	position = p; 
};

//DirectionalLight
DirectionalLight::DirectionalLight(optix::Context c)
{
	direction = QVector3D(0.0, -1.0, 0.0);
	radiance = QVector3D(1.0, 1.0, 1.0);
	type = DIRECTIONAL_LIGHT;
	context = c;
}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::readJSON(const QJsonObject &json)
{

	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}
	
	if (parameters.contains("direction") && parameters["direction"].isArray()) {
		QJsonArray tmp = parameters["direction"].toArray();
		direction = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	if (parameters.contains("radiance") && parameters["radiance"].isArray()) {
		QJsonArray tmp = parameters["radiance"].toArray();
		radiance = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}


}

void DirectionalLight::writeJSON(QJsonObject &json) const
{
	json["type"] = lightNames[type];
	QJsonObject parameters;
	parameters["direction"] = QJsonArray{ direction.x(), direction.y(), direction.z() };
	parameters["radiance"] = QJsonArray{ radiance.x(), radiance.y(), radiance.z() };
	json["parameters"] = parameters;
}

void DirectionalLight::setRadiance(QVector3D r) 
{ 
	radiance = r; 
};

void DirectionalLight::setDirection(QVector3D d) 
{ 
	direction = d; 
};


//AreaLight
TriangleAreaLight::TriangleAreaLight(optix::Context c)
{
	context = c;
	geometry_group = context->createGeometryGroup();
	transform = context->createTransform();
	transform->setChild(geometry_group);
	radiance = optix::make_float3(1.0f, 1.0f, 1.0f);
	translation = optix::make_float3(0.0f,0.0f, 0.0f);
	scale = optix::make_float3(1.0f, 1.0f, 1.0f);
	angle_deg = 0.0f;
	rotation_axis = optix::make_float3(0.0f, 1.0f, 0.0f);
	x_rotation = optix::Matrix4x4::identity();
	y_rotation = optix::Matrix4x4::identity();
	z_rotation = optix::Matrix4x4::identity();
	x_rot = 0.0f;
	y_rot = 0.0f;
	z_rot = 0.0f;
	rotation = optix::Matrix4x4::identity();
	type = TRIANGLES_AREA_LIGHT;
	path = "../../data/Plane.obj";
	computeTransformationMatrix();

}

TriangleAreaLight::~TriangleAreaLight()
{
	//transform->destroy();
	//geometry_group->destroy();
	/*triangle_light_buffer->destroy();
	transformed_light_buffer->destroy();*/
}

void TriangleAreaLight::readJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}

	if (parameters.contains("path") && parameters["path"].isString()) {
		QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
		QFileInfo fileInfo(dir, parameters["path"].toString());
		path = (fileInfo.absoluteFilePath());
	}
	//else {
	//	path = QString("../../data/closed_bunny_vn.obj");
	//}

	if (parameters.contains("radiance") && parameters["radiance"].isArray()) {
		QJsonArray tmp = parameters["radiance"].toArray();
		radiance = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}


	if (parameters.contains("position") && parameters["position"].isArray()) {
		QJsonArray tmp = parameters["position"].toArray();
		translation = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}



	if (parameters.contains("scale") && parameters["scale"].isArray()) {
		QJsonArray tmp = parameters["scale"].toArray();
		scale = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}


	if (parameters.contains("rotation") && parameters["rotation"].isArray()) {
		QJsonArray tmp = parameters["rotation"].toArray();
		angle_deg = tmp[0].toDouble();
		rotation_axis = optix::normalize(optix::make_float3(tmp[1].toDouble(), tmp[2].toDouble(), tmp[3].toDouble()));
		rotation = optix::Matrix4x4::rotate(angle_deg * M_PIf / 180.0f, rotation_axis);
	}

	if (parameters.contains("x_rotation") && parameters["x_rotation"].isDouble()) {
		x_rot = parameters["x_rotation"].toDouble();
		optix::float3 x_axis = optix::make_float3(1.0f, 0.0f, 0.0f);
		x_rotation = optix::Matrix4x4::rotate(x_rot * M_PIf / 180.0f, x_axis);
	}

	if (parameters.contains("y_rotation") && parameters["y_rotation"].isDouble()) {
		optix::float3 y_axis = optix::make_float3(0.0f, 1.0f, 0.0f);
		y_rot = parameters["y_rotation"].toDouble();
		y_rotation = optix::Matrix4x4::rotate(y_rot * M_PIf / 180.0f, y_axis);
	}

	if (parameters.contains("z_rotation") && parameters["z_rotation"].isDouble()) {
		z_rot = parameters["z_rotation"].toDouble();
		optix::float3 z_axis = optix::make_float3(0.0f, 0.0f, 1.0f);
		z_rotation = optix::Matrix4x4::rotate(z_rot * M_PIf / 180.0f, z_axis);
	}

	computeTransformationMatrix();
}

void TriangleAreaLight::writeJSON(QJsonObject &json) const
{
	
	json["type"] = lightNames[type];
	QJsonObject parameters;
	QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
	parameters["path"] = dir.relativeFilePath(path);
	parameters["radiance"] = QJsonArray{ radiance.x, radiance.y, radiance.z };
	parameters["position"] = QJsonArray{ translation.x, translation.y, translation.z };
	parameters["scale"] = QJsonArray{ scale.x, scale.y, scale.z };
	parameters["x_rotation"] = x_rot;
	parameters["y_rotation"] = y_rot;
	parameters["z_rotation"] = z_rot;
	parameters["rotation"] = QJsonArray{ angle_deg, rotation_axis.x, rotation_axis.y, rotation_axis.z };
	json["parameters"] = parameters;
}


void TriangleAreaLight::setLightMaterial(optix::Context context) 
{
	int tmp = geometry_group->getChildCount();
	for (unsigned int j = 0; j < geometry_group->getChildCount(); ++j)
	{
		optix::GeometryInstance& gi = geometry_group->getChild(j);
		optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "arealight_shader.cu"), "any_hit");
		optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "arealight_shader.cu"), "closest_hit");
		optix::Material& m = gi->getMaterial(0);
		
		m->setClosestHitProgram(0, closest_hit);
		m->setAnyHitProgram(1, any_hit);
		gi["radiance"]->setFloat(radiance);
	}

}


void TriangleAreaLight::loadLightGeometry()
{

	
	ObjLoader* loader = new ObjLoader(path.toStdString().c_str(), context, geometry_group);
	loader->loadAreaLight(optix::make_float3(radiance.x, radiance.y, radiance.z));
	transform->setMatrix(0, transformationMatrix.getData(), transformationMatrix.inverse().getData());
	
	setLightMaterial(context);
	triangle_light_buffer = loader->getLightBuffer();
	triangle_light_buffer->getSize(size);
	transformed_light_buffer = context->createBuffer(RT_BUFFER_INPUT);
	transformed_light_buffer->setFormat(RT_FORMAT_USER);
	transformed_light_buffer->setElementSize(sizeof(TriangleLight));
	transformed_light_buffer->setSize(size);
	applyTransformationMatrix();
	delete loader;
}

void TriangleAreaLight::computeTransformationMatrix()
{
	optix::Matrix4x4 total_rotation = z_rotation * y_rotation * x_rotation * rotation;
	transformationMatrix = optix::Matrix4x4::translate(translation) * total_rotation * optix::Matrix4x4::scale(scale);
	
}

void TriangleAreaLight::applyTransformationMatrix()
{
	TriangleLight* transformed_data = static_cast<TriangleLight*>(transformed_light_buffer->map());
	TriangleLight* original_data = static_cast<TriangleLight*>(triangle_light_buffer->map());
	optix::Matrix4x4 normalMatrix = transformationMatrix.inverse().transpose();
	for (int idx = 0; idx < size; idx++)
	{
		TriangleLight* transformed_light = &transformed_data[idx];
		TriangleLight* light = &original_data[idx];
		transformed_light->v0 = make_float3(transformationMatrix * optix::make_float4(light->v0, 1.0f));
		transformed_light->v1 = make_float3(transformationMatrix * optix::make_float4(light->v1, 1.0f));
		transformed_light->v2 = make_float3(transformationMatrix * optix::make_float4(light->v2, 1.0f));
		transformed_light->has_normals = 0;
		if (light->has_normals)
		{
			transformed_light->has_normals = 1;
			transformed_light->n0 = make_float3(normalMatrix * optix::make_float4(light->n0, 0.0f));
			transformed_light->n1 = make_float3(normalMatrix * optix::make_float4(light->n1, 0.0f));
			transformed_light->n2 = make_float3(normalMatrix * optix::make_float4(light->n2, 0.0f));
		}
		// normal vector
		optix::float3 perp_triangle = cross(transformed_light->v1 - transformed_light->v2, transformed_light->v0 - transformed_light->v2);
		transformed_light->area = 0.5*length(perp_triangle);
		transformed_light->emission = light->emission;
	}
	transformed_light_buffer->unmap();
	triangle_light_buffer->unmap();
	transform->setMatrix(0, transformationMatrix.getData(), transformationMatrix.inverse().getData());
	geometry_group->getAcceleration()->markDirty();
}
void TriangleAreaLight::setRadiance(QVector3D r)
{
	radiance = optix::make_float3(r.x(),r.y(),r.z());

}

void TriangleAreaLight::setPath(QString p)
{
	//QDir dir("./");
	//path = dir.relativeFilePath(p);
	QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
	QFileInfo fileInfo(dir, p);
	path = (fileInfo.absoluteFilePath());
}
//
void TriangleAreaLight::setTranslation(QVector3D t)
{
	translation = optix::make_float3(t.x(), t.y(), t.z());
	computeTransformationMatrix();
	applyTransformationMatrix();
}

void TriangleAreaLight::setScale(QVector3D s)
{
	scale = optix::make_float3(s.x(), s.y(), s.z());
	computeTransformationMatrix();
	applyTransformationMatrix();
}

void TriangleAreaLight::setRotation(float a_d, QVector3D a)
{
	angle_deg = a_d;
	rotation_axis = optix::normalize(optix::make_float3(a.x(), a.y(), a.z()));
	rotation = optix::Matrix4x4::rotate(angle_deg * M_PIf / 180.0f, rotation_axis);
	computeTransformationMatrix();
	applyTransformationMatrix();
}

void TriangleAreaLight::setRotationX(float a_d)
{
	x_rot = a_d;
	optix::float3 x_axis = optix::make_float3(1.0f, 0.0f, 0.0f);
	x_rotation = optix::Matrix4x4::rotate(x_rot * M_PIf / 180.0f, x_axis);
	computeTransformationMatrix();
	applyTransformationMatrix();
}
void TriangleAreaLight::setRotationY(float a_d)
{
	y_rot = a_d;
	optix::float3 y_axis = optix::make_float3(0.0f, 1.0f, 0.0f);
	y_rotation = optix::Matrix4x4::rotate(y_rot * M_PIf / 180.0f, y_axis);
	computeTransformationMatrix();
	applyTransformationMatrix();
}

void TriangleAreaLight::setRotationZ(float a_d)
{
	z_rot = a_d;
	optix::float3 z_axis = optix::make_float3(0.0f, 0.0f, 1.0f);
	z_rotation = optix::Matrix4x4::rotate(z_rot * M_PIf / 180.0f, z_axis);
	computeTransformationMatrix();
	applyTransformationMatrix();
}


//DiskLight
DiskAreaLight::DiskAreaLight(optix::Context c)
{
	position = optix::make_float3(0.0f, 0.0f, 0.0f);
	radiance = optix::make_float3(1.0f, 1.0f, 1.0f);
	radius = 1.0f;
	theta = 0.0f;
	phi = 0.0f;
	computeNormal();
	type = DISK_LIGHT;
	context = c;
	geometry_group = context->createGeometryGroup();
	disk = context->createGeometry();
	initGeometry();
	
}

DiskAreaLight::~DiskAreaLight()
{
	
}

void DiskAreaLight::readJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}

	if (parameters.contains("position") && parameters["position"].isArray()) {
		QJsonArray tmp = parameters["position"].toArray();
		position = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}


	if (parameters.contains("radiance") && parameters["radiance"].isArray()) {
		QJsonArray tmp = parameters["radiance"].toArray();
		radiance = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	if (parameters.contains("radius") && parameters["radius"].isDouble()) {
		radius = parameters["radius"].toDouble();
	}
	if (parameters.contains("theta") && parameters["theta"].isDouble()) {
		theta = parameters["theta"].toDouble();
	}
	if (parameters.contains("phi") && parameters["phi"].isDouble()) {
		phi = parameters["phi"].toDouble();
	}
	computeNormal();
	gi["radiance"]->setFloat(radiance);
	gi["position"]->setFloat(position);
	gi["normal"]->setFloat(normal);
	gi["radius"]->setFloat(radius);
}

void DiskAreaLight::writeJSON(QJsonObject &json) const
{
	json["type"] = lightNames[type];
	QJsonObject parameters;
	parameters["position"] = QJsonArray{ position.x, position.y, position.z};
	parameters["radiance"] = QJsonArray{ radiance.x, radiance.y, radiance.z};
	parameters["radius"] = radius;
	parameters["theta"] = theta;
	parameters["phi"] = phi;
	json["parameters"] = parameters;
}

void DiskAreaLight::setRadiance(QVector3D r)
{
	radiance = optix::make_float3(r.x(), r.y(), r.z());
	
	gi["radiance"]->setFloat(radiance);
};

void DiskAreaLight::setPosition(QVector3D p)
{
	position = optix::make_float3(p.x(), p.y(), p.z());
	
	gi["position"]->setFloat(position);
	geometry_group->getAcceleration()->markDirty();
};

void DiskAreaLight::setRadius(float r)
{
	radius = r;
	
	gi["radius"]->setFloat(radius);
	geometry_group->getAcceleration()->markDirty();
};

void DiskAreaLight::setNormal(float t, float p)
{
	theta = t;
	phi = p;
	computeNormal();
	gi["normal"]->setFloat(normal);
	geometry_group->getAcceleration()->markDirty();
};

void DiskAreaLight::computeNormal() 
{
	float t_rad = theta * M_PIf / 180.0f;
	float p_rad = phi * M_PIf / 180.0f;
	normal = optix::make_float3(sinf(t_rad) * sinf(p_rad), cosf(t_rad), sinf(t_rad) * cosf(p_rad));
}

void DiskAreaLight::initGeometry() {

	optix::Program disk_intersect = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "disk_mesh.cu"), "disk_intersect");
	optix::Program disk_bounds = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "disk_mesh.cu"), "disk_bounds");

	disk->setPrimitiveCount(1u);
	disk->setBoundingBoxProgram(disk_bounds);
	disk->setIntersectionProgram(disk_intersect);


	optix::Material m = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "arealight_shader.cu"), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "arealight_shader.cu"), "closest_hit");
	m->setClosestHitProgram(0, closest_hit);
	m->setAnyHitProgram(1, any_hit);
	gi = context->createGeometryInstance(disk, &m, &m+ 1);
	gi["radiance"]->setFloat(radiance);
	gi["position"]->setFloat(position);
	gi["normal"]->setFloat(normal);
	gi["radius"]->setFloat(radius);
	geometry_group->setChildCount(1);
	geometry_group->setChild(0, gi);
	optix::Acceleration acceleration = context->createAcceleration("Bvh");
	//geometry_group->setAcceleration(acceleration);
	geometry_group->setAcceleration(context->createAcceleration("NoAccel"));
	acceleration->markDirty();
}

//DiskLight
SphericalLight::SphericalLight(optix::Context c)
{
	position = optix::make_float3(0.0f, 0.0f, 0.0f);
	radiance = optix::make_float3(1.0f, 1.0f, 1.0f);
	radius = 0.05f;

	type = SPHERICAL_LIGHT;
	context = c;
	geometry_group = context->createGeometryGroup();
	sphere = context->createGeometry();
	initGeometry();

}

SphericalLight::~SphericalLight()
{

}

void SphericalLight::readJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}

	if (parameters.contains("position") && parameters["position"].isArray()) {
		QJsonArray tmp = parameters["position"].toArray();
		position = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	if (parameters.contains("radiance") && parameters["radiance"].isArray()) {
		QJsonArray tmp = parameters["radiance"].toArray();
		radiance = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	if (parameters.contains("radius") && parameters["radius"].isDouble()) {
		radius = parameters["radius"].toDouble();
	}

	gi["radiance"]->setFloat(radiance);
	gi["position"]->setFloat(position);
	gi["radius"]->setFloat(radius);
}

void SphericalLight::writeJSON(QJsonObject &json) const
{
	json["type"] = lightNames[type];
	QJsonObject parameters;
	parameters["position"] = QJsonArray{ position.x, position.y, position.z };
	parameters["radiance"] = QJsonArray{ radiance.x, radiance.y, radiance.z };
	parameters["radius"] = radius;
	json["parameters"] = parameters;
}

void SphericalLight::setRadiance(QVector3D r)
{
	radiance = optix::make_float3(r.x(), r.y(), r.z());

	gi["radiance"]->setFloat(radiance);
};

void SphericalLight::setPosition(QVector3D p)
{
	position = optix::make_float3(p.x(), p.y(), p.z());

	gi["position"]->setFloat(position);
	geometry_group->getAcceleration()->markDirty();
};

void SphericalLight::setRadius(float r)
{
	radius = r;

	gi["radius"]->setFloat(radius);
	geometry_group->getAcceleration()->markDirty();
};

void SphericalLight::initGeometry() {

	optix::Program disk_intersect = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "sphere_mesh.cu"), "sphere_intersect");
	optix::Program disk_bounds = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "sphere_mesh.cu"), "sphere_bounds");

	sphere->setPrimitiveCount(1u);
	sphere->setBoundingBoxProgram(disk_bounds);
	sphere->setIntersectionProgram(disk_intersect);

	optix::Material m = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "arealight_shader.cu"), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "arealight_shader.cu"), "closest_hit");
	m->setClosestHitProgram(0, closest_hit);
	m->setAnyHitProgram(1, any_hit);
	gi = context->createGeometryInstance(sphere, &m, &m + 1);
	gi["radiance"]->setFloat(radiance);
	gi["position"]->setFloat(position);
	gi["radius"]->setFloat(radius);
	geometry_group->setChildCount(1);
	geometry_group->setChild(0, gi);
	optix::Acceleration acceleration = context->createAcceleration("Bvh");
	//geometry_group->setAcceleration(acceleration);
	geometry_group->setAcceleration(context->createAcceleration("NoAccel"));
	acceleration->markDirty();
}