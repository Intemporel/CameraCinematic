#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <QBrush>
#include <QPainter>
#include <QGraphicsItem>

class Vect2D : public QGraphicsItem
{
public:
    Vect2D(quint8 id, double s, QColor color = Qt::blue);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    void setCoord(float from[3]);

private:
    /*
     * UID[0] = type
     * UID[1] = row
     * UID[2] = vector
     */
    quint8 UID[3];
    double size;
    float coord[3];
    QPen pen;
    QBrush brush;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
};

#endif // VECTOR2D_H
