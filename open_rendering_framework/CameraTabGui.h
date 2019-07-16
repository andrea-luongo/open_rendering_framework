#pragma once
#include <QtWidgets>
#include "OptixWindow.h"
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;

class CameraTab : public QWidget
{
	Q_OBJECT

public:
	explicit CameraTab(OptixWindow* optix_window,  QWidget *parent = 0);


protected:
	void initCameraTab();
	void initPinholeCameraParameters(PinholeCamera* camera);
	//QLineEdit* createNewEditElement(QString text, QWidget *parent, QString widgetName,
	//	const char *signal, const char *local_method, const char *local_signal,
	//	const QObject *external_receiver, const char *external_method);

	QGridLayout *cameraTabLayout;
	OptixWindow *optixWindow;
	uint camera_label_row = 0;
	uint width_height_row = 1;
	uint patch_origin_row = 2;
	uint patch_dims_row = 3;
	uint eye_row = 4;
	uint lookat_row = 5;
	uint up_row = 6;
	uint fov_row = 7;
public slots:
	void updateResizeLabels(int width, int height);
	void resizeCamera();
	void updateEye();
	void updateLookat();
	void updateUp();
	void updateFov();
	void updatePatchOrigin();
	void updatePatchDims();
signals:
	void resized_window(int width, int height);
	void updated_eye(QVector3D eye);
	void updated_lookat(QVector3D lookat);
	void updated_up(QVector3D up);
	void updated_vfov(float vfov);

};
