#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"


RoughDiffuseMaterial::RoughDiffuseMaterial(optix::Context c)
{
	context = c;
	type = ROUGH_DIFFUSE_SHADER;
	rho_d = optix::make_float3(1.0f, 1.0f, 1.0f);
	ior = 1.3f;
	roughness = optix::make_float2(0.1f, 0.1f);
	n_distribution = BECKMANN_DISTRIBUTION;
	m_model = WALTER_MODEL;
	shader_name = QString::fromStdString("rough_diffuse_shader.cu");
	initTable();
	initPrograms();
	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(rho_d);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(ior);
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
	mtl->declareVariable("microfacet_model");
	mtl["microfacet_model"]->setUint(m_model);
};

RoughDiffuseMaterial::RoughDiffuseMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = ROUGH_DIFFUSE_SHADER;
	rho_d = optix::make_float3(1.0f, 1.0f, 1.0f);
	ior = 1.3f;
	roughness = optix::make_float2(0.1f, 0.1f);
	n_distribution = BECKMANN_DISTRIBUTION;
	m_model = WALTER_MODEL;
	shader_name = QString::fromStdString("rough_diffuse_shader.cu");
	loadFromJSON(json);
};

void RoughDiffuseMaterial::initTable()
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
	table->setItem(roughness_row, 0, new QTableWidgetItem(tr("roughness_x")));
	table->item(roughness_row, 0)->setFlags(table->item(roughness_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(roughness_row, 1, new QTableWidgetItem(QString::number(roughness.x)));

	table->setItem(roughness_row, 2, new QTableWidgetItem(tr("roughness_y")));
	table->item(roughness_row, 2)->setFlags(table->item(roughness_row, 2)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(roughness_row, 3, new QTableWidgetItem(QString::number(roughness.y)));

	table->setItem(n_dist_row, 0, new QTableWidgetItem(tr("normals_distribution")));
	table->item(n_dist_row, 0)->setFlags(table->item(n_dist_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *normalsComboBox = new QComboBox();
	for (int idx = 0; idx < NUMBER_OF_NORMALS_DISTRIBUTION; idx++) {
		normalsComboBox->addItem(normalDistributionNames[idx]);
	}
	normalsComboBox->setCurrentIndex(n_distribution);
	QObject::connect(normalsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateNormalsDistributionFromTable(int)));
	table->setCellWidget(n_dist_row, 1, normalsComboBox);

	table->setItem(m_model_row, 0, new QTableWidgetItem(tr("microfacet_model")));
	table->item(m_model_row, 0)->setFlags(table->item(m_model_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *microfacetComboBox = new QComboBox();
	for (int idx = 0; idx < NUMBER_OF_MICROFACET_MODELS; idx++) {
		microfacetComboBox->addItem(microfacetModelNames[idx]);
	}
	microfacetComboBox->setCurrentIndex(m_model);
	QObject::connect(microfacetComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMicrofacetModelFromTable(int)));
	table->setCellWidget(m_model_row, 1, microfacetComboBox);

	QObject::connect(table, &QTableWidget::cellChanged, this, &RoughDiffuseMaterial::tableUpdate);
};

void RoughDiffuseMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "rough_depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);
};



void RoughDiffuseMaterial::loadFromJSON(const QJsonObject &json)
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
	if (parameters.contains("roughness") && parameters["roughness"].isArray()) {
		QJsonArray tmp = parameters["roughness"].toArray();
		roughness = optix::make_float2(tmp[0].toDouble(), tmp[1].toDouble());
	}
	if (parameters.contains("normal_distribution") && parameters["normal_distribution"].isDouble()) {
		n_distribution = static_cast<NormalsDistribution>(parameters["normal_distribution"].toInt());
	}
	if (parameters.contains("microfacet_model") && parameters["microfacet_model"].isDouble()) {
		m_model = static_cast<MicrofacetModel> (parameters["microfacet_model"].toInt());

	}
	initTable();
	initPrograms();

	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(rho_d);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(ior);
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
	mtl->declareVariable("microfacet_model");
	mtl["microfacet_model"]->setUint(m_model);
}

void RoughDiffuseMaterial::setColor(optix::float3 c)
{ 
	rho_d = c;
	mtl["diffuse_color"]->setFloat(rho_d);

};

void RoughDiffuseMaterial::setIor(float i)
{
	ior = i;
	mtl["ior"]->setFloat(ior);

};

void RoughDiffuseMaterial::setRoughness(optix::float2 r)
{
	roughness = r;
	mtl["roughness"]->setFloat(roughness);
}


void RoughDiffuseMaterial::setNormalDistribution(NormalsDistribution n)
{
	n_distribution = n;
	mtl["normal_distribution"]->setUint(n_distribution);
	table->cellChanged(n_dist_row, 1);
}


void RoughDiffuseMaterial::setMicrofacetModel(MicrofacetModel m)
{
	m_model = m;
	mtl["microfacet_model"]->setUint(m_model);
	table->cellChanged(m_model_row, 1);
}

void RoughDiffuseMaterial::tableUpdate(int row, int column)
{
	if (row == rho_d_row)
		updateColorFromTable(row);
	else if (row == ior_row)
		updateIORFromTable(row);
	else if (row == roughness_row)
		updateRoughnessFromTable(row);
}

void RoughDiffuseMaterial::updateIORFromTable(int row)
{
	float i = table->item(row, 1)->text().toFloat();
	setIor(i);
}


void RoughDiffuseMaterial::updateColorFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setColor(optix::make_float3(x, y, z));
}

void RoughDiffuseMaterial::updateRoughnessFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 3)->text().toFloat();
	setRoughness(optix::make_float2(x, y));
}

void RoughDiffuseMaterial::updateNormalsDistributionFromTable(int new_value)
{
	setNormalDistribution(static_cast<NormalsDistribution> (new_value));
}

void RoughDiffuseMaterial::updateMicrofacetModelFromTable(int new_value)
{
	setMicrofacetModel(static_cast<MicrofacetModel> (new_value));
}

void RoughDiffuseMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QJsonObject parameters;
	parameters["diffuse_color"] = QJsonArray{ rho_d.x, rho_d.y, rho_d.z };
	parameters["ior"] = ior;
	parameters["roughness"] = QJsonArray{ roughness.x, roughness.y };
	parameters["normal_distribution"] = n_distribution;
	parameters["microfacet_model"] = m_model;
	json["mtl_parameters"] = parameters;
}