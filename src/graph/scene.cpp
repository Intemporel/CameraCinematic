#include "scene.h"

Scene::Scene()
{
    generateSceneSize();
}

void Scene::joinToParent(Vect2D *item)
{
    int type = item->getType();
    int row = item->getRow();
    Vect2D *parent = NULL;

    if ( item == NULL )
        return;

    removeItemsFromScene({ZValue::JOIN});

    for (QGraphicsItem* i : items())
    {
        if (Vect2D *n = static_cast<Vect2D*>(i))
        {
            if (n->getType() == type
                    && n->getRow() == row
                    && static_cast<VectorPos>(n->getVec()) == POS_SELF)
            {
                parent = n;
                break;
            }
        }
    }

    if ( parent != NULL )
    {
        QGraphicsLineItem *line = new QGraphicsLineItem(item->x(), item->y(), parent->x(), parent->y());
        line->setPen(QPen(item->getBrush(), 1, Qt::DotLine));
        line->setZValue(ZValue::JOIN);

        addItem(line);
    }
}

void Scene::createVector(int row, VectorType type, VectorPos pos, QVector<float> data, int inter)
{
    if ( static_cast<Interpolation>(inter) == Interpolation::NONE
         || static_cast<Interpolation>(inter) == Interpolation::LINEAR )
        if ( pos == VectorPos::POS_IN || pos == VectorPos::POS_OUT )
            return;

    QColor color = Qt::black;

    switch (type) {
    case VectorType::POSITION:
        if ( row % 2 == 0 )
        {
            color = QColor(31, 97, 141);

            if ( pos != VectorPos::POS_SELF)
                color = QColor(41, 128, 185);
        }
        else
        {
            color = QColor(20, 143, 119);

            if ( pos != VectorPos::POS_SELF)
                color = QColor(26, 188, 156);
        }
        break;
    case VectorType::TARGET:
        if ( row % 2 == 0 )
        {
            color = QColor(146, 43, 33);

            if ( pos != VectorPos::POS_SELF)
                color = QColor(192, 57, 43);
        }
        else
        {
            color = QColor(118, 68, 138);

            if ( pos != VectorPos::POS_SELF)
                color = QColor(155, 89, 182);
        }
        break;
    case VectorType::ROLL:
        if ( row % 2 == 0 )
        {
            color = QColor(183, 149, 11);

            if ( pos != VectorPos::POS_SELF)
                color = QColor(241, 196, 15);
        }
        else
        {
            color = QColor(175, 96, 26);

            if ( pos != VectorPos::POS_SELF)
                color = QColor(235, 152, 78);
        }
        break;
    }

    if ( data.size() == 0 )
        return;

    Vect2D *vec = new Vect2D(type, row, pos, 4, color);
    vec->setPos(data[Axe1], data[Axe2]);
    vec->setZValue(ZValue::VECTOR);
    addItem(vec);
}

void Scene::createLinearInterpolation(VectorType type, QVector<QVector<QVector<float> > > data, QVector<int> stamp, bool acc)
{
    QColor color = Qt::black;

    switch (type) {
    case VectorType::POSITION:
        color = QColor(31, 97, 141);
        lengthPos.clear();
        break;
    case VectorType::TARGET:
        color = QColor(146, 43, 33);
        lengthTar.clear();
        break;
    case VectorType::ROLL:
        break;
    }

    for (int row = 0; row < data.size()-1; ++row)
    {
        QVector3D start = interpolationLinear(data[row], data[row+1], 0.0f);
        QVector3D end   = interpolationLinear(data[row], data[row+1], 1.0f);

        QGraphicsLineItem *line = new QGraphicsLineItem(start[Axe1], start[Axe2], end[Axe1], end[Axe2]);

        if (static_cast<VectorType>(type) == VectorType::POSITION) lengthPos.push_back(distance3D(start, end));
        else lengthTar.push_back(distance3D(start, end));

        line->setPen(QPen(QBrush(color), 1));
        line->setZValue(ZValue::CURVE);
        addItem(line);
    }
}

void Scene::createHermiteInterpolation(VectorType type, QVector<QVector<QVector<float> > > data, QVector<int> stamp, bool acc)
{
    QColor color = Qt::black;

    switch (type) {
    case VectorType::POSITION:
        color = QColor(31, 97, 141);
        lengthPos.clear();
        break;
    case VectorType::TARGET:
        color = QColor(146, 43, 33);
        lengthTar.clear();
        break;
    case VectorType::ROLL:
        break;
    }

    QVector<QVector3D> cur;
    for (int row = 1; row < data.size(); ++row)
    {
        cur.clear();

        float time;
        for (int n = 0; n < POINT+1; ++n)
        {
            time = (float)n / (float)POINT;
            cur.push_back(interpolationHermite(data[row], data[row-1], time));
        }

        QVector<float> lenght;

        for (int i = 0; i < POINT; ++i)
        {
            QGraphicsLineItem *line = new QGraphicsLineItem(cur[i][Axe1], cur[i][Axe2], cur[i+1][Axe1], cur[i+1][Axe2]);

            lenght.push_back(distance3D(cur[i], cur[i+1]));

            line->setPen(QPen(QBrush(color), 1));
            line->setZValue(ZValue::CURVE);
            addItem(line);
        }

        float total = 0.0f;
        for (int i = 0; i < POINT; ++i)
            total += lenght[i];

        if (static_cast<VectorType>(type) == VectorType::POSITION) lengthPos.push_back(total);
        else lengthTar.push_back(total);
    }
}

void Scene::createBezierInterpolation(VectorType type, QVector<QVector<QVector<float> > > data, QVector<int> stamp, bool acc)
{
    QColor color = Qt::black;

    switch (type) {
    case VectorType::POSITION:
        color = QColor(31, 97, 141);
        lengthPos.clear();
        break;
    case VectorType::TARGET:
        color = QColor(146, 43, 33);
        lengthTar.clear();
        break;
    case VectorType::ROLL:
        break;
    }

    QVector<QVector3D> cur;
    for (int row = 0; row < data.size()-1; ++row)
    {
        cur.clear();

        float time;
        for (int n = 0; n < POINT+1; ++n)
        {
            time = (float)n / (float)POINT;
            cur.push_back(interpolationBezier(data[row], data[row+1], time));
        }

        QVector<float> lenght;

        for (int i = 0; i < POINT; ++i)
        {
            QGraphicsLineItem *line = new QGraphicsLineItem(cur[i][Axe1], cur[i][Axe2], cur[i+1][Axe1], cur[i+1][Axe2]);

            lenght.push_back(distance3D(cur[i], cur[i+1]));

            /*int lastStamp = (row == 0) ? 0 : stamp[row];
            if ( acc )
                qDebug() << lenght.last()
                         << stamp[row+1]
                         << stamp[row+1]-lastStamp
                         << (float)(stamp[row+1]-lastStamp)/(float)POINT
                         << (float)(stamp[row+1]-lastStamp)/(float)POINT / lenght.last() ;*/

            if ( acc )
            {
                int lastStamp = (row == 0) ? 0 : stamp[row];
                color = accelerationColor((float)(stamp[row+1]-lastStamp)/(float)POINT / lenght.last());
            }

            line->setPen(QPen(QBrush(color), 1));
            line->setZValue(ZValue::CURVE);
            addItem(line);
        }

        float total = 0.0f;
        for (int i = 0; i < POINT; ++i)
            total += lenght[i];

        if (static_cast<VectorType>(type) == VectorType::POSITION) lengthPos.push_back(total);
        else lengthTar.push_back(total);
    }
}

void Scene::createViewLine(QVector<QVector<QVector<float> > > dataPos,
                           QVector<QVector<QVector<float> > > dataTar,
                           int rPos, int rTar,
                           float fPos, float fTar,
                           std::uint16_t iPos, std::uint16_t iTar)
{
    QVector3D stampPosition;
    QVector3D stampTarget;

    if ( rPos == dataPos.size() - 1 )
    {
        stampPosition = QVector3D(dataPos[rPos][POS_SELF][X],
                                  dataPos[rPos][POS_SELF][Y],
                                  dataPos[rPos][POS_SELF][Z]);
        goto jump_pos;
    }

    switch (static_cast<Interpolation>(iPos)) {
    case Interpolation::LINEAR:
        stampPosition  = interpolationLinear(dataPos[rPos], dataPos[rPos+1], fPos);
        break;
    case Interpolation::BEZIER:
        stampPosition  = interpolationBezier(dataPos[rPos], dataPos[rPos+1], fPos);
        break;
    case Interpolation::HERMITE:
        stampPosition  = interpolationHermite(dataPos[rPos], dataPos[rPos+1], fPos);
        break;
    case Interpolation::NONE:
        break;
    }

    jump_pos:

    if ( rTar == dataTar.size() - 1 )
    {
        stampTarget = QVector3D(dataTar[rTar][POS_SELF][X],
                                dataTar[rTar][POS_SELF][Y],
                                dataTar[rTar][POS_SELF][Z]);

        goto jump_tar;
    }

    switch (static_cast<Interpolation>(iTar)) {
    case Interpolation::LINEAR:
        stampTarget  = interpolationLinear(dataTar[rTar], dataTar[rTar+1], fTar);
        break;
    case Interpolation::BEZIER:
        stampTarget  = interpolationBezier(dataTar[rTar], dataTar[rTar+1], fTar);
        break;
    case Interpolation::HERMITE:
        stampTarget  = interpolationHermite(dataTar[rTar], dataTar[rTar+1], fTar);
        break;
    case Interpolation::NONE:
        break;
    }

    jump_tar:

    removeItemsFromScene({ZValue::VIEWLINE});

    QGraphicsLineItem *line = new QGraphicsLineItem(stampPosition[Axe1], stampPosition[Axe2], stampTarget[Axe1], stampTarget[Axe2]);
    QGraphicsEllipseItem *where = new QGraphicsEllipseItem(stampTarget[Axe1]-3, stampTarget[Axe2]-3, 6, 6);
    line->setPen(QPen(QBrush(Qt::black), 2, Qt::DotLine));
    where->setPen(line->pen());
    line->setZValue(ZValue::VIEWLINE);
    where->setZValue(line->zValue());
    addItem(line);
    addItem(where);
}

QVector3D Scene::interpolationLinear(QVector<QVector<float> > &value1, QVector<QVector<float> > &value2, float percent)
{
    float h1 = 1 - percent;
    float h2 = percent;

    QVector3D n1 = QVector3D(value1[POS_SELF][X], value1[POS_SELF][Y], value1[POS_SELF][Z]);
    QVector3D n2 = QVector3D(value2[POS_SELF][X], value2[POS_SELF][Y], value2[POS_SELF][Z]);

    return n1.operator*=(h1) + n2.operator*=(h2);
}

QVector3D Scene::interpolationHermite(QVector<QVector<float>> &value1, QVector<QVector<float>> &value2, float percent)
{
    float h1 = 2.0f*qPow(percent,3) - 3.0f*qPow(percent,2) + 1.0f;
    float h2 = -2.0f*qPow(percent,3) + 3.0f*qPow(percent,2);
    float h3 = qPow(percent,3) - 2.0f*qPow(percent,2) + percent;
    float h4 = qPow(percent,3) - qPow(percent,2);

    QVector3D n1 = QVector3D(value1[POS_SELF][X], value1[POS_SELF][Y], value1[POS_SELF][Z]);
    QVector3D n2 = QVector3D(value2[POS_SELF][X], value2[POS_SELF][Y], value2[POS_SELF][Z]);
    QVector3D n3 = QVector3D(value1[POS_IN][X],   value1[POS_IN][Y],   value1[POS_IN][Z]);
    QVector3D n4 = QVector3D(value2[POS_OUT][X],  value2[POS_OUT][Y],  value2[POS_OUT][Z]);

    return n1.operator*=(h1) + n2.operator*=(h2) + n3.operator*=(h3) + n4.operator*=(h4);
}

QVector3D Scene::interpolationBezier(QVector<QVector<float> > &value1, QVector<QVector<float> > &value2, float percent)
{
    float h0 = -1.0f*qPow(percent, 3) + 3.0f*qPow(percent, 2) - 3.0f*percent + 1.0f;
    float h1 = 3.0f*qPow(percent, 3) - 6.0f*qPow(percent, 2) + 3.0f*percent;
    float h2 = -3.0f*qPow(percent, 3) + 3.0f*qPow(percent, 2);
    float h3 = qPow(percent, 3);

    QVector3D p0 = QVector3D(value1[POS_SELF][X], value1[POS_SELF][Y], value1[POS_SELF][Z]);
    QVector3D p1 = QVector3D(value1[POS_OUT][X],  value1[POS_OUT][Y],  value1[POS_OUT][Z]);
    QVector3D p2 = QVector3D(value2[POS_IN][X],   value2[POS_IN][Y],   value2[POS_IN][Z]);
    QVector3D p3 = QVector3D(value2[POS_SELF][X], value2[POS_SELF][Y], value2[POS_SELF][Z]);

    return p0.operator*=(h0) + p1.operator*=(h1) + p2.operator*=(h2) + p3.operator*=(h3);
}

float Scene::distance3D(QVector3D &vec1, QVector3D &vec2)
{
    float h1 = qPow((vec2.x() - vec1.x()),2);
    float h2 = qPow((vec2.y() - vec1.y()),2);
    float h3 = qPow((vec2.z() - vec1.z()),2);

    return qSqrt(h1 + h2 + h3);
}

QColor Scene::accelerationColor(float reference)
{
    QColor c;

    float range_min = accRatio * (1.0f-((float)accPercent/100.0f)); // RED
    float range_max = accRatio * (1.0f+((float)accPercent/100.0f)); // BLUE

    if ( reference <= range_min )
    {
        c = QColor(highSpeed[0], highSpeed[1], highSpeed[2]);
    }
    else if ( reference >= range_max )
    {
        c = QColor(lowSpeed[0], lowSpeed[1], lowSpeed[2]);
    }
    else
    {
        float percent = (reference-range_min)*100.0f/(range_max-range_min);

        float newRed = highSpeed[0] - diffSpeed[0]*(percent/100.0f);
        float newGreen = highSpeed[1] + diffSpeed[1]*(percent/100.0f);
        float newBlue = highSpeed[2] + diffSpeed[2]*(percent/100.0f);

        c = QColor((int)newRed, (int)newGreen, (int)newBlue);
    }
    return c;
}

int Scene::getDisplay()
{
    if ( static_cast<Coord>(Axe1) == Coord::X )
    {
        if ( static_cast<Coord>(Axe2) == Coord::Y )
            return 0;

        return 1;
    }

    return 2;
}

void Scene::removeItemsFromScene(QVector<int> type)
{
    for (QGraphicsItem* item : items()) {
        if (type.contains(item->zValue())) {
            item->setSelected(true);
            clearSelection();
            removeItem(item);
        }
    }
}

void Scene::generateSceneSize()
{
    Vect2D *topLeft = new Vect2D(-1,-1,-1, 1);
    topLeft->setPos(-2500,2500);
    Vect2D *bottomRight = new Vect2D(-1,-1,-1, 1);
    bottomRight->setPos(2500,-2500);
    addItem(topLeft);
    addItem(bottomRight);
}
