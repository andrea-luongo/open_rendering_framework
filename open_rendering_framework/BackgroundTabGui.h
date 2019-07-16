#pragma once
#include <QtWidgets>
#include "OptixWindow.h"

class BackgroundTab : public QWidget
{
	Q_OBJECT

public:
	explicit BackgroundTab(OptixWindow* optix_window,  QWidget *parent = 0);


protected:
	void initBackgroundTab();
	void initConstantBackgroundParameters(ConstantBackground* background);
	void initEnvMapBackgroundParameters(EnvMapBackground* background);

	QHBoxLayout *backgroundTabLayout;
	OptixWindow *optixWindow;
	
public slots:
	void updateBackgroundColor();
	void updatePath();
	void changeBackgroundType(int bg_type);
signals:

};
