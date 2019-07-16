#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"
#include "Fresnel.h"

RoughTranslucentMaterial::RoughTranslucentMaterial(optix::Context c)
{
	context = c;
	type = ROUGH_TRANSLUCENT_SHADER;
	shader_name = QString::fromStdString("rough_subsurface_scattering_shader.cu");
	//concentration = 1.0f;
	scale = 100.0f;
	getDefaultMaterial(Apple);
	dipole_model = STANDARD_DIPOLE;
	roughness = optix::make_float2(0.1f, 0.1f);
	n_distribution = BECKMANN_DISTRIBUTION;
	m_model = VISIBLE_NORMALS_MODEL;
	initTable();
	initPrograms();
	loadParameters("scattering_properties");
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
	mtl->declareVariable("microfacet_model");
	mtl["microfacet_model"]->setUint(m_model);
};

RoughTranslucentMaterial::RoughTranslucentMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = ROUGH_TRANSLUCENT_SHADER;
	dipole_model = STANDARD_DIPOLE;
	current = Custom;
	roughness = optix::make_float2(0.1f, 0.1f);
	n_distribution = BECKMANN_DISTRIBUTION;
	m_model = VISIBLE_NORMALS_MODEL;
	shader_name = QString::fromStdString("rough_subsurface_scattering_shader.cu");
	loadFromJSON(json);

	loadParameters("scattering_properties");
};

void RoughTranslucentMaterial::initTable()
{
	table = new QTableWidget(table_rows, 4);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);

	table->setItem(def_material_row, 0, new QTableWidgetItem(tr("Material")));
	table->item(def_material_row, 0)->setFlags(table->item(def_material_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *materialComboBox = new QComboBox();
	materialComboBox->setObjectName("material_combo_box");
	for (int idx = 0; idx < NUMBER_OF_DEFAULT_SCATTERING_MATERIALS; idx++)
	{
		materialComboBox->addItem(DefaultScatteringMaterialNames[idx]);
	}
	materialComboBox->setCurrentIndex(current);
	QObject::connect(materialComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDefaultMaterial(int)));
	table->setCellWidget(def_material_row, 1, materialComboBox);

	table->setItem(ior_row, 0, new QTableWidgetItem(tr("ior")));
	table->item(ior_row, 0)->setFlags(table->item(ior_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(ior_row, 1, new QTableWidgetItem(QString::number(ior)));

	table->setItem(scale_row, 0, new QTableWidgetItem(tr("scale")));
	table->item(scale_row, 0)->setFlags(table->item(scale_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(scale_row, 1, new QTableWidgetItem(QString::number(scale)));

	table->setItem(absorption_row, 0, new QTableWidgetItem(tr("absorption")));
	table->item(absorption_row, 0)->setFlags(table->item(absorption_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(absorption_row, 1, new QTableWidgetItem(QString::number(absorption.x)));
	table->setItem(absorption_row, 2, new QTableWidgetItem(QString::number(absorption.y)));
	table->setItem(absorption_row, 3, new QTableWidgetItem(QString::number(absorption.z)));

	table->setItem(scattering_row, 0, new QTableWidgetItem(tr("scattering")));
	table->item(scattering_row, 0)->setFlags(table->item(scattering_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(scattering_row, 1, new QTableWidgetItem(QString::number(scattering.x)));
	table->setItem(scattering_row, 2, new QTableWidgetItem(QString::number(scattering.y)));
	table->setItem(scattering_row, 3, new QTableWidgetItem(QString::number(scattering.z)));

	table->setItem(meancosine_row, 0, new QTableWidgetItem(tr("meancosine")));
	table->item(meancosine_row, 0)->setFlags(table->item(meancosine_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(meancosine_row, 1, new QTableWidgetItem(QString::number(meancosine.x)));
	table->setItem(meancosine_row, 2, new QTableWidgetItem(QString::number(meancosine.y)));
	table->setItem(meancosine_row, 3, new QTableWidgetItem(QString::number(meancosine.z)));

	table->setItem(dipole_row, 0, new QTableWidgetItem(tr("dipole model")));
	table->item(dipole_row, 0)->setFlags(table->item(dipole_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *modelComboBox = new QComboBox();
	modelComboBox->addItem("standard dipole");
	modelComboBox->addItem("directional dipole");
	modelComboBox->setCurrentIndex(dipole_model);
	QObject::connect(modelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDipoleModel(int)));
	table->setCellWidget(dipole_row, 1, modelComboBox);
	
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

	QObject::connect(table, &QTableWidget::cellChanged, this, &RoughTranslucentMaterial::tableUpdate);
};

void RoughTranslucentMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "rough_depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);

};

void RoughTranslucentMaterial::tableUpdate(int row, int column)
{
	if (row == ior_row)
		updateIORFromTable(row);
	else if (row == scale_row)
		updateScaleFromTable(row);
	else if (row == absorption_row)
		updateAbsorptionFromTable(row);
	else if (row == scattering_row)
		updateScatteringFromTable(row);
	else if (row == meancosine_row)
		updateMeanCosineFromTable(row);
	else if (row == roughness_row)
		updateRoughnessFromTable(row);
}


void RoughTranslucentMaterial::loadFromJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("mtl_parameters") && json["mtl_parameters"].isObject()) {
		parameters = json["mtl_parameters"].toObject();
	}

	if (parameters.contains("ior") && parameters["ior"].isDouble()) {
		ior = parameters["ior"].toDouble();
	}
	if (parameters.contains("absorption") && parameters["absorption"].isArray())
	{
		QJsonArray tmp = parameters["absorption"].toArray();
		absorption = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}
	if (parameters.contains("scattering") && parameters["scattering"].isArray())
	{
		QJsonArray tmp = parameters["scattering"].toArray();
		scattering = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}
	if (parameters.contains("meancosine") && parameters["meancosine"].isArray())
	{
		QJsonArray tmp = parameters["meancosine"].toArray();
		meancosine = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}
	if (parameters.contains("scale") && parameters["scale"].isDouble()) {
		scale = parameters["scale"].toDouble();
	}
	if (parameters.contains("dipole_model") && parameters["dipole_model"].isDouble()) {
		dipole_model = static_cast<DipoleModel>(parameters["dipole_model"].toInt());
	}
	if (parameters.contains("material_type") && parameters["material_type"].isDouble()) {
		current = static_cast<DefaultScatteringMaterial>(parameters["material_type"].toInt());
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

	computeCoefficients();
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
	mtl->declareVariable("microfacet_model");
	mtl["microfacet_model"]->setUint(m_model);
	//current = Custom;
}

void RoughTranslucentMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QJsonObject parameters;
	parameters["ior"] = ior;
	parameters["scale"] = scale;
	//parameters["concentration"] = concentration;
	parameters["absorption"] = QJsonArray{ absorption.x, absorption.y, absorption.z};
	parameters["scattering"] = QJsonArray{ scattering.x, scattering.y, scattering.z };
	parameters["meancosine"] = QJsonArray{ meancosine.x, meancosine.y, meancosine.z };
	parameters["dipole_model"] = dipole_model;
	parameters["material_type"] = current;
	parameters["roughness"] = QJsonArray{ roughness.x, roughness.y };
	parameters["normal_distribution"] = n_distribution;
	parameters["microfacet_model"] = m_model;
	json["mtl_parameters"] = parameters;
}


void RoughTranslucentMaterial::getDefaultMaterial(DefaultScatteringMaterial material)
{
	current = material;
	//meancosine = optix::make_float3(0.0f);
	//absorption = optix::make_float3(0.0f);
	//scattering = optix::make_float3(0.0f);
	

	switch (material)
	{
	case Chicken:
		absorption = optix::make_float3(0.015f, 0.077f, 0.19f);
		scattering = optix::make_float3(0.15f, 0.21f, 0.38f);
		meancosine = optix::make_float3(0.0f, 0.0f, 0.0f);
		ior = 1.3f;
		break;
	case Skin:
		absorption = optix::make_float3(0.032f, 0.17f, 0.48f);
		scattering = optix::make_float3(0.74f, 0.88f, 1.01f);
		meancosine = optix::make_float3(0.0f, 0.0f, 0.0f);
		ior = 1.3f;
		break;
	case Wholemilk:
		absorption = optix::make_float3(0.0011f, 0.0024f, 0.014f);
		scattering = optix::make_float3(2.55f, 3.21f, 3.77f);
		meancosine = optix::make_float3(0.0f, 0.0f, 0.0f);
		ior = 1.3f;
		break;
	case Whitegrapefruit:
		absorption = optix::make_float3(0.096f, 0.131f, 0.395f);
		scattering = optix::make_float3(3.513f, 3.669f, 5.237f);
		meancosine = optix::make_float3(0.548f, 0.545f, 0.565f);
		ior = 1.3f;
		break;
	case Beer:
		absorption = optix::make_float3(0.1449f, 0.3141f, 0.7286f);
		scattering = optix::make_float3(0.0037f, 0.0069f, 0.0074f);
		meancosine = optix::make_float3(0.917f, 0.956f, 0.982f);
		ior = 1.3f;
		break;
	case Soymilk:
		absorption = optix::make_float3(0.0001f, 0.0005f, 0.0034f);
		scattering = optix::make_float3(2.433f, 2.714f, 4.563f);
		meancosine = optix::make_float3(0.873f, 0.858f, 0.832f);
		ior = 1.3f;
		break;
	case Coffee:
		absorption = optix::make_float3(0.1669f, 0.2287f, 0.3078f);
		scattering = optix::make_float3(0.2707f, 0.2828f, 0.297f);
		meancosine = optix::make_float3(0.907f, 0.896f, 0.88f);
		ior = 1.3f;
		break;
	case Marble:
		//ior_real = optix::make_float3(1.5f);
		ior = 1.5f;
		absorption = optix::make_float3(0.0021f, 0.0041f, 0.0071f);
		scattering = optix::make_float3(2.19f, 2.62f, 3.00f);
		meancosine = optix::make_float3(0.0f, 0.0f, 0.0f);
		ior = 1.3f;
		break;
	case Potato:
		absorption = optix::make_float3(0.0024f, 0.0090f, 0.12f);
		scattering = optix::make_float3(0.68f, 0.70f, 0.55f);
		meancosine = optix::make_float3(0.0f, 0.0f, 0.0f);
		ior = 1.3f;
		break;
	case Ketchup:
		absorption = optix::make_float3(0.061f, 0.97f, 1.45f);
		scattering = optix::make_float3(0.18f, 0.07f, 0.03f);
		meancosine = optix::make_float3(0.0f, 0.0f, 0.0f);
		ior = 1.3f;
		break;
	case Apple:
		absorption = optix::make_float3(0.0030f, 0.0034f, 0.0046f);
		scattering = optix::make_float3(2.29f, 2.39f, 1.97f);
		meancosine = optix::make_float3(0.0f, 0.0f, 0.0f);
		ior = 1.3f;
		break;
	case ChocolateMilk:
		scattering = optix::make_float3(0.7352f, 0.9142f, 1.0588f);
		absorption = optix::make_float3(0.7359f, 0.9172f, 1.0688f) - scattering;
		meancosine = optix::make_float3(0.862f, 0.838f, 0.806f);
		ior = 1.3f;
		break;
	case ReducedMilk:
		absorption = optix::make_float3(0.0001f, 0.0002f, 0.0005f);
		scattering = optix::make_float3(10.748f, 12.209f, 13.931f);
		meancosine = optix::make_float3(0.819f, 0.797f, 0.746f);
		ior = 1.3f;
		break;
	case Mustard:
		scattering = optix::make_float3(16.447f, 18.536f, 6.457f);
		absorption = optix::make_float3(0.057f, 0.061f, 0.451f);
		meancosine = optix::make_float3(0.155f, 0.173f, 0.351f);
		ior = 1.3f;
		break;
	case Shampoo:
		scattering = optix::make_float3(8.111f, 9.919f, 10.575f);
		absorption = optix::make_float3(0.178f, 0.328f, 0.439f);
		meancosine = optix::make_float3(0.907f, 0.882f, 0.874f);
		ior = 1.3f;
		break;
	case MixedSoap:
		scattering = optix::make_float3(3.923f, 4.018f, 4.351f);
		absorption = optix::make_float3(0.003f, 0.005f, 0.013f);
		meancosine = optix::make_float3(0.330f, 0.322f, 0.316f);
		ior = 1.3f;
		break;
	case GlycerineSoap:
		scattering = optix::make_float3(0.201f, 0.202f, 0.221f);
		absorption = optix::make_float3(0.001f, 0.001f, 0.002f);
		meancosine = optix::make_float3(0.955f, 0.949f, 0.943f);
		ior = 1.3f;
		break;
	case Custom:
		scattering = scattering;
		absorption = absorption;
		meancosine = meancosine;
		ior = ior;
		break;
	}
	computeCoefficients();
}

void RoughTranslucentMaterial::loadParameters(const char* name)
{
	mtl[name]->setUserData(sizeof(ScatteringMaterialProperties), &properties);
	mtl["dipole_model"]->setUint(dipole_model);
}

void RoughTranslucentMaterial::computeCoefficients(optix::float3 ior_outside)
{
	properties.absorption = absorption * scale;
	properties.scattering = scattering * scale;
	properties.meancosine = meancosine;
	properties.relative_ior = ior;
	properties.ior_real = optix::make_float3(ior);
	optix::float3 ior_rgb = properties.ior_real / ior_outside;
	optix::float3 reducedScattering = properties.scattering*(1.0f - properties.meancosine);
	properties.relative_ior /= (ior_outside.x + ior_outside.y + ior_outside.z) / 3.0f;
	properties.ior_real = ior_rgb;
	properties.extinction = properties.scattering + properties.absorption;
	properties.reducedExtinction = reducedScattering + properties.absorption;
	properties.deltaEddExtinction = properties.scattering*(1.0f - properties.meancosine*properties.meancosine) + properties.absorption;
	properties.D = optix::make_float3(1.0f) / (3.0f*properties.reducedExtinction);
	properties.transport = sqrtf(properties.absorption / properties.D);
	properties.C_phi = optix::make_float3(C_phi(ior_rgb.x), C_phi(ior_rgb.y), C_phi(ior_rgb.z));
	properties.C_phi_inv = optix::make_float3(C_phi(1.0f / ior_rgb.x), C_phi(1.0f / ior_rgb.y), C_phi(1.0f / ior_rgb.z));
	properties.C_E = optix::make_float3(C_E(ior_rgb.x), C_E(ior_rgb.y), C_E(ior_rgb.z));
	properties.albedo = properties.scattering / properties.extinction;
	properties.reducedAlbedo = reducedScattering / properties.reducedExtinction;
	properties.de = 2.131f*properties.D / sqrtf(properties.reducedAlbedo);
	properties.A = (1.0f - properties.C_E) / (2.0f*properties.C_phi);
	properties.three_D = 3.0f*properties.D;
	properties.two_A_de = 2.0f*properties.A*properties.de;
	properties.global_coeff = optix::make_float3(1.0f) / (4.0f*properties.C_phi_inv) * 1.0f / (4.0f*M_PIf*M_PIf);
	properties.one_over_three_ext = optix::make_float3(1.0) / (3.0f*properties.extinction);
	properties.mean_transport = (properties.transport.x + properties.transport.y + properties.transport.z) / 3.0f;
	//properties.min_transport = fminf(fminf(properties.transport.x, properties.transport.y), properties.transport.z);
}

void RoughTranslucentMaterial::updateIORFromTable(int row)
{
	float i = table->item(row, 1)->text().toFloat();
	setIor(i);
	table->findChild<QComboBox*>("material_combo_box")->setCurrentIndex(NUMBER_OF_DEFAULT_SCATTERING_MATERIALS-1);
}

void RoughTranslucentMaterial::updateScaleFromTable(int row)
{
	float s = table->item(row, 1)->text().toFloat();
	setScale(s);
}

void RoughTranslucentMaterial::updateAbsorptionFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setAbsorption(optix::make_float3(x, y,z));
	table->findChild<QComboBox*>("material_combo_box")->setCurrentIndex(NUMBER_OF_DEFAULT_SCATTERING_MATERIALS-1);
}

void RoughTranslucentMaterial::updateScatteringFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setScattering(optix::make_float3(x, y, z));
	table->findChild<QComboBox*>("material_combo_box")->setCurrentIndex(NUMBER_OF_DEFAULT_SCATTERING_MATERIALS-1);
}

void RoughTranslucentMaterial::updateMeanCosineFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setMeanCosine(optix::make_float3(x, y, z));
	table->findChild<QComboBox*>("material_combo_box")->setCurrentIndex(NUMBER_OF_DEFAULT_SCATTERING_MATERIALS-1);
}


void RoughTranslucentMaterial::updateNormalsDistributionFromTable(int new_value)
{
	setNormalDistribution(static_cast<NormalsDistribution> (new_value));
}

void RoughTranslucentMaterial::updateMicrofacetModelFromTable(int new_value)
{
	setMicrofacetModel(static_cast<MicrofacetModel> (new_value));
}

void RoughTranslucentMaterial::updateRoughnessFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 3)->text().toFloat();
	setRoughness(optix::make_float2(x, y));
}

void RoughTranslucentMaterial::setIor(float i)
{
	ior = i;
	computeCoefficients();
	loadParameters("scattering_properties");
}

void RoughTranslucentMaterial::setScale(float s)
{
	scale = s;
	computeCoefficients();
	loadParameters("scattering_properties");
}

void RoughTranslucentMaterial::setAbsorption(optix::float3 a)
{
	absorption = a;
	computeCoefficients();
	loadParameters("scattering_properties");
}

void RoughTranslucentMaterial::setScattering(optix::float3 s)
{
	scattering = s;
	computeCoefficients();
	loadParameters("scattering_properties");
}

void RoughTranslucentMaterial::setMeanCosine(optix::float3 m)
{
	meancosine = m;
	computeCoefficients();
	loadParameters("scattering_properties");
}

void RoughTranslucentMaterial::setNormalDistribution(NormalsDistribution n)
{
	n_distribution = n;
	mtl["normal_distribution"]->setUint(n_distribution);
	table->cellChanged(n_dist_row, 1);
}


void RoughTranslucentMaterial::setMicrofacetModel(MicrofacetModel m)
{
	m_model = m;
	mtl["microfacet_model"]->setUint(m_model);
	table->cellChanged(m_model_row, 1);
}

void RoughTranslucentMaterial::setRoughness(optix::float2 r)
{
	roughness = r;
	mtl["roughness"]->setFloat(roughness);
}

void RoughTranslucentMaterial::updateDefaultMaterial(int material)
{
	getDefaultMaterial(static_cast<DefaultScatteringMaterial>(material));
	loadParameters("scattering_properties");

	table->blockSignals(true);
	table->item(1, 1)->setText(QString::number(ior));
	table->item(2, 1)->setText(QString::number(scale));
	table->item(3, 1)->setText(QString::number(absorption.x));
	table->item(3, 2)->setText(QString::number(absorption.y));
	table->item(3, 3)->setText(QString::number(absorption.z));
	table->item(4, 1)->setText(QString::number(scattering.x));
	table->item(4, 2)->setText(QString::number(scattering.y));
	table->item(4, 3)->setText(QString::number(scattering.z));
	table->item(5, 1)->setText(QString::number(meancosine.x));
	table->item(5, 2)->setText(QString::number(meancosine.y));
	table->item(5, 3)->setText(QString::number(meancosine.z));
	table->blockSignals(false);
	table->cellChanged(0, 1);
}

void RoughTranslucentMaterial::updateDipoleModel(int model)
{
	dipole_model = static_cast<DipoleModel>(model);
	loadParameters("scattering_properties");
	table->cellChanged(6, 1);
}