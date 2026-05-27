#include "databasemain.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DatabaseMain w;
    w.show();
    return QCoreApplication::exec();
}
