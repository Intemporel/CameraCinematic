#ifndef SETTINGS_H
#define SETTINGS_H

#include "process.h"

#include <QDialog>
#include <QFileDialog>
#include <QColorDialog>
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

    /* PATH WORK SETTINGS */
    void storeFolderPathWork();
    void saveFolderPathWork(const QString &);

    /* COLOR SETTINGS */
    QString styleSheetBGC = "QPushButton {border-radius:18px; background-color:%1;}";
    QColor defaultPosPrimary = QColor(31, 97, 141);
    QColor defaultPosSecondary = QColor(20, 143, 119);
    QColor defaultTarPrimary = QColor(146, 43, 33);
    QColor defaultTarSecondary = QColor(118, 68, 138);
    QColor defaultHighSpeed = QColor(192, 57, 43);
    QColor defaultLowSpeed = QColor(46, 134, 193);
    QColor subColor(QColor c) {
        float delta = 1.25f;
        int nR = (c.red()*delta > 255) ? 255 : c.red()*delta;
        int nG = (c.green()*delta > 255) ? 255 : c.green()*delta;
        int nB = (c.blue()*delta > 255) ? 255 : c.blue()*delta;

        return QColor(nR, nG, nB);
    };

    void updateBackgroundColorShow();
    void resetDefaultColor();
    void saveColor(QString key);
};

#endif // SETTINGS_H
