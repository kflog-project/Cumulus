/***********************************************************************
**
**   LiveTrack24Logger.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013-2014 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class LiveTrack24Logger
 *
 * \author Axel Pauli
 *
 * \brief A LiveTrack24 logger
 *
 * Logger which reports GPS track data to a LiveTrack server. It implements
 * the Leonardo Live Tracking API.
 *
 * This class is triggered by the slot slotNewFixEntry.
 *
 * \see http://www.livetrack24.com/wiki/en/Leonardo%20Live%20Tracking%20API
 * \see http://livexc.dhv1.dedoc/index.php
 * \see https://www.skylines-project.org/tracking/info
 *
 * \date 2013-2014
 *
 * \version $Id$
 */

#ifndef LiveTrack24Logger_h
#define LiveTrack24Logger_h

#include <QByteArray>
#include <QObject>
#include <QPair>
#include <QQueue>
#include <QString>
#include <QTime>
#include <QTimer>

#include "generalconfig.h"
#include "LiveTrack24.h"

class LiveTrack24Logger : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( LiveTrack24Logger )

 public:

  LiveTrack24Logger( QObject* parent = 0 );

  virtual ~LiveTrack24Logger();

  /**
   * Get session status.
   *
   * \returns true, if session is running otherwise false
   */
  bool sessionStatus() const
  {
    return m_isFlying;
  };

  /**
   * Gets the working state.
   *
   * \return false, if no HTTP request is in work and the request queue is empty.
   *         Otherwise true is returned.
   */
  bool livetrackWorkingState()
  {
    return m_lt24Gateway.livetrackWorkingState();
  }

  /**
   * Retrieve the package statistics from the live tracking gateway.
   *
   * \param cachedPkgs Number of cached packages in the sending queue.
   *
   * \param sentPkgs Number of Sent packages to the server.
   */
  void getPackageStatistics( uint& cachedPkgs, uint& sentPkgs )
  {
    m_lt24Gateway.getPackageStatistics( cachedPkgs, sentPkgs );
    return;
  };

 public slots:

  /** Called from calculator, if a new GPS fix is available. */
  void slotNewFixEntry();

  /** Called, if the live tracking is switched on/off. */
  void slotNewSwitchState( bool state );

  /**
  * This method is called to finish a just running logger session.
  */
  void slotFinishLogging();

 private:

  /** Reports a new route point to LiveTrack server. */
  void reportRoutePoint();

  /** LiveTrack24 gateway */
  LiveTrack24 m_lt24Gateway;

  /** Status flag for flying. */
  bool m_isFlying;

  /** Last time point of track reporting. */
  QTime m_lastTrackReporting;

  /** Last time point of moving. */
  QTime m_lastMoveTimePoint;

  /**
   * Timer to close a running session, if no new fix is reported for a certain
   * time.
   */
  QTimer* m_closeSessionTimer;
};

#endif
