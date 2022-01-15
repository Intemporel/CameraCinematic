#ifndef SCENE_H
#define SCENE_H

#include "vect2d.h"
#include <QGraphicsScene>

enum VectorType {
    POSITION,
    TARGET,
    ROLL
};

enum VectorPos {
    POS_SELF,
    POS_IN,
    POS_OUT
};

enum Coord {
    X,
    Y,
    Z
};

class Scene : public QGraphicsScene
{

public:
    Scene();
    void changeDisplay(Coord a1, Coord a2) { Axe1 = a1; Axe2 = a2; };
    void createVector(int row, VectorType type, VectorPos pos, QVector<float>);

private:
    void generateSceneSize();
    int Axe1 = Coord::X;
    int Axe2 = Coord::Y;
};

#endif // SCENE_H
