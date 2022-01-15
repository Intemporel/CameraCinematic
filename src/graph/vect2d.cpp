#include "vect2d.h"

Vect2D::Vect2D(quint8 id, double s, QColor color)
{
    UID[0] = id;
    size = s;
    brush = QBrush(color);
    pen.setBrush(brush);
    pen.setWidth(1);
}

QRectF Vect2D::boundingRect() const
{
    return QRectF(-size/2,-size/2,size,size);
}

void Vect2D::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rec = boundingRect();

    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawEllipse(rec);

    QPen dark(QBrush(Qt::black), 1);
    painter->setPen(dark);
    painter->drawPoint(0,0);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void Vect2D::setCoord(float from[3])
{
    for (int n = 0; n < 3; n++)
        coord[n] = from[n];
}

void Vect2D::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
}
