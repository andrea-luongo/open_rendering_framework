#include "GeometryTabGui.h"



GeometryTab::GeometryTab(OptixWindow* optix_window, QWidget *parent)
	: QWidget(parent)
{
	optixWindow = optix_window;
	geometryTabLayout = new QHBoxLayout();
	setLayout(geometryTabLayout);
	initGeometryTab();
}

void GeometryTab::initGeometryTab()
{
	geometryWidgetVector.clear();
	geometryWidgetVector.reserve(optixWindow->getScene()->getGeometries()->size());

	commonSettingBox = new QGroupBox;
	QGridLayout *commonSettingsLayout = new QGridLayout();
	commonSettingBox->setLayout(commonSettingsLayout);

	QListWidget *listWidget = new QListWidget(commonSettingBox);
	listWidget->setObjectName("list_widget");
	listWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
	listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	QObject::connect(listWidget, &QListWidget::currentRowChanged, this, &GeometryTab::changeGeometryLayout);

	QPushButton *addButton = new QPushButton("Add Geometry", commonSettingBox);
	addButton->setObjectName("add_button");
	QObject::connect(addButton, &QPushButton::released, this, &GeometryTab::addGeometry);

	commonSettingsLayout->addWidget(listWidget, 0, 0, 4, 2);
	commonSettingsLayout->addWidget(addButton, 4, 0);
	//initialize lights layout;
	for (int idx = 0; idx < optixWindow->getScene()->getGeometries()->size(); idx++)
	{
		if (optixWindow->getScene()->getGeometries()->data()[idx]->getType() == OBJ) {
			addOBJParameters(reinterpret_cast<OBJGeometry*> (optixWindow->getScene()->getGeometries()->data()[idx]));
		} 
	}

	geometryTabLayout->addWidget(commonSettingBox);
	updateGeometryList();
}

void GeometryTab::addOBJParameters(OBJGeometry* geometry)
{
	QGroupBox *objBox = new QGroupBox;
	QGridLayout *objLayout = new QGridLayout();
	QLabel *pathLabel = new QLabel(tr("FileName"), objBox);
	pathLabel->setObjectName("path_label");
	QLineEdit *pathEdit = new QLineEdit(geometry->getName(), objBox);
	pathEdit->setObjectName("light_name");
	pathEdit->setReadOnly(true);

	//LIGHT POSITION WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *positionLabel = new QLabel(tr("Position"), objBox);
	positionLabel->setObjectName("position_label");
	QLineEdit *xPosEdit = new QLineEdit(QString::number(geometry->getPosition().x), objBox);
	QLineEdit *yPosEdit = new QLineEdit(QString::number(geometry->getPosition().y), objBox);
	QLineEdit *zPosEdit = new QLineEdit(QString::number(geometry->getPosition().z), objBox);
	xPosEdit->setObjectName("pos_x");
	yPosEdit->setObjectName("pos_y");
	zPosEdit->setObjectName("pos_z");
	QObject::connect(xPosEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryPosition);
	QObject::connect(yPosEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryPosition);
	QObject::connect(zPosEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryPosition);


	//LIGHT SCALE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *scaleLabel = new QLabel(tr("Scale"), objBox);
	scaleLabel->setObjectName("scale_label");
	QLineEdit *xScaleEdit = new QLineEdit(QString::number(geometry->getScale().x), objBox);
	QLineEdit *yScaleEdit = new QLineEdit(QString::number(geometry->getScale().y), objBox);
	QLineEdit *zScaleEdit = new QLineEdit(QString::number(geometry->getScale().z), objBox);
	xScaleEdit->setObjectName("scale_x");
	yScaleEdit->setObjectName("scale_y");
	zScaleEdit->setObjectName("scale_z");
	QObject::connect(xScaleEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryScale);
	QObject::connect(yScaleEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryScale);
	QObject::connect(zScaleEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryScale);


	//LIGHT ROTATION WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *rotationAxisLabel = new QLabel(tr("Rotation Axis"), objBox);
	rotationAxisLabel->setObjectName("rotation_axis_label");
	QLineEdit *xRotAxisEdit = new QLineEdit(QString::number(geometry->getRotationAxis().x), objBox);
	QLineEdit *yRotAxisEdit = new QLineEdit(QString::number(geometry->getRotationAxis().y), objBox);
	QLineEdit *zRotAxisEdit = new QLineEdit(QString::number(geometry->getRotationAxis().z), objBox);
	xRotAxisEdit->setObjectName("rot_axis_x");
	yRotAxisEdit->setObjectName("rot_axis_y");
	zRotAxisEdit->setObjectName("rot_axis_z");
	QObject::connect(xRotAxisEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryFreeRotation);
	QObject::connect(yRotAxisEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryFreeRotation);
	QObject::connect(zRotAxisEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryFreeRotation);

	QLabel *rotationAngleLabel = new QLabel(tr("Rotation Angle"), objBox);
	rotationAngleLabel->setObjectName("rotation_angle_label");
	QLineEdit *rotAngleEdit = new QLineEdit(QString::number(geometry->getRotationAngle()), objBox);
	rotAngleEdit->setObjectName("rot_angle");
	QObject::connect(rotAngleEdit, &QLineEdit::returnPressed, this, &GeometryTab::updateGeometryFreeRotation);

	QLabel *xRotSliderLabel = new QLabel(tr("X rot"), objBox);
	xRotSliderLabel->setObjectName("x_rot_label");
	QSlider *xRotSlider = new QSlider(Qt::Horizontal, objBox);
	xRotSlider->setObjectName("x_rot_slider");
	xRotSlider->setMinimum(-180);	xRotSlider->setMaximum(180); xRotSlider->setValue(geometry->getXRotation());
	xRotSlider->setTickInterval(45); xRotSlider->setTickPosition(QSlider::TicksBelow);
	QObject::connect(xRotSlider, &QSlider::valueChanged, this, &GeometryTab::updateGeometryXRotation);

	QLabel *yRotSliderLabel = new QLabel(tr("Y rot"), objBox);
	yRotSliderLabel->setObjectName("y_rot_label");
	QSlider *yRotSlider = new QSlider(Qt::Horizontal, objBox);
	yRotSlider->setObjectName("y_rot_slider");
	yRotSlider->setMinimum(-180);	yRotSlider->setMaximum(180); yRotSlider->setValue(geometry->getYRotation());
	yRotSlider->setTickInterval(45); yRotSlider->setTickPosition(QSlider::TicksBelow);
	QObject::connect(yRotSlider, &QSlider::valueChanged, this, &GeometryTab::updateGeometryXRotation);

	QLabel *zRotSliderLabel = new QLabel(tr("Z rot"), objBox);
	zRotSliderLabel->setObjectName("z_rot_label");
	QSlider *zRotSlider = new QSlider(Qt::Horizontal, objBox);
	zRotSlider->setObjectName("z_rot_slider");
	zRotSlider->setMinimum(-180);	zRotSlider->setMaximum(180); zRotSlider->setValue(geometry->getXRotation());
	zRotSlider->setTickInterval(45); zRotSlider->setTickPosition(QSlider::TicksBelow);
	QObject::connect(zRotSlider, &QSlider::valueChanged, this, &GeometryTab::updateGeometryXRotation);


	QSpinBox *xRotSpinBox = new QSpinBox(objBox);
	int tmp = geometry->getXRotation();
	xRotSpinBox->setMinimum(-180); 
	xRotSpinBox->setMaximum(180);
	xRotSpinBox->setValue((int)geometry->getXRotation());
	QObject::connect(xRotSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateGeometryXRotation()));
	QSpinBox *yRotSpinBox = new QSpinBox(objBox);
	yRotSpinBox->setMinimum(-180);
	yRotSpinBox->setMaximum(180);
	yRotSpinBox->setValue(geometry->getYRotation());
	QObject::connect(yRotSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateGeometryYRotation()));
	QSpinBox *zRotSpinBox = new QSpinBox(objBox);
	zRotSpinBox->setMinimum(-180);
	zRotSpinBox->setMaximum(180);
	zRotSpinBox->setValue(geometry->getZRotation());
	QObject::connect(zRotSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateGeometryZRotation()));
	xRotSpinBox->setObjectName("x_rot_spinbox");
	yRotSpinBox->setObjectName("y_rot_spinbox");
	zRotSpinBox->setObjectName("z_rot_spinbox");
	QObject::connect(xRotSlider, SIGNAL(valueChanged(int)), xRotSpinBox, SLOT(setValue(int)));
	QObject::connect(xRotSpinBox, SIGNAL(valueChanged(int)), xRotSlider, SLOT(setValue(int)));
	QObject::connect(yRotSlider, SIGNAL(valueChanged(int)), yRotSpinBox, SLOT(setValue(int)));
	QObject::connect(yRotSpinBox, SIGNAL(valueChanged(int)), yRotSlider, SLOT(setValue(int)));
	QObject::connect(zRotSlider, SIGNAL(valueChanged(int)), zRotSpinBox, SLOT(setValue(int)));
	QObject::connect(zRotSpinBox, SIGNAL(valueChanged(int)), zRotSlider, SLOT(setValue(int)));

	QPushButton *removeButton = new QPushButton("Remove", objBox);
	removeButton->setObjectName("remove_button");
	QObject::connect(removeButton, &QPushButton::released, this, &GeometryTab::removeGeometry);

	QLabel *comboBoxLabel = new QLabel(tr("Material"), objBox);
	comboBoxLabel->setObjectName("combobox_label");
	QComboBox *materialComboBox = new QComboBox(objBox);
	materialComboBox->setObjectName("material_combo_box");
	for (int idx = 0; idx < NUMBER_OF_MATERIALS; idx++) {
		materialComboBox->addItem(materialNames[idx]);
	}
	int index = geometry->getMaterial()->getType();
	materialComboBox->setCurrentIndex(index);
	QObject::connect(materialComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMaterial(int)));
	QTableWidget* tableWidget = setMaterialTable(geometry->getMaterial());
	tableWidget->setParent(objBox);
	tableWidget->setObjectName("material_table");
	QObject::connect(tableWidget, &QTableWidget::cellChanged, this, &GeometryTab::tableUpdate);
	objLayout->addWidget(pathLabel, 0, 0);
	objLayout->addWidget(pathEdit, 0, 1, 1, 2);
	objLayout->addWidget(removeButton, 0, 3);
	objLayout->addWidget(positionLabel, 1, 0);
	objLayout->addWidget(xPosEdit, 1, 1);
	objLayout->addWidget(yPosEdit, 1, 2);
	objLayout->addWidget(zPosEdit, 1, 3);
	objLayout->addWidget(scaleLabel, 2, 0);
	objLayout->addWidget(xScaleEdit, 2, 1);
	objLayout->addWidget(yScaleEdit, 2, 2);
	objLayout->addWidget(zScaleEdit, 2, 3);
	objLayout->addWidget(rotationAxisLabel, 3, 0);
	objLayout->addWidget(xRotAxisEdit, 3, 1);
	objLayout->addWidget(yRotAxisEdit, 3, 2);
	objLayout->addWidget(zRotAxisEdit, 3, 3);
	objLayout->addWidget(rotationAngleLabel, 4, 0);
	objLayout->addWidget(rotAngleEdit, 4, 1);
	objLayout->addWidget(xRotSliderLabel, 5, 0);
	objLayout->addWidget(xRotSlider, 5, 1);
	objLayout->addWidget(xRotSpinBox, 5, 2);
	objLayout->addWidget(yRotSliderLabel, 6, 0);
	objLayout->addWidget(yRotSlider, 6, 1);
	objLayout->addWidget(yRotSpinBox, 6, 2);
	objLayout->addWidget(zRotSliderLabel, 7, 0);
	objLayout->addWidget(zRotSlider, 7, 1);
	objLayout->addWidget(zRotSpinBox, 7, 2);
	objLayout->addWidget(comboBoxLabel, 8, 0);
	objLayout->addWidget(materialComboBox, 8, 1); 
	objLayout->addWidget(tableWidget, 9, 0, 1, 4);
	objBox->setLayout(objLayout);
	geometryWidgetVector.append(objBox);
	objBox->hide();
}

QTableWidget* GeometryTab::setMaterialTable(MyMaterial* mtl)
{
	optix::Material optix_mtl = mtl->getOptixMaterial();
	return mtl->getParametersTable();
}

void GeometryTab::updateGeometryList()
{

	QListWidget* listWidget = commonSettingBox->findChild<QListWidget*>("list_widget");
	//block signals to avoid exception when the list is cleared (change row signal is emitted!)
	listWidget->blockSignals(true);
	int size = optixWindow->getScene()->getGeometries()->size();
	/*listWidget->*/
	int listSize = listWidget->count();
	for (int idx = 0; idx < listSize; idx++) {
		QString element = listWidget->item(idx)->text();
	}
	listWidget->clear();

	for (int idx = 0; idx <size; idx++)
	{
		listWidget->addItem(optixWindow->getScene()->getGeometries()->data()[idx]->getName());

	}
	listWidget->blockSignals(false);
	currentGeometry = size - 1;
	if (size > 0) {
		listWidget->item(currentGeometry)->setSelected(true);
		geometryTabLayout->addWidget(geometryWidgetVector[currentGeometry]);
		geometryWidgetVector[currentGeometry]->show();
		geometryTabLayout->update();
	}

}


void GeometryTab::changeGeometryLayout(int newSelectedGeometry)
{
	QGroupBox *prevTmp = geometryWidgetVector[newSelectedGeometry];
	QGroupBox *currentTmp = geometryWidgetVector[currentGeometry];
	QObjectList list = currentTmp->children();

	geometryTabLayout->removeWidget(geometryWidgetVector[currentGeometry]);
	geometryWidgetVector[currentGeometry]->hide();

	geometryTabLayout->addWidget(geometryWidgetVector[newSelectedGeometry]);
	geometryWidgetVector[newSelectedGeometry]->show();
	geometryTabLayout->update();
	currentGeometry = newSelectedGeometry;
}

void GeometryTab::updateGeometryPosition()
{
	float pos_x = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("pos_x")->text().toFloat();
	float pos_y = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("pos_y")->text().toFloat();
	float pos_z = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("pos_z")->text().toFloat();
	reinterpret_cast<OBJGeometry*> (optixWindow->getScene()->getGeometries()->data()[currentGeometry])->setPosition(QVector3D(pos_x, pos_y, pos_z));
	MaterialType mtl_type = optixWindow->getScene()->getGeometries()->data()[currentGeometry]->getMaterial()->getType();
	if (mtl_type == TRANSLUCENT_SHADER || mtl_type == ROUGH_TRANSLUCENT_SHADER) {
		optixWindow->getScene()->computeTranslucentGeometries();
	}
	
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void GeometryTab::updateGeometryScale()
{
	float scale_x = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("scale_x")->text().toFloat();
	float scale_y = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("scale_y")->text().toFloat();
	float scale_z = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("scale_z")->text().toFloat();
	reinterpret_cast<OBJGeometry*> (optixWindow->getScene()->getGeometries()->data()[currentGeometry])->setScale(QVector3D(scale_x, scale_y, scale_z));
	MaterialType mtl_type = optixWindow->getScene()->getGeometries()->data()[currentGeometry]->getMaterial()->getType();
	if (mtl_type == TRANSLUCENT_SHADER || mtl_type == ROUGH_TRANSLUCENT_SHADER) {
		optixWindow->getScene()->computeTranslucentGeometries();
	}
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void GeometryTab::updateGeometryFreeRotation()
{
	float rot_x = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("rot_axis_x")->text().toFloat();
	float rot_y = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("rot_axis_y")->text().toFloat();
	float rot_z = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("rot_axis_z")->text().toFloat();
	float angle = geometryWidgetVector[currentGeometry]->findChild<QLineEdit*>("rot_angle")->text().toFloat();
	reinterpret_cast<OBJGeometry*> (optixWindow->getScene()->getGeometries()->data()[currentGeometry])->setRotation(angle, QVector3D(rot_x, rot_y, rot_z));
	MaterialType mtl_type = optixWindow->getScene()->getGeometries()->data()[currentGeometry]->getMaterial()->getType();
	if (mtl_type == TRANSLUCENT_SHADER || mtl_type == ROUGH_TRANSLUCENT_SHADER) {
		optixWindow->getScene()->computeTranslucentGeometries();
	}
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}


void GeometryTab::updateGeometryXRotation() 
{
	float angle = geometryWidgetVector[currentGeometry]->findChild<QSpinBox*>("x_rot_spinbox")->value();
	reinterpret_cast<OBJGeometry*> (optixWindow->getScene()->getGeometries()->data()[currentGeometry])->setRotationX(angle);
	MaterialType mtl_type = optixWindow->getScene()->getGeometries()->data()[currentGeometry]->getMaterial()->getType();
	if (mtl_type == TRANSLUCENT_SHADER || mtl_type == ROUGH_TRANSLUCENT_SHADER) {
		optixWindow->getScene()->computeTranslucentGeometries();
	}
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void GeometryTab::updateGeometryYRotation() 
{
	float angle = geometryWidgetVector[currentGeometry]->findChild<QSpinBox*>("y_rot_spinbox")->value();
	reinterpret_cast<OBJGeometry*> (optixWindow->getScene()->getGeometries()->data()[currentGeometry])->setRotationY(angle);
	MaterialType mtl_type = optixWindow->getScene()->getGeometries()->data()[currentGeometry]->getMaterial()->getType();
	if (mtl_type == TRANSLUCENT_SHADER || mtl_type == ROUGH_TRANSLUCENT_SHADER) {
		optixWindow->getScene()->computeTranslucentGeometries();
	}
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void GeometryTab::updateGeometryZRotation()
{
	float angle = geometryWidgetVector[currentGeometry]->findChild<QSpinBox*>("z_rot_spinbox")->value();
	reinterpret_cast<OBJGeometry*> (optixWindow->getScene()->getGeometries()->data()[currentGeometry])->setRotationZ(angle);
	MaterialType mtl_type = optixWindow->getScene()->getGeometries()->data()[currentGeometry]->getMaterial()->getType();
	if (mtl_type == TRANSLUCENT_SHADER || mtl_type == ROUGH_TRANSLUCENT_SHADER) {
		optixWindow->getScene()->computeTranslucentGeometries();
	}
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}

void GeometryTab::addGeometry()
{
	QFileInfo fileInfo = QFileDialog::getOpenFileName(this, tr("Open file"), "../../data", tr("Obj file (*.obj)"));
	if (fileInfo.path().isNull() || fileInfo.path().isEmpty()) {
		return;
	}
	QDir dir("./");
	if (fileInfo.suffix().compare(QString("obj"), Qt::CaseInsensitive) == 0)
	{
		OBJGeometry *obj = new OBJGeometry(optixWindow->getContext(), dir.relativeFilePath(fileInfo.filePath()));
		optixWindow->getScene()->addGeometry(obj);
		addOBJParameters(obj);		
	}
	optixWindow->restartFrame();
	if (currentGeometry >= 0) {
		geometryTabLayout->removeWidget(geometryWidgetVector[currentGeometry]);
		geometryWidgetVector[currentGeometry]->hide();
	}
	updateGeometryList();
	
}
void GeometryTab::removeGeometry()
{
	unsigned int geometryType = optixWindow->getScene()->getGeometries()->data()[currentGeometry]->getType();
	optixWindow->getScene()->removeGeometry(currentGeometry);
	QGroupBox* widgetTmp = geometryWidgetVector[currentGeometry];
	geometryTabLayout->removeWidget(widgetTmp);
	widgetTmp->hide();
	geometryWidgetVector.remove(currentGeometry);
	optixWindow->restartFrame();
	updateGeometryList();
	delete widgetTmp;

}

void GeometryTab::tableUpdate(int row, int column)
{
	MaterialType mtl_type = optixWindow->getScene()->getGeometries()->data()[currentGeometry]->getMaterial()->getType();
	if (mtl_type == TRANSLUCENT_SHADER || mtl_type == ROUGH_TRANSLUCENT_SHADER) {
		optixWindow->getScene()->computeTranslucentGeometries();
	}
	
	optixWindow->getScene()->updateAcceleration();
	optixWindow->restartFrame();
}


void GeometryTab::updateMaterial(int mtl_type)
{
	reinterpret_cast<OBJGeometry*> (optixWindow->getScene()->getGeometries()->data()[currentGeometry])->loadMaterial(static_cast<MaterialType>(mtl_type));
	QTableWidget* newTableWidget = optixWindow->getScene()->getGeometries()->data()[currentGeometry]->getMaterial()->getParametersTable();
	newTableWidget->setParent(geometryWidgetVector[currentGeometry]);
	newTableWidget->setObjectName("material_table");
	QObject::connect(newTableWidget, &QTableWidget::cellChanged, this, &GeometryTab::tableUpdate);
	//QObject::connect(newTableWidget, &QTableWidget::itemChanged, this, &GeometryTab::tableUpdateTmp);
	QTableWidget* oldTableWidget = geometryWidgetVector[currentGeometry]->findChild<QTableWidget*>("material_table");
	geometryWidgetVector[currentGeometry]->layout()->replaceWidget(oldTableWidget, newTableWidget);
	oldTableWidget->deleteLater();	
	optixWindow->getScene()->computeTranslucentGeometries();
	optixWindow->restartFrame();
}
