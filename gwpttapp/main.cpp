#include "GWPttLoginWidget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	a.setWindowIcon(QIcon(":/gwpttapp/PTT.png"));

	GWPTTLoginWidget login;
    login.show();
    return a.exec();
}
