#include "CameraTabGui.h"



CameraTab::CameraTab(OptixWindow* optix_window, QWidget *parent)
	: QWidget(parent)
{
	optixWindow = optix_window;

	QObject::connect(optixWindow, &OptixWindow::resized_window, this, &CameraTab::updateResizeLabels);
	QObject::connect(this, &CameraTab::resized_window, optixWindow, &OptixWindow::externalResize);
	initCameraTab();
}

void CameraTab::initCameraTab()
{

	
	if (optixWindow->getScene()->getCamera()->getType() == PINHOLE_CAMERA) {
		initPinholeCameraParameters(reinterpret_cast<PinholeCamera*> (optixWindow->getScene()->getCamera()));
	}

	setLayout(cameraTabLayout);
}

void CameraTab::initPinholeCameraParameters(PinholeCamera* camera) {
	

	/*while ( QWidget* w = cameraLabelWidget->findChild<QWidget*>() )
		delete w;
	while (QWidget* w = cameraEditWidget->findChild<QWidget*>())
		delete w;
*/
	cameraTabLayout = new QGridLayout();

	QLabel *cameraNameLabel = new QLabel(tr("Camera Type"), this);
	cameraNameLabel->setObjectName("camera_label");
	QComboBox *cameraBox = new QComboBox(this);
	cameraBox->setObjectName("cameras");
	for (int idx = 0; idx < NUMBER_OF_CAMERAS; idx++) {
		cameraBox->addItem(cameraNames[idx]);
	}
	cameraBox->setCurrentIndex(camera->getType());
	
	//WIDTH AND HEIGHT WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *widthLabel= new QLabel(tr("Width"), this);
	widthLabel->setObjectName("width_label");
	QLineEdit *widthEdit = new QLineEdit(QString::number(camera->getWidth()), this);
	widthEdit->setObjectName("width");
	QLabel *heightLabel = new QLabel(tr("Height"), this);
	heightLabel->setObjectName("height_label");
	QLineEdit *heightEdit = new QLineEdit(QString::number(camera->getHeight()), this);
	heightEdit->setObjectName("height");
	QObject::connect(widthEdit, &QLineEdit::returnPressed, this, &CameraTab::resizeCamera);
	QObject::connect(heightEdit, &QLineEdit::returnPressed, this, &CameraTab::resizeCamera);

	//PATCH WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *patchOriginLabel = new QLabel(tr("Patch Origin"), this);
	patchOriginLabel->setObjectName("patch_origin_label");
	QLineEdit *xPatchOriginEdit = new QLineEdit(QString::number(camera->getPatchOrigin().x), this);
	QLineEdit *yPatchOriginEdit = new QLineEdit(QString::number(camera->getPatchOrigin().y), this);
	xPatchOriginEdit->setObjectName("patch_origin_x");
	yPatchOriginEdit->setObjectName("patch_origin_y");
	QObject::connect(xPatchOriginEdit, &QLineEdit::returnPressed, this, &CameraTab::updatePatchOrigin);
	QObject::connect(yPatchOriginEdit, &QLineEdit::returnPressed, this, &CameraTab::updatePatchOrigin);

	QLabel *patchDimsLabel = new QLabel(tr("Patch Size"), this);
	patchDimsLabel->setObjectName("patch_dims_label");
	QLineEdit *xPatchDimsEdit = new QLineEdit(QString::number(camera->getPatchDims().x), this);
	QLineEdit *yPatchDimsEdit = new QLineEdit(QString::number(camera->getPatchDims().y), this);
	xPatchDimsEdit->setObjectName("patch_dims_x");
	yPatchDimsEdit->setObjectName("patch_dims_y");
	QObject::connect(xPatchDimsEdit, &QLineEdit::returnPressed, this, &CameraTab::updatePatchDims);
	QObject::connect(yPatchDimsEdit, &QLineEdit::returnPressed, this, &CameraTab::updatePatchDims);
	//CAMERA EYE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *eyeLabel = new QLabel(tr("Eye"), this);
	eyeLabel->setObjectName("eye_label");
	QLineEdit *xEyeEdit = new QLineEdit(QString::number(camera->getEye().x()), this);
	QLineEdit *yEyeEdit = new QLineEdit(QString::number(camera->getEye().y()), this);
	QLineEdit *zEyeEdit = new QLineEdit(QString::number(camera->getEye().z()), this);
	xEyeEdit->setObjectName("eye_x");
	yEyeEdit->setObjectName("eye_y");
	zEyeEdit->setObjectName("eye_z");
	QObject::connect(xEyeEdit, &QLineEdit::returnPressed, this, &CameraTab::updateEye);
	QObject::connect(yEyeEdit, &QLineEdit::returnPressed, this, &CameraTab::updateEye);
	QObject::connect(zEyeEdit, &QLineEdit::returnPressed, this, &CameraTab::updateEye);
	QObject::connect(this, &CameraTab::updated_eye, camera, &PinholeCamera::setEye);

	//CAMERA LOOKAT WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *lookatLabel = new QLabel(tr("Lookat"), this);
	lookatLabel->setObjectName("lookat_label");
	QLineEdit *xLookatEdit = new QLineEdit(QString::number(camera->getLookat().x()), this);
	QLineEdit *yLookatEdit = new QLineEdit(QString::number(camera->getLookat().y()), this);
	QLineEdit *zLookatEdit = new QLineEdit(QString::number(camera->getLookat().z()), this);
	xLookatEdit->setObjectName("lookat_x");
	yLookatEdit->setObjectName("lookat_y");
	zLookatEdit->setObjectName("lookat_z");
	QObject::connect(xLookatEdit, &QLineEdit::returnPressed, this, &CameraTab::updateLookat);
	QObject::connect(yLookatEdit, &QLineEdit::returnPressed, this, &CameraTab::updateLookat);
	QObject::connect(zLookatEdit, &QLineEdit::returnPressed, this, &CameraTab::updateLookat);
	QObject::connect(this, &CameraTab::updated_lookat, camera, &PinholeCamera::setLookat);
	
	//CAMERA UP WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *upLabel = new QLabel(tr("Up"), this);
	upLabel->setObjectName("up_label");
	QLineEdit *xUpEdit = new QLineEdit(QString::number(camera->getUp().x()), this);
	QLineEdit *yUpEdit = new QLineEdit(QString::number(camera->getUp().y()), this);
	QLineEdit *zUpEdit = new QLineEdit(QString::number(camera->getUp().z()), this);
	xUpEdit->setObjectName("up_x");
	yUpEdit->setObjectName("up_y");
	zUpEdit->setObjectName("up_z");
	QObject::connect(xUpEdit, &QLineEdit::returnPressed, this, &CameraTab::updateUp);
	QObject::connect(yUpEdit, &QLineEdit::returnPressed, this, &CameraTab::updateUp);
	QObject::connect(zUpEdit, &QLineEdit::returnPressed, this, &CameraTab::updateUp);
	QObject::connect(this, &CameraTab::updated_up, camera, &PinholeCamera::setUp);
	
	//CAMERA VFOV WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *vfovLabel = new QLabel(tr("Vfov"), this);
	vfovLabel->setObjectName("vfov_label");
	QLineEdit *vfovEdit = new QLineEdit(QString::number(camera->getFov()), this);
	vfovEdit->setObjectName("vfov");
	QObject::connect(vfovEdit, &QLineEdit::returnPressed, this, &CameraTab::updateFov);
	QObject::connect(this, &CameraTab::updated_vfov, camera, &PinholeCamera::setFov);

	//QLineEdit *vfovEdit = createNewEditElement(QString::number(camera->getFov()), this, "vfov",
	//	SIGNAL(editingFinished()), SLOT(updateFov()), SIGNAL(updated_vfov(float)),
	//	camera, SLOT(setFov(float)));

	
	cameraTabLayout->addWidget(cameraNameLabel, camera_label_row, 0);
	cameraTabLayout->addWidget(cameraBox, camera_label_row, 1);
	cameraTabLayout->addWidget(widthLabel, width_height_row, 0);
	cameraTabLayout->addWidget(widthEdit, width_height_row, 1);
	cameraTabLayout->addWidget(heightLabel, width_height_row, 2);
	cameraTabLayout->addWidget(heightEdit, width_height_row, 3);
	cameraTabLayout->addWidget(patchOriginLabel, patch_origin_row, 0);
	cameraTabLayout->addWidget(xPatchOriginEdit, patch_origin_row, 1);
	cameraTabLayout->addWidget(yPatchOriginEdit, patch_origin_row, 2);
	cameraTabLayout->addWidget(patchDimsLabel, patch_dims_row, 0);
	cameraTabLayout->addWidget(xPatchDimsEdit, patch_dims_row, 1);
	cameraTabLayout->addWidget(yPatchDimsEdit, patch_dims_row, 2);
	cameraTabLayout->addWidget(eyeLabel, eye_row, 0);
	cameraTabLayout->addWidget(xEyeEdit, eye_row, 1);
	cameraTabLayout->addWidget(yEyeEdit, eye_row, 2);
	cameraTabLayout->addWidget(zEyeEdit, eye_row, 3);
	cameraTabLayout->addWidget(lookatLabel, lookat_row, 0);
	cameraTabLayout->addWidget(xLookatEdit, lookat_row, 1);
	cameraTabLayout->addWidget(yLookatEdit, lookat_row, 2);
	cameraTabLayout->addWidget(zLookatEdit, lookat_row, 3);
	cameraTabLayout->addWidget(upLabel, up_row, 0);
	cameraTabLayout->addWidget(xUpEdit, up_row, 1);
	cameraTabLayout->addWidget(yUpEdit, up_row, 2);
	cameraTabLayout->addWidget(zUpEdit, up_row, 3);
	cameraTabLayout->addWidget(vfovLabel, fov_row, 0);
	cameraTabLayout->addWidget(vfovEdit, fov_row, 1);
}

void CameraTab::updateResizeLabels(int width, int height)
{
	this->findChild<QLineEdit*>("width")->setText(QString::number(width));
	this->findChild<QLineEdit*>("height")->setText(QString::number(height));
	this->findChild<QLineEdit*>("patch_dims_x")->setText(QString::number(width));
	this->findChild<QLineEdit*>("patch_dims_y")->setText(QString::number(height));
}

void CameraTab::resizeCamera()
{
	int width = this->findChild<QLineEdit*>("width")->text().toInt();
	int height = this->findChild<QLineEdit*>("height")->text().toInt();
	emit resized_window(width, height);
}

void CameraTab::updateEye()
{
	float eye_x = this->findChild<QLineEdit*>("eye_x")->text().toFloat();
	float eye_y = this->findChild<QLineEdit*>("eye_y")->text().toFloat();
	float eye_z = this->findChild<QLineEdit*>("eye_z")->text().toFloat();
	emit updated_eye(QVector3D(eye_x, eye_y, eye_z));
	optixWindow->restartFrame();
}
void CameraTab::updateLookat()
{
	float lookat_x = this->findChild<QLineEdit*>("lookat_x")->text().toFloat();
	float lookat_y = this->findChild<QLineEdit*>("lookat_y")->text().toFloat();
	float lookat_z = this->findChild<QLineEdit*>("lookat_z")->text().toFloat();
	emit updated_lookat(QVector3D(lookat_x, lookat_y, lookat_z));
	optixWindow->restartFrame();
}
void CameraTab::updateUp()
{
	float up_x = this->findChild<QLineEdit*>("up_x")->text().toFloat();
	float up_y = this->findChild<QLineEdit*>("up_y")->text().toFloat();
	float up_z = this->findChild<QLineEdit*>("up_z")->text().toFloat();
	emit updated_up(QVector3D(up_x, up_y, up_z));
	optixWindow->restartFrame();
}

void CameraTab::updateFov()
{
	float vfov = this->findChild<QLineEdit*>("vfov")->text().toFloat();

	emit updated_vfov(vfov);
	optixWindow->restartFrame();
}

void CameraTab::updatePatchOrigin()
{
	int patch_x = this->findChild<QLineEdit*>("patch_origin_x")->text().toInt();
	int patch_y = this->findChild<QLineEdit*>("patch_origin_y")->text().toInt();
	optix::uint2 patch_origin = optix::make_uint2(patch_x, patch_y);
	reinterpret_cast<PinholeCamera*>(optixWindow->getScene()->getCamera())->setPatchOrigin(patch_origin);
	optixWindow->restartFrame();
}

void CameraTab::updatePatchDims()
{
	float patch_x = this->findChild<QLineEdit*>("patch_dims_x")->text().toInt();
	float patch_y = this->findChild<QLineEdit*>("patch_dims_y")->text().toInt();
	optix::uint2 patch_dims = optix::make_uint2(patch_x, patch_y);
	reinterpret_cast<PinholeCamera*>(optixWindow->getScene()->getCamera())->setPatchDims(patch_dims);

	optixWindow->restartFrame();
}

//QLineEdit* CameraTab::createNewEditElement(QString text, QWidget *parent, QString widgetName,
//	const char *widget_signal, const char *local_method, const char *local_signal,
//	const QObject *external_receiver, const char *external_method)
//{
//	QLineEdit *editElement = new QLineEdit(text, parent);
//	editElement->setObjectName(widgetName);
//	QObject::connect(editElement, widget_signal, this, local_method);
//	QObject::connect(this, local_signal, external_receiver, external_method);
//	return editElement;
//}