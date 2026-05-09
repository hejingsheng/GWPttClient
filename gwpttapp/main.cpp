#include "GWPttLoginWidget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	QTranslator translator;

	QString locale = QLocale::system().name();
	if (translator.load("Translation_" + locale+".qm")) 
	{
		a.installTranslator(&translator);
	}
	else
	{
		if (translator.load("Translation_en_US.qm"))
		{
			a.installTranslator(&translator);
		}
	}

	a.setWindowIcon(QIcon(":/gwpttapp/PTT.png"));

	GWPTTLoginWidget login;
    login.show();
    return a.exec();
}
