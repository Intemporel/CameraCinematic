#ifndef SKIN_H
#define SKIN_H

#include <QDir>
#include <QFile>
#include <QThread>
#include <QSettings>
#include <QDataStream>

class Skin : public QThread
{
    Q_OBJECT
public:
    Skin(QObject *parent) : QThread(parent) {};
    virtual ~Skin() {};
    void run();

    void setName(QString s) { name = s; };
    void setPath(QString p) { path = p; };

signals:
    void skinDone();

private:
    QString name;
    QString path;
};

#endif // SKIN_H
