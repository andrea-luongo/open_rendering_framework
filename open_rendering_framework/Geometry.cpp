#pragma once
//#include "Geometry.h"
#include <QJsonArray>
#include "ObjLoader.h"
#include "OptixScene.h"
#include "sampleConfig.h"
#include "Material.h"
using optix::Matrix4x4;
using optix::float3;


OBJGeometry::OBJGeometry(optix::Context c)
{
	QFileInfo fileInfo = QString("../../data/closed_bunny_vn.obj");
	OBJGeometry(c, QString("../../data/closed_bunny_vn.obj"));
}

OBJGeometry::OBJGeometry(optix::Context c, QString filename)
{
	context = c;
	type = OBJ;
	geometry_group = context->createGeometryGroup();
	transform = context->createTransform();
	transform->setChild(geometry_group);
	QFileInfo fileInfo = filename;
	path = fileInfo.absoluteFilePath();
	//path = filename;
	position = optix::make_float3(0.0f, 0.0f, 0.0f);
	scale = optix::make_float3(1.0f, 1.0f, 1.0f);
	rotation = Matrix4x4::identity();
	x_rotation = Matrix4x4::identity();
	y_rotation = Matrix4x4::identity();
	z_rotation = Matrix4x4::identity();
	x_rot = 0.0f;
	y_rot = 0.0f;
	z_rot = 0.0f;
	angle_deg = 0.0f;
	rotation_axis = optix::make_float3(0.0f, 1.0f, 0.0f);
	mtl = new DiffuseMaterial(context);
	computeTransformationMatrix();
	loadGeometry();
	loadMaterial(mtl);

}

OBJGeometry::OBJGeometry(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = OBJ;
	geometry_group = context->createGeometryGroup();
	transform = context->createTransform();
	transform->setChild(geometry_group);
	position = optix::make_float3(0.0f, 0.0f, 0.0f);
	scale = optix::make_float3(1.0f, 1.0f, 1.0f);
	rotation = Matrix4x4::identity();
	x_rotation = Matrix4x4::identity();
	y_rotation = Matrix4x4::identity();
	z_rotation = Matrix4x4::identity();
	x_rot = 0.0f;
	y_rot = 0.0f;
	z_rot = 0.0f;
	angle_deg = 0.0f;
	rotation_axis = optix::make_float3(0.0f, 1.0f, 0.0f);
	mtl = new DiffuseMaterial(context);
	readJSON(json);
}

OBJGeometry::~OBJGeometry()
{

}

void OBJGeometry::readJSON(const QJsonObject &json)
{
	if (json.contains("type") && json["type"].isString()) {
		type = OBJ;
	}
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}


	if (parameters.contains("path") && parameters["path"].isString()) {
		QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
		QFileInfo fileInfo(dir, parameters["path"].toString());
		path = (fileInfo.absoluteFilePath());
		//std::cerr << "objpath( '" << path.toStdString() << "' ) failed to load file: " << '\n';
	}


	if (parameters.contains("position") && parameters["position"].isArray()) {
		QJsonArray tmp = parameters["position"].toArray();
		position = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}


	if (parameters.contains("scale") && parameters["scale"].isArray()) {
		QJsonArray tmp = parameters["scale"].toArray();
		scale = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
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

	if (parameters.contains("rotation") && parameters["rotation"].isArray()) {		
		QJsonArray tmp = parameters["rotation"].toArray();
		angle_deg = tmp[0].toDouble();
		rotation_axis = optix::normalize(optix::make_float3(tmp[1].toDouble(), tmp[2].toDouble(), tmp[3].toDouble()));
		rotation = Matrix4x4::rotate(angle_deg * M_PIf / 180.0f, rotation_axis);
	}

	loadGeometry();
	if (parameters.contains("material") && parameters["material"].isObject()) {
		loadMaterialFromJSON(parameters["material"].toObject());
	}
	else {
		loadMaterial(mtl);
	}

	//sampler = loadTexture(texture_path, context);
	//mtl->getOptixMaterial()->declareVariable("texture_sampler");
	//mtl->getOptixMaterial()["texture_sampler"]->setTextureSampler(sampler);

	
}

void OBJGeometry::writeJSON(QJsonObject &json) const
{
	json["type"] = geometryNames[type];
	QJsonObject parameters;
	QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
	parameters["path"] = dir.relativeFilePath(path);
	parameters["position"] = QJsonArray{ position.x, position.y, position.z};
	parameters["scale"] = QJsonArray{ scale.x, scale.y, scale.z };
	parameters["rotation"] = QJsonArray{ angle_deg, rotation_axis.x, rotation_axis.y, rotation_axis.z };
	parameters["x_rotation"] = x_rot;
	parameters["y_rotation"] = y_rot;
	parameters["z_rotation"] = z_rot;
	
	QJsonObject material;
	mtl->writeJSON(material);
	parameters["material"] = material;
	//if (parameters.contains("material") && parameters["material"].isObject()) {
	//	loadMaterialFromJSON(parameters["material"].toObject());
	//}
	json["parameters"] = parameters;
}

void OBJGeometry::loadMaterial(MyMaterial* material)
{
	//if (mtl != NULL) {
	//	delete mtl;
	//}
	mtl = material;
	for (unsigned int j = 0; j < geometry_group->getChildCount(); ++j)
	{
		optix::GeometryInstance& gi = geometry_group->getChild(j);
		gi->setMaterial(0, (mtl->getOptixMaterial()));
	}

}

void OBJGeometry::loadMaterialFromJSON(const QJsonObject &material_json)
{
	QString mtl_name;
	if (material_json.contains("mtl_name") && material_json["mtl_name"].isString()) {
		mtl_name = material_json["mtl_name"].toString();
	}

	if (mtl_name.compare(QString("diffuse_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new DiffuseMaterial(context, material_json);
	} else if (mtl_name.compare(QString("lambertian_interface_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new LambertianInterfaceMaterial(context, material_json);
	} else if (mtl_name.compare(QString("normal_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new NormalMaterial(context, material_json);
	} else if (mtl_name.compare(QString("transparent_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new TransparentMaterial(context, material_json);
	} else if (mtl_name.compare(QString("rough_transparent_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new RoughTransparentMaterial(context, material_json);
	} else if (mtl_name.compare(QString("metallic_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new MetallicMaterial(context, material_json);
	} else if (mtl_name.compare(QString("translucent_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new TranslucentMaterial(context, material_json);
	} else if (mtl_name.compare(QString("flat_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new FlatMaterial(context, material_json);	
	} else if (mtl_name.compare(QString("rough_diffuse_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new RoughDiffuseMaterial(context, material_json);
	} else if (mtl_name.compare(QString("rough_translucent_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new RoughTranslucentMaterial(context, material_json);
	} else if (mtl_name.compare(QString("anisotropic_structure_shader"), Qt::CaseInsensitive) == 0) {
		mtl = new AnisotropicMaterial(context, material_json);
	}

	for (unsigned int j = 0; j < geometry_group->getChildCount(); ++j)
	{
		optix::GeometryInstance& gi = geometry_group->getChild(j);
		gi->setMaterial(0, (mtl->getOptixMaterial()));
	}

}

void OBJGeometry::loadMaterial(MaterialType mtl_type)
{

	if (mtl_type == DIFFUSE_SHADER) {
		mtl = new DiffuseMaterial(context);
	}
	else if (mtl_type == LAMBERTIAN_INTERFACE_SHADER) {
		mtl = new LambertianInterfaceMaterial(context);
	}
	else if (mtl_type == NORMAL_SHADER) {
		mtl = new NormalMaterial(context);
	}
	else if (mtl_type == TRANSPARENT_SHADER) {
		mtl = new TransparentMaterial(context);
	}
	else if (mtl_type == ROUGH_TRANSPARENT_SHADER) {
		mtl = new RoughTransparentMaterial(context);
	}
	else if (mtl_type == METALLIC_SHADER) {
		mtl = new MetallicMaterial(context);
	} 
	else if (mtl_type == TRANSLUCENT_SHADER) {
		mtl = new TranslucentMaterial(context);
		//TODO add geometry group to translucent buffer
	}
	else if (mtl_type == FLAT_SHADER) {
		mtl = new FlatMaterial(context);
		//TODO add geometry group to translucent buffer
	}
	else if (mtl_type == ROUGH_DIFFUSE_SHADER) {
		mtl = new RoughDiffuseMaterial(context);
		//TODO add geometry group to translucent buffer
	}
	else if (mtl_type == ROUGH_TRANSLUCENT_SHADER) {
		mtl = new RoughTranslucentMaterial(context);
		//TODO add geometry group to translucent buffer
	}
	else if (mtl_type == ANISOTROPIC_MATERIAL) {
		mtl = new AnisotropicMaterial(context);
		//TODO add geometry group to translucent buffer
	}
	for (unsigned int j = 0; j < geometry_group->getChildCount(); ++j)
	{
		optix::GeometryInstance& gi = geometry_group->getChild(j);
		gi->setMaterial(0, (mtl->getOptixMaterial()));
	}
}

void OBJGeometry::loadGeometry()
{
	ObjLoader* loader = new ObjLoader(path.toStdString().c_str(), context, geometry_group);
	loader->load();
	optix::Aabb test_box = loader->getSceneBBox();

	const float3 extents = test_box.extent();
	float3 mmax = test_box.m_max;
	float3 mmin = test_box.m_min;
	float x_length = mmax.x - mmin.x;
	float y_length = mmax.y - mmin.y;
	float z_length = mmax.z - mmin.z;
	float3 diagonal = mmax - mmin;
	float diag = length(diagonal);
	computeTransformationMatrix();
	delete loader;
}



void OBJGeometry::computeTransformationMatrix()
{
	optix::Matrix4x4 total_rotation = z_rotation * y_rotation * x_rotation * rotation;
	transformationMatrix = optix::Matrix4x4::translate(position) * total_rotation * optix::Matrix4x4::scale(scale);
	transform->setMatrix(0, transformationMatrix.getData(), transformationMatrix.inverse().getData());
}

void OBJGeometry::setPosition(QVector3D t)
{
	position = optix::make_float3(t.x(), t.y(), t.z());
	computeTransformationMatrix();
}

void OBJGeometry::setScale(QVector3D s)
{
	scale = optix::make_float3(s.x(), s.y(), s.z());
	computeTransformationMatrix();
}

void OBJGeometry::setRotation(float a_d, QVector3D a)
{
	angle_deg = a_d;
	rotation_axis = optix::normalize(optix::make_float3(a.x(), a.y(), a.z()));
	rotation = optix::Matrix4x4::rotate(angle_deg * M_PIf / 180.0f, rotation_axis);
	computeTransformationMatrix();
}

void OBJGeometry::setRotationX(float a_d)
{
	x_rot = a_d;
	optix::float3 x_axis = optix::make_float3(1.0f, 0.0f, 0.0f);
	x_rotation = optix::Matrix4x4::rotate(x_rot * M_PIf / 180.0f, x_axis);
	computeTransformationMatrix();
}
void OBJGeometry::setRotationY(float a_d)
{
	y_rot = a_d;
	optix::float3 y_axis = optix::make_float3(0.0f, 1.0f, 0.0f);
	y_rotation = optix::Matrix4x4::rotate(y_rot * M_PIf / 180.0f, y_axis);
	computeTransformationMatrix();
}

void OBJGeometry::setRotationZ(float a_d)
{
	z_rot = a_d;
	optix::float3 z_axis = optix::make_float3(0.0f, 0.0f, 1.0f);
	z_rotation = optix::Matrix4x4::rotate(z_rot * M_PIf / 180.0f, z_axis);
	computeTransformationMatrix();
}

