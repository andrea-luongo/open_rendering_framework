#pragma once
#include <QtWidgets>
#include "OptixWindow.h"
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;

class IntegratorTab : public QWidget
{
	Q_OBJECT

public:
	explicit IntegratorTab(OptixWindow* optix_window,  QWidget *parent = 0);


protected:
	void initIntegratorTab();
	void initPathTracerParameters(PathTracer* integrator);
	void initDepthTracerParameters(DepthTracer* integrator);

	QHBoxLayout *integratorTabLayout;
	OptixWindow *optixWindow;

public slots:
	void updateMaxDepth();
	void updateExceptionColor();
	void updateSceneEpsilon();
	void changeIntegratorType(int integratorType);
signals:

};
