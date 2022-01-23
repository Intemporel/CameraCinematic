#ifndef CAMERACINEMATIC_H
#define CAMERACINEMATIC_H

#include "settings.h"

#include "process.h"
#include "file/DBCReader.h"
#include "file/skin.h"
#include "file/m2.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QIntValidator>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QVector3D>
#include <QTimer>
#include <QtMath>

#include <QAction>
#include <QSettings>

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

public slots:
    void openModelFile();

private slots:
    void selectItem(int, int, int);
    void storePosition(QVector3D, bool);

    /* Begin Menu - File option */
    void getFileInformation();
    /* End Menu - File Option */

    /* Begin QTableWidget Option */
    void addRow();
    void removeRow();
    void brushTime();
    void moveRow(bool up);
    void generateFile();
    /* End QTableWidget Option */

    void updateModelInfo();
    void updateDBCInfo(bool check = true);
    void skinFileDone();
    void m2FileDone();
    void m2FileRead();
    void m2FileUpdate();
    void alignVector();
    void normalizeSpeed();
    void updateCinematic();

private:
    Ui::CameraCinematic *ui;
    Settings* settings;

    QTableWidget* selectedTable;
    QString name;
    //QString modelPath;

    float origin[3] = {0.0f,0.0f,0.0f};
    float facing = 0.0f;
    std::uint32_t animationLength = 0;
    std::uint16_t interpolation[3] = {0x0001,0x0001,0x0001};

    QTimer *updateTimeView;

    uint64_t StartTime;
    uint64_t EndTime;
    uint64_t OffTime;

    uint64_t time() {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::system_clock::now().time_since_epoch()
              ).count();
    };

    float distance3D(QVector3D &vec1, QVector3D &vec2) {
        float h1 = qPow((vec2.x() - vec1.x()),2);
        float h2 = qPow((vec2.y() - vec1.y()),2);
        float h3 = qPow((vec2.z() - vec1.z()),2);

        return qSqrt(h1 + h2 + h3);
    };

    void sendVectors();
    void sendCurves();
    void updateDBC();
    void updateRowList();
    void updateVectorList();
    void updateViewTime(int);
    void createPointFromStoredPosition();

    /* File */
    void createFileMenu();
    QAction *newFileAct;
    QAction *openFileAct;
    QAction *refreshDBCAct;
    QAction *settingsAct;
    QAction *closeAct;
};
#endif // CAMERACINEMATIC_H
