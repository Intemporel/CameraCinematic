#include "menu.h"
#include "ui_menu.h"

Menu::Menu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);

    connect(ui->createFile, &QPushButton::clicked, [=]() { emit createNewFile(); close(); });
    connect(ui->openFile, &QPushButton::clicked, [=]() { openModelFile(); });
    connect(ui->settings, &QPushButton::clicked, [=]() { openSettings(); });

    checkSettings();
}

Menu::~Menu() { delete ui; }

void Menu::openModelFile()
{
    QString modelPath = QFileDialog::getOpenFileName(this, tr("Open Model Camera"), "/",
                                                     tr("Model Files (*.m2 *.mdx)"));
    QString path = QFileInfo(modelPath).absoluteDir().absolutePath();

    if ( modelPath.isEmpty() )
        return;

    QSettings setting("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("MODEL-SETTINGS");
    setting.setValue("model", modelPath);
    setting.setValue("name", modelPath.remove(path).remove(".m2"));
    setting.setValue("path", path);
    setting.endGroup();

    emit openedFile();
    close();
}

void Menu::checkSettings()
{
    bool open = false;

    QSettings setting("WOW-EDITOR", "CameraCinematic");

    setting.beginGroup("DBC-SETTINGS");

        if (setting.value("folder-path", "").toString() == "")
        {
            open = true;
        }

    setting.endGroup();

    setting.beginGroup("CLIENT-SETTINGS");

        if (setting.value("client-window", "").toString() == "")
        {
            open = true;
        }

    setting.endGroup();

    setting.beginGroup("WORK-SETTINGS");

        if (setting.value("folder-path", "").toString() == "")
        {
            open = true;
        }

    setting.endGroup();

    if (open)
    {
        openSettings();
    }
}

void Menu::openSettings()
{
    settings = new Settings(this);
    settings->show();
}
