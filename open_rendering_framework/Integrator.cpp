#pragma once
#include "Integrator.h"
#include "OptixScene.h"
#include "sampleConfig.h"

PathTracer::PathTracer(optix::Context c)
{
	context = c;
	type = PATH_TRACER;
	max_depth = 50;
	scene_epsilon = 1e-4;
	exception_color = QVector3D(1, 0, 0);
	context["max_depth"]->setInt(max_depth);
	context["scene_epsilon"]->setFloat(scene_epsilon);
	// Ray generation program
	const std::string ptx_camera_path = OptixScene::ptxPath(SAMPLE_NAME, "path_tracer.cu");
	optix::Program ray_gen_program = context->createProgramFromPTXFile(ptx_camera_path, "path_tracer");
	context->setRayGenerationProgram(integrator_pass, ray_gen_program);

	// Exception program
	optix::Program exception_program = context->createProgramFromPTXFile(ptx_camera_path, "exception");
	context->setExceptionProgram(integrator_pass, exception_program);
	context["exception_color"]->setFloat(exception_color.x(), exception_color.y(), exception_color.z());
}

PathTracer::~PathTracer()
{

}

void PathTracer::readJSON(const QJsonObject &json )
{
	if (json.contains("type") && json["type"].isString()) {
		type = PATH_TRACER;
	}
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}

	if (parameters.contains("max_depth") && parameters["max_depth"].isDouble())
		max_depth = parameters["max_depth"].toInt();

	if (parameters.contains("scene_epsilon") && parameters["scene_epsilon"].isDouble())
		scene_epsilon = (float) parameters["scene_epsilon"].toDouble();

	if (parameters.contains("exception_color") && parameters["exception_color"].isArray()) {
		QJsonArray tmp = parameters["exception_color"].toArray();
		exception_color = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	context["max_depth"]->setInt(max_depth);
	context["scene_epsilon"]->setFloat(scene_epsilon);
	// Ray generation program
	const std::string ptx_camera_path = OptixScene::ptxPath(SAMPLE_NAME, "path_tracer.cu");
	optix::Program ray_gen_program = context->createProgramFromPTXFile(ptx_camera_path, "path_tracer");
	context->setRayGenerationProgram(integrator_pass, ray_gen_program);

	// Exception program
	optix::Program exception_program = context->createProgramFromPTXFile(ptx_camera_path, "exception");
	context->setExceptionProgram(integrator_pass, exception_program);
	context["exception_color"]->setFloat(exception_color.x(), exception_color.y(), exception_color.z());

}

void PathTracer::writeJSON(QJsonObject &json) const
{
	json["type"] = integratorNames[type];
	QJsonObject parameters;
	parameters["max_depth"] = (int)max_depth;
	parameters["scene_epsilon"] = scene_epsilon;
	parameters["exception_color"] = QJsonArray{ exception_color.x(), exception_color.y(), exception_color.z() };
	json["parameters"] = parameters;
}


void PathTracer::setMaxDepth(uint max_d) 
{ 
	max_depth = max_d; 
	context["max_depth"]->setInt(max_depth);	
}

void PathTracer::setSceneEpsilon(float scene_eps) 
{ 
	scene_epsilon = scene_eps; 
	context["scene_epsilon"]->setFloat(scene_epsilon);
}

void PathTracer::setExceptionColor(QVector3D exc_color) 
{ 
	exception_color = exc_color; 
	context["exception_color"]->setFloat(exception_color.x(), exception_color.y(), exception_color.z());
}



//--------------------------------------------------------------------------------------------
//DEPTH TRACER

DepthTracer::DepthTracer(optix::Context c)
{
	context = c;
	type = DEPTH_TRACER;
	max_depth = 50;
	scene_epsilon = 1e-4;
	exception_color = QVector3D(1, 0, 0);
	context["max_depth"]->setInt(max_depth);
	context["scene_epsilon"]->setFloat(scene_epsilon);
	// Ray generation program
	const std::string ptx_camera_path = OptixScene::ptxPath(SAMPLE_NAME, "depth_tracer.cu");
	optix::Program ray_gen_program = context->createProgramFromPTXFile(ptx_camera_path, "depth_tracer");
	context->setRayGenerationProgram(integrator_pass, ray_gen_program);

	// Exception program
	optix::Program exception_program = context->createProgramFromPTXFile(ptx_camera_path, "exception");
	context->setExceptionProgram(integrator_pass, exception_program);
	context["exception_color"]->setFloat(exception_color.x(), exception_color.y(), exception_color.z());
}

DepthTracer::~DepthTracer()
{

}

void DepthTracer::readJSON(const QJsonObject &json)
{
	if (json.contains("type") && json["type"].isString()) {
		type = DEPTH_TRACER;
	}
	QJsonObject parameters;
	if (json.contains("parameters") && json["parameters"].isObject()) {
		parameters = json["parameters"].toObject();
	}

	if (parameters.contains("max_depth") && parameters["max_depth"].isDouble())
		max_depth = parameters["max_depth"].toInt();

	if (parameters.contains("scene_epsilon") && parameters["scene_epsilon"].isDouble())
		scene_epsilon = (float)parameters["scene_epsilon"].toDouble();

	if (parameters.contains("exception_color") && parameters["exception_color"].isArray()) {
		QJsonArray tmp = parameters["exception_color"].toArray();
		exception_color = QVector3D(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	context["max_depth"]->setInt(max_depth);
	context["scene_epsilon"]->setFloat(scene_epsilon);
	// Ray generation program
	const std::string ptx_camera_path = OptixScene::ptxPath(SAMPLE_NAME, "depth_tracer.cu");
	optix::Program ray_gen_program = context->createProgramFromPTXFile(ptx_camera_path, "depth_tracer");
	context->setRayGenerationProgram(integrator_pass, ray_gen_program);

	// Exception program
	optix::Program exception_program = context->createProgramFromPTXFile(ptx_camera_path, "exception");
	context->setExceptionProgram(integrator_pass, exception_program);
	context["exception_color"]->setFloat(exception_color.x(), exception_color.y(), exception_color.z());

}

void DepthTracer::writeJSON(QJsonObject &json) const
{
	json["type"] = integratorNames[type];
	QJsonObject parameters;
	parameters["max_depth"] = (int)max_depth;
	parameters["scene_epsilon"] = scene_epsilon;
	parameters["exception_color"] = QJsonArray{ exception_color.x(), exception_color.y(), exception_color.z() };
	json["parameters"] = parameters;
}


void DepthTracer::setMaxDepth(uint max_d)
{
	max_depth = max_d;
	context["max_depth"]->setInt(max_depth);
}

void DepthTracer::setSceneEpsilon(float scene_eps)
{
	scene_epsilon = scene_eps;
	context["scene_epsilon"]->setFloat(scene_epsilon);
}

void DepthTracer::setExceptionColor(QVector3D exc_color)
{
	exception_color = exc_color;
	context["exception_color"]->setFloat(exception_color.x(), exception_color.y(), exception_color.z());
}