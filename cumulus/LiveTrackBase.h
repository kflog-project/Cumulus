/***********************************************************************
 **
 **   LiveTrackBase.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2018 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#ifndef LiveTrackBase_h
#define LiveTrackBase_h

#include <QtGlobal>
#include <QObject>
#include <QPoint>

/**
 * \class LiveTrackBase
 *
 * \author Axel Pauli
 *
 * \brief Base class for live tracking activities.
 */
class LiveTrackBase : public QObject
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( LiveTrackBase )

public:

  /**
   * Class constructor. This class is derived from QObject.
   */
  LiveTrackBase (QObject* parent = 0) : QObject (parent)
  {
  }

  virtual ~LiveTrackBase()
  {
  }

  /*
   * Called to start the live tracking session.
   */
  virtual bool startTracking() = 0;

  /**
   * Sends a "GPS route point" packet to the tracking server
   *
   * \param position Coordinates as WGS84 in KFLog format
   * \param altitude Altitude in meters above MSL
   * \param groundSpeed Speed over ground in km/h
   * \param course Course over ground 0...360 degrees
   * \param vario vertical speed in m/s
   * \param utcTimeStamp UTC seconds since 1970
   *
   * \return True on success otherwise false.
   */
  virtual bool routeTracking( const QPoint& position,
                              const int altitude,
                              const uint groundSpeed,
                              const uint course,
                              const double vario,
                              qint64 utcTimeStamp ) = 0;

  /**
   * Called to finish the live tracking session.
   */
  virtual bool endTracking() = 0;

  /**
   * Provides a package statistics about the current live tracking session.
   *
   * \param cachedPkgs Package number in cache waiting for sending
   *
   * \param sentPkgs Package number transfered to the server
   */
  virtual void getPackageStatistics( uint& cachedPkgs, uint& sentPkgs ) = 0;

  /**
   * Informs about the live tracking working state.
   *
   * \return False, if live tracking has not to send stored packages
   *         otherwise true.
   */
  virtual bool livetrackWorkingState() = 0;
};

#endif /* LiveTrackBase_h */
