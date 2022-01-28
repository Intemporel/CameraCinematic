#include "skin.h"

void Skin::run()
{
    if (name.isEmpty())
        return;

    if (!path.isEmpty())
    {
        name = path;
    }
    else
    {
        QSettings setting("WOW-EDITOR", "CameraCinematic");
        QString path;

        setting.beginGroup("WORK-SETTINGS");
        path = setting.value("folder-path", QDir::currentPath()).toString();
        setting.endGroup();

        name = path + "/" + name + "00.skin";
    }

    QFile file(name);
    if (file.open(QIODevice::ReadWrite))
    {
        QDataStream in(&file);
        in.setByteOrder(QDataStream::LittleEndian);

        in.writeRawData(QString("SKIN").toUtf8().data(), 0x4);
        for (int i = 0; i < 10; ++i)
            in << qint32(0);
        in << qint32(21);
    }

    file.close();
    emit skinDone();
}
