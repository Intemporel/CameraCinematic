#include "scene.h"

Scene::Scene()
{
    generateSceneSize();
}

void Scene::viewSelected(Vect2D *item)
{
    if ( item == NULL )
        return;

    int type = item->getType();
    int row = item->getRow();

    Vect2D *first = NULL;
    Vect2D *second = NULL;

    removeItemsFromScene({ZValue::JOIN, ZValue::SELECTOR});

    for (QGraphicsItem* i : items())
    {
        if (Vect2D *n = static_cast<Vect2D*>(i))
        {
            if (n->getType() == type
                    && n->getRow() == row
                    && n->getVec() != item->getVec())
            {
                first = n;
                break;
            }
        }
    }

    if ( first != NULL )
    {

        for (QGraphicsItem* i : items())
        {
            if (Vect2D *n = static_cast<Vect2D*>(i))
            {
                if (n->getType() == type
                        && n->getRow() == row
                        && n->getVec() != item->getVec()
                        && n->getVec() != first->getVec())
                {
                    second = n;
                    break;
                }
            }
        }

        Vect2D *parent = NULL;
        Vect2D *in = NULL;
        Vect2D *out = NULL;

        switch (static_cast<VectorPos>(item->getVec()))
        {
        case VectorPos::POS_SELF: parent = item; break;
        case VectorPos::POS_IN: in = item; break;
        case VectorPos::POS_OUT: out = item; break;
        }

        switch (static_cast<VectorPos>(first->getVec()))
        {
        case VectorPos::POS_SELF: parent = first; break;
        case VectorPos::POS_IN: in = first; break;
        case VectorPos::POS_OUT: out = first; break;
        }

        if ( second != NULL )
        {
            switch (static_cast<VectorPos>(second->getVec()))
            {
            case VectorPos::POS_SELF: parent = second; break;
            case VectorPos::POS_IN: in = second; break;
            case VectorPos::POS_OUT: out = second; break;
            }
        }

        if ( parent == NULL )
            goto selectorOnly;

        if ( in != NULL )
        {
            QGraphicsLineItem *line = new QGraphicsLineItem(in->x(), in->y(), parent->x(), parent->y());
            line->setPen(QPen(QBrush(Qt::black), 1, Qt::DotLine, Qt::PenCapStyle::RoundCap, Qt::PenJoinStyle::RoundJoin));
            line->setZValue(ZValue::JOIN);
            addItem(line);
        }

        if ( out != NULL )
        {
            QGraphicsLineItem *line = new QGraphicsLineItem(out->x(), out->y(), parent->x(), parent->y());
            line->setPen(QPen(QBrush(Qt::black), 1, Qt::DotLine, Qt::PenCapStyle::RoundCap, Qt::PenJoinStyle::RoundJoin));
            line->setZValue(ZValue::JOIN);
            addItem(line);
        }
    }

    selectorOnly:

    QGraphicsEllipseItem *r = new QGraphicsEllipseItem(item->x() - item->getSize()/2,
                                                       item->y() - item->getSize()/2,
                                                       item->getSize(), item->getSize());
    r->setPen(QPen(QBrush(Qt::black), 1, Qt::DotLine, Qt::PenCapStyle::RoundCap, Qt::PenJoinStyle::RoundJoin));
    r->setZValue(ZValue::SELECTOR);
    addItem(r);
}

void Scene::createVector(int row, VectorType type, VectorPos pos, QVector<float> data, int inter)
{
    if ( static_cast<Interpolation>(inter) == Interpolation::NONE
         || static_cast<Interpolation>(inter) == Interpolation::LINEAR )
        if ( pos == VectorPos::POS_IN || pos == VectorPos::POS_OUT )
            return;

    QSettings setting = QSettings("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("COLOR-SETTINGS");
    QColor posPrim = QColor(setting.value("pos-prim", QColor(31, 97, 141).name()).toString());
    QColor posSec = QColor(setting.value("pos-sec", QColor(20, 143, 119).name()).toString());
    QColor tarPrim = QColor(setting.value("tar-prim", QColor(146, 43, 33).name()).toString());
    QColor tarSec = QColor(setting.value("tar-sec", QColor(118, 68, 138).name()).toString());
    setting.endGroup();

    QColor color = Qt::black;


    switch (type) {
    case VectorType::POSITION:
        if ( row % 2 == 0 )
        {
            color = posPrim;

            if ( pos != VectorPos::POS_SELF)
                color = subColor(posPrim);
        }
        else
        {
            color = posSec;

            if ( pos != VectorPos::POS_SELF)
                color = subColor(posSec);
        }
        break;
    case VectorType::TARGET:
        if ( row % 2 == 0 )
        {
            color = tarPrim;

            if ( pos != VectorPos::POS_SELF)
                color = subColor(tarPrim);
        }
        else
        {
            color = tarSec;

            if ( pos != VectorPos::POS_SELF)
                color = subColor(tarSec);
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
    ZValue CurveZ = (drawCurveOnTop) ? ZValue::FORCED_CURVE : ZValue::CURVE;

    QSettings setting = QSettings("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("COLOR-SETTINGS");
    QColor posPrim = QColor(setting.value("pos-prim", QColor(31, 97, 141).name()).toString());
    QColor tarPrim = QColor(setting.value("tar-prim", QColor(146, 43, 33).name()).toString());
    setting.endGroup();

    switch (type) {
    case VectorType::POSITION:
        color = posPrim;
        lenghtPos.clear();
        break;
    case VectorType::TARGET:
        color = tarPrim;
        lenghtTar.clear();
        break;
    case VectorType::ROLL:
        break;
    }

    for (int row = 0; row < data.size()-1; ++row)
    {
        QVector3D start = interpolationLinear(data[row], data[row+1], 0.0f);
        QVector3D end   = interpolationLinear(data[row], data[row+1], 1.0f);

        QGraphicsLineItem *line = new QGraphicsLineItem(start[Axe1], start[Axe2], end[Axe1], end[Axe2]);

        if (static_cast<VectorType>(type) == VectorType::POSITION) lenghtPos.push_back(distance3D(start, end));
        else lenghtTar.push_back(distance3D(start, end));

        if ( acc )
        {
            int lastStamp = (row == 0) ? 0 : stamp[row];

            if (static_cast<VectorType>(type) == VectorType::POSITION) color = accelerationColor((float)(stamp[row+1]-lastStamp)/(float)POINT / lenghtPos.last());
            else color = accelerationColor((float)(stamp[row+1]-lastStamp)/(float)POINT / lenghtTar.last());
        }

        line->setPen(QPen(QBrush(color), 1));
        line->setZValue(CurveZ);
        addItem(line);
    }
}

void Scene::createHermiteInterpolation(VectorType type, QVector<QVector<QVector<float> > > data, QVector<int> stamp, bool acc)
{
    QColor color = Qt::black;
    ZValue CurveZ = (drawCurveOnTop) ? ZValue::FORCED_CURVE : ZValue::CURVE;

    QSettings setting = QSettings("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("COLOR-SETTINGS");
    QColor posPrim = QColor(setting.value("pos-prim", QColor(31, 97, 141).name()).toString());
    QColor tarPrim = QColor(setting.value("tar-prim", QColor(146, 43, 33).name()).toString());
    setting.endGroup();

    switch (type) {
    case VectorType::POSITION:
        color = posPrim;
        lenghtPos.clear();
        break;
    case VectorType::TARGET:
        color = tarPrim;
        lenghtTar.clear();
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

            if ( acc )
            {
                int lastStamp = (row == 0) ? 0 : stamp[row];
                color = accelerationColor((float)(stamp[row+1]-lastStamp)/(float)POINT / lenght.last());
            }

            line->setPen(QPen(QBrush(color), 1));
            line->setZValue(CurveZ);
            addItem(line);
        }

        float total = 0.0f;
        for (int i = 0; i < POINT; ++i)
            total += lenght[i];

        if (static_cast<VectorType>(type) == VectorType::POSITION) lenghtPos.push_back(total);
        else lenghtTar.push_back(total);
    }
}

void Scene::createBezierInterpolation(VectorType type, QVector<QVector<QVector<float> > > data, QVector<int> stamp, bool acc)
{
    QColor color = Qt::black;
    ZValue CurveZ = (drawCurveOnTop) ? ZValue::FORCED_CURVE : ZValue::CURVE;

    QSettings setting = QSettings("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("COLOR-SETTINGS");
    QColor posPrim = QColor(setting.value("pos-prim", QColor(31, 97, 141).name()).toString());
    QColor tarPrim = QColor(setting.value("tar-prim", QColor(146, 43, 33).name()).toString());
    setting.endGroup();

    switch (type) {
    case VectorType::POSITION:
        color = posPrim;
        lenghtPos.clear();
        break;
    case VectorType::TARGET:
        color = tarPrim;
        lenghtTar.clear();
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

            if ( acc )
            {
                int lastStamp = (row == 0) ? 0 : stamp[row];
                color = accelerationColor((float)(stamp[row+1]-lastStamp)/(float)POINT / lenght.last());
            }

            line->setPen(QPen(QBrush(color), 1));
            line->setZValue(CurveZ);
            addItem(line);
        }

        float total = 0.0f;
        for (int i = 0; i < POINT; ++i)
            total += lenght[i];

        if (static_cast<VectorType>(type) == VectorType::POSITION) lenghtPos.push_back(total);
        else lenghtTar.push_back(total);
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

    QSettings setting = QSettings("WOW-EDITOR", "CameraCinematic");
    setting.beginGroup("COLOR-SETTINGS");
    QColor highSpeed = QColor(setting.value("high-speed", QColor(192, 57, 43).name()).toString());
    QColor lowSpeed = QColor(setting.value("low-speed", QColor(46, 134, 193).name()).toString());
    setting.endGroup();

    float range_min = accRatio * (1.0f-((float)accPercent/100.0f)); // HIGH
    float range_max = accRatio * (1.0f+((float)accPercent/100.0f)); // LOW

    if ( reference <= range_min )
    {
        c = highSpeed;
    }
    else if ( reference >= range_max )
    {
        c = lowSpeed;
    }
    else
    {
        float percent = (reference-range_min)*100.0f/(range_max-range_min);

        int diffSpeed[3] = {
            (lowSpeed.red()-highSpeed.red()),
            (lowSpeed.green()-highSpeed.green()),
            (lowSpeed.blue()-highSpeed.blue())
        };

        int coeffR = (diffSpeed[0] < 0) ? -1 : 1;
        int coeffG = (diffSpeed[1] < 0) ? -1 : 1;
        int coeffB = (diffSpeed[2] < 0) ? -1 : 1;

        float newRed =   highSpeed.red()   + (qAbs(diffSpeed[0])*(percent/100.0f))*coeffR;
        float newGreen = highSpeed.green() + (qAbs(diffSpeed[1])*(percent/100.0f))*coeffG;
        float newBlue =  highSpeed.blue()  + (qAbs(diffSpeed[2])*(percent/100.0f))*coeffB;

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
