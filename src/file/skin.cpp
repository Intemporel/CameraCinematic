#include "skin.h"

void Skin::run()
{
    if (name.isEmpty())
        return;

    //if (!path.isEmpty())
    //    name = path;

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
