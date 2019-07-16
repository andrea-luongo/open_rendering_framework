#pragma once
#include <QtWidgets>
#include "OptixWindow.h"
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;

class LightTab : public QWidget
{
	Q_OBJECT

public:
	explicit LightTab(OptixWindow* optix_window,  QWidget *parent = 0);


protected:
	void initLightTab();
	void addPointLightParameters(PointLight* light, PointLightStruct* lightStruct);
	void addDirectionalLightParameters(DirectionalLight* light, DirectionalLightStruct* lightStruct);
	void addTriangleAreaLightParameters(TriangleAreaLight* light, TrianglesAreaLightStruct* lightStruct);
	void addDiskAreaLightParameters(DiskAreaLight* light, DiskLightStruct* lightStruct);
	void addSphericalLightParameters(SphericalLight* light, SphericalLightStruct* lightStruct);
	void updateLightList();

	QHBoxLayout *lightTabLayout;
	QVector<QGroupBox*> lightWidgetVector;
	QGroupBox *commonSettingBox;
	OptixWindow *optixWindow;
	int currentLight;

protected slots:
	void changeLightLayout(int newSelectedLight);
	void updatePointLightPosition();
	void updatePointLightRadiance();
	void updateDirectionalLightDirection();
	void updateDirectionalLightRadiance();
	void updateTriangleLightRadiance();
	void updateTriangleLightPosition();
	void updateTriangleLightScale();
	void updateTriangleLightRotation();
	void updateTriangleLightXRotation();
	void updateTriangleLightYRotation();
	void updateTriangleLightZRotation();
	void updateDiskLightPosition();
	void updateDiskLightRadiance();
	void updateDiskLightRadius();
	void updateDiskLightNormal();
	void updateSphericalLightPosition();
	void updateSphericalLightRadiance();
	void updateSphericalLightRadius();
	void addLight();
	void removeLight();
signals:
	
};
