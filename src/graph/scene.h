#ifndef SCENE_H
#define SCENE_H

#include "vect2d.h"
#include <QGraphicsScene>
#include <QVector3D>

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

enum Interpolation {
    NONE,
    LINEAR,
    BEZIER,
    HERMITE,
};

enum ZValue {
    BACKGROUND,
    CURVE,
    JOIN,
    VIEWLINE,
    VECTOR,
    SELECTOR
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
    void createVector(int row, VectorType type, VectorPos pos, QVector<float> data, int interpolation);

    void createLinearInterpolation(VectorType type, QVector<QVector<QVector<float>>> data, QVector<int> stamp, bool acc);
    void createHermiteInterpolation(VectorType type, QVector<QVector<QVector<float>>> data, QVector<int> stamp, bool acc);
    void createBezierInterpolation(VectorType type, QVector<QVector<QVector<float>>> data, QVector<int> stamp, bool acc);

    void createViewLine(QVector<QVector<QVector<float>>>,
                        QVector<QVector<QVector<float>>>,
                        int, int,
                        float, float,
                        std::uint16_t, std::uint16_t);

    void viewSelected(Vect2D*);

    inline QVector3D interpolationLinear(QVector<QVector<float>> &value1, QVector<QVector<float>> &value2, float percent);
    inline QVector3D interpolationHermite(QVector<QVector<float>> &value1, QVector<QVector<float>> &value2, float percent);
    inline QVector3D interpolationBezier(QVector<QVector<float>> &value1, QVector<QVector<float>> &value2, float percent);

    float distance3D(QVector3D &vec1, QVector3D &vec2);
    QColor accelerationColor(float reference);

    int getDisplay();

    void setAccRation(float r) { accRatio = r; };
    void setAccPercent(int p) { accPercent = p; };

    QVector<float> getLenghtPos() { return lenghtPos; };
    QVector<float> getLenghtTar() { return lenghtTar; };

    void removeItemsFromScene(QVector<int>);

private:
    void generateSceneSize();

    const int POINT = 50;

    int Axe1 = Coord::X;
    int Axe2 = Coord::Y;

    int lowSpeed[3] = {46, 134, 193};
    int highSpeed[3] = {192, 57, 43};
    int diffSpeed[3] = {(192-46), (134-57), (193-43)};

    float accRatio = 25.0f;
    int accPercent = 50;

    QVector<float> lenghtPos;
    QVector<float> lenghtTar;
};

#endif // SCENE_H
