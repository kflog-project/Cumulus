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
 * WindStore receives single wind measurements and stores these. It uses
 * single measurements to provide a mean value, differentiated for altitude.
 *
 * @author André Somers
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
   * Called with new measurements. The quality is a measure for how good the
   * measurement is. Higher quality measurements are more important in the
   * end result and stay in the store longer.
   */
  void slot_Measurement( const Vector& windvector, int quality );

  /**
   * Called if the altitude changes.
   * Determines where measurements are stored and may result in a newWind
   * signal. */
  void slot_Altitude(const Altitude& altitude);

 signals:
  /**
   * Send if a new wind vector has been established. This may happen as
   * new measurements flow in, but also if the altitude changes.
   */
  void newWind(Vector& wind);

 private:

  Vector _lastWind;
  Altitude _lastAltitude;
  WindMeasurementList windlist;

  /** Recalculates the wind from the stored measurements.
   * May result in a newWind signal.
   */
  void recalculateWind();

};

#endif
