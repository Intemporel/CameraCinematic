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

    connect(ui->recentFileName, &QCheckBox::stateChanged, [=]() { generateRecentFile(); });

    checkSettings();
    generateRecentFile();
}

Menu::~Menu() { delete ui; }

void Menu::openModelFile(QString replacement)
{
    QString modelPath = (!replacement.isEmpty()) ?
                replacement
              : QFileDialog::getOpenFileName(this, tr("Open Model Camera"), "/",
                                                           tr("Model Files (*.m2 *.mdx)"));

    QString path = QFileInfo(modelPath).absoluteDir().absolutePath();

    if (modelPath.isEmpty())
        return;

    QString keep = modelPath;
    QSettings setting("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("MODEL-SETTINGS");
    setting.setValue("model", modelPath);
    setting.setValue("name", modelPath.remove(path).remove(".m2"));
    setting.setValue("path", path);
    setting.endGroup();

    updateRecentFile(keep);
    emit openedFile();
    close();
}

void Menu::checkSettings()
{
    bool open = false;

    QSettings setting("WOW-EDITOR", "CameraCinematic");

    setting.beginGroup("DBC-SETTINGS");
    if (setting.value("folder-path", "").toString() == "") open = true;
    setting.endGroup();

    setting.beginGroup("CLIENT-SETTINGS");
    if (setting.value("client-window", "").toString() == "") open = true;
    setting.endGroup();

    setting.beginGroup("WORK-SETTINGS");
    if (setting.value("folder-path", "").toString() == "") open = true;
    setting.endGroup();

    if (open)
        openSettings();
}

void Menu::openSettings()
{
    settings = new Settings(this);
    settings->show();
}

void Menu::generateRecentFile()
{
    QSettings setting("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("recent-file");
    int count = setting.value("count", 0).toUInt();

    if (count == 0)
        return;


    QLayoutItem* item;
    while ( ( item = ui->layoutRecentFile->layout()->takeAt( 0 ) ) != NULL )
    {
        delete item->widget();
        delete item;
    }

    QVector<QString> stored;
    for (int i = 0; i < count; ++i)
    {
        QString path = setting.value(QString::number(i)).toString();
        QString name = QFileInfo(QFile(path).fileName()).fileName();

        if (QFile(path).exists())
        {
            stored.push_back(path);

            QPushButton *recentFile = new QPushButton((ui->recentFileName->isChecked()) ? name : path, this);
            recentFile->setProperty("path", path);

            ui->layoutRecentFile->addWidget(recentFile);

            connect(recentFile, &QPushButton::clicked, [=]() {
                openModelFile(recentFile->property("path").toString());
            });
        }
    }

    // error an file doesn't exist
    if (stored.size() != count)
    {
        for (int i = 0; i < count; ++i)
            setting.remove(QString::number(i));

        for (int i = 0; i < stored.size(); ++i)
            setting.setValue(QString::number(i), stored[i]);

        setting.setValue("count", stored.size());
    }

    setting.endGroup();
}

void Menu::updateRecentFile(QString file)
{
    QSettings setting("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("recent-file");
    int count = setting.value("count", 0).toUInt();

    if (count <= 0)
    {
        setting.setValue("count", 1);
        setting.setValue("0", file);
    }
    else
    {
        int found = -1;
        QVector<QString> stored;

        for (int i = 0; i < count; ++i)
        {
            stored.push_back(setting.value(QString::number(i)).toString());

            // try to found if file already stored
            if (file == stored.last() && found == -1)
                found = i;
        }

        if (found == -1)
        {
            setting.setValue("count", count+1);
            stored.push_front(file);

            for (int i = 0; i < stored.size(); ++i)
                setting.setValue(QString::number(i), stored[i]);

            if ( count == 11 ) // max count = 10
                setting.remove("10");
        }
        else
        {

            stored.remove(found);
            stored.push_front(setting.value(QString::number(found)).toString());

            for (int i = 0; i < stored.size(); ++i)
                setting.setValue(QString::number(i), stored[i]);
        }
    }

    setting.endGroup();
}
