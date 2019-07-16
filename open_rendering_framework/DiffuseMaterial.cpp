#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"


DiffuseMaterial::DiffuseMaterial(optix::Context c)
{
	context = c;
	type = DIFFUSE_SHADER;
	color = optix::make_float3(1.0f, 1.0f, 1.0f);
	shader_name = QString::fromStdString("diffuse_shader.cu");
	initTable();
	initPrograms();
	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(color);
};

DiffuseMaterial::DiffuseMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = DIFFUSE_SHADER;
	shader_name = QString::fromStdString("diffuse_shader.cu");
	loadFromJSON(json);
};

void DiffuseMaterial::initTable()
{
	table = new QTableWidget(table_rows, 4);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);
	table->setItem(color_row, 0, new QTableWidgetItem(tr("diffuse_color")));
	table->item(color_row, 0)->setFlags(table->item(color_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(color_row, 1, new QTableWidgetItem(QString::number(color.x)));
	table->setItem(color_row, 2, new QTableWidgetItem(QString::number(color.y)));
	table->setItem(color_row, 3, new QTableWidgetItem(QString::number(color.z)));
	QObject::connect(table, &QTableWidget::cellChanged, this, &DiffuseMaterial::tableUpdate);
};

void DiffuseMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);
};

void DiffuseMaterial::tableUpdate(int row, int column)
{

	if (row == 0)
		updateColorFromTable(row);
}

void DiffuseMaterial::updateColorFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setColor(optix::make_float3(x, y, z));
}

void DiffuseMaterial::loadFromJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("mtl_parameters") && json["mtl_parameters"].isObject()) {
		parameters = json["mtl_parameters"].toObject();
	}

	if (parameters.contains("diffuse_color") && parameters["diffuse_color"].isArray()) {
		QJsonArray tmp = parameters["diffuse_color"].toArray();
		color = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	initTable();
	initPrograms();

	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(color);
}

void DiffuseMaterial::setColor(optix::float3 c)
{ 
	color = c; 
	mtl["diffuse_color"]->setFloat(color); 

};

void DiffuseMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QJsonObject parameters;
	parameters["diffuse_color"] = QJsonArray{ color.x, color.y, color.z };
	json["mtl_parameters"] = parameters;
}