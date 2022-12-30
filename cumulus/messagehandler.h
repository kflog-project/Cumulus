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
***********************************************************************/

#pragma once

/**
 * \author Axel Pauli
 *
 * \brief A own message handler function for Cumulus.
 *
 * This function is used to handle the messages generated in Cumulus by
 * using qDebug, qWarning, qFatal.
 *
 * \date 2005-2022
 **/

#include <QApplication>

void messageHandler(QtMsgType type, const char *msg);
