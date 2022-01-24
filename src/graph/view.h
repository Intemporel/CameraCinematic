#ifndef VIEW_H
#define VIEW_H

#include "scene.h"
#include "vect2d.h"
#include <QApplication>
#include <QGraphicsView>
#include <QMouseEvent>

class View : public QGraphicsView
{
    Q_OBJECT

public:
    explicit View(QObject* object = nullptr);
    void gentleZoom(double factor);
    void changeDisplay(int index);

    void setVectors(std::uint16_t inter[3], bool show[2],
                    QVector<QVector<QVector<float>>> pos,
                    QVector<QVector<QVector<float>>> tar//,
                    /*QVector<QVector<QVector<float>>> roll*/);
    void setCurves(bool curve[2], std::uint16_t inter[3], QVector<int> stampPos, QVector<int> stampTar, float accRatio, int accPercent, bool acc[2]);
    void setViewLine(int rowPos, int rowTar, float p, float t, std::uint16_t inter[3]);

    void clear() { scene->clear(); };
    void clearViewLine() { scene->removeItemsFromScene({ZValue::VIEWLINE}); };

    QVector<float> getLenghtPositions() { return scene->getLenghtPos(); };
    QVector<float> getLenghtTargets() { return scene->getLenghtTar(); };

private:
    Scene* scene;
    double zoomFactor;
    double globalFactor;
    bool pressed;
    QPointF targetScenePos, targetViewportPos;

    std::uint16_t interpolation[3];
    QVector<QVector<QVector<float> > > positions;
    QVector<QVector<QVector<float> > > targets;
    QVector<QVector<QVector<float> > > rolls;
    int itemSelected[3] = {-1,-1,-1};

protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

signals:
    void receivedVectors();
    void selectedItem(int, int, int);
    void savePosition(QVector3D, bool);
    void createPoint(QVector3D, bool);
};

#endif // VIEW_H
