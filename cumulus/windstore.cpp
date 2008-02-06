/***********************************************************************
**
**   windstore.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2007 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <stdlib.h>
#include "windstore.h"
#include "cucalc.h"


WindStore::WindStore(QObject * parent) : QObject(parent)
{
  //create the lists
  windlist = new WindMeasurementList;
}


WindStore::~WindStore()
{
  delete windlist;
}


/**
 * Called with new measurements. The quality is a measure for how
 * good the measurement is. Higher quality measurements are more
 * important in the end result and stay in the store longer.
 */
void WindStore::slot_measurement(Vector windvector, int quality)
{
  windlist->addMeasurement(windvector, calculator->getlastAltitude(), quality);

  //we may have a new wind value, so make sure it's emitted if needed!
  recalculateWind();
}


/**
 * Called if the altitude changes.
 * Determines where measurements are stored and may result in a
 * newWind signal.
 */
void WindStore::slot_Altitude()
{
  if (abs((int)(calculator->getlastAltitude()-_lastAltitude).getMeters())>10) //only recalculate if there is a significant change
    recalculateWind();

  _lastAltitude=calculator->getlastAltitude();
}


/** Recalculates the wind from the stored measurements.
 * May result in a newWind signal. */
void WindStore::recalculateWind()
{
  Vector CurWind=windlist->getWind(calculator->getlastAltitude());
  if (CurWind!=_lastWind) {
    _lastWind=CurWind;
    qDebug("emit newWind: %d/%f",_lastWind.getAngleDeg(),_lastWind.getSpeed().getKph() );
    emit newWind(_lastWind);
  }
}

