################################################################################
# NMEA Simulator project file of Cumulus for qmake
#
# (c) 2008-2010 Axel Pauli
#
# This template generates a makefile for the NMEA Simulator binary.
#
# $Id$
#
################################################################################

TEMPLATE    = app
CONFIG      = qt warn_on release

QT -= gui # Only the core module is used.

HEADERS     = \
glider.h \
gpgga.h \
gprmc.h \
gpgsa.h \
../cumulus/speed.h \
vector.h

SOURCES     = \
glider.cpp \
gpgga.cpp \
gprmc.cpp \
gpgsa.cpp \
main.cpp \
../cumulus/speed.cpp \
vector.cpp \

TARGET = nmeaSimu
DESTDIR     = .
INCLUDEPATH += ../cumulus

LIBS += -lstdc++ -lm
