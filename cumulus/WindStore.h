/***********************************************************************
 **
 **   WindStore.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by André Somers
 **                   2009-2021 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \class WindStore
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Stores single wind measurements and provides wind result on request.
 *
 * WindStore receives single wind measurements and stores these. It uses
 * single measurements to provide wind values, differentiated for altitude,
 * quality and time range.
 *
 * \date 2002-2021
 */

#pragma once

#include <QObject>
#include <QList>
#include <QMap>

#include "altitude.h"
#include "vector.h"
#include "WindMeasurement.h"

class WindStore : public QObject
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( WindStore )

  public:

  WindStore( QObject *parent );

  virtual ~WindStore()
  {}

  /**
   * Get the number of samples in wind map.
   */

  int numberOfWindSamples()
  {
    return windMap.size();
  }

  /**
   * Called to get the stored wind for the required altitude. If no wind is
   * available, an invalid wind vector is returned. If exact is false,
   * the next close altitude is searched, if the required altitude has no wind
   * value.
   */
  Vector getWind( const Altitude &altitude, bool exact=false );

  /**
   * Returns the first stored wind measurement (lowest altitude) from the
   * wind map. If the measurement is not available, the wind vector is set
   * to invalid.
   */
  WindMeasurement getFirstWindMeasurement()
  {
    WindMeasurement wm;

    if( windMap.isEmpty() == true )
      {
        return wm;
      }

    QList<int> keys = windMap.keys();

    return windMap[ keys.first() ];
  }

  /**
   * Returns the last stored wind measurement (highest altitude) from the
   * wind map. If the measurement is not available, the wind vector is set
   * to invalid.
   */
  WindMeasurement getLastWindMeasurement()
  {
    WindMeasurement wm;

    if( windMap.isEmpty() == true )
      {
        return wm;
      }

    QList<int> keys = windMap.keys();

    return windMap[ keys.last() ];
  }

  public slots:

  /**
   * Called with a new wind measurement. The quality is a measure for how good
   * the measurement is. Higher quality measurements are more important in the
   * end result and stay in the store longer.
   */
  void slot_Measurement( Vector& windvector,
                         const Altitude& altitude,
                         float quality );

 signals:

  /**
   * Send if a new wind vector has been delivered. This may happen as
   * new measurements flow in.
   */
  void newWind( Vector& wind );

 private:

  /**
   * Stores the last reported wind for update checks.
   */
  Vector m_lastReportedWind;

  /**
   * Stores the last required wind altitude interval.
   */
  int m_lastRequiredAltitudeInterval;

  /**
   * Stores the last required wind related to the required altitude interval.
   */
  Vector m_lastRequiredWind;

  /**
   * Contains the single wind measuements separated per 100 meters. The keys
   * are the altitude in meter intervals 0, 100, ... Every interval contains
   * one wind measurement.
   */
  QMap<int, WindMeasurement> windMap;
};
