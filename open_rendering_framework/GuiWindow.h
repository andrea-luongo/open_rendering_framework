#pragma once
#include <QtWidgets>
#include "OptixWindow.h"
#include "CameraTabGui.h"
#include "LightTabGui.h"
#include "BackgroundTabGui.h"
#include "IntegratorTabGui.h"
#include "GeometryTabGui.h"
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;

class GuiWindow : public QWidget
{
    Q_OBJECT
		
public:
   explicit GuiWindow(OptixWindow* optix_window, QWidget *parent = 0);
   ~GuiWindow();
   void closeEvent(QCloseEvent *event) { QWidget::closeEvent(event); emit terminate_application(); };
protected:
	QSize minimumSizeHint() const;
	void initGUI();
	void reloadGUI();
	void initTabWidget();
	void initCommonSettingsWidget();
private: 
	QTabWidget *tabWidget;
	QGroupBox *frameGroupBox;
	//QLabel *fpsLabel;
	//QLabel *frameCountLabel;
	//
	CameraTab *cameraTab;
	LightTab *lightTab;
	GeometryTab *geometryTab;
	BackgroundTab *backgroundTab;
	IntegratorTab *integratorTab;
	QVBoxLayout *mainLayout;
	QPushButton *saveScene;
	QPushButton *loadScene;
	QPushButton *saveScreenshot;

	OptixWindow *optixWindow;
signals:
	void terminate_application();
	void pause_rendering(bool is_paused);
public slots:
	void update_fps(float fps);
	void update_frame_count(int frames);
	void update_max_frame();
	void save_scene();
	void load_scene();
	void save_screenshot();
};

