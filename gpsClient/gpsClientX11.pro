################################################################################
# X11 Cumulus-GpsClient project file for qmake
#
# (c) 2008-2010 Axel Pauli
#
# This template generate a makefile for the gpsClient binary.
#
# $Id$
#
################################################################################

TEMPLATE   = app
CONFIG     = qt warn_on
#CONFIG     = qt warn_on debug

QT -= gui # Only the core module is used.

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

DESTDIR = .
TARGET = gpsClient
INCLUDEPATH += ../cumulus

LIBS += -lstdc++
