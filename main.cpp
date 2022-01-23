#include "src/cameracinematic.h"
#include "src/menu.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CameraCinematic editor;

    Menu menu;
    menu.show();

    QObject::connect(&menu, &Menu::createNewFile, [&]() {
        editor.show();
    });

    QObject::connect(&menu, &Menu::openedFile, [&]() {
        editor.show();
        editor.openModelFile();
    });

    return a.exec();
}
