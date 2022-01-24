#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent)
    , ui(new Ui::Settings)
    , hProc(this)
{
    ui->setupUi(this);

    /* DBC-SETTINGS */
    connect(ui->searchFolderPathDBC, &QToolButton::clicked, [=]() { storeFolderPathDBC(); });
    connect(ui->editFolderPathDBC, &QLineEdit::textChanged, [=](const QString &path) { saveFolderPathDBC(path); });

    /* PROCESS-SETTINGS */
    connect(ui->refreshClientPosition, &QPushButton::clicked, [=]() { showCurrentClientPosition(); });
    connect(ui->editClientWindowName, &QLineEdit::textChanged, [=](const QString &path) { saveProcessName(path); });

    /* PATH WORK */
    connect(ui->searchFolderPathWork, &QToolButton::clicked, [=]() { storeFolderPathWork(); });
    connect(ui->editFolderPathWork, &QLineEdit::textChanged, [=](const QString &path) { saveFolderPathWork(path); });

    /* COLOR SETTINGS */
    ui->selectPosPrimary->setFlat(true);
    ui->selectPosPrimary->setAttribute(Qt::WA_TranslucentBackground);
    ui->posPrim2->setFlat(true);
    ui->posPrim2->setAttribute(Qt::WA_TranslucentBackground);
    ui->selectPosSecondary->setFlat(true);
    ui->selectPosSecondary->setAttribute(Qt::WA_TranslucentBackground);
    ui->posSec2->setFlat(true);
    ui->posSec2->setAttribute(Qt::WA_TranslucentBackground);
    ui->selectTarPrimary->setFlat(true);
    ui->selectTarPrimary->setAttribute(Qt::WA_TranslucentBackground);
    ui->tarPrim2->setFlat(true);
    ui->tarPrim2->setAttribute(Qt::WA_TranslucentBackground);
    ui->selectTarSecondary->setFlat(true);
    ui->selectTarSecondary->setAttribute(Qt::WA_TranslucentBackground);
    ui->tarSec2->setFlat(true);
    ui->tarSec2->setAttribute(Qt::WA_TranslucentBackground);
    ui->selectHighSpeed->setFlat(true);
    ui->selectHighSpeed->setAttribute(Qt::WA_TranslucentBackground);
    ui->selectLowSpeed->setFlat(true);
    ui->selectLowSpeed->setAttribute(Qt::WA_TranslucentBackground);

    connect(ui->resetDefaultColor, &QPushButton::clicked, [=]() { resetDefaultColor(); });
    connect(ui->selectPosPrimary, &QPushButton::clicked, [=]() { saveColor("pos-prim"); });
    connect(ui->selectPosSecondary, &QPushButton::clicked, [=]() { saveColor("pos-sec"); });
    connect(ui->selectTarPrimary, &QPushButton::clicked, [=]() { saveColor("tar-prim"); });
    connect(ui->selectTarSecondary, &QPushButton::clicked, [=]() { saveColor("tar-sec"); });
    connect(ui->selectHighSpeed, &QPushButton::clicked, [=]() { saveColor("high-speed"); });
    connect(ui->selectLowSpeed, &QPushButton::clicked, [=]() { saveColor("low-speed"); });

    loadSettings();
}

Settings::~Settings()
{
    delete ui;
}

void Settings::loadSettings()
{
    setting = new QSettings("WOW-EDITOR", "CameraCinematic");

    setting->beginGroup("DBC-SETTINGS");
        ui->editFolderPathDBC->setText(setting->value("folder-path", "").toString());
        checkFileExistDBC();
    setting->endGroup();

    setting->beginGroup("CLIENT-SETTINGS");
        ui->editClientWindowName->setText(setting->value("client-window", "").toString());
    setting->endGroup();

    setting->beginGroup("WORK-SETTINGS");
        ui->editFolderPathWork->setText(setting->value("folder-path", "").toString());
    setting->endGroup();

    updateBackgroundColorShow();
}

void Settings::storeFolderPathDBC()
{
    QString output = QFileDialog::getExistingDirectory(this, ("Select your DBC Folder"), QDir::currentPath());

    if (!output.isEmpty())
    {
        ui->editFolderPathDBC->setText(output);
    }
}

void Settings::saveFolderPathDBC(const QString &path)
{
    setting->beginGroup("DBC-SETTINGS");
    setting->setValue("folder-path", path);
    setting->endGroup();

    checkFileExistDBC();
}

void Settings::checkFileExistDBC()
{
    if (QFile(ui->editFolderPathDBC->text() + "/CinematicCamera.dbc").exists())
    {
        ui->check_CinematicCamera->setIcon(QIcon(":/icons/icons/done.svg"));
    }
    else
    {
        ui->check_CinematicCamera->setIcon(QIcon(":/icons/icons/error.svg"));
    }
}

void Settings::saveProcessName(const QString &name)
{
    setting->beginGroup("CLIENT-SETTINGS");
    setting->setValue("client-window", name);
    setting->endGroup();

    checkProcessExist();
}

void Settings::checkProcessExist()
{
    hProc.run();

    if (hProc.getError().isEmpty() && hProc.getSuccess()) // success
    {
        ui->check_ClientWindowName->setIcon(QIcon(":/icons/icons/done.svg"));
        showCurrentClientPosition();
    }
    else // error
    {
        ui->check_ClientWindowName->setIcon(QIcon(":/icons/icons/error.svg"));
    }
}

void Settings::showCurrentClientPosition(/*Process &hProc*/)
{
    float o[3] = {.0f,.0f,.0f};
    hProc.run();

    if (hProc.getSuccess())
    {
        ui->clientX->setText(QString("X : %1").arg(hProc.getCoord(0, 0, false, o, false)));
        ui->clientY->setText(QString("Y : %1").arg(hProc.getCoord(1, 0, false, o, false)));
        ui->clientZ->setText(QString("Z : %1").arg(hProc.getCoord(2, 0, false, o, false)));
    }
}

void Settings::storeFolderPathWork()
{
    QString output = QFileDialog::getExistingDirectory(this, ("Select where you want to save your work"), QDir::currentPath());

    if (!output.isEmpty())
    {
        ui->editFolderPathWork->setText(output);
    }
}

void Settings::saveFolderPathWork(const QString &path)
{
    setting->beginGroup("WORK-SETTINGS");
    setting->setValue("folder-path", path);
    setting->endGroup();
}

void Settings::updateBackgroundColorShow()
{
    setting->beginGroup("COLOR-SETTINGS");
        ui->selectPosPrimary->setStyleSheet(styleSheetBGC.arg(setting->value("pos-prim", defaultPosPrimary.name()).toString()));
        ui->posPrim2->setStyleSheet(styleSheetBGC.arg(subColor(setting->value("pos-prim", defaultPosPrimary.name()).toString()).name()));
        ui->selectPosSecondary->setStyleSheet(styleSheetBGC.arg(setting->value("pos-sec", defaultPosSecondary.name()).toString()));
        ui->posSec2->setStyleSheet(styleSheetBGC.arg(subColor(setting->value("pos-sec", defaultPosSecondary.name()).toString()).name()));
        ui->selectTarPrimary->setStyleSheet(styleSheetBGC.arg(setting->value("tar-prim", defaultTarPrimary.name()).toString()));
        ui->tarPrim2->setStyleSheet(styleSheetBGC.arg(subColor(setting->value("tar-prim", defaultTarPrimary.name()).toString()).name()));
        ui->selectTarSecondary->setStyleSheet(styleSheetBGC.arg(setting->value("tar-sec", defaultTarSecondary.name()).toString()));
        ui->tarSec2->setStyleSheet(styleSheetBGC.arg(subColor(setting->value("tar-sec", defaultTarSecondary.name()).toString()).name()));
        ui->selectHighSpeed->setStyleSheet(styleSheetBGC.arg(setting->value("high-speed", defaultHighSpeed.name()).toString()));
        ui->selectLowSpeed->setStyleSheet(styleSheetBGC.arg(setting->value("low-speed", defaultLowSpeed.name()).toString()));
    setting->endGroup();
}

void Settings::resetDefaultColor()
{
    setting->beginGroup("COLOR-SETTINGS");
    setting->setValue("pos-prim", defaultPosPrimary.name());
    setting->setValue("pos-sec", defaultPosSecondary.name());
    setting->setValue("tar-prim", defaultTarPrimary.name());
    setting->setValue("tar-sec", defaultTarSecondary.name());
    setting->setValue("high-speed", defaultHighSpeed.name());
    setting->setValue("low-speed", defaultLowSpeed.name());
    setting->endGroup();

    updateBackgroundColorShow();
}

void Settings::saveColor(QString key)
{
    QColor current;

    setting->beginGroup("COLOR-SETTINGS");
    current = QColor(setting->value(key, "#000").toString());
    setting->endGroup();

    QColor color = QColorDialog::getColor(current, this, "Select new color");

    if (color.isValid())
    {
        setting->beginGroup("COLOR-SETTINGS");
        setting->setValue(key, color.name());
        setting->endGroup();
    }

    updateBackgroundColorShow();
}
