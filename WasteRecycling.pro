QT += core gui widgets sql serialport charts qml quick quickwidgets network multimedia positioning printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets charts


CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Camion.cpp \
    affectation.cpp \
    arduino.cpp \
    client.cpp \
    connection.cpp \
    conteneur.cpp \
    employee.cpp \
    machine.cpp \
    main.cpp \
    point_de_collecte.cpp \
    recyclage.cpp \
    routemanager.cpp \
    smtpclient.cpp \
    wasterecycling.cpp

HEADERS += \
    Camion.h \
    affectation.h \
    arduino.h \
    client.h \
    connection.h \
    conteneur.h \
    employee.h \
    machine.h \
    point_de_collecte.h \
    recyclage.h \
    routemanager.h \
    smtpclient.h \
    wasterecycling.h

FORMS += \
    wasterecycling.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    CamionsMetier.qrc \
    sidebarRessources.qrc
QT+= sql
CONFIG += console
QT+= quickwidgets location positioning network
INCLUDEPATH += C:/Qt/6.7.3/mingw_64/include/QtCharts
LIBS += -LC:/Qt/6.7.3/mingw_64/lib -lQt6Charts
