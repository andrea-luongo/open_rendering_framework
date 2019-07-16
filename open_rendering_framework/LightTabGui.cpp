#include "LightTabGui.h"



LightTab::LightTab(OptixWindow* optix_window, QWidget *parent)
	: QWidget(parent)
{
	optixWindow = optix_window;
	lightTabLayout = new QHBoxLayout();
	setLayout(lightTabLayout);
	initLightTab();
}

void LightTab::initLightTab()
{
	lightWidgetVector.clear();
	lightWidgetVector.reserve(optixWindow->getScene()->getLights()->size());

	commonSettingBox = new QGroupBox;
	QGridLayout *commonSettingsLayout = new QGridLayout();
	commonSettingBox->setLayout(commonSettingsLayout);

	QListWidget *listWidget = new QListWidget(commonSettingBox);
	listWidget->setObjectName("list_widget");
	listWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
	listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	QObject::connect(listWidget, &QListWidget::currentRowChanged, this, &LightTab::changeLightLayout);

	QPushButton *addButton = new QPushButton("Add Light", commonSettingBox);
	addButton->setObjectName("add_button");
	QObject::connect(addButton, &QPushButton::released, this, &LightTab::addLight);
	QComboBox *lightComboBox = new QComboBox(commonSettingBox);
	lightComboBox->setObjectName("light_combo_box");
	for (int idx = 0; idx < NUMBER_OF_LIGHTS; idx++) {
		lightComboBox->addItem(lightNames[idx]);
	}

	commonSettingsLayout->addWidget(listWidget, 0, 0, 4, 2);
	commonSettingsLayout->addWidget(addButton, 4, 0);
	commonSettingsLayout->addWidget(lightComboBox, 4, 1);
	//initialize lights layout;
	for (int idx = 0; idx < optixWindow->getScene()->getLights()->size(); idx++)
	{
		if (optixWindow->getScene()->getLights()->data()[idx]->getType() == POINT_LIGHT) {
			addPointLightParameters(reinterpret_cast<PointLight*> (optixWindow->getScene()->getLights()->data()[idx]), reinterpret_cast<PointLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[idx]) );
		} else if (optixWindow->getScene()->getLights()->data()[idx]->getType() == DIRECTIONAL_LIGHT) {
			addDirectionalLightParameters(reinterpret_cast<DirectionalLight*> (optixWindow->getScene()->getLights()->data()[idx]), reinterpret_cast<DirectionalLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[idx]));
		} else if (optixWindow->getScene()->getLights()->data()[idx]->getType() == TRIANGLES_AREA_LIGHT) {
			addTriangleAreaLightParameters(reinterpret_cast<TriangleAreaLight*> (optixWindow->getScene()->getLights()->data()[idx]), reinterpret_cast<TrianglesAreaLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[idx]));
		} else if (optixWindow->getScene()->getLights()->data()[idx]->getType() == DISK_LIGHT) {
			addDiskAreaLightParameters(reinterpret_cast<DiskAreaLight*> (optixWindow->getScene()->getLights()->data()[idx]), reinterpret_cast<DiskLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[idx]));
		} else if (optixWindow->getScene()->getLights()->data()[idx]->getType() == SPHERICAL_LIGHT) {
			addSphericalLightParameters(reinterpret_cast<SphericalLight*> (optixWindow->getScene()->getLights()->data()[idx]), reinterpret_cast<SphericalLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[idx]));
		}
	}

	lightTabLayout->addWidget(commonSettingBox);
	updateLightList();

}

void LightTab::addPointLightParameters(PointLight* light, PointLightStruct* lightStruct)
{
	QGroupBox *pointLightBox = new QGroupBox;
	QGridLayout *pointLightLayout = new QGridLayout();
	QLabel *lightNameLabel = new QLabel(tr("Light Type"), pointLightBox);
	lightNameLabel->setObjectName("light_label");
	QLineEdit *lightNameEdit = new QLineEdit(lightNames[light->getType()], pointLightBox);
	lightNameEdit->setObjectName("light_name");
	lightNameEdit->setReadOnly(true);

	//LIGHT POSITION WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *positionLabel = new QLabel(tr("Position"), pointLightBox);
	positionLabel->setObjectName("position_label");
	QLineEdit *xPosEdit = new QLineEdit(QString::number(light->getPosition().x()), pointLightBox);
	QLineEdit *yPosEdit = new QLineEdit(QString::number(light->getPosition().y()), pointLightBox);
	QLineEdit *zPosEdit = new QLineEdit(QString::number(light->getPosition().z()), pointLightBox);
	xPosEdit->setObjectName("pos_x");
	yPosEdit->setObjectName("pos_y");
	zPosEdit->setObjectName("pos_z");
	QObject::connect(xPosEdit, &QLineEdit::returnPressed, this, &LightTab::updatePointLightPosition);
	QObject::connect(yPosEdit, &QLineEdit::returnPressed, this, &LightTab::updatePointLightPosition);
	QObject::connect(zPosEdit, &QLineEdit::returnPressed, this, &LightTab::updatePointLightPosition);


	//LIGHT RADIANCE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *radianceLabel = new QLabel(tr("Radiance"), pointLightBox);
	radianceLabel->setObjectName("radiance_label");
	QLineEdit *xRadianceEdit = new QLineEdit(QString::number(light->getRadiance().x()), pointLightBox);
	QLineEdit *yRadianceEdit = new QLineEdit(QString::number(light->getRadiance().y()), pointLightBox);
	QLineEdit *zRadianceEdit = new QLineEdit(QString::number(light->getRadiance().z()), pointLightBox);
	xRadianceEdit->setObjectName("radiance_x");
	yRadianceEdit->setObjectName("radiance_y");
	zRadianceEdit->setObjectName("radiance_z");
	QObject::connect(xRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updatePointLightRadiance);
	QObject::connect(yRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updatePointLightRadiance);
	QObject::connect(zRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updatePointLightRadiance);
	

	QPushButton *removeButton = new QPushButton("Remove", pointLightBox);
	removeButton->setObjectName("remove_button");
	QObject::connect(removeButton, &QPushButton::released, this, &LightTab::removeLight);

	pointLightLayout->addWidget(lightNameLabel, 0, 0);
	pointLightLayout->addWidget(lightNameEdit, 0, 1);
	pointLightLayout->addWidget(positionLabel, 1, 0);
	pointLightLayout->addWidget(xPosEdit, 1, 1);
	pointLightLayout->addWidget(yPosEdit, 1, 2);
	pointLightLayout->addWidget(zPosEdit, 1, 3);
	pointLightLayout->addWidget(radianceLabel, 2, 0);
	pointLightLayout->addWidget(xRadianceEdit, 2, 1);
	pointLightLayout->addWidget(yRadianceEdit, 2, 2);
	pointLightLayout->addWidget(zRadianceEdit, 2, 3);
	pointLightLayout->addWidget(removeButton, 0, 3);

	pointLightBox->setLayout(pointLightLayout);
	lightWidgetVector.append(pointLightBox);
	pointLightBox->hide();
}

void LightTab::addDirectionalLightParameters(DirectionalLight* light, DirectionalLightStruct* lightStruct)
{
	QGroupBox *directionalLightBox = new QGroupBox;
	QGridLayout *directionalLightLayout = new QGridLayout();
	
	QLabel *lightNameLabel = new QLabel(tr("Light Type"), directionalLightBox);
	lightNameLabel->setObjectName("light_label");
	
	QLineEdit *lightNameEdit = new QLineEdit(lightNames[light->getType()], directionalLightBox);
	lightNameEdit->setObjectName("light_name");
	lightNameEdit->setReadOnly(true);

	//LIGHT POSITION WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *directionLabel = new QLabel(tr("Direction"), directionalLightBox);
	directionLabel->setObjectName("direction_label");
	QLineEdit *xDirEdit = new QLineEdit(QString::number(light->getDirection().x()), directionalLightBox);
	QLineEdit *yDirEdit = new QLineEdit(QString::number(light->getDirection().y()), directionalLightBox);
	QLineEdit *zDirEdit = new QLineEdit(QString::number(light->getDirection().z()), directionalLightBox);
	xDirEdit->setObjectName("dir_x");
	yDirEdit->setObjectName("dir_y");
	zDirEdit->setObjectName("dir_z");
	QObject::connect(xDirEdit, &QLineEdit::returnPressed, this, &LightTab::updateDirectionalLightDirection);
	QObject::connect(yDirEdit, &QLineEdit::returnPressed, this, &LightTab::updateDirectionalLightDirection);
	QObject::connect(zDirEdit, &QLineEdit::returnPressed, this, &LightTab::updateDirectionalLightDirection);
	
	//LIGHT RADIANCE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *radianceLabel = new QLabel(tr("Radiance"), this);
	radianceLabel->setObjectName("radiance_label");
	QLineEdit *xRadianceEdit = new QLineEdit(QString::number(light->getRadiance().x()), directionalLightBox);
	QLineEdit *yRadianceEdit = new QLineEdit(QString::number(light->getRadiance().y()), directionalLightBox);
	QLineEdit *zRadianceEdit = new QLineEdit(QString::number(light->getRadiance().z()), directionalLightBox);
	xRadianceEdit->setObjectName("radiance_x");
	yRadianceEdit->setObjectName("radiance_y");
	zRadianceEdit->setObjectName("radiance_z");
	QObject::connect(xRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateDirectionalLightRadiance);
	QObject::connect(yRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateDirectionalLightRadiance);
	QObject::connect(zRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateDirectionalLightRadiance);


	QPushButton *removeButton = new QPushButton("Remove", directionalLightBox);
	removeButton->setObjectName("remove_button");
	QObject::connect(removeButton, &QPushButton::released, this, &LightTab::removeLight);

	directionalLightLayout->addWidget(lightNameLabel, 0, 0);
	directionalLightLayout->addWidget(lightNameEdit, 0, 1, 1, 2);
	directionalLightLayout->addWidget(directionLabel, 1, 0);
	directionalLightLayout->addWidget(xDirEdit, 1, 1);
	directionalLightLayout->addWidget(yDirEdit, 1, 2);
	directionalLightLayout->addWidget(zDirEdit, 1, 3);
	directionalLightLayout->addWidget(radianceLabel, 2, 0);
	directionalLightLayout->addWidget(xRadianceEdit, 2, 1);
	directionalLightLayout->addWidget(yRadianceEdit, 2, 2);
	directionalLightLayout->addWidget(zRadianceEdit, 2, 3);
	directionalLightLayout->addWidget(removeButton, 0, 3);

	directionalLightBox->setLayout(directionalLightLayout);
	lightWidgetVector.append(directionalLightBox);
	directionalLightBox->hide();
}

void LightTab::addDiskAreaLightParameters(DiskAreaLight* light, DiskLightStruct* lightStruct)
{
	QGroupBox *diskLightBox = new QGroupBox;
	QGridLayout *disklLightLayout = new QGridLayout();

	QLabel *lightNameLabel = new QLabel(tr("Light Type"), diskLightBox);
	lightNameLabel->setObjectName("light_label");

	QLineEdit *lightNameEdit = new QLineEdit(lightNames[light->getType()], diskLightBox);
	lightNameEdit->setObjectName("light_name");
	lightNameEdit->setReadOnly(true);

	//LIGHT POSITION WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *positionLabel = new QLabel(tr("Position"), diskLightBox);
	positionLabel->setObjectName("position_label");
	QLineEdit *xPosEdit = new QLineEdit(QString::number(light->getPosition().x()), diskLightBox);
	QLineEdit *yPosEdit = new QLineEdit(QString::number(light->getPosition().y()), diskLightBox);
	QLineEdit *zPosEdit = new QLineEdit(QString::number(light->getPosition().z()), diskLightBox);
	xPosEdit->setObjectName("pos_x");
	yPosEdit->setObjectName("pos_y");
	zPosEdit->setObjectName("pos_z");
	QObject::connect(xPosEdit, &QLineEdit::returnPressed, this, &LightTab::updateDiskLightPosition);
	QObject::connect(yPosEdit, &QLineEdit::returnPressed, this, &LightTab::updateDiskLightPosition);
	QObject::connect(zPosEdit, &QLineEdit::returnPressed, this, &LightTab::updateDiskLightPosition);

	//LIGHT RADIANCE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *radianceLabel = new QLabel(tr("Radiance"), this);
	radianceLabel->setObjectName("radiance_label");
	QLineEdit *xRadianceEdit = new QLineEdit(QString::number(light->getRadiance().x()), diskLightBox);
	QLineEdit *yRadianceEdit = new QLineEdit(QString::number(light->getRadiance().y()), diskLightBox);
	QLineEdit *zRadianceEdit = new QLineEdit(QString::number(light->getRadiance().z()), diskLightBox);
	xRadianceEdit->setObjectName("radiance_x");
	yRadianceEdit->setObjectName("radiance_y");
	zRadianceEdit->setObjectName("radiance_z");
	QObject::connect(xRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateDiskLightRadiance);
	QObject::connect(yRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateDiskLightRadiance);
	QObject::connect(zRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateDiskLightRadiance);

	//LIGHT RADIUS WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *radiusLabel = new QLabel(tr("Radius"), this);
	radiusLabel->setObjectName("radius_label");
	QLineEdit *radiusEdit = new QLineEdit(QString::number(light->getRadius()), diskLightBox);	
	radiusEdit->setObjectName("radius");
	QObject::connect(radiusEdit, &QLineEdit::returnPressed, this, &LightTab::updateDiskLightRadius);
	
	//LIGHT NORMAL WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *polarLabel = new QLabel(tr("Normal Polar angle"), this);
	polarLabel->setObjectName("polar_label");
	QLineEdit *polarEdit = new QLineEdit(QString::number(light->getTheta()), diskLightBox);
	polarEdit->setObjectName("polar");
	QLabel *azimuthLabel = new QLabel(tr("Normal Azimuthal angle"), this);
	azimuthLabel->setObjectName("azimuth_label");
	QLineEdit *azimuthEdit = new QLineEdit(QString::number(light->getPhi()), diskLightBox);
	azimuthEdit->setObjectName("azimuth");
	QObject::connect(polarEdit, &QLineEdit::returnPressed, this, &LightTab::updateDiskLightNormal);
	QObject::connect(azimuthEdit, &QLineEdit::returnPressed, this, &LightTab::updateDiskLightNormal);

	//REMOVE LIGHT BUTTON
	QPushButton *removeButton = new QPushButton("Remove", diskLightBox);
	removeButton->setObjectName("remove_button");
	QObject::connect(removeButton, &QPushButton::released, this, &LightTab::removeLight);

	disklLightLayout->addWidget(lightNameLabel, 0, 0);
	disklLightLayout->addWidget(lightNameEdit, 0, 1, 1, 2);
	disklLightLayout->addWidget(positionLabel, 1, 0);
	disklLightLayout->addWidget(xPosEdit, 1, 1);
	disklLightLayout->addWidget(yPosEdit, 1, 2);
	disklLightLayout->addWidget(zPosEdit, 1, 3);
	disklLightLayout->addWidget(radianceLabel, 2, 0);
	disklLightLayout->addWidget(xRadianceEdit, 2, 1);
	disklLightLayout->addWidget(yRadianceEdit, 2, 2);
	disklLightLayout->addWidget(zRadianceEdit, 2, 3);
	disklLightLayout->addWidget(radiusLabel, 3, 0);
	disklLightLayout->addWidget(radiusEdit, 3, 1);
	disklLightLayout->addWidget(polarLabel, 4, 0);
	disklLightLayout->addWidget(polarEdit, 4, 1);
	disklLightLayout->addWidget(azimuthLabel, 4, 2);
	disklLightLayout->addWidget(azimuthEdit, 4, 3);
	disklLightLayout->addWidget(removeButton, 0, 3);

	diskLightBox->setLayout(disklLightLayout);
	lightWidgetVector.append(diskLightBox);
	diskLightBox->hide();
}

void LightTab::addSphericalLightParameters(SphericalLight* light, SphericalLightStruct* lightStruct)
{
	QGroupBox *sphereLightBox = new QGroupBox;
	QGridLayout *spherelLightLayout = new QGridLayout();

	QLabel *lightNameLabel = new QLabel(tr("Light Type"), sphereLightBox);
	lightNameLabel->setObjectName("light_label");

	QLineEdit *lightNameEdit = new QLineEdit(lightNames[light->getType()], sphereLightBox);
	lightNameEdit->setObjectName("light_name");
	lightNameEdit->setReadOnly(true);

	//LIGHT POSITION WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *positionLabel = new QLabel(tr("Position"), sphereLightBox);
	positionLabel->setObjectName("position_label");
	QLineEdit *xPosEdit = new QLineEdit(QString::number(light->getPosition().x()), sphereLightBox);
	QLineEdit *yPosEdit = new QLineEdit(QString::number(light->getPosition().y()), sphereLightBox);
	QLineEdit *zPosEdit = new QLineEdit(QString::number(light->getPosition().z()), sphereLightBox);
	xPosEdit->setObjectName("pos_x");
	yPosEdit->setObjectName("pos_y");
	zPosEdit->setObjectName("pos_z");
	QObject::connect(xPosEdit, &QLineEdit::returnPressed, this, &LightTab::updateSphericalLightPosition);
	QObject::connect(yPosEdit, &QLineEdit::returnPressed, this, &LightTab::updateSphericalLightPosition);
	QObject::connect(zPosEdit, &QLineEdit::returnPressed, this, &LightTab::updateSphericalLightPosition);

	//LIGHT RADIANCE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *radianceLabel = new QLabel(tr("Radiance"), this);
	radianceLabel->setObjectName("radiance_label");
	QLineEdit *xRadianceEdit = new QLineEdit(QString::number(light->getRadiance().x()), sphereLightBox);
	QLineEdit *yRadianceEdit = new QLineEdit(QString::number(light->getRadiance().y()), sphereLightBox);
	QLineEdit *zRadianceEdit = new QLineEdit(QString::number(light->getRadiance().z()), sphereLightBox);
	xRadianceEdit->setObjectName("radiance_x");
	yRadianceEdit->setObjectName("radiance_y");
	zRadianceEdit->setObjectName("radiance_z");
	QObject::connect(xRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateSphericalLightRadiance);
	QObject::connect(yRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateSphericalLightRadiance);
	QObject::connect(zRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateSphericalLightRadiance);

	//LIGHT RADIUS WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *radiusLabel = new QLabel(tr("Radius"), this);
	radiusLabel->setObjectName("radius_label");
	QLineEdit *radiusEdit = new QLineEdit(QString::number(light->getRadius()), sphereLightBox);
	radiusEdit->setObjectName("radius");
	QObject::connect(radiusEdit, &QLineEdit::returnPressed, this, &LightTab::updateSphericalLightRadius);


	//REMOVE LIGHT BUTTON
	QPushButton *removeButton = new QPushButton("Remove", sphereLightBox);
	removeButton->setObjectName("remove_button");
	QObject::connect(removeButton, &QPushButton::released, this, &LightTab::removeLight);

	spherelLightLayout->addWidget(lightNameLabel, 0, 0);
	spherelLightLayout->addWidget(lightNameEdit, 0, 1, 1, 2);
	spherelLightLayout->addWidget(positionLabel, 1, 0);
	spherelLightLayout->addWidget(xPosEdit, 1, 1);
	spherelLightLayout->addWidget(yPosEdit, 1, 2);
	spherelLightLayout->addWidget(zPosEdit, 1, 3);
	spherelLightLayout->addWidget(radianceLabel, 2, 0);
	spherelLightLayout->addWidget(xRadianceEdit, 2, 1);
	spherelLightLayout->addWidget(yRadianceEdit, 2, 2);
	spherelLightLayout->addWidget(zRadianceEdit, 2, 3);
	spherelLightLayout->addWidget(radiusLabel, 3, 0);
	spherelLightLayout->addWidget(radiusEdit, 3, 1);
	spherelLightLayout->addWidget(removeButton, 0, 3);

	sphereLightBox->setLayout(spherelLightLayout);
	lightWidgetVector.append(sphereLightBox);
	sphereLightBox->hide();
}

void LightTab::addTriangleAreaLightParameters(TriangleAreaLight* light, TrianglesAreaLightStruct* lightStruct)
{
	QGroupBox *triangleLightBox = new QGroupBox;
	QGridLayout *triangleLightLayout = new QGridLayout();

	QLabel *lightNameLabel = new QLabel(tr("Light Type"), triangleLightBox);
	lightNameLabel->setObjectName("light_label");

	QLineEdit *lightNameEdit = new QLineEdit(lightNames[light->getType()], triangleLightBox);
	lightNameEdit->setObjectName("light_name");
	lightNameEdit->setReadOnly(true);


	//LIGHT RADIANCE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *radianceLabel = new QLabel(tr("Radiance"), this);
	radianceLabel->setObjectName("radiance_label");
	QLineEdit *xRadianceEdit = new QLineEdit(QString::number(light->getRadiance().x()), triangleLightBox);
	QLineEdit *yRadianceEdit = new QLineEdit(QString::number(light->getRadiance().y()), triangleLightBox);
	QLineEdit *zRadianceEdit = new QLineEdit(QString::number(light->getRadiance().z()), triangleLightBox);
	xRadianceEdit->setObjectName("radiance_x");
	yRadianceEdit->setObjectName("radiance_y");
	zRadianceEdit->setObjectName("radiance_z");
	QObject::connect(xRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightRadiance);
	QObject::connect(yRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightRadiance);
	QObject::connect(zRadianceEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightRadiance);
	

	//LIGHT POSITION WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *positionLabel = new QLabel(tr("Position"), triangleLightBox);
	positionLabel->setObjectName("position_label");
	QLineEdit *xPosEdit = new QLineEdit(QString::number(light->getTranslation().x), triangleLightBox);
	QLineEdit *yPosEdit = new QLineEdit(QString::number(light->getTranslation().y), triangleLightBox);
	QLineEdit *zPosEdit = new QLineEdit(QString::number(light->getTranslation().z), triangleLightBox);
	xPosEdit->setObjectName("pos_x");
	yPosEdit->setObjectName("pos_y");
	zPosEdit->setObjectName("pos_z");
	QObject::connect(xPosEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightPosition);
	QObject::connect(yPosEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightPosition);
	QObject::connect(zPosEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightPosition);


	//LIGHT SCALE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *scaleLabel = new QLabel(tr("Scale"), triangleLightBox);
	scaleLabel->setObjectName("scale_label");
	QLineEdit *xScaleEdit = new QLineEdit(QString::number(light->getScale().x), triangleLightBox);
	QLineEdit *yScaleEdit = new QLineEdit(QString::number(light->getScale().y), triangleLightBox);
	QLineEdit *zScaleEdit = new QLineEdit(QString::number(light->getScale().z), triangleLightBox);
	xScaleEdit->setObjectName("scale_x");
	yScaleEdit->setObjectName("scale_y");
	zScaleEdit->setObjectName("scale_z");
	QObject::connect(xScaleEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightScale);
	QObject::connect(yScaleEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightScale);
	QObject::connect(zScaleEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightScale);


	//LIGHT ROTATION WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	//light free axis rotation
	QLabel *rotationAxisLabel = new QLabel(tr("Rotation Axis"), triangleLightBox);
	rotationAxisLabel->setObjectName("rotation_axis_label");
	QLineEdit *xRotAxisEdit = new QLineEdit(QString::number(light->getRotationAxis().x), triangleLightBox);
	QLineEdit *yRotAxisEdit = new QLineEdit(QString::number(light->getRotationAxis().y), triangleLightBox);
	QLineEdit *zRotAxisEdit = new QLineEdit(QString::number(light->getRotationAxis().z), triangleLightBox);
	xRotAxisEdit->setObjectName("rot_axis_x");
	yRotAxisEdit->setObjectName("rot_axis_y");
	zRotAxisEdit->setObjectName("rot_axis_z");
	QObject::connect(xRotAxisEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightRotation);
	QObject::connect(yRotAxisEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightRotation);
	QObject::connect(zRotAxisEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightRotation);
	QLabel *rotationAngleLabel = new QLabel(tr("Rotation Angle"), triangleLightBox);
	rotationAngleLabel->setObjectName("rotation_angle_label");
	QLineEdit *rotAngleEdit = new QLineEdit(QString::number(light->getRotationAngle()), triangleLightBox);
	QObject::connect(rotAngleEdit, &QLineEdit::returnPressed, this, &LightTab::updateTriangleLightRotation);
	rotAngleEdit->setObjectName("rot_angle");
	//light X axis rotation
	QLabel *xRotSliderLabel = new QLabel(tr("X rot"), triangleLightBox);
	xRotSliderLabel->setObjectName("x_rot_label");
	QSlider *xRotSlider = new QSlider(Qt::Horizontal, triangleLightBox);
	xRotSlider->setObjectName("x_rot_slider");
	xRotSlider->setMinimum(-180);	xRotSlider->setMaximum(180); xRotSlider->setValue(light->getXRotation());
	xRotSlider->setTickInterval(45); xRotSlider->setTickPosition(QSlider::TicksBelow);
	QObject::connect(xRotSlider, &QSlider::valueChanged, this, &LightTab::updateTriangleLightXRotation);

	QSpinBox *xRotSpinBox = new QSpinBox(triangleLightBox);
	int tmp = light->getXRotation();
	xRotSpinBox->setMinimum(-180);
	xRotSpinBox->setMaximum(180);
	xRotSpinBox->setValue((int)light->getXRotation());
	QObject::connect(xRotSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTriangleLightXRotation()));
	xRotSpinBox->setObjectName("x_rot_spinbox");

	QObject::connect(xRotSlider, SIGNAL(valueChanged(int)), xRotSpinBox, SLOT(setValue(int)));
	QObject::connect(xRotSpinBox, SIGNAL(valueChanged(int)), xRotSlider, SLOT(setValue(int)));

	//light Y axis rotation
	QLabel *yRotSliderLabel = new QLabel(tr("Y rot"), triangleLightBox);
	yRotSliderLabel->setObjectName("y_rot_label");
	QSlider *yRotSlider = new QSlider(Qt::Horizontal, triangleLightBox);
	yRotSlider->setObjectName("y_rot_slider");
	yRotSlider->setMinimum(-180);	yRotSlider->setMaximum(180); yRotSlider->setValue(light->getYRotation());
	yRotSlider->setTickInterval(45); yRotSlider->setTickPosition(QSlider::TicksBelow);
	QObject::connect(yRotSlider, &QSlider::valueChanged, this, &LightTab::updateTriangleLightYRotation);

	QSpinBox *yRotSpinBox = new QSpinBox(triangleLightBox);
	yRotSpinBox->setMinimum(-180);
	yRotSpinBox->setMaximum(180);
	yRotSpinBox->setValue(light->getYRotation());
	QObject::connect(yRotSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTriangleLightYRotation()));
	yRotSpinBox->setObjectName("y_rot_spinbox");

	QObject::connect(yRotSlider, SIGNAL(valueChanged(int)), yRotSpinBox, SLOT(setValue(int)));
	QObject::connect(yRotSpinBox, SIGNAL(valueChanged(int)), yRotSlider, SLOT(setValue(int)));
	//light Z axis rotation
	QLabel *zRotSliderLabel = new QLabel(tr("Z rot"), triangleLightBox);
	zRotSliderLabel->setObjectName("z_rot_label");
	QSlider *zRotSlider = new QSlider(Qt::Horizontal, triangleLightBox);
	zRotSlider->setObjectName("z_rot_slider");
	zRotSlider->setMinimum(-180);	zRotSlider->setMaximum(180); zRotSlider->setValue(light->getXRotation());
	zRotSlider->setTickInterval(45); zRotSlider->setTickPosition(QSlider::TicksBelow);
	QObject::connect(zRotSlider, &QSlider::valueChanged, this, &LightTab::updateTriangleLightZRotation);

	QSpinBox *zRotSpinBox = new QSpinBox(triangleLightBox);
	zRotSpinBox->setMinimum(-180);
	zRotSpinBox->setMaximum(180);
	zRotSpinBox->setValue(light->getZRotation());
	QObject::connect(zRotSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTriangleLightZRotation()));
	zRotSpinBox->setObjectName("z_rot_spinbox");

	QObject::connect(zRotSlider, SIGNAL(valueChanged(int)), zRotSpinBox, SLOT(setValue(int)));
	QObject::connect(zRotSpinBox, SIGNAL(valueChanged(int)), zRotSlider, SLOT(setValue(int)));


	//REMOVE BUTTON
	QPushButton *removeButton = new QPushButton("Remove", triangleLightBox);
	removeButton->setObjectName("remove_button");
	QObject::connect(removeButton, &QPushButton::released, this, &LightTab::removeLight);

	triangleLightLayout->addWidget(lightNameLabel, 0, 0);
	triangleLightLayout->addWidget(lightNameEdit, 0, 1, 1, 2);
	triangleLightLayout->addWidget(radianceLabel, 1, 0);
	triangleLightLayout->addWidget(xRadianceEdit, 1, 1);
	triangleLightLayout->addWidget(yRadianceEdit, 1, 2);
	triangleLightLayout->addWidget(zRadianceEdit, 1, 3);
	triangleLightLayout->addWidget(positionLabel, 2, 0);
	triangleLightLayout->addWidget(xPosEdit, 2, 1);
	triangleLightLayout->addWidget(yPosEdit, 2, 2);
	triangleLightLayout->addWidget(zPosEdit, 2, 3);
	triangleLightLayout->addWidget(scaleLabel, 3, 0);
	triangleLightLayout->addWidget(xScaleEdit, 3, 1);
	triangleLightLayout->addWidget(yScaleEdit, 3, 2);
	triangleLightLayout->addWidget(zScaleEdit, 3, 3);
	triangleLightLayout->addWidget(rotationAxisLabel, 4, 0);
	triangleLightLayout->addWidget(xRotAxisEdit, 4, 1);
	triangleLightLayout->addWidget(yRotAxisEdit, 4, 2);
	triangleLightLayout->addWidget(zRotAxisEdit, 4, 3);
	triangleLightLayout->addWidget(rotationAngleLabel, 5, 0);
	triangleLightLayout->addWidget(rotAngleEdit, 5, 1);
	triangleLightLayout->addWidget(removeButton, 0, 3);
	triangleLightLayout->addWidget(xRotSliderLabel, 6, 0);
	triangleLightLayout->addWidget(xRotSlider, 6, 1);
	triangleLightLayout->addWidget(xRotSpinBox, 6, 2);
	triangleLightLayout->addWidget(yRotSliderLabel, 7, 0);
	triangleLightLayout->addWidget(yRotSlider, 7, 1);
	triangleLightLayout->addWidget(yRotSpinBox, 7, 2);
	triangleLightLayout->addWidget(zRotSliderLabel, 8, 0);
	triangleLightLayout->addWidget(zRotSlider, 8, 1);
	triangleLightLayout->addWidget(zRotSpinBox, 8, 2);

	triangleLightBox->setLayout(triangleLightLayout);
	lightWidgetVector.append(triangleLightBox);
	triangleLightBox->hide();
}

void LightTab::changeLightLayout(int newSelectedLight)
{
	QGroupBox *prevTmp = lightWidgetVector[newSelectedLight];
	QGroupBox *currentTmp = lightWidgetVector[currentLight];
	QObjectList list = currentTmp->children();

	lightTabLayout->removeWidget(lightWidgetVector[currentLight]);
	lightWidgetVector[currentLight]->hide();
	
	lightTabLayout->addWidget(lightWidgetVector[newSelectedLight]);
	lightWidgetVector[newSelectedLight]->show();
	lightTabLayout->update();
	currentLight = newSelectedLight;
}

void LightTab::updatePointLightPosition()
{
	float pos_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_x")->text().toFloat();
	float pos_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_y")->text().toFloat();
	float pos_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_z")->text().toFloat();
	reinterpret_cast<PointLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->position = optix::make_float3(pos_x, pos_y, pos_z);
	reinterpret_cast<PointLight*>(optixWindow->getScene()->getLights()->data()[currentLight])->setPosition(QVector3D(pos_x, pos_y, pos_z));
	//emit updated_pointlight_position(QVector3D(pos_x, pos_y, pos_z));
	optixWindow->getScene()->updateLights(false);
	optixWindow->restartFrame();
}

void LightTab::updatePointLightRadiance()
{
	float rad_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_x")->text().toFloat();
	float rad_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_y")->text().toFloat();
	float rad_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_z")->text().toFloat();
	reinterpret_cast<PointLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->emitted_radiance = optix::make_float3(rad_x, rad_y, rad_z);
	reinterpret_cast<PointLight*>(optixWindow->getScene()->getLights()->data()[currentLight])->setRadiance(QVector3D(rad_x, rad_y, rad_z));
	//emit updated_pointlight_radiance(QVector3D(rad_x, rad_y, rad_z));
	optixWindow->getScene()->updateLights(false);
	optixWindow->restartFrame();
}

void LightTab::updateDirectionalLightDirection()
{
	float dir_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("dir_x")->text().toFloat();
	float dir_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("dir_y")->text().toFloat();
	float dir_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("dir_z")->text().toFloat();
	reinterpret_cast<DirectionalLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->direction = optix::make_float3(dir_x, dir_y, dir_z);
	reinterpret_cast<DirectionalLight*> (optixWindow->getScene()->getLights()->data()[currentLight])->setDirection(QVector3D(dir_x, dir_y, dir_z));
	//emit updated_directionallight_direction(QVector3D(dir_x, dir_y, dir_z));
	optixWindow->getScene()->updateLights(false);
	optixWindow->restartFrame();
}

void LightTab::updateDirectionalLightRadiance() 
{
	float rad_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_x")->text().toFloat();
	float rad_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_y")->text().toFloat();
	float rad_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_z")->text().toFloat();
	reinterpret_cast<DirectionalLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->emitted_radiance = optix::make_float3(rad_x, rad_y, rad_z);
	reinterpret_cast<DirectionalLight*> (optixWindow->getScene()->getLights()->data()[currentLight])->setRadiance(QVector3D(rad_x, rad_y, rad_z));
	//emit updated_directionallight_radiance(QVector3D(rad_x, rad_y, rad_z));
	optixWindow->getScene()->updateLights(false);
	optixWindow->restartFrame();
}


void LightTab::updateTriangleLightRadiance()
{
	float rad_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_x")->text().toFloat();
	float rad_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_y")->text().toFloat();
	float rad_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_z")->text().toFloat();
	reinterpret_cast<TrianglesAreaLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->emitted_radiance = optix::make_float3(rad_x, rad_y, rad_z);
	reinterpret_cast<TriangleAreaLight*> (optixWindow->getScene()->getLights()->data()[currentLight])->setRadiance(QVector3D(rad_x, rad_y, rad_z));
	//emit updated_trianglelight_radiance(QVector3D(rad_x, rad_y, rad_z));
	optixWindow->getScene()->updateLights(false);
	optixWindow->restartFrame();
}

void LightTab::updateTriangleLightPosition()
{
	float pos_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_x")->text().toFloat();
	float pos_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_y")->text().toFloat();
	float pos_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_z")->text().toFloat();
	reinterpret_cast<TriangleAreaLight*> (optixWindow->getScene()->getLights()->data()[currentLight])->setTranslation(QVector3D(pos_x, pos_y, pos_z));
	//emit updated_trianglelight_position(QVector3D(pos_x, pos_y, pos_z));
	optixWindow->getScene()->updateLights(true);
	optixWindow->getScene()->updateAcceleration();
	
	optixWindow->restartFrame();
}

void LightTab::updateTriangleLightScale()
{
	float scale_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("scale_x")->text().toFloat();
	float scale_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("scale_y")->text().toFloat();
	float scale_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("scale_z")->text().toFloat();
	reinterpret_cast<TriangleAreaLight*> (optixWindow->getScene()->getLights()->data()[currentLight])->setScale(QVector3D(scale_x, scale_y, scale_z));
	optixWindow->getScene()->updateLights(true);
	optixWindow->getScene()->updateAcceleration();
	
	optixWindow->restartFrame();
}

void LightTab::updateTriangleLightRotation()
{
	float rot_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("rot_axis_x")->text().toFloat();
	float rot_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("rot_axis_y")->text().toFloat();
	float rot_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("rot_axis_z")->text().toFloat();
	float angle = lightWidgetVector[currentLight]->findChild<QLineEdit*>("rot_angle")->text().toFloat();
	reinterpret_cast<TriangleAreaLight*> (optixWindow->getScene()->getLights()->data()[currentLight])->setRotation(angle, QVector3D(rot_x, rot_y, rot_z));
	optixWindow->getScene()->updateLights(true);
	optixWindow->getScene()->updateAcceleration();
	
	optixWindow->restartFrame();
}

void LightTab::updateTriangleLightXRotation()
{
	float angle = lightWidgetVector[currentLight]->findChild<QSpinBox*>("x_rot_spinbox")->value();
	reinterpret_cast<TriangleAreaLight*> (optixWindow->getScene()->getLights()->data()[currentLight])->setRotationX(angle);
	optixWindow->getScene()->updateLights(true);
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void LightTab::updateTriangleLightYRotation()
{
	float angle = lightWidgetVector[currentLight]->findChild<QSpinBox*>("y_rot_spinbox")->value();
	reinterpret_cast<TriangleAreaLight*> (optixWindow->getScene()->getLights()->data()[currentLight])->setRotationY(angle);
	optixWindow->getScene()->updateLights(true);
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void LightTab::updateTriangleLightZRotation()
{
	float angle = lightWidgetVector[currentLight]->findChild<QSpinBox*>("z_rot_spinbox")->value();
	reinterpret_cast<TriangleAreaLight*> (optixWindow->getScene()->getLights()->data()[currentLight])->setRotationZ(angle);
	optixWindow->getScene()->updateLights(true);
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}


void LightTab::updateDiskLightPosition()
{
	float pos_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_x")->text().toFloat();
	float pos_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_y")->text().toFloat();
	float pos_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_z")->text().toFloat();
	reinterpret_cast<DiskLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->position = optix::make_float3(pos_x, pos_y, pos_z);
	reinterpret_cast<DiskAreaLight*>(optixWindow->getScene()->getLights()->data()[currentLight])->setPosition(QVector3D(pos_x, pos_y, pos_z));
	optixWindow->getScene()->updateLights(false);
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void LightTab::updateDiskLightRadiance()
{
	float rad_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_x")->text().toFloat();
	float rad_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_y")->text().toFloat();
	float rad_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_z")->text().toFloat();
	reinterpret_cast<DiskLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->emitted_radiance = optix::make_float3(rad_x, rad_y, rad_z);
	reinterpret_cast<DiskAreaLight*>(optixWindow->getScene()->getLights()->data()[currentLight])->setRadiance(QVector3D(rad_x, rad_y, rad_z));
	optixWindow->getScene()->updateLights(false);
	optixWindow->restartFrame();
}

void LightTab::updateDiskLightRadius()
{
	float radius = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radius")->text().toFloat();
	reinterpret_cast<DiskLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->radius = radius;
	reinterpret_cast<DiskAreaLight*>(optixWindow->getScene()->getLights()->data()[currentLight])->setRadius(radius);
	optixWindow->getScene()->updateLights(false);
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void LightTab::updateDiskLightNormal()
{
	float theta = lightWidgetVector[currentLight]->findChild<QLineEdit*>("polar")->text().toFloat();
	float phi = lightWidgetVector[currentLight]->findChild<QLineEdit*>("azimuth")->text().toFloat();
	reinterpret_cast<DiskLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->theta = theta;
	reinterpret_cast<DiskLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->phi = phi;
	reinterpret_cast<DiskAreaLight*>(optixWindow->getScene()->getLights()->data()[currentLight])->setNormal(theta, phi);
	optixWindow->getScene()->updateLights(false);
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void LightTab::updateSphericalLightPosition()
{
	float pos_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_x")->text().toFloat();
	float pos_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_y")->text().toFloat();
	float pos_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("pos_z")->text().toFloat();
	reinterpret_cast<SphericalLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->position = optix::make_float3(pos_x, pos_y, pos_z);
	reinterpret_cast<SphericalLight*>(optixWindow->getScene()->getLights()->data()[currentLight])->setPosition(QVector3D(pos_x, pos_y, pos_z));
	optixWindow->getScene()->updateLights(false);
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void LightTab::updateSphericalLightRadiance()
{
	float rad_x = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_x")->text().toFloat();
	float rad_y = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_y")->text().toFloat();
	float rad_z = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radiance_z")->text().toFloat();
	reinterpret_cast<SphericalLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->emitted_radiance = optix::make_float3(rad_x, rad_y, rad_z);
	reinterpret_cast<SphericalLight*>(optixWindow->getScene()->getLights()->data()[currentLight])->setRadiance(QVector3D(rad_x, rad_y, rad_z));
	optixWindow->getScene()->updateLights(false);
	optixWindow->restartFrame();
}

void LightTab::updateSphericalLightRadius()
{
	float radius = lightWidgetVector[currentLight]->findChild<QLineEdit*>("radius")->text().toFloat();
	reinterpret_cast<SphericalLightStruct*> (&optixWindow->getScene()->getLightStructs()->data()[currentLight])->radius = radius;
	reinterpret_cast<SphericalLight*>(optixWindow->getScene()->getLights()->data()[currentLight])->setRadius(radius);
	optixWindow->getScene()->updateLights(false);
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}



void LightTab::addLight()
{
	QComboBox* comboBox = commonSettingBox->findChild<QComboBox*>("light_combo_box");
	int selectedLight = comboBox->currentIndex();
	
	if (selectedLight == POINT_LIGHT) {
		PointLight *light = new PointLight(optixWindow->getContext());
		optixWindow->getScene()->addLight(light);
		PointLightStruct *  lightStruct = reinterpret_cast<PointLightStruct*> (&optixWindow->getScene()->getLightStructs()->last());
		optixWindow->getScene()->updateLights(false);
		addPointLightParameters(light, lightStruct);
	}
	else if (selectedLight == DIRECTIONAL_LIGHT) {
		DirectionalLight * light = new DirectionalLight(optixWindow->getContext());
		optixWindow->getScene()->addLight(light);
		DirectionalLightStruct *  lightStruct = reinterpret_cast<DirectionalLightStruct*> (&optixWindow->getScene()->getLightStructs()->last());
		optixWindow->getScene()->updateLights(false);
		addDirectionalLightParameters(light, lightStruct);
	}
	else if (selectedLight == TRIANGLES_AREA_LIGHT) {
		
		QString path = QFileDialog::getOpenFileName(this,
			tr("Open OBJ file"), "../../data", tr("Obj file (*.obj)"));
		if (path.isNull() || path.isEmpty()) {
			return;
		}
		TriangleAreaLight *light = new TriangleAreaLight(optixWindow->getContext());
		light->setPath(path);
		
		optixWindow->getScene()->addLight(light);
		TrianglesAreaLightStruct *  lightStruct = reinterpret_cast<TrianglesAreaLightStruct*> (&optixWindow->getScene()->getLightStructs()->last());
		optixWindow->getScene()->updateLights(true);
		addTriangleAreaLightParameters(light, lightStruct);
	}
	else if (selectedLight == DISK_LIGHT) {
		DiskAreaLight *light = new DiskAreaLight(optixWindow->getContext());
		optixWindow->getScene()->addLight(light);
		DiskLightStruct * lightStruct = reinterpret_cast<DiskLightStruct*> (&optixWindow->getScene()->getLightStructs()->last());
		optixWindow->getScene()->updateLights(false);
		addDiskAreaLightParameters(light, lightStruct);
	}
	else if (selectedLight == SPHERICAL_LIGHT) {
		SphericalLight *light = new SphericalLight(optixWindow->getContext());
		optixWindow->getScene()->addLight(light);
		SphericalLightStruct * lightStruct = reinterpret_cast<SphericalLightStruct*> (&optixWindow->getScene()->getLightStructs()->last());
		optixWindow->getScene()->updateLights(false);
		addSphericalLightParameters(light, lightStruct);
	}
	optixWindow->restartFrame();
	if (currentLight >= 0) {
		lightTabLayout->removeWidget(lightWidgetVector[currentLight]);
		lightWidgetVector[currentLight]->hide();
	}
	updateLightList();
	
}

void LightTab::removeLight()
{
	unsigned int lightType = optixWindow->getScene()->getLights()->data()[currentLight]->getType();
	optixWindow->getScene()->removeLight(currentLight);

	QGroupBox* widgetTmp = lightWidgetVector[currentLight];
	lightTabLayout->removeWidget(widgetTmp);
	widgetTmp->hide();
	lightWidgetVector.remove(currentLight);

	optixWindow->restartFrame();
	updateLightList();
	//if (lightType == TRIANGLES_AREA_LIGHT) {
	//	optixWindow->getScene()->updateLights(true);
	//}
	//else {
	//	optixWindow->getScene()->updateLights(false);
	//}

	delete widgetTmp;

}

void LightTab::updateLightList()
{
	
	QListWidget* listWidget = commonSettingBox->findChild<QListWidget*>("list_widget");
	//block signals to avoid exception when the list is cleared (change row signal is emitted!)
	listWidget->blockSignals(true);
	int size = optixWindow->getScene()->getLights()->size();
	/*listWidget->*/
	int listSize = listWidget->count();
	for (int idx = 0; idx < listSize; idx++) {
		QString element = listWidget->item(idx)->text();
	}
	listWidget->clear();

	for (int idx = 0; idx <size; idx++)
	{
		listWidget->addItem(lightNames[optixWindow->getScene()->getLights()->data()[idx]->getType()]);

	}
	listWidget->blockSignals(false);
	currentLight = size - 1;
	if (size > 0) {
		listWidget->item(currentLight)->setSelected(true);
		lightTabLayout->addWidget(lightWidgetVector[currentLight]);
		lightWidgetVector[currentLight]->show();
		lightTabLayout->update();
	}
	
}
