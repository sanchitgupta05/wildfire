#include "himitsu.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    himitsu w;
    w.show();
    w.mainLoop();
    return 0;
}
