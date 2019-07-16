#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"


FlatMaterial::FlatMaterial(optix::Context c)
{
	context = c;
	type = FLAT_SHADER;
	color = optix::make_float3(1.0f, 1.0f, 1.0f);
	flat_shadow = 0;
	highlight = 0;
	highlight_color = optix::make_float3(1.0f, 1.0f, 1.0f);
	shininess = 1.0f;
	roughness = optix::make_float2(0.999f, 0.999f);
	n_distribution = BECKMANN_DISTRIBUTION;
	ior = 1.3f;
	highlight_threshold = 0.0f;
	shader_name = QString::fromStdString("flat_shader.cu");
	initTable();
	initPrograms();
	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(color);
	mtl->declareVariable("flat_shadow");
	mtl["flat_shadow"]->setUint(flat_shadow);
	mtl->declareVariable("highlight_color");
	mtl["highlight_color"]->setFloat(highlight_color);
	mtl->declareVariable("highlight");
	mtl["highlight"]->setUint(highlight);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
	mtl->declareVariable("shininess");
	mtl["shininess"]->setFloat(shininess);
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(ior);
	mtl->declareVariable("highlight_threshold");
	mtl["highlight_threshold"]->setFloat(highlight_threshold);
};

FlatMaterial::FlatMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = FLAT_SHADER;
	color = optix::make_float3(1.0f, 1.0f, 1.0f);
	flat_shadow = 0;
	highlight = 0;
	highlight_color = optix::make_float3(1.0f, 1.0f, 1.0f);
	shininess = 1.0f;
	ior = 1.3f;
	highlight_threshold = 0.0f;
	n_distribution = BECKMANN_DISTRIBUTION;
	roughness = optix::make_float2(0.999f, 0.999f);
	shader_name = QString::fromStdString("flat_shader.cu");
	loadFromJSON(json);
};

void FlatMaterial::initTable()
{
	table = new QTableWidget(table_rows, 4);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);
	table->setItem(color_row, 0, new QTableWidgetItem(tr("diffuse_color")));
	table->item(color_row, 0)->setFlags(table->item(color_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(color_row, 1, new QTableWidgetItem(QString::number(color.x)));
	table->setItem(color_row, 2, new QTableWidgetItem(QString::number(color.y)));
	table->setItem(color_row, 3, new QTableWidgetItem(QString::number(color.z)));
	
	table->setItem(shadow_row, 0, new QTableWidgetItem(tr("flat_shadow")));
	table->item(shadow_row, 0)->setFlags(table->item(shadow_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *comboBox = new QComboBox();
	comboBox->setObjectName("combo_box");
	comboBox->addItem("Off");
	comboBox->addItem("On");
	comboBox->setCurrentIndex(flat_shadow);
	QObject::connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFlatShadow(int)));
	table->setCellWidget(shadow_row, 1, comboBox);

	table->setItem(highlight_row, 0, new QTableWidgetItem(tr("highlight")));
	table->item(highlight_row, 0)->setFlags(table->item(highlight_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *highlightComboBox = new QComboBox();
	highlightComboBox->setObjectName("combo_box");
	highlightComboBox->addItem("Off");
	highlightComboBox->addItem("On");
	highlightComboBox->setCurrentIndex(highlight);
	QObject::connect(highlightComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateHighlightOn(int)));
	table->setCellWidget(highlight_row, 1, highlightComboBox);

	table->setItem(high_color_row, 0, new QTableWidgetItem(tr("highlight_color")));
	table->item(high_color_row, 0)->setFlags(table->item(high_color_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(high_color_row, 1, new QTableWidgetItem(QString::number(highlight_color.x)));
	table->setItem(high_color_row, 2, new QTableWidgetItem(QString::number(highlight_color.y)));
	table->setItem(high_color_row, 3, new QTableWidgetItem(QString::number(highlight_color.z)));

	table->setItem(shininess_row, 0, new QTableWidgetItem(tr("shininess")));
	table->item(shininess_row, 0)->setFlags(table->item(shininess_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(shininess_row, 1, new QTableWidgetItem(QString::number(shininess)));
	
	table->setItem(roughness_row, 0, new QTableWidgetItem(tr("roughness")));
	table->item(roughness_row, 0)->setFlags(table->item(roughness_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(roughness_row, 1, new QTableWidgetItem(QString::number(roughness.x)));
	table->setItem(roughness_row, 2, new QTableWidgetItem(QString::number(roughness.y)));

	table->setItem(ior_row, 0, new QTableWidgetItem(tr("index_of_refraction")));
	table->item(ior_row, 0)->setFlags(table->item(ior_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(ior_row, 1, new QTableWidgetItem(QString::number(ior)));

	table->setItem(threshold_row, 0, new QTableWidgetItem(tr("highlight_threshold")));
	table->item(threshold_row, 0)->setFlags(table->item(threshold_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(threshold_row, 1, new QTableWidgetItem(QString::number(highlight_threshold)));

	table->setItem(n_dist_row, 0, new QTableWidgetItem(tr("normals_distribution")));
	table->item(n_dist_row, 0)->setFlags(table->item(n_dist_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *normalsComboBox = new QComboBox();
	for (int idx = 0; idx < NUMBER_OF_NORMALS_DISTRIBUTION; idx++) {
		normalsComboBox->addItem(normalDistributionNames[idx]);
	}
	normalsComboBox->setCurrentIndex(n_distribution);
	QObject::connect(normalsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateNormalsDistributionFromTable(int)));
	table->setCellWidget(n_dist_row, 1, normalsComboBox);

	QObject::connect(table, &QTableWidget::cellChanged, this, &FlatMaterial::tableUpdate);
};

void FlatMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);

};

void FlatMaterial::tableUpdate(int row, int column)
{

	if (row == color_row)
		updateColorFromTable(row);
	if (row == high_color_row)
		updateHighlightColorFromTable(row);
	if (row == shininess_row)
		updateShininessFromTable(row);
	if (row == roughness_row)
		updateRoughnessFromTable(row);
	if (row == threshold_row)
		updateThresholdFromTable(row);
	if (row == ior_row)
		updateIORFromTable(row);
}

void FlatMaterial::updateColorFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setColor(optix::make_float3(x, y, z));
}

void FlatMaterial::updateNormalsDistributionFromTable(int new_value)
{
	setNormalDistribution(static_cast<NormalsDistribution> (new_value));
}

void FlatMaterial::updateHighlightColorFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	float z = table->item(row, 3)->text().toFloat();
	setHighlightColor(optix::make_float3(x, y, z));
}

void FlatMaterial::updateShininessFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	setShininess(x);
}

void FlatMaterial::updateThresholdFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	setThreshold(x);
}

void FlatMaterial::updateIORFromTable(int row)
{
	float i = table->item(row, 1)->text().toFloat();
	setIor(i);
}

void FlatMaterial::updateRoughnessFromTable(int row)
{
	float x = table->item(row, 1)->text().toFloat();
	float y = table->item(row, 2)->text().toFloat();
	setRoughness(optix::make_float2(x,y));
}

void FlatMaterial::loadFromJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("mtl_parameters") && json["mtl_parameters"].isObject()) {
		parameters = json["mtl_parameters"].toObject();
	}

	if (parameters.contains("diffuse_color") && parameters["diffuse_color"].isArray()) {
		QJsonArray tmp = parameters["diffuse_color"].toArray();
		color = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}
	if (parameters.contains("flat_shadow") && parameters["flat_shadow"].isDouble()) {
		
		flat_shadow = static_cast<uint>(parameters["flat_shadow"].toDouble());
	}
	if (parameters.contains("highlight_color") && parameters["highlight_color"].isArray()) {

		QJsonArray tmp = parameters["highlight_color"].toArray();
		highlight_color = optix::make_float3(tmp[0].toDouble(), tmp[1].toDouble(), tmp[2].toDouble());
	}
	if (parameters.contains("roughness") && parameters["roughness"].isArray()) {

		QJsonArray tmp = parameters["roughness"].toArray();
		roughness = optix::make_float2(tmp[0].toDouble(), tmp[1].toDouble());
	}
	if (parameters.contains("highlight") && parameters["highlight"].isDouble()) {

		highlight = static_cast<uint>(parameters["highlight"].toDouble());
	}
	if (parameters.contains("shininess") && parameters["shininess"].isDouble()) {

		shininess = parameters["shininess"].toDouble();
	}
	if (parameters.contains("ior") && parameters["ior"].isDouble()) {
		ior = parameters["ior"].toDouble();
	}
	if (parameters.contains("normal_distribution") && parameters["normal_distribution"].isDouble()) {
		n_distribution = static_cast<NormalsDistribution>(parameters["normal_distribution"].toInt());
	}
	if (parameters.contains("highlight_threshold") && parameters["highlight_threshold"].isDouble()) {

		highlight_threshold = parameters["highlight_threshold"].toDouble();
	}
	initTable();
	initPrograms();

	mtl->declareVariable("diffuse_color");
	mtl["diffuse_color"]->setFloat(color);
	mtl->declareVariable("flat_shadow");
	mtl["flat_shadow"]->setUint(flat_shadow);
	mtl->declareVariable("highlight_color");
	mtl["highlight_color"]->setFloat(highlight_color);
	mtl->declareVariable("highlight");
	mtl["highlight"]->setUint(highlight);
	mtl->declareVariable("shininess");
	mtl["shininess"]->setFloat(shininess);
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
	mtl->declareVariable("ior");
	mtl["ior"]->setFloat(ior);
	mtl->declareVariable("highlight_threshold");
	mtl["highlight_threshold"]->setFloat(highlight_threshold);
}

void FlatMaterial::setColor(optix::float3 c)
{ 
	color = c; 
	mtl["diffuse_color"]->setFloat(color); 

};

void FlatMaterial::setHighlightColor(optix::float3 c)
{
	highlight_color = c;
	mtl["highlight_color"]->setFloat(highlight_color);

};

void FlatMaterial::setShininess(float s)
{
	shininess = s;
	mtl["shininess"]->setFloat(shininess);

};

void FlatMaterial::setThreshold(float t)
{
	highlight_threshold = t;
	mtl["highlight_threshold"]->setFloat(highlight_threshold);

};

void FlatMaterial::setRoughness(optix::float2 r)
{
	roughness = r;
	mtl["roughness"]->setFloat(roughness);

};

void FlatMaterial::setIor(float i)
{
	ior = i;
	mtl["ior"]->setFloat(ior);

};

void FlatMaterial::updateFlatShadow(int new_value)
{
	setFlatShadow(new_value);
}

void FlatMaterial::updateHighlightOn(int new_value)
{
	setHighlightOn(new_value);
}

void FlatMaterial::setFlatShadow(uint new_value)
{
	flat_shadow = new_value;
	mtl["flat_shadow"]->setUint(flat_shadow);
	table->cellChanged(shadow_row, 1);
}

void FlatMaterial::setHighlightOn(uint new_value)
{
	highlight = new_value;
	mtl["highlight"]->setUint(highlight);
	table->cellChanged(shininess_row, 1);
}

void FlatMaterial::setNormalDistribution(NormalsDistribution n)
{
	n_distribution = n;
	mtl["normal_distribution"]->setUint(n_distribution);
	table->cellChanged(n_dist_row, 1);
}

void FlatMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QJsonObject parameters;
	parameters["diffuse_color"] = QJsonArray{ color.x, color.y, color.z };
	parameters["highlight_color"] = QJsonArray{ highlight_color.x, highlight_color.y, highlight_color.z };
	parameters["flat_shadow"] = static_cast<int>(flat_shadow);
	parameters["highlight"] = static_cast<int>(highlight);
	parameters["shininess"] = (shininess);
	parameters["roughness"] = QJsonArray{roughness.x, roughness.y};
	parameters["normal_distribution"] = n_distribution;
	parameters["ior"] = ior;
	parameters["highlight_threshold"] = highlight_threshold;
	json["mtl_parameters"] = parameters;
}