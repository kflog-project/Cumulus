#-------------------------------------------------
#
# Project created by QtCreator 2012-10-09T15:43:08
#
# Source: https://github.com/5in4/qjson-backport
#
#-------------------------------------------------

QT       -= gui

CONFIG += staticlib

TARGET = ../.obj/qjson-backport
TEMPLATE = lib

# Put all generated objects into an extra directory
OBJECTS_DIR = ../.obj

DEFINES += QJSONBACKPORT_LIBRARY

SOURCES += qjsonwriter.cpp \
    qjsonvalue.cpp \
    qjsonparser.cpp \
    qjsonobject.cpp \
    qjsondocument.cpp \
    qjsonarray.cpp \
    qjson.cpp

HEADERS += qjsonwriter_p.h \
    qjsonvalue.h \
    qjsonparser_p.h \
    qjson_p.h \
    qjsonobject.h \
    qjsonexport.h \
    qjsondocument.h \
    qjsonarray.h
