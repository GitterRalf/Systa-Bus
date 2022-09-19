#-------------------------------------------------
#
# Project created by QtCreator 2013-11-27T09:31:09
#
#-------------------------------------------------

OBJECTS_DIR=generated_files #Intermediate object files directory
MOC_DIR=generated_files     #Intermediate moc files directory
RCC_DIR=generated_files     #Intermediate ressource files directory


QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport printsupport network

TARGET = Systa_Bus
TEMPLATE = app
RC_FILE = windows.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot/qcustomplot.cpp \
    server.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    qcustomplot/qcustomplot.h \
    server.h \
    aboutdialog.h

FORMS    += mainwindow.ui \
    setting.ui \
    aboutdialog.ui

OTHER_FILES += \
    ReleaseNotes.txt \
    windows.rc

RESOURCES += \
    Ressource.qrc
