#include "BackgroundTabGui.h"



BackgroundTab::BackgroundTab(OptixWindow* optix_window, QWidget *parent)
	: QWidget(parent)
{
	optixWindow = optix_window;
	backgroundTabLayout = new QHBoxLayout();
	
	initBackgroundTab();
	setLayout(backgroundTabLayout);
}

void BackgroundTab::initBackgroundTab()
{

	//
	if (optixWindow->getScene()->getBackground()->getType() == CONSTANT_BACKGROUND) {
		initConstantBackgroundParameters(reinterpret_cast<ConstantBackground*> (optixWindow->getScene()->getBackground()));
	} else if (optixWindow->getScene()->getBackground()->getType() == ENVMAP_BACKGROUND)
	{
		initEnvMapBackgroundParameters(reinterpret_cast<EnvMapBackground*> (optixWindow->getScene()->getBackground()));
	}
	
	//setLayout(cameraTabLayout);
}

void BackgroundTab::initConstantBackgroundParameters(ConstantBackground* background) {
	
	QGroupBox *bgGroupBox = new QGroupBox;
	bgGroupBox->setObjectName("bg_group_box");
	QGridLayout *bgLayout = new QGridLayout();
	QLabel *backgroundNameLabel = new QLabel(tr("Background Type"), bgGroupBox);
	backgroundNameLabel->setObjectName("background_label");
	QComboBox *bgBox = new QComboBox(bgGroupBox);
	bgBox->setObjectName("background_combobox");
	for (int idx = 0; idx < NUMBER_OF_BACKGROUND; idx++) {
		bgBox->addItem(backgroundNames[idx]);
	}
	bgBox->setCurrentIndex(background->getType());
	QObject::connect(bgBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeBackgroundType(int)));
	//CAMERA EYE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *bgColorLabel = new QLabel(tr("Background Color"), bgGroupBox);
	bgColorLabel->setObjectName("bg_color_label");
	QLineEdit *xColorEdit = new QLineEdit(QString::number(background->getBackgroundColor().x()), bgGroupBox);
	QLineEdit *yColorEdit = new QLineEdit(QString::number(background->getBackgroundColor().y()), bgGroupBox);
	QLineEdit *zColorEdit = new QLineEdit(QString::number(background->getBackgroundColor().z()), bgGroupBox);
	xColorEdit->setObjectName("bg_color_x");
	yColorEdit->setObjectName("bg_color_y");
	zColorEdit->setObjectName("bg_color_z");
	QObject::connect(xColorEdit, &QLineEdit::returnPressed, this, &BackgroundTab::updateBackgroundColor);
	QObject::connect(yColorEdit, &QLineEdit::returnPressed, this, &BackgroundTab::updateBackgroundColor);
	QObject::connect(zColorEdit, &QLineEdit::returnPressed, this, &BackgroundTab::updateBackgroundColor);

	bgLayout->addWidget(backgroundNameLabel, 0, 0);
	bgLayout->addWidget(bgBox, 0, 1);
	bgLayout->addWidget(bgColorLabel, 1, 0);
	bgLayout->addWidget(xColorEdit, 1, 1);
	bgLayout->addWidget(yColorEdit, 1, 2);
	bgLayout->addWidget(zColorEdit, 1, 3);
	bgGroupBox->setLayout(bgLayout);
	backgroundTabLayout->addWidget(bgGroupBox);
}


void BackgroundTab::initEnvMapBackgroundParameters(EnvMapBackground* background)
{
	QGroupBox *bgGroupBox = new QGroupBox;
	bgGroupBox->setObjectName("bg_group_box");
	QGridLayout *bgLayout = new QGridLayout();
	QLabel *backgroundNameLabel = new QLabel(tr("Background Type"), bgGroupBox);
	backgroundNameLabel->setObjectName("background_label");
	QComboBox *bgBox = new QComboBox(bgGroupBox);
	bgBox->setObjectName("background_combobox");
	for (int idx = 0; idx < NUMBER_OF_BACKGROUND; idx++) {
		bgBox->addItem(backgroundNames[idx]);
	}
	bgBox->setCurrentIndex(background->getType());
	QObject::connect(bgBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeBackgroundType(int)));
	//CAMERA EYE WIDGETS AND SIGNALS-SLOTS CONNECTIONS
	QLabel *pathLabel = new QLabel(tr("Environment Map Path"), bgGroupBox);
	pathLabel->setObjectName("envmap_path_label");
	QLineEdit *pathEdit = new QLineEdit(background->getPath(), bgGroupBox);
	pathEdit->setObjectName("envmap_path");
	pathEdit->setReadOnly(true);

	QPushButton *pathButton = new QPushButton("...", bgGroupBox);
	pathButton->setObjectName("change_path_button");
	QObject::connect(pathButton, &QPushButton::released, this, &BackgroundTab::updatePath);

	bgLayout->addWidget(backgroundNameLabel, 0, 0);
	bgLayout->addWidget(bgBox, 0, 1);
	bgLayout->addWidget(pathLabel, 1, 0);
	bgLayout->addWidget(pathEdit, 1, 1);
	bgLayout->addWidget(pathButton, 1, 2);
	bgGroupBox->setLayout(bgLayout);
	backgroundTabLayout->addWidget(bgGroupBox);
}
void BackgroundTab::updateBackgroundColor()
{
	float color_x = this->findChild<QLineEdit*>("bg_color_x")->text().toFloat();
	float color_y = this->findChild<QLineEdit*>("bg_color_y")->text().toFloat();
	float color_z = this->findChild<QLineEdit*>("bg_color_z")->text().toFloat();
	reinterpret_cast<ConstantBackground*> (optixWindow->getScene()->getBackground())->setBackgroundColor(QVector3D(color_x, color_y, color_z));
	optixWindow->restartFrame();
}

void BackgroundTab::updatePath()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Open file"), "../../data", tr("HDR file (*.hdr *.ppm)"));
	if (path.isNull() || path.isEmpty()) {
		return;
	}
	//QDir dir("./");
	//path = dir.relativeFilePath(path);
	reinterpret_cast<EnvMapBackground*> (optixWindow->getScene()->getBackground())->changePath(path);
	
	QLineEdit *pathEdit = this->findChild<QLineEdit*>("envmap_path");
	pathEdit->setText(path);
	optixWindow->restartFrame();
}

void BackgroundTab::changeBackgroundType(int bg_type)
{
	QGroupBox *bgGroupBox = this->findChild<QGroupBox*>("bg_group_box");
	bgGroupBox->hide();
	backgroundTabLayout->removeWidget(bgGroupBox);
	bgGroupBox->deleteLater();
	BackgroundType bg = static_cast<BackgroundType>(bg_type);
	if (bg == CONSTANT_BACKGROUND) {
		optixWindow->getScene()->setBackground(new ConstantBackground(optixWindow->getContext()));
		initConstantBackgroundParameters(reinterpret_cast<ConstantBackground*> (optixWindow->getScene()->getBackground()));
	}
	else if (bg == ENVMAP_BACKGROUND)
	{
		optixWindow->getScene()->setBackground(new EnvMapBackground(optixWindow->getContext()));
		initEnvMapBackgroundParameters(reinterpret_cast<EnvMapBackground*> (optixWindow->getScene()->getBackground()));
	}
	optixWindow->restartFrame();
}