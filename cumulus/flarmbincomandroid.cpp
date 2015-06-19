/***********************************************************************
**
**   flarmbincomandroid.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2012-2015 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***********************************************************************/

#include <QtGui>

#include "flarmbincomandroid.h"
#include "gpsconandroid.h"

FlarmBinComAndroid::FlarmBinComAndroid() : FlarmBinCom()
{
}

FlarmBinComAndroid::~FlarmBinComAndroid()
{
}

int FlarmBinComAndroid::writeChar(const unsigned char c)
{
  if( GpsConAndroid::sndByte( c ) )
    {
      return 1;
    }

  return -1;
}

int FlarmBinComAndroid::readChar(unsigned char* b, const int timeout)
{
  if( GpsConAndroid::getByte( b, timeout ) )
    {
       return 1;
    }

  return -1;
}
