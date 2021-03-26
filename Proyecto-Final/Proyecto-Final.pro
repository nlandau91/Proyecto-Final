QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    analyzer.cpp \
    appsettings.cpp \
    comparator.cpp \
    configdialog.cpp \
    database.cpp \
    edge.cpp \
    fingerprinttemplate.cpp \
    main.cpp \
    mainwindow.cpp \
    outputwindow.cpp \
    preprocesser.cpp \
    stats.cpp \
    tester.cpp \
    triangle.cpp \
    utils.cpp

HEADERS += \
    analyzer.h \
    appsettings.h \
    common.h \
    comparator.h \
    configdialog.h \
    database.h \
    edge.h \
    fingerprinttemplate.h \
    mainwindow.h \
    outputwindow.h \
    preprocesser.h \
    stats.h \
    tester.h \
    triangle.h \
    utils.h

FORMS += \
    configdialog.ui \
    mainwindow.ui \
    outputwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# OpenCV
INCLUDEPATH += /usr/local/include/opencv4
LIBS += `pkg-config --cflags --libs opencv4`

#This is a release build
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
LIBS += -fopenmp

DISTFILES += \
    default.ini
