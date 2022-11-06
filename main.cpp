#include "dispatcher.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dispatcher w;
    w.show();
    return a.exec();
}
