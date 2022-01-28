#ifndef M2_H
#define M2_H

#include <QDir>
#include <QThread>
#include <QSettings>

template<int N>
using VecF = std::array<float, N>;

#pragma pack(push, 1)
template<typename  T>
struct timestampedValue
{
    std::uint32_t stamp;
    T data;
};
#pragma pack(pop)

template<typename T>
void write(FILE* file, int where, T value)
{
    fseek(file, where, SEEK_SET);
    fwrite(&value, sizeof (value), 1, file);
}

#define _CHR(a,b,c,d) ( ((a)<<24) | ((b)<<16) | ((c)<<8) | (d) )

class M2 : public QThread
{
    Q_OBJECT
public:
    explicit M2(QObject *parent) : QThread(parent) {};
    virtual ~M2() {};

    void run();
    void read();
    void update();

    void setName(QString s) { name = s; };
    void setPath(QString p) { path = p; };
    void setInterpolation(std::uint16_t i[3]) {
        for(int n = 0; n < 3; ++n)
            interpolation[n] = i[n];
    };
    void setAnimationLength(std::uint32_t al) { animationLength = al; };
    void setPositions(std::vector<timestampedValue<VecF<9> > > p) { positions = p; };
    void setTargets(std::vector<timestampedValue<VecF<9> > > t) { targets = t; };
    void setRolls(std::vector<timestampedValue<VecF<3> > > r) { rolls = r; };

    QString getName() { return read_name; };
    bool getIsRead() { return isRead; };
    std::uint16_t getInterpolation(int i) { return read_interpolation[i]; };
    std::uint32_t getAnimationLength() { return read_animationLength; };
    std::vector<timestampedValue<VecF<9>>> getPositions() { return positions; };
    std::vector<timestampedValue<VecF<9>>> getTargets() { return targets; };
    std::vector<timestampedValue<VecF<3>>> getRolls() { return rolls; };

signals:
    void runDone();
    void readDone();
    void updateDone();

private:
    void writeHeader(FILE*);
    void writeAnimations(FILE*);
    void writeBones(FILE*);
    void writeCameras(FILE*);

    QString name;
    QString path;
    std::uint32_t animationLength;
    std::uint16_t interpolation[3] = {0x0001,0x0001,0x0001};
    std::vector<timestampedValue<VecF<9> > > positions;
    std::vector<timestampedValue<VecF<9> > > targets;
    std::vector<timestampedValue<VecF<3> > > rolls;

    // read
    bool isRead = false;
    std::uint16_t read_interpolation[3];
    std::uint32_t read_ofsTimestamp_struct[3];
    std::uint32_t read_ofsValues_struct[3];
    std::uint32_t read_ofsTimestamp[3];
    std::uint32_t read_ofsValues[3];
    std::uint32_t read_nVec[3];
    std::uint32_t read_animationLength;
    QString read_name;
};

#endif // M2_H
