#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <QBrush>
#include <QPainter>
#include <QGraphicsItem>

class Vect2D : public QGraphicsItem
{
public:
    Vect2D(int type, int row, int vec, double s, QColor color = Qt::blue);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    void setCoord(float from[3]);

    int getType() { return UID[0]; };
    int getRow() { return UID[1]; };
    int getVec() { return UID[2]; };
    double getSize() { return size; };
    QBrush getBrush() { return brush; };

private:
    /*
     * UID[0] = type
     * UID[1] = row
     * UID[2] = vector
     */
    int UID[3];
    double size;
    float coord[3];
    QPen pen;
    QBrush brush;
};

#endif // VECTOR2D_H
