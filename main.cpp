#include "logging.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Logging w;
    w.show();

    return a.exec();
}
