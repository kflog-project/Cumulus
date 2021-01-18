################################################################################
# X11 Cumulus-GpsClient project file for qmake
#
# (c) 2008-2013 Axel Pauli
#
# This template generate a makefile for the gpsClient binary.
#
# $Id$
#
################################################################################

TEMPLATE   = app
CONFIG     = qt warn_on
#CONFIG     = qt warn_on debug

# Put all generated objects into an extra directory
OBJECTS_DIR = .obj
MOC_DIR     = .obj

#version check for Qt 4.7, 4.8 and 5.x
! contains(QT_VERSION, ^4\\.[78]\\..*|^5\\..*) {
  message("Cannot build Cumulus with Qt version $${QT_VERSION}.")
  error("Use at least Qt 4.7. or higher!")
}

# Set a Qt5 compiler define
contains (QT_VERSION, ^5\\..*) {
DEFINES += QT_5
}

# Enable TCP feature.
QT += network

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
    HEADERS += ../cumulus/flarmbase.h \
               ../cumulus/flarmbincom.h \
               ../cumulus/flarmbincomlinux.h \
               ../cumulus/flarmcrc.h
               
    SOURCES += ../cumulus/flarmbase.cpp \
               ../cumulus/flarmbincom.cpp \
               ../cumulus/flarmbincomlinux.cpp \
               ../cumulus/flarmcrc.cpp
               
    DEFINES += FLARM
}

DESTDIR = .
TARGET = gpsClient
INCLUDEPATH += ../cumulus

LIBS += -lstdc++
