#include "OptixWindow.h"
#include "GuiWindow.h"
#include <QtWidgets>
#include "sampleConfig.h"
GLuint WIDTH = 512;
GLuint HEIGHT = 512;

GLuint GUI_WIDTH = 400;
GLuint GUI_HEIGHT = 600;

int main(int argc, char **argv)
{

	
	QApplication app(argc, argv);

	bool quit_and_save = false;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg(argv[i]);
		if (arg == "-q" || arg == "--quit")
		{
			quit_and_save = true;
		}
	}


    QSurfaceFormat format;
    format.setSamples(16);
	OptixWindow optix_window(WIDTH, HEIGHT);
	optix_window.show();
	QObject::connect(&optix_window, &OptixWindow::terminate_application, &app, &QApplication::quit);

	if (!quit_and_save) {
		GuiWindow gui_window(&optix_window);
		gui_window.resize(GUI_WIDTH, GUI_HEIGHT);
		gui_window.show();
		gui_window.setWindowTitle(
		QApplication::translate("toplevel", "GUI"));
		QObject::connect(&gui_window, &GuiWindow::terminate_application, &app, &QApplication::quit);
		return app.exec();
	}
	else {
		return app.exec();
	}
}