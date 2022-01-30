#ifndef MENU_H
#define MENU_H

#include "settings.h"

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>

namespace Ui {
class Menu;
}

class Menu : public QMainWindow
{
    Q_OBJECT

public:
    explicit Menu(QWidget *parent = nullptr);
    ~Menu();

signals:
    void createNewFile();
    void openedFile();

private:
    Ui::Menu *ui;
    Settings* settings;

    void openModelFile(QString path = "");
    void checkSettings();
    void openSettings();

    void generateRecentFile();
    void updateRecentFile(QString file);
};

#endif // MENU_H
