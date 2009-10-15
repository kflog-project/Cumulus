/***********************************************************************
**
**   glider.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003      by André Somers
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "glider.h"

#include <QStringList>

#warning FIXME: the unit for water should not be an integer representing liters, but a separate unit that can also represent gallons.

Glider::Glider()
{
  _polar = new Polar( "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
}


Glider::~Glider()
{
  delete _polar;
}


void Glider::setType(QString newValue)
{
  _type = newValue;
}


/** This function loads the glider-data from the config-object.
  * The config object needs to be initialized to the correct
  * section beforehand! It tries to load the glider with id id,
  * and returns true if it succeeds, and false otherwise. */
bool Glider::load(QSettings *config, int id)
{
  QString keyname = "Glider%1";

  //check if entry exists
  if( config->contains( keyname.arg( id ) ) )
    {

      QStringList data = config->value( keyname.arg( id ), "" ).toString().split(';');

      // qDebug("Glider::load(): No of fetched glider items is %d", data.count());

      // check to see if we have enough data members
      if( data.count() < 15 )
        {
          return false;
        }

      _type = data[0];
      _registration = data[1];

      // @AP 18.12.03: A call sign is stored as space, if it is an empty
      // string. That must be done otherwise the read from config file
      // will retrieve to less elements and you will never see your
      // stored glider entry. The space will be removed now.
      if( data[2] == " " ) // Check for space
        {
          _callSign = ""; // remove space
        }
      else
        {
          _callSign = data[2];
        }

      if( data[3].toInt() == 1 )
        {
          _seats = singleSeater;
        }
      else
        {
          _seats = doubleSeater;
        }

      _maxWater = data[4].toInt();

      delete _polar;

      Speed V1, V2, V3, W1, W2, W3;
      V1.setKph( data[5].toDouble() );
      V2.setKph( data[7].toDouble() );
      V3.setKph( data[9].toDouble() );

      W1.setMps( data[6].toDouble() );
      W2.setMps( data[8].toDouble() );
      W3.setMps( data[10].toDouble() );

      _polar = new Polar( _type, V1, W1, // v/w pair 1
                          V2, W2, // v/w pair 2
                          V3, W3, // v/w pair 3
                          data[11].toDouble(), data[12].toDouble(), // wingload, wingarea
                          data[13].toDouble(), data[14].toDouble() ); // empty weight, gross weight
      _lastSafeID = id;
      return true;
    }

  return false;
}


/** Stores the glider info contained in config-object config under ID id. */
void Glider::safe(QSettings *config, int id)
{
  QStringList data;

  data.append( _type );
  data.append( _registration );

  // @AP 18.12.03: If call sign is empty a space is stored as
  // workaround. Otherwise the load from config data will fail. The
  // space is removed during load action.

  if( _callSign.isEmpty() )
    {
      data.append( " " );
    }
  else
    {
      data.append( _callSign );
    }

  if( _seats == singleSeater )
    {
      data.append( "1" );
    }
  else
    {
      data.append( "2" );
    }

  data.append( QString::number( _maxWater ) );
  data.append( QString::number( _polar->v1().getKph() ) );
  data.append( QString::number( _polar->w1().getMps() ) );
  data.append( QString::number( _polar->v2().getKph() ) );
  data.append( QString::number( _polar->w2().getMps() ) );
  data.append( QString::number( _polar->v3().getKph() ) );
  data.append( QString::number( _polar->w3().getMps() ) );
  data.append( QString::number( 0.0 ) ); // wing load not used
  data.append( QString::number( 0.0 ) ); // wing area not used
  data.append( QString::number( _polar->emptyWeight() ) ); // empty weight now used
  data.append( QString::number( _polar->grossWeight() ) ); // gross weight now used

  QString keyname = "Glider%1";
  config->setValue( keyname.arg( id ), data.join( ";" ) );
  _lastSafeID = id;
}


void Glider::setPolar(Polar * newPolar)
{
  delete _polar;
  _polar = newPolar;
}

