#ifndef SETTINGS_H
#define SETTINGS_H

#include "process.h"

#include <QDialog>
#include <QFileDialog>
#include <QSettings>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

private:
    Ui::Settings *ui;
    QSettings *setting;

    Process hProc;

    void loadSettings();

    /* DBC SETTINGS */
    void storeFolderPathDBC();
    void saveFolderPathDBC(const QString &);
    void checkFileExistDBC();

    /* PROCESS SETTINGS */
    void saveProcessName(const QString &);
    void checkProcessExist();
    void showCurrentClientPosition(/*Process &hProc*/);

    /* PATH WORK */
    void storeFolderPathWork();
    void saveFolderPathWork(const QString &);
};

#endif // SETTINGS_H
