#pragma once
#include <QtWidgets>
#include "OptixWindow.h"
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;

class GeometryTab : public QWidget
{
	Q_OBJECT

public:
	explicit GeometryTab(OptixWindow* optix_window,  QWidget *parent = 0);


protected:
	void initGeometryTab();
	void addOBJParameters(OBJGeometry* obj);
	void updateGeometryList();
	QTableWidget* setMaterialTable(MyMaterial* mtl);

	QHBoxLayout *geometryTabLayout;
	QVector<QGroupBox*> geometryWidgetVector;
	QGroupBox *commonSettingBox;
	OptixWindow *optixWindow;
	int currentGeometry;

protected slots:
	void changeGeometryLayout(int newSelectedGeometry);
	void updateGeometryPosition();
	void updateGeometryScale();
	void updateGeometryFreeRotation();
	void updateGeometryXRotation();
	void updateGeometryYRotation();
	void updateGeometryZRotation();
	void addGeometry();
	void removeGeometry();
	void tableUpdate(int row, int column);
	//void tableUpdateTmp(QTableWidgetItem *item);
	void updateMaterial(int mtl_type);
	//void addGeometry();

signals:
	
};
