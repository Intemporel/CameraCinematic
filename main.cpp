#include "src/cameracinematic.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CameraCinematic w;
    w.show();
    return a.exec();
}
