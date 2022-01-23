QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    src/cameracinematic.cpp \
    src/file/m2.cpp \
    src/file/skin.cpp \
    src/graph/scene.cpp \
    src/graph/vect2d.cpp \
    src/graph/view.cpp \
    src/menu.cpp \
    src/settings.cpp

HEADERS += \
    src/cameracinematic.h \
    src/file/DBCReader.h \
    src/file/m2.h \
    src/file/skin.h \
    src/graph/scene.h \
    src/graph/vect2d.h \
    src/graph/view.h \
    src/menu.h \
    src/process.h \
    src/settings.h

FORMS += \
    src/Editor.ui \
    src/menu.ui \
    src/settings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ressources/rec.qrc
