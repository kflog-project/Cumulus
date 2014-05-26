################################################################################
# NMEA Simulator project file of Cumulus for qmake
#
# (c) 2008-2014 Axel Pauli
#
# This template generates a makefile for the NMEA Simulator binary.
#
# $Id$
#
################################################################################

TEMPLATE    = app
CONFIG      = qt warn_on release

# Put all generated objects into an extra directory
OBJECTS_DIR = .obj
MOC_DIR     = .obj

QT -= gui # Only the core module is used.

HEADERS     = \
    glider.h \
    gpgga.h \
    gprmc.h \
    gpgsa.h \
    pgrmz.h \
    play.h \
    sentence.h \
    ../cumulus/speed.h \
    vector.h

SOURCES     = \
    glider.cpp \
    gpgga.cpp \
    gprmc.cpp \
    gpgsa.cpp \
    pgrmz.cpp \
    play.cpp \
    sentence.cpp \
    main.cpp \
    ../cumulus/speed.cpp \
    vector.cpp \

TARGET = nmeaSimu
DESTDIR     = .
INCLUDEPATH += ../cumulus

LIBS += -lstdc++ -lm
