################################################################################
# GPS Maemo4 Client project file of Cumulus for qmake
#
# (c) 2010 Axel Pauli
#
# This template generates a makefile for the gpsMaemoClient binary.
#
# $Id: gpsMaemoClient.pro 3873 2010-03-16 21:39:28Z axel $
#
################################################################################

TEMPLATE   = app
CONFIG     = qt warn_on release
#CONFIG    = qt warn_on debug

QT -= gui # Only the core module is used.

HEADERS = \
  gpsmaemoclient.h \
  ../cumulus/ipc.h \
  ../cumulus/protocol.h \
  ../cumulus/signalhandler.h

SOURCES = \
  gpsmaemoclient.cpp \
  gpsmaemomain.cpp \
  ../cumulus/ipc.cpp \
  ../cumulus/signalhandler.cpp

DESTDIR = .
TARGET = gpsMaemoClient
INCLUDEPATH += ../cumulus \
	       /usr/lib/glib-2.0/include \
	       /usr/include/glib-2.0

DEFINES += MAEMO4

LIBS += -lstdc++ \
    -lgpsbt \
    -lgps \
    -lgpsmgr \
    -llocation

