#ifndef PROCESS_H
#define PROCESS_H

#include <QThread>
#include <QSettings>
#include <windows.h>

class Process : public QThread
{
    Q_OBJECT

public:
    explicit Process(QObject *parent) : QThread(parent) {};
    virtual ~Process() {};

    void run() {
        Success = false;
        Error = NULL;

        if ( clientName().isEmpty() )
        {
            Error = QString("Error: You must set an window name");
            return;
        }

        HWND hWnd = FindWindowA(0, (clientName().toStdString().c_str()));

        if ( NULL == hWnd )
        {
            Error = QString("Error: Can't find window: \'%1\'").arg(clientName());
            return;
        }

        GetWindowThreadProcessId(hWnd, &pid);
        HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

        for (int n = 0; n < 3; ++n)
            ReadProcessMemory(pHandle, reinterpret_cast<LPVOID>(addressPlayerPos[n]), &playerPos[n], sizeof (float), 0);

        ReadProcessMemory(pHandle, reinterpret_cast<LPVOID>(addressPlayerO), &playerOrientation, sizeof (float), 0);

        /*ReadProcessMemory(pHandle, reinterpret_cast<LPVOID>(startingPoint+staticTransport), &p1, sizeof (p1), 0);
        ReadProcessMemory(pHandle, reinterpret_cast<LPVOID>(p1 + 0x8), &p2, sizeof (p2), 0);
        ReadProcessMemory(pHandle, reinterpret_cast<LPVOID>(p2 + 0x18), &transportEntity, sizeof (transportEntity), 0);

        for (int n = 0; n < 3; ++n)
            ReadProcessMemory(pHandle, reinterpret_cast<LPVOID>(transportEntity + 0x10 + (n*0x4)), &transPos[n], sizeof (float), 0);

        ReadProcessMemory(pHandle, reinterpret_cast<LPVOID>(transportEntity + orientationOffset), &transOrientation, sizeof (float), 0);

        if (!diffCalculated && transOrientation != -1.0f)
        {
            diffOrientation = (playerOrientation - transOrientation);

            if ( diffOrientation < 0.0000001f )
            {
                diffOrientation += M_PI*2;
            }

            diffCalculated = true;
        }*/

        Success = true;
    };

    float getCoord(int n, float r, float origin[3], bool applyRotation, bool applyOffset)
    {
        float ret = (applyOffset) ? playerPos[n] - origin[n] : playerPos[n];
        float offX = playerPos[0] - origin[0];
        float offY = playerPos[1] - origin[1];

        if (applyRotation)
        {
            switch(n)
            {
            case 0:
                ret = (ret*cos(r)-offY*sin(r));
                break;
            case 1:
                ret = (offX*sin(r)+ret*cos(r));
                break;
            }
        }

        return ret;
    };

    /*float getTransCoord(int n, float r, bool applyRotation, float origin[3], bool applyOffset)
    {
        float ret = (applyOffset) ? transPos[n] - origin[n] : transPos[n];
        float offX = transPos[0] - origin[0];
        float offY = transPos[1] - origin[1];

        switch(n)
        {
            case 0: if ( applyRotation ) ret = (ret*cos(r)-offY*sin(r)); break;
            case 1: if ( applyRotation ) ret = (offX*sin(r)+ret*cos(r)); break;
        }

        return ret;
    };*/

    //float getTransOrientation() { return transOrientation; };
    float getPlayerOrientation() { return playerOrientation; };
    //float getDiffOrientation() { return diffOrientation; };

    QString clientName() {
        QSettings setting("WOW-EDITOR", "CameraCinematic");
        QString name;

        setting.beginGroup("CLIENT-SETTINGS");
        name = setting.value("client-window", "").toString();
        setting.endGroup();

        return name;
    };

    QString getError() { return Error; };
    bool getSuccess() { return Success; };

private:
    DWORD pid;
    DWORD addressPlayerPos[3] = {
        0x00adf4e4,
        0x00adf4e8,
        0x00adf4ec
    };
    DWORD addressPlayerO = 0x00beba70;

    //DWORD startingPoint = 0x00400000;
    //DWORD staticTransport = 0x006DB750;
    //DWORD p1, p2, transportEntity;
    DWORD orientationOffset = 0x02C8;

    bool Success = false;
    float playerPos[3] = {.0f,.0f,.0f};
    float playerOrientation = -1.0f;
    //float transPos[3] = {.0f,.0f,.0f};
    //float transOrientation = -1.0f;

    //bool diffCalculated = false;
    //float diffOrientation = -1.0f;
    QString Error = NULL;
};

#endif // PROCESS_H
