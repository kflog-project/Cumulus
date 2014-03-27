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

#include <cmath>

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
  m_windlist.addMeasurement( windVector, calculator->getlastAltitude(), quality );

  // we may have a new wind value, so make sure it's emitted if needed!
  recalculateWind();
}

/**
 * Called if the altitude changes. Can recalculate the wind and may result
 * in a newWind signal.
 */
void WindStore::slot_Altitude( const Altitude& altitude )
{
  if( calculator->currentFlightMode() != Calculator::circlingL &&
      calculator->currentFlightMode() != Calculator::circlingR &&
      fabs( (altitude - m_lastAltitude).getMeters() ) >= 25.0 )
    {
      // Only recalculate wind, if we are not circling and there is a
      // significant altitude change. During circling newer wind is always
      // calculated and distributed.
      recalculateWind();

      m_lastAltitude = calculator->getlastAltitude();
    }
}

/**
 * Recalculates the wind from the stored measurements. May result in a
 * newWind signal.
 */
void WindStore::recalculateWind()
{
  Vector wind = m_windlist.getWind( calculator->getlastAltitude() );

  if( wind.isValid() && wind != m_lastWind )
    {
      m_lastWind = wind;
      //qDebug("emit newWind: %d/%f",_lastWind.getAngleDeg(),_lastWind.getSpeed().getKph() );
      emit newWind( m_lastWind );
    }
}

