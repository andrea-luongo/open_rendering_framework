#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"


TransparentMaterial::TransparentMaterial(optix::Context c)
{
	context = c;
	type = TRANSPARENT_SHADER;
	index_of_refraction = 1.3f;
	glass_absorption = optix::make_float3(0.0f, 0.0f, 0.0f);
	shader_name = QString::fromStdString("transparent_shader.cu");
	initTable();
	initPrograms();
	mtl->declareVariable("glass_absorption");
	mtl["glass_absorption"]->setFloat(glass_absorption);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(index_of_refraction);
};

TransparentMaterial::TransparentMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = TRANSPARENT_SHADER;
	shader_name = QString::fromStdString("transparent_shader.cu");
	loadFromJSON(json);
};

void TransparentMaterial::initTable()
{
	table = new QTableWidget(table_rows, 4);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);
	table->setItem(absoprtion_row, 0, new QTableWidgetItem(tr("glass_absorption")));
	table->item(absoprtion_row, 0)->setFlags(table->item(absoprtion_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(absoprtion_row, 1, new QTableWidgetItem(QString::number(glass_absorption.x)));
	table->setItem(absoprtion_row, 2, new QTableWidgetItem(QString::number(glass_absorption.y)));
	table->setItem(absoprtion_row, 3, new QTableWidgetItem(QString::number(glass_absorption.z)));
	table->setItem(ior_row, 0, new QTableWidgetItem(tr("index_of_refraction")));
	table->item(ior_row, 0)->setFlags(table->item(ior_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(ior_row, 1, new QTableWidgetItem(QString::number(index_of_refraction)));

	QObject::connect(table, &QTableWidget::cellChanged, this, &TransparentMaterial::tableUpdate);
};

void TransparentMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);

};

void TransparentMaterial::tableUpdate(int row, int column)
{

	if (row == 0)
		updateAbsorptionFromTable(row);
	else if (row == 1)
		updateIORFromTable(row);
}

void TransparentMaterial::updateAbsorptionFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setAbsorption(optix::make_float3(x, y, z));
}

void TransparentMaterial::updateIORFromTable(int row)
{
	float ior = table->item(row, 1)->text().toFloat();
	setIor(ior);
}

void TransparentMaterial::loadFromJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("mtl_parameters") && json["mtl_parameters"].isObject()) {
		parameters = json["mtl_parameters"].toObject();
	}

	if (parameters.contains("glass_absorption") && parameters["glass_absorption"].isArray()) {
		QJsonArray tmp = parameters["glass_absorption"].toArray();
		glass_absorption = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	if (parameters.contains("ior") && parameters["ior"].isDouble()) {
		index_of_refraction = parameters["ior"].toDouble();
	}

	initTable();
	initPrograms();

	mtl->declareVariable("glass_absorption");
	mtl["glass_absorption"]->setFloat(glass_absorption);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(index_of_refraction);
}

void TransparentMaterial::setAbsorption(optix::float3 c)
{ 
	glass_absorption = c;
	mtl["glass_absorption"]->setFloat(glass_absorption);

};

void TransparentMaterial::setIor(float ior)
{
	index_of_refraction = ior;
	mtl["ior"]->setFloat(index_of_refraction);
};

void TransparentMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QJsonObject parameters;
	parameters["glass_absorption"] = QJsonArray{ glass_absorption.x, glass_absorption.y, glass_absorption.z };
	parameters["ior"] = index_of_refraction;
	json["mtl_parameters"] = parameters;
}
