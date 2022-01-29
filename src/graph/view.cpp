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

void View::setVectors(std::uint16_t inter[3], bool show[2], QVector<QVector<QVector<float> > > pos, QVector<QVector<QVector<float> > > tar/*, QVector<QVector<QVector<float> > > roll*/)
{
    scene->clear();

    positions = pos;
    targets = tar;
    //rolls = roll;

    if ( show[0] )
    {
        for (int row = 0; row < pos.size(); ++row)
            for (int vec = 0; vec < pos[row].size(); ++vec)
                scene->createVector(row, VectorType::POSITION, static_cast<VectorPos>(vec), pos[row][vec], inter[0]);
    }

    if ( show[1] )
    {
        for (int row = 0; row < tar.size(); ++row)
            for (int vec = 0; vec < tar[row].size(); ++vec)
                scene->createVector(row, VectorType::TARGET, static_cast<VectorPos>(vec), tar[row][vec], inter[1]);
    }

    /*if ( show[2] )
    {
        for (int row = 0; row < roll.size(); ++row)
            for (int vec = 0; vec < roll[row].size(); ++vec)
                scene->createVector(row, VectorType::ROLL, static_cast<VectorPos>(vec), roll[row][vec]);
    }*/

    if (itemSelected[0] != -1)
    {
        for (QGraphicsItem* i : scene->items())
        {
            if (Vect2D *n = static_cast<Vect2D*>(i))
            {
                if (n->getType() == itemSelected[0] &&
                        n->getRow() == itemSelected[1] &&
                        n->getVec() == itemSelected[2])
                {
                    scene->viewSelected(n);
                    break;
                }
            }
        }
    }
}

void View::setCurves(bool curve[2], std::uint16_t inter[3], QVector<int> stampPos, QVector<int> stampTar, float accRatio, int accPercent, bool acc[2], bool curveOnTop)
{
    scene->removeItemsFromScene({ZValue::CURVE, ZValue::FORCED_CURVE});

    QVector<QVector<float>> data;
    QVector<QVector<QVector<float>>> current;
    QVector<int> stamp;
    bool acceleration;

    scene->setAccRation(accRatio);
    scene->setAccPercent(accPercent);
    scene->setForceDisplayCurveOnTop(curveOnTop);

    for (int i = 0; i < 2; ++i)
    {
       if ( curve[i] )
       {
           data.clear();
           acceleration = acc[i];

           if ( i == 0 ) // position
           {
               current = positions;
               stamp = stampPos;
           }
           else // target
           {
               current = targets;
               stamp = stampTar;
           }

           switch (static_cast<Interpolation>(inter[i])) {
           case Interpolation::LINEAR:
               scene->createLinearInterpolation(static_cast<VectorType>(i), current, stamp, acceleration);
               break;
           case Interpolation::BEZIER:
               scene->createBezierInterpolation(static_cast<VectorType>(i), current, stamp, acceleration);
               break;
           case Interpolation::HERMITE:
               scene->createHermiteInterpolation(static_cast<VectorType>(i), current, stamp, acceleration);
               break;
           case Interpolation::NONE:
               break;
           }
       }
    }
}

void View::setViewLine(int rowPos, int rowTar, float p, float t, std::uint16_t inter[3])
{
    scene->createViewLine(positions, targets, rowPos, rowTar, p, t, inter[0], inter[1]);
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

    if (Vect2D *item = static_cast<Vect2D*>(scene->itemAt(mapToScene(event->pos()), transform())))
    {
        VectorType type = static_cast<VectorType>(item->getType());
        VectorPos vec = static_cast<VectorPos>(item->getVec());

        if ( type >= POSITION && type <= ROLL && vec >= POS_SELF && vec <= POS_OUT )
        {
            QString vectorName[3] = {"Self", "In", "Out"};
            QString stdType = ( type == POSITION ) ? "Position" : ( type == TARGET ) ? "Target" : "Roll";
            QString stdRow = QString("\nRow : %1").arg(item->getRow()+1);//.arg(vectorName[item->getVec()]);
            int vec = item->getVec();
            if (vec < VectorPos::POS_SELF || vec > VectorPos::POS_OUT)
                vec = 0;
            QString stdVec = QString("\nVector : %1").arg(vectorName[vec]);

            QToolTip::showText(event->globalPosition().toPoint(), stdType + stdRow + stdVec, this, QRect(), 2500);
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

void View::mousePressEvent(QMouseEvent *event)
{
    pressed = true;

    QPointF pos = mapToScene(event->pos());
    QVector3D vec;

    switch (scene->getDisplay()) {
    case 0: // X; Y
        vec.setX(pos.x());
        vec.setY(pos.y());
        break;
    case 1: // X; Z
        vec.setX(pos.x());
        vec.setZ(pos.y());
        break;
    case 2: // Y; Z
        vec.setY(pos.x());
        vec.setZ(pos.y());
        break;
    }

    if ( QApplication::keyboardModifiers() & Qt::ControlModifier )
    {
        emit createPoint(vec, true);
    }
    else if ( QApplication::keyboardModifiers() & Qt::AltModifier )
    {
        if (Vect2D *item = static_cast<Vect2D*>(scene->itemAt(mapToScene(event->pos()), transform())))
        {
            scene->viewSelected(item);

            itemSelected[0] = item->getType();
            itemSelected[1] = item->getRow();
            itemSelected[2] = item->getVec();

            emit selectedItem(item->getType(), item->getRow(), item->getVec());
        }
    }
    else
    {        
        emit savePosition(vec, false);
    }

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
