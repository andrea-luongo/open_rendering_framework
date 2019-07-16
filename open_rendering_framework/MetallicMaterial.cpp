#pragma once
#include "Material.h"
#include "OptixScene.h"
#include "sampleConfig.h"


MetallicMaterial::MetallicMaterial(optix::Context c)
{
	context = c;
	type = METALLIC_SHADER;
	index_of_refraction = gold;
	roughness = optix::make_float2(0.1f, 0.1f);
	m_model = WALTER_MODEL;
	n_distribution = BECKMANN_DISTRIBUTION;
	shader_name = QString::fromStdString("metallic_shader.cu");
	initTable();
	initPrograms();
	mtl->declareVariable("ior");
	mtl["ior"]->setUserData(sizeof(MyComplex3), &index_of_refraction);
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("microfacet_model");
	mtl["microfacet_model"]->setUint(m_model);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
};

MetallicMaterial::MetallicMaterial(optix::Context c, const QJsonObject &json)
{
	context = c;
	type = METALLIC_SHADER;
	shader_name = QString::fromStdString("metallic_shader.cu");
	loadFromJSON(json);
};

void MetallicMaterial::initTable()
{
	table = new QTableWidget(table_rows, 4);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);

	table->setItem(default_ior_row, 0, new QTableWidgetItem(tr("Default IOR")));
	table->item(default_ior_row, 0)->setFlags(table->item(default_ior_row, 0)->flags() &  ~Qt::ItemIsEditable);
	QComboBox *iorComboBox = new QComboBox();
	iorComboBox->setObjectName("ior_combo_box");
	iorComboBox->addItem("Custom");
	iorComboBox->addItem("Gold");
	iorComboBox->addItem("Silver");
	iorComboBox->addItem("Copper");
	iorComboBox->addItem("Aluminium");
	iorComboBox->setCurrentIndex(0);
	QObject::connect(iorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCustomIOR(int)));
	table->setCellWidget(default_ior_row, 1, iorComboBox);

	table->setItem(x_ior_row, 0, new QTableWidgetItem(tr("ior_x (Re)")));
	table->item(x_ior_row, 0)->setFlags(table->item(x_ior_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(x_ior_row, 1, new QTableWidgetItem(QString::number(index_of_refraction.x.real)));
	table->setItem(x_ior_row, 2, new QTableWidgetItem(tr("ior_x (Im)")));
	table->item(x_ior_row, 2)->setFlags(table->item(x_ior_row, 2)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(x_ior_row, 3, new QTableWidgetItem(QString::number(index_of_refraction.x.im)));

	table->setItem(y_ior_row, 0, new QTableWidgetItem(tr("ior_y (Re)")));
	table->item(y_ior_row, 0)->setFlags(table->item(y_ior_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(y_ior_row, 1, new QTableWidgetItem(QString::number(index_of_refraction.y.real)));
	table->setItem(y_ior_row, 2, new QTableWidgetItem(tr("ior_y (Im)")));
	table->item(y_ior_row, 2)->setFlags(table->item(y_ior_row, 2)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(y_ior_row, 3, new QTableWidgetItem(QString::number(index_of_refraction.y.im)));

	table->setItem(z_ior_row, 0, new QTableWidgetItem(tr("ior_z (Re)")));
	table->item(z_ior_row, 0)->setFlags(table->item(z_ior_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(z_ior_row, 1, new QTableWidgetItem(QString::number(index_of_refraction.z.real)));
	table->setItem(z_ior_row, 2, new QTableWidgetItem(tr("ior_z (Im)")));
	table->item(z_ior_row, 2)->setFlags(table->item(z_ior_row, 2)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(z_ior_row, 3, new QTableWidgetItem(QString::number(index_of_refraction.z.im)));

	table->setItem(roughness_row, 0, new QTableWidgetItem(tr("roughness_x")));
	table->item(roughness_row, 0)->setFlags(table->item(roughness_row, 0)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(roughness_row, 1, new QTableWidgetItem(QString::number(roughness.x)));

	table->setItem(roughness_row, 2, new QTableWidgetItem(tr("roughness_y")));
	table->item(roughness_row, 2)->setFlags(table->item(roughness_row, 2)->flags() &  ~Qt::ItemIsEditable);
	table->setItem(roughness_row, 3, new QTableWidgetItem(QString::number(roughness.y)));

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
	QObject::connect(table, &QTableWidget::cellChanged, this, &MetallicMaterial::tableUpdate);


};

void MetallicMaterial::initPrograms()
{
	mtl = context->createMaterial();
	optix::Program any_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "any_hit");
	optix::Program closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, shader_name.toStdString()), "closest_hit");
	optix::Program depth_closest_hit = context->createProgramFromPTXFile(OptixScene::ptxPath(SAMPLE_NAME, "rough_depth_shader.cu"), "closest_hit");
	mtl->setClosestHitProgram(radiance_ray_type, closest_hit);
	mtl->setAnyHitProgram(shadow_ray_type, any_hit);
	mtl->setClosestHitProgram(depth_ray_type, depth_closest_hit);

};

void MetallicMaterial::tableUpdate(int row, int column)
{
	if (row == 1 || row == 2 || row== 3)
		updateIORFromTable(row);
	else if (row == 4)
		updateRoughnessFromTable(row);
}


void MetallicMaterial::updateIORFromTable(int row)
{
	MyComplex3 ior;
	ior.x.real = table->item(1, 1)->text().toFloat();
	ior.x.im = table->item(1, 3)->text().toFloat();
	ior.y.real = table->item(2, 1)->text().toFloat();
	ior.y.im = table->item(2, 3)->text().toFloat();
	ior.z.real = table->item(3, 1)->text().toFloat();
	ior.z.im = table->item(3, 3)->text().toFloat();
	setIor(ior);
	table->findChild<QComboBox*>("ior_combo_box")->setCurrentIndex(0);
}

void MetallicMaterial::updateRoughnessFromTable(int row)
{
	float x= table->item(row, 1)->text().toFloat();
	float y = table->item(row, 3)->text().toFloat();
	setRoughness(optix::make_float2(x,y));
}

void MetallicMaterial::updateMicrofacetModelFromTable(int new_value)
{
	setMicrofacetModel(static_cast<MicrofacetModel> (new_value));
}

void MetallicMaterial::updateNormalsDistributionFromTable(int new_value)
{
	setNormalDistribution(static_cast<NormalsDistribution> (new_value));
}

void MetallicMaterial::updateCustomIOR(int new_value)
{
	if (new_value == 1) {
		setIor(gold);
		table->cellChanged(0, 1);
	}
	else if (new_value == 2) {
		setIor(silver);
		table->cellChanged(0, 1);
	}
	else if (new_value == 3) {
		setIor(copper);
		table->cellChanged(0, 1);
	}
	else if (new_value == 4) {
		setIor(alum);
		table->cellChanged(0, 1);
	}
	table->blockSignals(true);
	table->item(1, 1)->setText(QString::number(index_of_refraction.x.real));
	table->item(1, 3)->setText(QString::number(index_of_refraction.x.im));
	table->item(2, 1)->setText(QString::number(index_of_refraction.y.real));
	table->item(2, 3)->setText(QString::number(index_of_refraction.y.im));
	table->item(3, 1)->setText(QString::number(index_of_refraction.z.real));
	table->item(3, 3)->setText(QString::number(index_of_refraction.z.im));
	table->blockSignals(false);
}

void MetallicMaterial::loadFromJSON(const QJsonObject &json)
{
	QJsonObject parameters;
	if (json.contains("mtl_parameters") && json["mtl_parameters"].isObject()) {
		parameters = json["mtl_parameters"].toObject();
	}

	if (parameters.contains("real_ior") && parameters["real_ior"].isArray()) {
		QJsonArray tmp = parameters["real_ior"].toArray();
		index_of_refraction.x.real = tmp[0].toDouble();
		index_of_refraction.y.real = tmp[1].toDouble();
		index_of_refraction.z.real = tmp[2].toDouble();
	}

	if (parameters.contains("imaginary_ior") && parameters["imaginary_ior"].isArray()) {
		QJsonArray tmp = parameters["imaginary_ior"].toArray();
		index_of_refraction.x.im = tmp[0].toDouble();
		index_of_refraction.y.im = tmp[1].toDouble();
		index_of_refraction.z.im = tmp[2].toDouble();
	}

	if (parameters.contains("roughness") && parameters["roughness"].isArray()) {
		QJsonArray tmp = parameters["roughness"].toArray();
		roughness = optix::make_float2(tmp[0].toDouble(), tmp[1].toDouble());
	}
	if (parameters.contains("microfacet_model") && parameters["microfacet_model"].isDouble()) {
		m_model = static_cast<MicrofacetModel> (parameters["microfacet_model"].toInt());

	}
	if (parameters.contains("normal_distribution") && parameters["normal_distribution"].isDouble()) {
		n_distribution = static_cast<NormalsDistribution>( parameters["normal_distribution"].toInt());
	}

	initTable();
	initPrograms();

	mtl->declareVariable("ior");
	mtl["ior"]->setUserData(sizeof(MyComplex3), &index_of_refraction);
	mtl->declareVariable("roughness");
	mtl["roughness"]->setFloat(roughness);
	mtl->declareVariable("microfacet_model");
	mtl["microfacet_model"]->setUint(m_model);
	mtl->declareVariable("normal_distribution");
	mtl["normal_distribution"]->setUint(n_distribution);
}



void MetallicMaterial::setIor(MyComplex3 ior)
{
	index_of_refraction = ior;
	mtl["ior"]->setUserData(sizeof(MyComplex3), &index_of_refraction);

};

void MetallicMaterial::setRoughness(optix::float2 r)
{
	roughness = r;
	mtl["roughness"]->setFloat(roughness);
}

void MetallicMaterial::setMicrofacetModel(MicrofacetModel m)
{
	m_model = m;
	mtl["microfacet_model"]->setUint(m_model);
	table->cellChanged(3,1);
}

void MetallicMaterial::setNormalDistribution(NormalsDistribution n)
{
	n_distribution = n;
	mtl["normal_distribution"]->setUint(n_distribution);
	table->cellChanged(4, 1);
}

void MetallicMaterial::writeJSON(QJsonObject &json) const
{
	json["mtl_name"] = materialNames[type];
	QJsonObject parameters;
	//parameters["ior"] = index_of_refraction;
	parameters["real_ior"] = QJsonArray{index_of_refraction.x.real, index_of_refraction.y.real, index_of_refraction.z.real};
	parameters["imaginary_ior"] = QJsonArray{index_of_refraction.x.im, index_of_refraction.y.im, index_of_refraction.z.im};
	parameters["roughness"] = QJsonArray{ roughness.x, roughness.y };
	parameters["microfacet_model"] = m_model;
	parameters["normal_distribution"] = n_distribution;
	json["mtl_parameters"] = parameters;
}
