#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"
#include "Texture.h"


AnisotropicMaterial::AnisotropicMaterial(optix::Context c)
{
	context = c;
	type = ANISOTROPIC_MATERIAL;
	structure_type = RIDGED_STRUCTURE;
	texture_path = QString();
	sampler = loadTexture(texture_path, context);
	pattern_angle = 0.0f;
	ior = 1.3f;
	roughness = optix::make_float2(0.001f);
	sinusoid_amplitude = 1.0f;
	ridge_angle = 5.0f;
	sinusoid_wavelengths = optix::make_float2(1.0f);
	rho_d = make_float3(1.0f);
	shader_name = QString::fromStdString("anisotropic_structure_shader.cu");
	initTable();
	initPrograms();
	mtl->declareVariable("structure_type");
	mtl["structure_type"]->setUint(structure_type);
	mtl->declareVariable("texture_sampler");
	mtl["texture_sampler"]->setTextureSampler(sampler);
	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(rho_d);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(ior);
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("ridge_angle");
	mtl["ridge_angle"]->setFloat(ridge_angle);
	mtl->declareVariable("sinusoid_wavelengths");
	mtl["sinusoid_wavelengths"]->setFloat(sinusoid_wavelengths);
	mtl->declareVariable("sinusoid_amplitude");
	mtl["sinusoid_amplitude"]->setFloat(sinusoid_amplitude);
	mtl->declareVariable("pattern_angle");
	mtl["pattern_angle"]->setFloat(pattern_angle);
};

AnisotropicMaterial::AnisotropicMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = ANISOTROPIC_MATERIAL;
	structure_type = RIDGED_STRUCTURE;
	texture_path = QString();
	sampler = loadTexture(texture_path, context);
	pattern_angle = 0.0f;
	sinusoid_amplitude = 1.0f;
	ior = 1.3f;
	roughness = optix::make_float2(0.001f);
	ridge_angle = 5.0f;
	sinusoid_wavelengths = optix::make_float2(1.0f);
	rho_d = make_float3(1.0f);
	shader_name = QString::fromStdString("anisotropic_structure_shader.cu");
	loadFromJSON(json);
};


void AnisotropicMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);
};

void AnisotropicMaterial::loadFromJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("mtl_parameters") && json["mtl_parameters"].isObject()) {
		parameters = json["mtl_parameters"].toObject();
	}

	if (parameters.contains("structure_type") && parameters["structure_type"].isDouble()) {
		structure_type = static_cast<AnisotropicStructure>(parameters["structure_type"].toInt());
	}

	if (parameters.contains("texture") && parameters["texture"].isString()) {
		QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
		QFileInfo fileInfo(dir, parameters["texture"].toString());
		texture_path = (fileInfo.absoluteFilePath());
		//std::cerr << "objpath( '" << path.toStdString() << "' ) failed to load file: " << '\n';
	}
	if (parameters.contains("diffuse_color") && parameters["diffuse_color"].isArray()) {
		QJsonArray tmp = parameters["diffuse_color"].toArray();
		rho_d = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}

	if (parameters.contains("ior") && parameters["ior"].isDouble()) {
		ior = parameters["ior"].toDouble();
	}

	if (parameters.contains("ridge_angle") && parameters["ridge_angle"].isDouble()) {
		ridge_angle = parameters["ridge_angle"].toDouble();
	}
	if (parameters.contains("roughness") && parameters["roughness"].isArray()) {
		QJsonArray tmp = parameters["roughness"].toArray();
		roughness = optix::make_float2(tmp[0].toDouble(), tmp[1].toDouble());
	}
	if (parameters.contains("sinusoid_wavelengths") && parameters["sinusoid_wavelengths"].isArray()) {
		QJsonArray tmp = parameters["sinusoid_wavelengths"].toArray();
		sinusoid_wavelengths = optix::make_float2(tmp[0].toDouble(), tmp[1].toDouble());
	}
	if (parameters.contains("pattern_angle") && parameters["pattern_angle"].isDouble()) {
		pattern_angle = parameters["pattern_angle"].toDouble();
	}
	if (parameters.contains("sinusoid_amplitude") && parameters["sinusoid_amplitude"].isDouble()) {
		sinusoid_amplitude = parameters["sinusoid_amplitude"].toDouble();
	}
	sampler = loadTexture(texture_path, context);
	initTable();
	initPrograms();
	mtl->declareVariable("structure_type");
	mtl["structure_type"]->setUint(structure_type);
	mtl->declareVariable("texture_sampler");
	mtl["texture_sampler"]->setTextureSampler(sampler);
	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(rho_d);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(ior);
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("ridge_angle");
	mtl["ridge_angle"]->setFloat(ridge_angle);
	mtl->declareVariable("sinusoid_wavelengths");
	mtl["sinusoid_wavelengths"]->setFloat(sinusoid_wavelengths);
	mtl->declareVariable("pattern_angle");
	mtl["pattern_angle"]->setFloat(pattern_angle);
	mtl->declareVariable("sinusoid_amplitude");
	mtl["sinusoid_amplitude"]->setFloat(sinusoid_amplitude);
}



void AnisotropicMaterial::initTable()
{
	table = new QTableWidget(table_rows, 4);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);
	table->setItem(structure_row, 0, new QTableWidgetItem(tr("structure_type")));
	table->item(structure_row, 0)->setFlags(table->item(structure_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *structureComboBox = new QComboBox();
	for (int idx = 0; idx < NUMBER_OF_STRUCTURES; idx++) {
		structureComboBox->addItem(anisotropicStructureNames[idx]);
	}
	structureComboBox->setCurrentIndex(structure_type);
	QObject::connect(structureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTypeFromTable(int)));
	table->setCellWidget(structure_row, 1, structureComboBox);
	table->setItem(texture_row, 0, new QTableWidgetItem(tr("texture_path")));
	table->item(texture_row, 0)->setFlags(table->item(texture_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(texture_row, 1, new QTableWidgetItem(texture_path));
	table->item(texture_row, 1)->setFlags(table->item(texture_row, 1)->flags() &  ~Qt::ItemIsEditable);
	QPushButton *pathButton = new QPushButton("...");
	pathButton->setObjectName("change_path_button");
	QObject::connect(pathButton, &QPushButton::released, this, &AnisotropicMaterial::updateTextureFromTable);
	table->setCellWidget(texture_row, 2, pathButton);
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
	table->setItem(ridge_angle_row, 0, new QTableWidgetItem(tr("ridge_angle")));
	table->item(ridge_angle_row, 0)->setFlags(table->item(ridge_angle_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(ridge_angle_row, 1, new QTableWidgetItem(QString::number(ridge_angle)));
	table->setItem(sinusoid_amplitude_row, 0, new QTableWidgetItem(tr("sinusoid_amplitude")));
	table->item(sinusoid_amplitude_row, 0)->setFlags(table->item(sinusoid_amplitude_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(sinusoid_amplitude_row, 1, new QTableWidgetItem(QString::number(sinusoid_amplitude)));
	table->setItem(sinusoid_wavelengths_row, 0, new QTableWidgetItem(tr("sinusoid_wavelengths")));
	table->item(sinusoid_wavelengths_row, 0)->setFlags(table->item(sinusoid_wavelengths_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(sinusoid_wavelengths_row, 1, new QTableWidgetItem(QString::number(sinusoid_wavelengths.x)));
	table->setItem(sinusoid_wavelengths_row, 2, new QTableWidgetItem(QString::number(sinusoid_wavelengths.y)));
	table->setItem(pattern_angle_row, 0, new QTableWidgetItem(tr("pattern_angle")));
	table->item(pattern_angle_row, 0)->setFlags(table->item(pattern_angle_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(pattern_angle_row, 1, new QTableWidgetItem(QString::number(pattern_angle)));
	QObject::connect(table, &QTableWidget::cellChanged, this, &AnisotropicMaterial::tableUpdate);
};


void AnisotropicMaterial::tableUpdate(int row, int column)
{
	if (row == rho_d_row)
		updateColorFromTable(row);
	else if (row == ior_row)
		updateIORFromTable(row);
	else if (row == roughness_row)
		updateRoughnessFromTable(row);
	else if (row == ridge_angle_row)
		updateRidgeAngleFromTable(row);
	else if (row == sinusoid_amplitude_row)
		updateSinusoidAmplitudeFromTable(row);
	else if (row == sinusoid_wavelengths_row)
		updateSinusoidWavelengthsFromTable(row);
	else if (row == pattern_angle_row)
		updatePatternAngleFromTable(row);
}

void AnisotropicMaterial::setType(AnisotropicStructure type)
{
	structure_type = type;
	mtl["structure_type"]->setUint(structure_type);
	table->cellChanged(structure_row, 1);
}

void AnisotropicMaterial::updateTypeFromTable(int type)
{
	setType(static_cast<AnisotropicStructure> (type));
}

void AnisotropicMaterial::setIor(float i)
{
	ior = i;
	mtl["ior"]->setFloat(ior);
}


void AnisotropicMaterial::updateIORFromTable(int row)
{
	float i = table->item(row, 1)->text().toFloat();
	setIor(i);
}

void AnisotropicMaterial::setPatternAngle(float angle)
{
	pattern_angle = angle;
	mtl["pattern_angle"]->setFloat(pattern_angle);
}

void AnisotropicMaterial::updatePatternAngleFromTable(int row)
{
	float angle = table->item(row, 1)->text().toFloat();
	setPatternAngle(angle);
}

void AnisotropicMaterial::setRoughness(optix::float2 r)
{
	roughness = r;
	mtl["roughness"]->setFloat(roughness);
}

void AnisotropicMaterial::updateRoughnessFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 3)->text().toFloat();
	setRoughness(optix::make_float2(x, y));
}

void AnisotropicMaterial::setRidgeAngle(float angle)
{
	ridge_angle = angle;
	mtl["ridge_angle"]->setFloat(ridge_angle);
}

void AnisotropicMaterial::updateRidgeAngleFromTable(int row)
{
	float a = table->item(row, 1)->text().toFloat();
	setRidgeAngle(a);
}

void AnisotropicMaterial::setColor(optix::float3 c)
{
	rho_d = c;
	mtl["diffuse_color"]->setFloat(rho_d);

};

void AnisotropicMaterial::updateColorFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setColor(optix::make_float3(x, y, z));
}

void AnisotropicMaterial::setSinusoidAmplitude(float a)
{
	sinusoid_amplitude = a;
	mtl["sinusoid_amplitude"]->setFloat(sinusoid_amplitude);
}

void AnisotropicMaterial::updateSinusoidAmplitudeFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	setSinusoidAmplitude(x);
}


void AnisotropicMaterial::setSinusoidWavelengths(optix::float2 f)
{
	sinusoid_wavelengths = f;
	mtl["sinusoid_wavelengths"]->setFloat(sinusoid_wavelengths);
}

void AnisotropicMaterial::updateSinusoidWavelengthsFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	setSinusoidWavelengths(optix::make_float2(x, y));
}


void AnisotropicMaterial::setTexture(QString t)
{
	QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
	QFileInfo fileInfo(dir, t);
	texture_path = (fileInfo.absoluteFilePath());
	table->setItem(texture_row, 1, new QTableWidgetItem(fileInfo.fileName()));
	table->item(texture_row, 1)->setFlags(table->item(texture_row, 1)->flags() &  ~Qt::ItemIsEditable);
	table->cellChanged(texture_row, 1);

	sampler = loadTexture(texture_path, context);
	mtl["texture_sampler"]->setTextureSampler(sampler);
}


void AnisotropicMaterial::updateTextureFromTable()
{
	QString path = QFileDialog::getOpenFileName(0, tr("Open file"), "../../data", tr("texture file (*.hdr *.ppm *.png *.jpg)"));
	if (path.isNull() || path.isEmpty()) {
		return;
	}
	//QDir dir("./");
	//path = dir.relativeFilePath(path);
	setTexture(path);
	
}

void AnisotropicMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QDir dir(SAMPLES_DIR + QString("/build/") + SAMPLE_NAME);
	QJsonObject parameters;
	parameters["structure_type"] = structure_type;
	parameters["texture"] = dir.relativeFilePath(texture_path);
	parameters["diffuse_color"] = QJsonArray{ rho_d.x, rho_d.y, rho_d.z };
	parameters["ior"] = ior;
	parameters["roughness"] = QJsonArray{ roughness.x, roughness.y };
	parameters["ridge_angle"] = ridge_angle;
	parameters["sinusoid_wavelengths"] = QJsonArray{ sinusoid_wavelengths.x, sinusoid_wavelengths.y };
	parameters["sinusoid_amplitude"] = sinusoid_amplitude;
	parameters["pattern_angle"] = pattern_angle;
	json["mtl_parameters"] = parameters;
}
