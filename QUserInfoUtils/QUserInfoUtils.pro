#-------------------------------------------------
#
# Project created by QtCreator 2016-01-14T09:36:13
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = QUserInfoUtils
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    QProcessUtils.cpp \
    QDiskInfoUtils.cpp \
    QMemoryInfoUtils.cpp \
    QCpuInfoUtils.cpp

HEADERS += \
    QProcessUtils.h \
    QDiskInfoUtils.h \
    QMemoryInfoUtils.h \
    QCpuInfoUtils.h \
