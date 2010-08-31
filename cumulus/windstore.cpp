/***********************************************************************
**
**   windstore.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2009-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <stdlib.h>

#include "windstore.h"
#include "calculator.h"

WindStore::WindStore(QObject* parent) : QObject(parent)
{
}

WindStore::~WindStore()
{
}

/**
 * Called with new measurements. The quality is a measure for how
 * good the measurement is. Higher quality measurements are more
 * important in the end result and stay in the store longer.
 */
void WindStore::slot_Measurement( const Vector& windVector, int quality )
{
  windlist.addMeasurement( windVector, calculator->getlastAltitude(), quality );

  // we may have a new wind value, so make sure it's emitted if needed!
  recalculateWind();
}

/**
 * Called if the altitude changes.
 * Determines where measurements are stored and may result in a
 * newWind signal.
 */
void WindStore::slot_Altitude( const Altitude& altitude )
{
  if( fabs( (altitude - _lastAltitude).getMeters() ) >= 25.0 )
    {
      // only recalculate if there is a significant change
      recalculateWind();
    }

  _lastAltitude = calculator->getlastAltitude();
}

/**
 * Recalculates the wind from the stored measurements. May result in a
 * newWind signal.
 */
void WindStore::recalculateWind()
{
  Vector wind = windlist.getWind( calculator->getlastAltitude() );

  if( wind.isValid() && wind != _lastWind )
    {
      _lastWind = wind;
      //qDebug("emit newWind: %d/%f",_lastWind.getAngleDeg(),_lastWind.getSpeed().getKph() );
      emit newWind( _lastWind );
    }
}

