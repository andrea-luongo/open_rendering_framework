#pragma once
#include "Background.h"
#include "OptixScene.h"
#include "sampleConfig.h"

ConstantBackground::ConstantBackground(optix::Context c)
{
	type = CONSTANT_BACKGROUND;
	context = c;
	background_color = QVector3D(1.0, 1.0, 1.0);
	context->setMissProgram(radiance_ray_type, context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "constantbg.cu"), "miss"));
	context->setMissProgram(depth_ray_type, context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "constantbg.cu"), "depth_miss"));
	context["background_color"]->setFloat(background_color.x(), background_color.y(), background_color.z());
}

ConstantBackground::~ConstantBackground()
{

}

void ConstantBackground::readJSON(const QJsonObject &json)
{
	if (json.contains("type") && json["type"].isString()) {
		type = CONSTANT_BACKGROUND;
	}
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}

	if (parameters.contains("background_color") && parameters["background_color"].isArray()) {
		QJsonArray tmp = parameters["background_color"].toArray();
		background_color = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	// Miss program
	context->setMissProgram(radiance_ray_type, context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "constantbg.cu"), "miss"));
	context->setMissProgram(depth_ray_type, context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "constantbg.cu"), "depth_miss"));
	context["background_color"]->setFloat(background_color.x(), background_color.y(), background_color.z());
}

void ConstantBackground::writeJSON(QJsonObject &json) const
{
	json["type"] = backgroundNames[type];
	QJsonObject parameters;
	parameters["background_color"] = QJsonArray{ background_color.x(), background_color.y(), background_color.z() };
	json["parameters"] = parameters;
}

void ConstantBackground::setBackgroundColor(QVector3D back_color)
{
	background_color = back_color;
	context["background_color"]->setFloat(background_color.x(), background_color.y(), background_color.z());

}

EnvMapBackground::EnvMapBackground(optix::Context c)
{
	type = ENVMAP_BACKGROUND;
	context = c;
	background_color = QVector3D(1.0, 1.0, 1.0);
	path = QString::fromStdString("../../data/CedarCity.hdr");
	envmap = loadTexture(context, path.toStdString(),optix::make_float3(background_color.x(), background_color.y(), background_color.z()));
	// Miss program
	context->setMissProgram(radiance_ray_type, context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "envmap_background.cu"), "miss"));
	context->setMissProgram(depth_ray_type, context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "envmap_background.cu"), "depth_miss"));
	context["envmap"]->setTextureSampler(envmap);

}

EnvMapBackground::~EnvMapBackground()
{

}

void EnvMapBackground::readJSON(const QJsonObject &json)
{
	if (json.contains("type") && json["type"].isString()) {
		type = ENVMAP_BACKGROUND;
	}
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}

	if (parameters.contains("background_color") && parameters["background_color"].isArray()) {
		QJsonArray tmp = parameters["background_color"].toArray();
		background_color = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}
	if (parameters.contains("path") && parameters["path"].isString()) {
		//QFileInfo fileInfo = parameters["path"].toString();
		//QDir dir("./");
		//path = dir.relativeFilePath(fileInfo.absoluteFilePath());

		QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
		QFileInfo fileInfo(dir, parameters["path"].toString());
		path = (fileInfo.absoluteFilePath());
		//path = fileInfo.absoluteFilePath();
		envmap = loadTexture(context, path.toStdString(), optix::make_float3(background_color.x(), background_color.y(), background_color.z()));
		context["envmap"]->setTextureSampler(envmap);
	}
	context->setMissProgram(radiance_ray_type, context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "envmap_background.cu"), "miss"));
	context->setMissProgram(depth_ray_type, context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "envmap_background.cu"), "depth_miss"));
	
}

void EnvMapBackground::writeJSON(QJsonObject &json) const
{
	json["type"] = backgroundNames[type];
	QJsonObject parameters;
	parameters["background_color"] = QJsonArray{ background_color.x(), background_color.y(), background_color.z() };
	QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
	parameters["path"] = dir.relativeFilePath(path);
	json["parameters"] = parameters;
}

void EnvMapBackground::changePath(QString new_path) 
{
	path = new_path;
	/*QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
	path = dir.relativeFilePath(path);*/
	envmap = loadTexture(context, path.toStdString(), optix::make_float3(background_color.x(), background_color.y(), background_color.z()));
	context["envmap"]->setTextureSampler(envmap);
}

