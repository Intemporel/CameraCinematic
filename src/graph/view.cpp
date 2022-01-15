#include "view.h"

View::View(QObject* object)
{
    pressed = false;
    zoomFactor = 1.0015;

    viewport()->installEventFilter(this);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);

    //setMaximumSize()

    scene = new Scene();
    setScene(scene);
    scale(1, -1);
    centerOn(0,0);

    connect(this, SIGNAL(receivedVectors()), scene, SLOT(update()));

    Q_UNUSED(object);
}

void View::gentleZoom(double factor)
{
    scale(factor, factor);
    centerOn(targetScenePos);
    QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
    QPointF viewportCenter = mapFromScene(targetScenePos) - deltaViewportPos;
    centerOn(mapToScene(viewportCenter.toPoint()));
}

void View::changeDisplay(int index)
{
    switch (index)
    {
    case 0:
        scene->changeDisplay(Coord::X, Coord::Y);
        break;
    case 1:
        scene->changeDisplay(Coord::X, Coord::Z);
        break;
    case 2:
        scene->changeDisplay(Coord::Y, Coord::Z);
        break;
    }
}

void View::setVectors(uint16_t inter[3], bool show[3], QVector<QVector<QVector<float> > > pos, QVector<QVector<QVector<float> > > tar, QVector<QVector<QVector<float> > > roll)
{
    scene->clear();

    if ( show[0] )
    {
        for (int row = 0; row < pos.size(); ++row)
            for (int vec = 0; vec < pos[row].size(); ++vec)
                scene->createVector(row, VectorType::POSITION, static_cast<VectorPos>(vec), pos[row][vec]);
    }

    if ( show[1] )
    {
        for (int row = 0; row < tar.size(); ++row)
            for (int vec = 0; vec < tar[row].size(); ++vec)
                scene->createVector(row, VectorType::TARGET, static_cast<VectorPos>(vec), tar[row][vec]);
    }

    if ( show[2] )
    {
        for (int row = 0; row < roll.size(); ++row)
            for (int vec = 0; vec < roll[row].size(); ++vec)
                scene->createVector(row, VectorType::ROLL, static_cast<VectorPos>(vec), roll[row][vec]);
    }

    Q_UNUSED(inter)
}

void View::drawBackground(QPainter *painter, const QRectF &rect)
{
    globalFactor = rect.width()/1000;
    int gridSize;

    if ( globalFactor < 1.0 && globalFactor > 0.5 ) gridSize = 100;
    else if ( globalFactor < 0.5 && globalFactor > 0.2 ) gridSize = 50;
    else if ( globalFactor < 0.2 && globalFactor > 0.0 ) gridSize = 20;
    else gridSize = 150;

    qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % gridSize);

    QVarLengthArray<QLineF, 100> lines;

    for (qreal x = left; x < rect.right(); x += gridSize)
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += gridSize)
        lines.append(QLineF(rect.left(), y, rect.right(), y));

    QPen little(QBrush(QColor(170, 183, 184, 100)), 1,
                Qt::PenStyle::DotLine,
                Qt::PenCapStyle::RoundCap,
                Qt::PenJoinStyle::RoundJoin);

    painter->setPen(little);
    painter->drawLines(lines.data(), lines.size());

    QPen axes(Qt::black);
    axes.setWidth(2);
    painter->setPen(axes);
    painter->drawLine(rect.left(), 0, rect.right(), 0);
    painter->drawLine(0, rect.top(), 0, rect.bottom());
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    QPointF delta = targetViewportPos - event->pos();
    if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
    {
        targetViewportPos = event->pos();
        targetScenePos = mapToScene(event->pos());
    }


    if ( pressed )
        translate(-delta.x()/(3/globalFactor), delta.y()/(2/globalFactor));

    QGraphicsView::mouseMoveEvent(event);
}

void View::mousePressEvent(QMouseEvent *event)
{
    pressed = true;
    QGraphicsView::mousePressEvent(event);
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    pressed = false;
    QGraphicsView::mouseReleaseEvent(event);
}

void View::wheelEvent(QWheelEvent *event)
{
    double angle = event->angleDelta().y();
    double factor = qPow(zoomFactor, angle);
    gentleZoom(factor);
    QGraphicsView::wheelEvent(event);
}
