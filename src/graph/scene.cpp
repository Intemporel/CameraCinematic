#include "scene.h"

Scene::Scene()
{
    generateSceneSize();
}

void Scene::createVector(int row, VectorType type, VectorPos pos, QVector<float> position)
{
    QColor color = Qt::black;

    switch (type) {
    case VectorType::POSITION:
        if ( row % 2 == 0 )
        {
            color = QColor(31, 97, 141);

            if ( pos != VectorPos::POS_SELF )
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

            if ( pos != VectorPos::POS_SELF )
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

            if ( pos != VectorPos::POS_SELF )
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

    if ( position.size() == 0 )
        return;

    Vect2D *vec = new Vect2D(0, 4, color);
    vec->setPos(position[Axe1], position[Axe2]);
    addItem(vec);
}

void Scene::generateSceneSize()
{
    Vect2D *topLeft = new Vect2D(0, 1);
    topLeft->setPos(-2500,2500);
    Vect2D *bottomRight = new Vect2D(0, 1);
    bottomRight->setPos(2500,-2500);
    addItem(topLeft);
    addItem(bottomRight);
}
