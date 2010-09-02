/***********************************************************************
 **
 **   windstore.h
 **
 **   This file is part of Cumulus.
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

#ifndef WIND_STORE_H
#define WIND_STORE_H

#include <QObject>

#include "vector.h"
#include "windmeasurementlist.h"

/**
 *  \author André Somers
 *
 *  \brief Stores single wind measurements and provides mean wind result.
 *
 * WindStore receives single wind measurements and stores these. It uses
 * single measurements to provide a mean value, differentiated for altitude,
 * quality and time range.
 *
 */
class WindStore : public QObject
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( WindStore )

  public:

  WindStore(QObject* parent);

  virtual ~WindStore();

  public slots:

  /**
   * Called with a new wind measurement. The quality is a measure for how good
   * the measurement is. Higher quality measurements are more important in the
   * end result and stay in the store longer.
   */
  void slot_Measurement( const Vector& windvector, int quality );

  /**
   * Called if the altitude changes. Can recalculate the wind and may result
   * in a newWind signal.
   */
  void slot_Altitude(const Altitude& altitude);

 signals:

  /**
   * Send if a new wind vector has been established. This may happen as
   * new measurements flow in, but also if the altitude changes.
   */
  void newWind(Vector& wind);

 private:

  /**
   * Recalculates the wind from the stored measurements and may result
   * in a newWind signal.
   */
  void recalculateWind();

  Vector _lastWind;
  Altitude _lastAltitude;
  WindMeasurementList windlist;
};

#endif
