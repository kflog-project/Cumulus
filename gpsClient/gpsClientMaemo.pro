################################################################################
# Maemo Cumulus-GpsClient project file for qmake
#
# (c) 2008-2012 Axel Pauli
#
# This template generates a makefile for the gpsClient binary.
#
# $Id$
#
################################################################################

TEMPLATE   = app
CONFIG     = qt warn_on release
#CONFIG     = qt warn_on debug

# Put all generated objects into an extra directory
OBJECTS_DIR = .obj
MOC_DIR     = .obj

# Enable bluetooth feature, if not wanted comment out the next line with a hash
CONFIG += bluetooth

# Enable Flarm feature, if not wanted comment out the next line with a hash
CONFIG += flarm

HEADERS = \
  gpsclient.h \
  ../cumulus/ipc.h \
  ../cumulus/protocol.h \
  ../cumulus/signalhandler.h

SOURCES = \
  gpsclient.cpp \
  gpsmain.cpp \
  ../cumulus/ipc.cpp \
  ../cumulus/signalhandler.cpp

bluetooth {
  DEFINES += BLUEZ
  LIBS += -lbluetooth
}

flarm {
    HEADERS += flarmbincom.h \
               flarmcrc.h
               
    SOURCES += flarmbincom.cpp \
               flarmcrc.cpp
               
    DEFINES += FLARM
}

DESTDIR = .
TARGET = gpsClient
INCLUDEPATH += ../cumulus

LIBS += -lstdc++
