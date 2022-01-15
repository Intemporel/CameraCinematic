#ifndef CAMERACINEMATIC_H
#define CAMERACINEMATIC_H

#include "process.h"
#include "file/DBCReader.h"
#include "file/skin.h"
#include "file/m2.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QIntValidator>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QtMath>

QT_BEGIN_NAMESPACE
namespace Ui { class CameraCinematic; }
QT_END_NAMESPACE

class CameraCinematic : public QMainWindow
{
    Q_OBJECT

public:
    CameraCinematic(QWidget *parent = nullptr);
    ~CameraCinematic();

    DBCReader dbcCinematicCamera;
    M2 camModel;
    Skin camSkin;
    Process hProc;

private slots:
    void addRow();
    void removeRow();
    void updateModelInfo();
    void updateDBCInfo(bool check = true);
    void generateFile();
    void skinFileDone();
    void m2FileDone();
    void m2FileRead();
    void m2FileUpdate();

private:
    Ui::CameraCinematic *ui;
    QTableWidget* selectedTable;
    QString name;
    QString modelPath;
    float origin[3] = {0.0f,0.0f,0.0f};
    float facing = 0.0f;
    std::uint32_t animationLength = 0;
    std::uint16_t interpolation[3] = {0x0001,0x0001,0x0001};

    void sendVectors();
    void updateDBC();
    void updateRowList();
    void updateVectorList();
    void applyItemOrientation(QTableWidgetItem*, QTableWidgetItem*);
    void applyItemOffset(QVector<QTableWidgetItem*>);
};
#endif // CAMERACINEMATIC_H
