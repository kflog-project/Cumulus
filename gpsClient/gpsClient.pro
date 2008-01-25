# X11 Cumulus-GpsClient project file for qmake
# (c) 2007 Axel Pauli
#
# $Id$

TEMPLATE   = app
CONFIG     = qt warn_on
#CONFIG     = qt warn_on debug

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
