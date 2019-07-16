#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"


LambertianInterfaceMaterial::LambertianInterfaceMaterial(optix::Context c)
{
	context = c;
	type = LAMBERTIAN_INTERFACE_SHADER;
	rho_d = optix::make_float3(1.0f, 1.0f, 1.0f);
	ior = 1.3f;
	shader_name = QString::fromStdString("lambertian_interface_shader.cu");
	initTable();
	initPrograms();
	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(rho_d);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(ior);
};

LambertianInterfaceMaterial::LambertianInterfaceMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = LAMBERTIAN_INTERFACE_SHADER;
	shader_name = QString::fromStdString("lambertian_interface_shader.cu");
	loadFromJSON(json);
};

void LambertianInterfaceMaterial::initTable()
{
	table = new QTableWidget(table_rows, 4);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);
	table->setItem(rho_d_row, 0, new QTableWidgetItem(tr("diffuse_color")));
	table->item(rho_d_row, 0)->setFlags(table->item(rho_d_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(rho_d_row, 1, new QTableWidgetItem(QString::number(rho_d.x)));
	table->setItem(rho_d_row, 2, new QTableWidgetItem(QString::number(rho_d.y)));
	table->setItem(rho_d_row, 3, new QTableWidgetItem(QString::number(rho_d.z)));
	table->setItem(ior_row, 0, new QTableWidgetItem(tr("index_of_refraction")));
	table->item(ior_row, 0)->setFlags(table->item(ior_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(ior_row, 1, new QTableWidgetItem(QString::number(ior)));
	QObject::connect(table, &QTableWidget::cellChanged, this, &LambertianInterfaceMaterial::tableUpdate);
};

void LambertianInterfaceMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);
};

void LambertianInterfaceMaterial::tableUpdate(int row, int column)
{
	if (row == rho_d_row)
		updateColorFromTable(row);
	else if (row == ior_row)
		updateIORFromTable(row);
}

void LambertianInterfaceMaterial::updateColorFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setColor(optix::make_float3(x, y, z));
}

void LambertianInterfaceMaterial::loadFromJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("mtl_parameters") && json["mtl_parameters"].isObject()) {
		parameters = json["mtl_parameters"].toObject();
	}

	if (parameters.contains("diffuse_color") && parameters["diffuse_color"].isArray()) {
		QJsonArray tmp = parameters["diffuse_color"].toArray();
		rho_d = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	if (parameters.contains("ior") && parameters["ior"].isDouble()) {
		ior = parameters["ior"].toDouble();
	}

	initTable();
	initPrograms();

	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(rho_d);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(ior);
}

void LambertianInterfaceMaterial::setColor(optix::float3 c)
{ 
	rho_d = c;
	mtl["diffuse_color"]->setFloat(rho_d);

};

void LambertianInterfaceMaterial::setIor(float i)
{
	ior = i;
	mtl["ior"]->setFloat(ior);

};

void LambertianInterfaceMaterial::updateIORFromTable(int row)
{
	float i = table->item(row, 1)->text().toFloat();
	setIor(i);
}

void LambertianInterfaceMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QJsonObject parameters;
	parameters["diffuse_color"] = QJsonArray{ rho_d.x, rho_d.y, rho_d.z };
	parameters["ior"] = ior;
	json["mtl_parameters"] = parameters;
}