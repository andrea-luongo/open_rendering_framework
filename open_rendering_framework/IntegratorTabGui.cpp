#include "IntegratorTabGui.h"



IntegratorTab::IntegratorTab(OptixWindow* optix_window, QWidget *parent)
	: QWidget(parent)
{
	optixWindow = optix_window;
	integratorTabLayout = new QHBoxLayout();

	initIntegratorTab();
	setLayout(integratorTabLayout);

}

void IntegratorTab::initIntegratorTab()
{
	if (optixWindow->getScene()->getIntegrator()->getType() == PATH_TRACER) {
		initPathTracerParameters(reinterpret_cast<PathTracer*> (optixWindow->getScene()->getIntegrator()));
	}
	if (optixWindow->getScene()->getIntegrator()->getType() == DEPTH_TRACER) {
		initDepthTracerParameters(reinterpret_cast<DepthTracer*> (optixWindow->getScene()->getIntegrator()));
	}
}

void IntegratorTab::initPathTracerParameters(PathTracer* integrator) {
	

	QGroupBox *integratorGroupBox = new QGroupBox;
	integratorGroupBox->setObjectName("integrator_group_box");
	QGridLayout *integratorLayout = new QGridLayout();

	QLabel *integratorNameLabel = new QLabel(tr("Integrator Type"), integratorGroupBox);
	integratorNameLabel->setObjectName("integrator_label");

	
	QComboBox *integratorComboBox = new QComboBox(integratorGroupBox);
	integratorComboBox->setObjectName("integrator_combobox");
	for (int idx = 0; idx < NUMBER_OF_INTEGRATOR; idx++) {
		integratorComboBox->addItem(integratorNames[idx]);
	}
	integratorComboBox->setCurrentIndex(integrator->getType());
	QObject::connect(integratorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeIntegratorType(int)));

	QLabel *maxDepthLabel = new QLabel(tr("Max Depth"), integratorGroupBox);
	maxDepthLabel->setObjectName("max_depth_label");
	QLineEdit *maxDepthEdit = new QLineEdit(QString::number(integrator->getMaxDepth()), integratorGroupBox);
	maxDepthEdit->setObjectName("max_depth_edit");
	QObject::connect(maxDepthEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateMaxDepth);

	QLabel *sceneEpsilonLabel = new QLabel(tr("Scene Epsilon"), integratorGroupBox);
	sceneEpsilonLabel->setObjectName("scene_epsilon_label");
	QLineEdit *sceneEpsilonEdit = new QLineEdit(QString::number(integrator->getSceneEpsilon()), integratorGroupBox);
	sceneEpsilonEdit->setObjectName("scene_epsilon_edit");
	QObject::connect(sceneEpsilonEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateSceneEpsilon);

	QLabel *colorLabel = new QLabel(tr("Exception Color"), integratorGroupBox);
	colorLabel->setObjectName("exc_color_label");
	QLineEdit *xColorEdit = new QLineEdit(QString::number(integrator->getExceptionColor().x()), integratorGroupBox);
	QLineEdit *yColorEdit = new QLineEdit(QString::number(integrator->getExceptionColor().y()), integratorGroupBox);
	QLineEdit *zColorEdit = new QLineEdit(QString::number(integrator->getExceptionColor().z()), integratorGroupBox);
	xColorEdit->setObjectName("exc_color_x");
	yColorEdit->setObjectName("exc_color_y");
	zColorEdit->setObjectName("exc_color_z");
	QObject::connect(xColorEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateExceptionColor);
	QObject::connect(yColorEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateExceptionColor);
	QObject::connect(zColorEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateExceptionColor);

	integratorLayout->addWidget(integratorNameLabel, 0, 0);
	integratorLayout->addWidget(integratorComboBox, 0, 1);
	integratorLayout->addWidget(maxDepthLabel, 1, 0);
	integratorLayout->addWidget(maxDepthEdit, 1, 1);
	integratorLayout->addWidget(sceneEpsilonLabel, 2, 0);
	integratorLayout->addWidget(sceneEpsilonEdit, 2, 1);
	integratorLayout->addWidget(colorLabel, 3, 0);
	integratorLayout->addWidget(xColorEdit, 3, 1);
	integratorLayout->addWidget(yColorEdit, 3, 2);
	integratorLayout->addWidget(zColorEdit, 3, 3);
	integratorGroupBox->setLayout(integratorLayout);
	integratorTabLayout->addWidget(integratorGroupBox);
}

void IntegratorTab::initDepthTracerParameters(DepthTracer* integrator) {


	QGroupBox *integratorGroupBox = new QGroupBox;
	integratorGroupBox->setObjectName("integrator_group_box");
	QGridLayout *integratorLayout = new QGridLayout();

	QLabel *integratorNameLabel = new QLabel(tr("Integrator Type"), integratorGroupBox);
	integratorNameLabel->setObjectName("integrator_label");


	QComboBox *integratorComboBox = new QComboBox(integratorGroupBox);
	integratorComboBox->setObjectName("integrator_combobox");
	for (int idx = 0; idx < NUMBER_OF_INTEGRATOR; idx++) {
		integratorComboBox->addItem(integratorNames[idx]);
	}
	integratorComboBox->setCurrentIndex(integrator->getType());
	QObject::connect(integratorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeIntegratorType(int)));

	QLabel *maxDepthLabel = new QLabel(tr("Max Depth"), integratorGroupBox);
	maxDepthLabel->setObjectName("max_depth_label");
	QLineEdit *maxDepthEdit = new QLineEdit(QString::number(integrator->getMaxDepth()), integratorGroupBox);
	maxDepthEdit->setObjectName("max_depth_edit");
	QObject::connect(maxDepthEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateMaxDepth);

	QLabel *sceneEpsilonLabel = new QLabel(tr("Scene Epsilon"), integratorGroupBox);
	sceneEpsilonLabel->setObjectName("scene_epsilon_label");
	QLineEdit *sceneEpsilonEdit = new QLineEdit(QString::number(integrator->getSceneEpsilon()), integratorGroupBox);
	sceneEpsilonEdit->setObjectName("scene_epsilon_edit");
	QObject::connect(sceneEpsilonEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateSceneEpsilon);

	QLabel *colorLabel = new QLabel(tr("Exception Color"), integratorGroupBox);
	colorLabel->setObjectName("exc_color_label");
	QLineEdit *xColorEdit = new QLineEdit(QString::number(integrator->getExceptionColor().x()), integratorGroupBox);
	QLineEdit *yColorEdit = new QLineEdit(QString::number(integrator->getExceptionColor().y()), integratorGroupBox);
	QLineEdit *zColorEdit = new QLineEdit(QString::number(integrator->getExceptionColor().z()), integratorGroupBox);
	xColorEdit->setObjectName("exc_color_x");
	yColorEdit->setObjectName("exc_color_y");
	zColorEdit->setObjectName("exc_color_z");
	QObject::connect(xColorEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateExceptionColor);
	QObject::connect(yColorEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateExceptionColor);
	QObject::connect(zColorEdit, &QLineEdit::returnPressed, this, &IntegratorTab::updateExceptionColor);

	integratorLayout->addWidget(integratorNameLabel, 0, 0);
	integratorLayout->addWidget(integratorComboBox, 0, 1);
	integratorLayout->addWidget(maxDepthLabel, 1, 0);
	integratorLayout->addWidget(maxDepthEdit, 1, 1);
	integratorLayout->addWidget(sceneEpsilonLabel, 2, 0);
	integratorLayout->addWidget(sceneEpsilonEdit, 2, 1);
	integratorLayout->addWidget(colorLabel, 3, 0);
	integratorLayout->addWidget(xColorEdit, 3, 1);
	integratorLayout->addWidget(yColorEdit, 3, 2);
	integratorLayout->addWidget(zColorEdit, 3, 3);
	integratorGroupBox->setLayout(integratorLayout);
	integratorTabLayout->addWidget(integratorGroupBox);
}


void IntegratorTab::updateMaxDepth()
{
	int max_depth = this->findChild<QLineEdit*>("max_depth_edit")->text().toInt();
	reinterpret_cast<PathTracer*> (optixWindow->getScene()->getIntegrator())->setMaxDepth(max_depth);
	optixWindow->restartFrame();
}

void IntegratorTab::updateExceptionColor()
{
	float color_x = this->findChild<QLineEdit*>("exc_color_x")->text().toFloat();
	float color_y = this->findChild<QLineEdit*>("exc_color_y")->text().toFloat();
	float color_z = this->findChild<QLineEdit*>("exc_color_z")->text().toFloat();
	reinterpret_cast<PathTracer*> (optixWindow->getScene()->getIntegrator())->setExceptionColor(QVector3D(color_x, color_y, color_z));
	optixWindow->restartFrame();
}

void IntegratorTab::updateSceneEpsilon()
{
	float scene_epsilon = this->findChild<QLineEdit*>("scene_epsilon_edit")->text().toFloat();
	reinterpret_cast<PathTracer*> (optixWindow->getScene()->getIntegrator())->setSceneEpsilon(scene_epsilon);
	optixWindow->restartFrame();
}


void IntegratorTab::changeIntegratorType(int integratorType)
{
	QGroupBox *integratorGroupBox = this->findChild<QGroupBox*>("integrator_group_box");
	integratorGroupBox->hide();
	integratorTabLayout->removeWidget(integratorGroupBox);
	integratorGroupBox->deleteLater();
	IntegratorType integrator = static_cast<IntegratorType>(integratorType);
	if (integrator == PATH_TRACER) {
		optixWindow->getScene()->setIntegrator(new PathTracer(optixWindow->getContext()));
		initPathTracerParameters(reinterpret_cast<PathTracer*> (optixWindow->getScene()->getIntegrator()));
	}
	if (integrator == DEPTH_TRACER) {
		optixWindow->getScene()->setIntegrator(new DepthTracer(optixWindow->getContext()));
		initDepthTracerParameters(reinterpret_cast<DepthTracer*> (optixWindow->getScene()->getIntegrator()));
	}
	optixWindow->restartFrame();
}