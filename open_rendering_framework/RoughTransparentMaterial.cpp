#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"


RoughTransparentMaterial::RoughTransparentMaterial(optix::Context c)
{
	context = c;
	type = ROUGH_TRANSPARENT_SHADER;
	index_of_refraction = 1.3f;
	glass_absorption = optix::make_float3(0.0f, 0.0f, 0.0f);
	roughness = 0.1f;
	m_model = WALTER_MODEL;
	n_distribution = BECKMANN_DISTRIBUTION;
	shader_name = QString::fromStdString("rough_transparent_shader.cu");
	initTable();
	initPrograms();
	mtl->declareVariable("glass_absorption");
	mtl["glass_absorption"]->setFloat(glass_absorption);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(index_of_refraction);
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("microfacet_model");
	mtl["microfacet_model"]->setUint(m_model);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
};

RoughTransparentMaterial::RoughTransparentMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = ROUGH_TRANSPARENT_SHADER;
	shader_name = QString::fromStdString("rough_transparent_shader.cu");
	loadFromJSON(json);
};

void RoughTransparentMaterial::initTable()
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
	table->setItem(roughness_row, 0, new QTableWidgetItem(tr("roughness")));
	table->item(roughness_row, 0)->setFlags(table->item(roughness_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(roughness_row, 1, new QTableWidgetItem(QString::number(roughness)));

	table->setItem(m_model_row, 0, new QTableWidgetItem(tr("microfacet_model")));
	table->item(m_model_row, 0)->setFlags(table->item(m_model_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *microfacetComboBox = new QComboBox();
	for (int idx = 0; idx < NUMBER_OF_MICROFACET_MODELS; idx++) {
		microfacetComboBox->addItem(microfacetModelNames[idx]);
	}
	microfacetComboBox->setCurrentIndex(m_model);
	QObject::connect(microfacetComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMicrofacetModelFromTable(int)));
	table->setCellWidget(m_model_row, 1, microfacetComboBox);

	table->setItem(n_dist_row, 0, new QTableWidgetItem(tr("normals_distribution")));
	table->item(n_dist_row, 0)->setFlags(table->item(n_dist_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *normalsComboBox= new QComboBox();
	for (int idx = 0; idx < NUMBER_OF_NORMALS_DISTRIBUTION; idx++) {
		normalsComboBox->addItem(normalDistributionNames[idx]);
	}
	normalsComboBox->setCurrentIndex(n_distribution);
	QObject::connect(normalsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateNormalsDistributionFromTable(int)));
	table->setCellWidget(n_dist_row, 1, normalsComboBox);

	QObject::connect(table, &QTableWidget::cellChanged, this, &RoughTransparentMaterial::tableUpdate);
};

void RoughTransparentMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);

};

void RoughTransparentMaterial::tableUpdate(int row, int column)
{

	if (row == 0)
		updateAbsorptionFromTable(row);
	else if (row == 1)
		updateIORFromTable(row);
	else if (row == 2)
		updateRoughnessFromTable(row);
}

void RoughTransparentMaterial::updateAbsorptionFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setAbsorption(optix::make_float3(x, y, z));
}

void RoughTransparentMaterial::updateIORFromTable(int row)
{
	float ior = table->item(row, 1)->text().toFloat();
	setIor(ior);
}

void RoughTransparentMaterial::updateRoughnessFromTable(int row)
{
	float r= table->item(row, 1)->text().toFloat();
	setRoughness(r);
}

void RoughTransparentMaterial::updateMicrofacetModelFromTable(int new_value)
{
	setMicrofacetModel(static_cast<MicrofacetModel> (new_value));
}

void RoughTransparentMaterial::updateNormalsDistributionFromTable(int new_value)
{
	setNormalDistribution(static_cast<NormalsDistribution> (new_value));
}

void RoughTransparentMaterial::loadFromJSON(const QJsonObject &json)
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
	if (parameters.contains("roughness") && parameters["roughness"].isDouble()) {
		roughness = parameters["roughness"].toDouble();
	}
	if (parameters.contains("microfacet_model") && parameters["microfacet_model"].isDouble()) {
		m_model = static_cast<MicrofacetModel> (parameters["microfacet_model"].toInt());

	}
	if (parameters.contains("normal_distribution") && parameters["normal_distribution"].isDouble()) {
		n_distribution = static_cast<NormalsDistribution>( parameters["normal_distribution"].toInt());
	}

	initTable();
	initPrograms();

	mtl->declareVariable("glass_absorption");
	mtl["glass_absorption"]->setFloat(glass_absorption);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(index_of_refraction);
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("microfacet_model");
	mtl["microfacet_model"]->setUint(m_model);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
}

void RoughTransparentMaterial::setAbsorption(optix::float3 c)
{ 
	glass_absorption = c;
	mtl["glass_absorption"]->setFloat(glass_absorption);

};

void RoughTransparentMaterial::setIor(float ior)
{
	index_of_refraction = ior;
	mtl["ior"]->setFloat(index_of_refraction);

};

void RoughTransparentMaterial::setRoughness(float r)
{
	roughness = r;
	mtl["roughness"]->setFloat(roughness);
}

void RoughTransparentMaterial::setMicrofacetModel(MicrofacetModel m)
{
	m_model = m;
	mtl["microfacet_model"]->setUint(m_model);
	table->cellChanged(3,1);
}

void RoughTransparentMaterial::setNormalDistribution(NormalsDistribution n)
{
	n_distribution = n;
	mtl["normal_distribution"]->setUint(n_distribution);
	table->cellChanged(4, 1);
}

void RoughTransparentMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QJsonObject parameters;
	parameters["glass_absorption"] = QJsonArray{ glass_absorption.x, glass_absorption.y, glass_absorption.z };
	parameters["ior"] = index_of_refraction;
	parameters["roughness"] = roughness;
	parameters["microfacet_model"] = m_model;
	parameters["normal_distribution"] = n_distribution;
	json["mtl_parameters"] = parameters;
}
