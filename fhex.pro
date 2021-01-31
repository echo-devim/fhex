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

# Headers
INCLUDEPATH += C:\Users\icarus\Desktop\DEV\C\keystone-0.9.2\include
INCLUDEPATH += C:\Users\icarus\Desktop\DEV\C\capstone-4.0.2\include

# Libs 64 Bits
win32:contains(QMAKE_HOST.arch, x86_64) {
    LIBS += -L$$PWD/../build-keystone-0.9.2-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/llvm/lib/ -lkeystone
    LIBS += -L$$PWD/../build-capstone-4.0.2-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/ -lcapstone_dll

    INCLUDEPATH += $$PWD/../build-keystone-0.9.2-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/llvm/include
    INCLUDEPATH += $$PWD/../build-capstone-4.0.2-Desktop_Qt_5_15_2_MSVC2019_64bit-Release

    DEPENDPATH += $$PWD/../build-keystone-0.9.2-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/llvm/include
    DEPENDPATH += $$PWD/../build-capstone-4.0.2-Desktop_Qt_5_15_2_MSVC2019_64bit-Release
} else {
# Libs 32 Bits
    LIBS += -L$$PWD/../build-keystone-0.9.2-Desktop_Qt_5_15_2_MSVC2019_32bit-Release/llvm/lib/ -lkeystone
    LIBS += -L$$PWD/../build-capstone-4.0.2-Desktop_Qt_5_15_2_MSVC2019_32bit-Release/ -lcapstone_dll

    INCLUDEPATH += $$PWD/../build-keystone-0.9.2-Desktop_Qt_5_15_2_MSVC2019_32bit-Release/llvm/include
    INCLUDEPATH += $$PWD/../build-capstone-4.0.2-Desktop_Qt_5_15_2_MSVC2019_32bit-Release

    DEPENDPATH += $$PWD/../build-keystone-0.9.2-Desktop_Qt_5_15_2_MSVC2019_32bit-Release/llvm/include
    DEPENDPATH += $$PWD/../build-capstone-4.0.2-Desktop_Qt_5_15_2_MSVC2019_32bit-Release
}

QMAKE_LFLAGS += -lkeystone -lcapstone
