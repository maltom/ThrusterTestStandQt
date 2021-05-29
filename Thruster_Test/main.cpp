#include "Thruster_Test.h"
#include <QtWidgets/QApplication>
#include <QtMultimedia/qaudiodeviceinfo.h>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	
	Thruster_Test w;

	
	w.showMaximized();
	w.show();
	return a.exec();

}
