#-------------------------------------------------
#
# Project created by QtCreator 2019-10-04T18:27:53
#
#-------------------------------------------------

QT += core gui
QT += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fhex
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Set MINIMAL version by default, i.e. without capstone/keystone support
# to add the support comment the following line:
DEFINES += MINIMAL

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# App Icon (Windows)
RC_ICONS = icon.ico

SOURCES += \
    src/fasm.cpp \
    src/chunks.cpp \
    src/commands.cpp \
    src/core/patternmatching.cpp \
    src/main.cpp \
    src/fhex.cpp \
    src/core/hexeditor.cpp \
    src/qhexedit.cpp

HEADERS += \
    src/fasm.h \
    src/chunks.h \
    src/commands.h \
    src/core/json.h \
    src/core/patternmatching.h \
    src/fhex.h \
    src/core/hexeditor.h \
    src/qhexedit.h

QMAKE_LFLAGS += -lkeystone -lcapstone -L./lib
