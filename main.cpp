#include <QApplication>
#include <QDir>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Q_INIT_RESOURCE(Ressource);

    MainWindow w;
    w.show();

    return a.exec();
}
