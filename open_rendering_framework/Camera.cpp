#pragma once
#include "Camera.h"
#include "OptixScene.h"
#include "sampleConfig.h"


PinholeCamera::PinholeCamera(optix::Context c)
{
	context = c;
}

PinholeCamera::~PinholeCamera()
{

}

void PinholeCamera::readJSON(const QJsonObject &json, uint& width, uint&  height)
{
	if (json.contains("type") && json["type"].isString()) {
		type = PINHOLE_CAMERA;
	}
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}

	if (parameters.contains("eye") && parameters["eye"].isArray()) {
		QJsonArray tmp = parameters["eye"].toArray();
		eye = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}
	if (parameters.contains("lookat") && parameters["lookat"].isArray()) {
		QJsonArray tmp = parameters["lookat"].toArray();
		lookat = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}
	if (parameters.contains("up") && parameters["up"].isArray()) {
		QJsonArray tmp = parameters["up"].toArray();
		up = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	if (parameters.contains("vfov") && parameters["vfov"].isDouble())
		vfov = parameters["vfov"].toDouble();

	if (parameters.contains("width") && parameters["width"].isDouble())
		WIDTH = parameters["width"].toInt();

	if (parameters.contains("height") && parameters["height"].isDouble())
		HEIGHT = parameters["height"].toInt();

	if (parameters.contains("patch_origin") && parameters["patch_origin"].isArray()) {
		QJsonArray tmp = parameters["patch_origin"].toArray();
		patch_origin = optix::make_uint2(tmp[0].toInt(), tmp[1].toInt());
	}
	else {
		patch_origin = optix::make_uint2(0, 0);
	}
	if (parameters.contains("patch_dims") && parameters["patch_dims"].isArray()) {
		QJsonArray tmp = parameters["patch_dims"].toArray();
		patch_dims = optix::make_uint2(tmp[0].toInt(), tmp[1].toInt());
	}
	else {
		patch_dims = optix::make_uint2(WIDTH, HEIGHT);
	}
	calculateCameraVariables();
	width = getWidth();
	height = getHeight();
}

void PinholeCamera::writeJSON(QJsonObject &json) const
{
	json["type"] = QString(cameraNames[type]);
	QJsonObject parameters;
	parameters["eye"] = QJsonArray{ eye.x(), eye.y(), eye.z() };
	parameters["lookat"] = QJsonArray{ lookat.x(), lookat.y(), lookat.z() };
	parameters["up"] = QJsonArray{ up.x(), up.y(), up.z() };
	parameters["vfov"] = vfov;
	parameters["width"] =(int) WIDTH;
	parameters["height"] = (int) HEIGHT;
	parameters["patch_origin"] = QJsonArray{ (int)patch_origin.x, (int)patch_origin.y };
	parameters["patch_dims"] = QJsonArray{ (int)patch_dims.x, (int)patch_dims.y };
	json["parameters"] = parameters;
}

void PinholeCamera::calculateCameraVariables()
{
	float ulen, vlen, wlen;
	W = lookat - eye; // Do not normalize W -- it implies focal length

	wlen = W.length();
	U = QVector3D::crossProduct(W, up).normalized();
	V = QVector3D::crossProduct(U, W).normalized();

	vlen = wlen * tanf(0.5f * vfov * M_PIf / 180.0f);
	V *= vlen;
	float aspect_ratio = (1.0f*WIDTH) / HEIGHT;
	ulen = vlen * aspect_ratio;
	U *= ulen;
	context["eye"]->setFloat(eye.x(), eye.y(), eye.z());
	context["U"]->setFloat(U.x(), U.y(), U.z());
	context["V"]->setFloat(V.x(), V.y(), V.z());
	context["W"]->setFloat(W.x(), W.y(), W.z());
	context["patch_origin"]->setUint(patch_origin);
	context["patch_dims"]->setUint(patch_dims);
}

void PinholeCamera::resizeCamera(uint width, uint height)
{
	WIDTH = width;
	HEIGHT = height;
	setPatchDims(optix::make_uint2(width, height));
	calculateCameraVariables();
}

void PinholeCamera::setEye(QVector3D e) { eye = e; calculateCameraVariables();};
void PinholeCamera::setLookat(QVector3D l) { lookat = l; calculateCameraVariables();};
void PinholeCamera::setUp(QVector3D u) { up = u; calculateCameraVariables();};
void PinholeCamera::setFov(float f) { vfov = f; calculateCameraVariables();};
void PinholeCamera::setPatchOrigin(optix::uint2 origin){ patch_origin = origin; calculateCameraVariables(); };
void PinholeCamera::setPatchDims(optix::uint2 dims){ patch_dims = dims; calculateCameraVariables(); };