#include "GuiWindow.h"


GuiWindow::GuiWindow(OptixWindow* optix_window, QWidget *parent)
	: QWidget(parent)
{

	optixWindow = optix_window;
	QObject::connect(this, &GuiWindow::pause_rendering, optix_window, &OptixWindow::pause);
	//mainLayout = new QVBoxLayout;
	//initGUI();
	//setLayout(mainLayout);
	initTabWidget();
	initCommonSettingsWidget();
	
	
	mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(frameGroupBox);
	//mainLayout->addWidget(frameCountLabel);
	setLayout(mainLayout);
}

GuiWindow::~GuiWindow()
{
	delete tabWidget, frameGroupBox, mainLayout;
}

void GuiWindow::initGUI()
{
	initTabWidget();
	initCommonSettingsWidget();
	
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(frameGroupBox);
	//mainLayout->addWidget(frameCountLabel);
	
}

QSize GuiWindow::minimumSizeHint() const
{
	return QSize(400, 400);
}

void GuiWindow::initTabWidget()
{
	tabWidget = new QTabWidget;
	cameraTab = new CameraTab(optixWindow);
	lightTab = new LightTab(optixWindow);
	geometryTab = new GeometryTab(optixWindow);
	backgroundTab = new BackgroundTab(optixWindow);
	integratorTab = new IntegratorTab(optixWindow);
	tabWidget->addTab(cameraTab, tr("Camera"));
	tabWidget->addTab(lightTab, tr("Lights"));
	tabWidget->addTab(geometryTab, tr("Geometry"));
	tabWidget->addTab(backgroundTab, tr("Background"));
	tabWidget->addTab(integratorTab, tr("Integrator"));
}

void GuiWindow::initCommonSettingsWidget()
{
	frameGroupBox = new QGroupBox;
	QLabel *fpsLabel = new QLabel(tr("fps: 0"), frameGroupBox);
	fpsLabel->setObjectName("fps_label");
	QLabel *frameCountLabel = new QLabel(tr("frames: 0"), frameGroupBox);
	frameCountLabel->setObjectName("frame_count_label");
	QObject::connect(optixWindow, &OptixWindow::updated_fps, this, &GuiWindow::update_fps);
	QObject::connect(optixWindow, &OptixWindow::updated_frame_count, this, &GuiWindow::update_frame_count);
	QLabel* maxFrameLabel = new QLabel(tr("Max Frames:"), frameGroupBox);
	maxFrameLabel->setObjectName("max_frame_label");
	QLineEdit *maxFrameEdit = new QLineEdit(QString::number(optixWindow->getMaxFrame()), frameGroupBox);
	maxFrameEdit->setObjectName("max_frame_edit");
	maxFrameEdit->setFixedWidth(50);
	QObject::connect(maxFrameEdit, &QLineEdit::returnPressed, this, &GuiWindow::update_max_frame);

	saveScene = new QPushButton(tr("Save Scene"), frameGroupBox);
	QObject::connect(saveScene, &QPushButton::released, this, &GuiWindow::save_scene);
	loadScene = new QPushButton(tr("Load Scene"), frameGroupBox);
	QObject::connect(loadScene, &QPushButton::released, this, &GuiWindow::load_scene);
	saveScreenshot = new QPushButton(tr("Screenshot"), frameGroupBox);
	QObject::connect(saveScreenshot, &QPushButton::released, this, &GuiWindow::save_screenshot);
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(fpsLabel);
	layout->addWidget(frameCountLabel);
	layout->addWidget(maxFrameLabel);
	layout->addWidget(maxFrameEdit);
	layout->addWidget(saveScene);
	layout->addWidget(loadScene);
	layout->addWidget(saveScreenshot);
	frameGroupBox->setLayout(layout);

}

void GuiWindow::update_fps(float fps)
{
	QString text = QString("fps: ") + QString::number(fps, 'f', 2);
	frameGroupBox->findChild<QLabel*>("fps_label")->setText(text);
}

void GuiWindow::update_frame_count(int frames)
{
	QString text = QString("frames: ") + QString::number(frames);
	frameGroupBox->findChild<QLabel*>("frame_count_label")->setText(text);
}

void GuiWindow::update_max_frame()
{	
	int max_frame = this->findChild<QLineEdit*>("max_frame_edit")->text().toInt();
	optixWindow->setMaxFrame(max_frame);
}

void GuiWindow::save_scene()
{
	emit(pause_rendering(true));
	QString path = QFileDialog::getSaveFileName(this, tr("Open file"), "../../scenes", tr("JSON file (*.json)"));
	if (path.isNull() || path.isEmpty()) {
		emit(pause_rendering(false));
		return;
	}
	QDir dir("./");
	path = dir.relativeFilePath(path);
	optixWindow->saveScene(path);
	emit(pause_rendering(false));
	/*reinterpret_cast<EnvMapBackground*> (optixWindow->getScene()->getBackground())->changePath(path);

	QLineEdit *pathEdit = this->findChild<QLineEdit*>("envmap_path");
	pathEdit->setText(path);
	optixWindow->restartFrame();*/

}
void GuiWindow::load_scene()
{
	emit(pause_rendering(true));
	QString path = QFileDialog::getOpenFileName(this, tr("Open file"), "../../scenes", tr("JSON file (*.json)"));
	if (path.isNull() || path.isEmpty()) {
		emit(pause_rendering(false));
		return;
	}
	QDir dir("./");
	path = dir.relativeFilePath(path);
	optixWindow->loadScene(path);
	reloadGUI();
	emit(pause_rendering(false));
}

void GuiWindow::save_screenshot()
{
	emit(pause_rendering(true));
	QString path = QFileDialog::getSaveFileName(this, tr("Open file"), "../../scenes", tr("PNG JPG file (*.png *.jpg)"));
	if (path.isNull() || path.isEmpty()) {
		emit(pause_rendering(false));
		return;
	}
	QDir dir("./");
	path = dir.relativeFilePath(path);
	optixWindow->saveScreenshot(path, true);
	emit(pause_rendering(false));
}

void GuiWindow::reloadGUI()
{
	
	mainLayout->removeWidget(tabWidget);
	mainLayout->removeWidget(frameGroupBox);
	tabWidget->hide();
	frameGroupBox->hide();
	QList<QWidget *> widgets = this->findChildren<QWidget *>(QString(),Qt::FindDirectChildrenOnly);
	foreach(QWidget * w, widgets) {
		delete w;
	}
	//QLineEdit * tmp = this->findChild<QLineEdit*>("max_frame_edit");
	//delete tabWidget, frameGroupBox, tmp;
	initTabWidget();
	initCommonSettingsWidget();
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(frameGroupBox);	
	tabWidget->show();
	frameGroupBox->show();
	mainLayout->update();
}
