/***********************************************************************
**
**   messagehandler.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2005 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

/**
 * \author Axel Pauli
 *
 * \brief A own message handler function for Cumulus.
 *
 * This function is used to handle the messages generated in Cumulus by
 * using qDebug, qWarning, qFatal.
 *
 * \date 2005-2010
 **/

#include <QApplication>

#if QT_VERSION >= 0x050000
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#else
void messageHandler(QtMsgType type, const char *msg);
#endif

#endif
