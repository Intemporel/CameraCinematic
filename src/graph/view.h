#ifndef VIEW_H
#define VIEW_H

#include "scene.h"
#include <QApplication>
#include <QGraphicsView>
//#include <QTableWidgetItem>
#include <QMouseEvent>

class View : public QGraphicsView
{
    Q_OBJECT

public:
    explicit View(QObject* object = nullptr);
    void gentleZoom(double factor);
    void clear() { scene->clear(); };
    void changeDisplay(int index);
    void setVectors(std::uint16_t inter[3],
                    bool show[3],
                    QVector<QVector<QVector<float>>> pos,
                    QVector<QVector<QVector<float>>> tar,
                    QVector<QVector<QVector<float>>> roll);

private:
    Scene* scene;
    double zoomFactor;
    double globalFactor;
    bool pressed;
    QPointF targetScenePos, targetViewportPos;

    std::uint16_t interpolation[3];
protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

signals:
    void receivedVectors();
};

#endif // VIEW_H
