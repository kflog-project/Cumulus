/***********************************************************************
**
**   glider.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003      by André Somers
**                   2008-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "glider.h"

#include <QtCore>

Glider::Glider() :
  _seats(singleSeater),
  _maxWater(9),
  _lastSafeID(-1)
{
}

bool Glider::load(QSettings *config, int id)
{
  QString keyname = "Glider%1";

  //check if an entry exists
  if( config->contains( keyname.arg( id ) ) )
    {
      QStringList data = config->value( keyname.arg( id ), "" ).toString().split(';', QString::KeepEmptyParts);

      // qDebug("Glider::load(): No of fetched glider items is %d", data.count());

      // check to see if we have enough data members
      if( data.count() < 15 )
        {
          return false;
        }

      _type = data[0];
      _registration = data[1];
      _callSign = data[2];

      if( data[3].toInt() == 1 )
        {
          _seats = singleSeater;
        }
      else
        {
          _seats = doubleSeater;
        }

      _maxWater = data[4].toInt();

      Speed V1, V2, V3, W1, W2, W3;
      V1.setKph( data[5].toDouble() );
      V2.setKph( data[7].toDouble() );
      V3.setKph( data[9].toDouble() );

      W1.setMps( data[6].toDouble() );
      W2.setMps( data[8].toDouble() );
      W3.setMps( data[10].toDouble() );

      _polar = Polar( _type, V1, W1, // v/w pair 1
                      V2, W2, // v/w pair 2
                      V3, W3, // v/w pair 3
                      data[11].toDouble(), data[12].toDouble(), // wingload, wingarea
                      data[13].toDouble(), data[14].toDouble() ); // empty weight, gross weight

      _lastSafeID = id;

      if( data.count() == 17 )
        {
          _polar.setWater( data[15].toInt(), 0 );
          _coPilot = data[16];
        }

      return true;
    }

  return false;
}

/**
 * Stores the glider information contained in the configuration file
 */
void Glider::safe(QSettings *config, int id)
{
  QStringList data;

  data.append( _type );
  data.append( _registration );
  data.append( _callSign );

  if( _seats == singleSeater )
    {
      data.append( "1" );
    }
  else
    {
      data.append( "2" );
    }

  data.append( QString::number( _maxWater ) );
  data.append( QString::number( _polar.v1().getKph() ) );
  data.append( QString::number( _polar.w1().getMps() ) );
  data.append( QString::number( _polar.v2().getKph() ) );
  data.append( QString::number( _polar.w2().getMps() ) );
  data.append( QString::number( _polar.v3().getKph() ) );
  data.append( QString::number( _polar.w3().getMps() ) );
  data.append( QString::number( 0.0 ) ); // wing load not used
  data.append( QString::number( _polar.wingArea() ) );
  data.append( QString::number( _polar.emptyWeight() ) );
  data.append( QString::number( _polar.grossWeight() ) );
  data.append( QString::number( _polar.water() ) );
  data.append( _coPilot );

  QString keyname = "Glider%1";
  config->setValue( keyname.arg( id ), data.join( ";" ) );
  _lastSafeID = id;
}
