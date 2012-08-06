/***********************************************************************
**
**   flarmbincomandroid.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

#ifndef FLARM_BIN_COM_ANDROID_H_
#define FLARM_BIN_COM_ANDROID_H_

#include "flarmbincom.h"

/**
 * \class FlarmBinComAndroid
 *
 * \author Axel Pauli
 *
 * \date 2012
 *
 * \brief Flarm binary low level port routines for Android.
 *
 * \version $Id$
 *
 */

class FlarmBinComAndroid : public FlarmBinCom
{

 public:

  FlarmBinComAndroid();

  virtual ~FlarmBinComAndroid();

 protected:

  /** Low level write character port method. */
  virtual int writeChar(const unsigned char c);

  /** Low level read character port method. */
  virtual int readChar(unsigned char* b);

};

#endif /* FLARM_BIN_COM_ANDROID_H_ */
