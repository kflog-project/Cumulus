/***********************************************************************
**
**   glider.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003      by André Somers
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include "glider.h"

#include <QtCore>

Glider::Glider() :
  m_seats(singleSeater),
  m_maxWater(0),
  m_lastSafeID(-1),
  m_isSelected(false)
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

      // check to see if we have enough data members available
      if( data.count() < 15 )
        {
          return false;
        }

      m_type = data[0];
      m_registration = data[1];
      m_callSign = data[2];

      if( data[3].toInt() == 1 )
        {
          m_seats = singleSeater;
        }
      else
        {
          m_seats = doubleSeater;
        }

      m_maxWater = data[4].toInt();

      Speed V1, V2, V3, W1, W2, W3;
      V1.setKph( data[5].toDouble() );
      V2.setKph( data[7].toDouble() );
      V3.setKph( data[9].toDouble() );

      W1.setMps( data[6].toDouble() );
      W2.setMps( data[8].toDouble() );
      W3.setMps( data[10].toDouble() );

      m_polar = Polar( m_type,
                      V1, W1, // v/w pair 1
                      V2, W2, // v/w pair 2
                      V3, W3, // v/w pair 3
                      data[12].toDouble(), // wing area
                      data[13].toDouble(), // empty weight
                      data[14].toDouble() ); // gross weight

      m_lastSafeID = id;

      if( data.count() >= 17 )
        {
	  m_polar.setWater( data[15].toInt() );
          m_coPilot = data[16];
        }

      if( data.count() >= 18 )
	{
          m_polar.setAddLoad( data[17].toDouble() );
	}

      if( data.count() >= 19 )
	{
	  m_isSelected = data[18].toInt();
	}

      return true;
    }

  return false;
}

void Glider::safe(QSettings *config, int id)
{
  QStringList data;

  data.append( m_type );
  data.append( m_registration );
  data.append( m_callSign );

  if( m_seats == singleSeater )
    {
      data.append( "1" );
    }
  else
    {
      data.append( "2" );
    }

  data.append( QString::number( m_maxWater ) );
  data.append( QString::number( m_polar.v1().getKph() ) );
  data.append( QString::number( m_polar.w1().getMps() ) );
  data.append( QString::number( m_polar.v2().getKph() ) );
  data.append( QString::number( m_polar.w2().getMps() ) );
  data.append( QString::number( m_polar.v3().getKph() ) );
  data.append( QString::number( m_polar.w3().getMps() ) );
  data.append( QString::number( 0.0 ) ); // wing load not used
  data.append( QString::number( m_polar.wingArea() ) );
  data.append( QString::number( m_polar.emptyWeight() ) );
  data.append( QString::number( m_polar.grossWeight() ) );
  data.append( QString::number( m_polar.water() ) );
  data.append( m_coPilot );
  data.append( QString::number( m_polar.addLoad() ) );
  data.append( QString::number( m_isSelected ) );

  QString keyname = "Glider%1";
  config->setValue( keyname.arg( id ), data.join( ";" ) );
  m_lastSafeID = id;
}
