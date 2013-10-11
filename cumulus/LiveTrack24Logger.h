/***********************************************************************
**
**   LiveTrack24Logger.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class LiveTrackLogger24
 *
 * \author Axel Pauli
 *
 * \brief LiveTrack24 logger
 *
 * Logger which reports tracks to www.livetrack24.com. This class is triggered
 * by the slot slotNewFixEntry.
 *
 * @see http://www.livetrack24.com/wiki/en/Leonardo%20Live%20Tracking%20API
 *
 * \date 2013
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

 public slots:

  /** Called from calculator, if a new GPS fix is available. */
  void slotNewFixEntry();

 private:

  /** Reports a new route point to LiveTrack24. */
  void reportRoutePoint();

  /** LiveTrack24 gateway */
  LiveTrack24 m_lt24Gateway;

  /** Status flag for flying. */
  bool m_isFlying;

  /** Last time point of track reporting. */
  QTime m_lastTrackReporting;

  /** Last time point of moving. */
  QTime m_lastMoveTimePoint;
};

#endif
