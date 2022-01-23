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
