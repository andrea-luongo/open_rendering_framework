#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"


NormalMaterial::NormalMaterial(optix::Context c)
{
	context = c;
	type = NORMAL_SHADER;
	shader_name = QString::fromStdString("normal_shader.cu");
	initTable();
	initPrograms();
};

NormalMaterial::NormalMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = NORMAL_SHADER;
	shader_name = QString::fromStdString("normal_shader.cu");
	loadFromJSON(json);
};

void NormalMaterial::initTable()
{
	table = new QTableWidget(0, 0);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);

	QObject::connect(table, &QTableWidget::cellChanged, this, &NormalMaterial::tableUpdate);
};

void NormalMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);

};

void NormalMaterial::tableUpdate(int row, int column)
{

}


void NormalMaterial::loadFromJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("mtl_parameters") && json["mtl_parameters"].isObject()) {
		parameters = json["mtl_parameters"].toObject();
	}
	initTable();
	initPrograms();
}

void NormalMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QJsonObject parameters;
	json["mtl_parameters"] = parameters;
	//parameters["path"] = path;
	//parameters["position"] = QJsonArray{ position.x, position.y, position.z };
	//parameters["scale"] = QJsonArray{ scale.x, scale.y, scale.z };
	//parameters["rotation"] = QJsonArray{ angle_deg, rotation_axis.x, rotation_axis.y, rotation_axis.z };
	//parameters["x_rotation"] = x_rot;
	//parameters["y_rotation"] = y_rot;
	//parameters["z_rotation"] = z_rot;
	////if (parameters.contains("material") && parameters["material"].isObject()) {
	////	loadMaterialFromJSON(parameters["material"].toObject());
	////}
	
}
