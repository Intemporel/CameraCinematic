#ifndef PROCESS_H
#define PROCESS_H

#include <QThread>
#include <QDebug>
#include <windows.h>

enum AXES {
    X,
    Y,
    Z
};

class Process : public QThread
{
    Q_OBJECT

public:
    explicit Process(QObject *parent) : QThread(parent) {};
    virtual ~Process() {};

    void run() {
        Success = false;
        Error = NULL;

        if ( windowName.isEmpty() )
        {
            Error = QString("Error: You must set an window name");
            return;
        }

        HWND hWnd = FindWindowA(0, (windowName.toStdString().c_str()));

        if ( NULL == hWnd )
        {
            Error = QString("Error: Can't find window: \'%1\'").arg(windowName);
            return;
        }

        GetWindowThreadProcessId(hWnd, &pid);
        HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

        for (int n = 0; n < 3; ++n)
            ReadProcessMemory(pHandle, (LPVOID)(addressCam[n]), &cam[n], sizeof(float), 0);

        Success = true;
    };

    float getCoord(int n,
                   float r, bool applyRotation,
                   float origin[3], bool applyOffset) {

        float ret = (applyOffset) ? cam[n] - origin[n] : cam[n];
        float offX = cam[X] - origin[X];
        float offY = cam[Y] - origin[Y];

        switch(n)
        {
            case X: if ( applyRotation ) ret = (ret*cos(r)-offY*sin(r)); break;
            case Y: if ( applyRotation ) ret = (offX*sin(r)+ret*cos(r)); break;
        }

        return ret;
    };
    QString getError() { return Error; };
    bool getSuccess() { return Success; };
    void setWindowName(QString s) { windowName = s; };

private:
    DWORD pid;
    DWORD addressCam[3] = {
        0x00adf4e4,
        0x00adf4e8,
        0x00adf4ec
    };

    bool Success = false;
    float cam[3] = {.0f,.0f,.0f};
    QString windowName;
    QString Error = NULL;
};

#endif // PROCESS_H
