/***********************************************************************
**
 **   SkyLinesTracker.h
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

/**
 * \class SkyLinesTracker
 *
 * \author Axel Pauli
 *
 * \brief API for the SkyLinesTracker server at skylines.aero
 *
 * This class implements the API used by the SkylinesTracking server at
 * skylines.aero.
 *
 * Implemented methods:
 *
 * - Generate a Package Identifier.
 *
 * - Send Start-of-Track packet on flight start or application start in midair
 *
 * - Send GPS Route Point packet(s)
 *
 * - Send End-of-Track packet on landing or application close
 *
 * \see https://skylines.aero/tracking/info
 *
 * \date 2018
 *
 * \version 1.0
 */

#ifndef SkyLinesTracker_h
#define SkyLinesTracker_h

#include <QByteArray>
#include <QObject>
#include <QHostAddress>
#include <QHostInfo>
#include <QQueue>
#include <QString>
#include <QTimer>
#include <QUdpSocket>

#include "generalconfig.h"
#include "LiveTrackBase.h"
#include "skyLinesTrackingProtocol.h"

class SkyLinesTracker : public LiveTrackBase
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SkyLinesTracker )

 public:

  SkyLinesTracker(QObject* parent = 0);

  virtual ~SkyLinesTracker();

  /** User's live tracking key, a 8 byte hex number */
  typedef quint64 LiveTrackingKey;

  /** package identifier as sequence number. */
  typedef quint16 PackageId;

  /*
   * Returns the default UDP server port as integer.
   */
  static unsigned getDefaultPort()
  {
    return 5597;
  }

  /*
   * Returns the default UDP server port as string.
   */
  static const char* getDefaultPortAsString()
  {
    return "5597";
  }

  /**
   * Returns the SkyLines server name.
   */
  static QString getServerName()
  {
    return "skylines.aero";
  }

  /**
   * Requests the IP address of the skylines server and after that
   * send out a ping to the server.
   *
   * \return True on success otherwise false.
   */
  virtual bool startTracking();

  /**
   * Sends a "GPS route point" packet to the tracking server
   *
   * \param position Coordinates as WGS84 in KFLog format
   * \param altitude Altitude in meters above MSL
   * \param groundSpeed Speed over ground in km/h
   * \param course Course over ground 0...359 degrees
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
                              qint64 utcTimeStamp );

  /**
   * Sends the "end of track" packet to the tracking server.
   *
   * \return True on success otherwise false.
   */
  virtual bool endTracking();

  /**
   * Provides a package statistics about the current session.
   *
   * \param cachedPkgs Package number in cache waiting for sending
   *
   * \param sentPkgs Package number transfered to the server
   */
  virtual void getPackageStatistics( uint& cachedPkgs, uint& sentPkgs )
  {
    cachedPkgs = m_fixPacketQueue.size();
    sentPkgs   = m_sentPackages;
  };

  /**
   * Informs about the livetrack working state.
   *
   * \return false, if no HTTP request is in work and the request queue is empty.
   *         Otherwise true is returned.
   */
  virtual bool livetrackWorkingState()
  {
    return ( m_fixPacketQueue.isEmpty() == false );
  };

 private:

  /**
   * Check, if service is requested by the user.
   */
  bool isServiceRequested()
  {
    GeneralConfig* conf = GeneralConfig::instance();

    // Check if skyline tracking is switched on
    if( conf->isLiveTrackOnOff() == false ||
        conf->getLiveTrackServer() != getServerName() )
      {
        return false;
      }

      return true;
  }

  /**
   * Called to process an received UDP diagram.
   */
  void processDatagram( QByteArray& datagram );

  /**
   * Puts a HTTP request into the queue and activates the sending to the server.
   *
   * \param keyAndUrl A pair consisting of a key identifier and an URL
   */
  bool queueRequest( QPair<uchar, QString> keyAndUrl );

  /**
   * Check if the queue limit is observed to avoid a memory problem. If the
   * queue is full, the oldest GPS route point is removed from the queue.
   */
  void checkQueueLimit();

  /**
   * Sends the next request from the request queue to the server.
   *
   * \return True in case of success otherwise false.
   */
  bool sendUdpRequest();

  /**
   * Generates a package identifier. Every call increments the internal counter.
   *
   * \return A package identifier.
   */
  static PackageId getId()
  {
    m_packetId++;
    return m_packetId;
  }

  /**
   * Stops the live tracking and informs the user about that fact.
   */
  void stopLiveTracking();

 signals:

 private slots:

  /**
  * Called to return a requested host info.
  *
  * \return Info about the requested host.
  */
  void slotHostInfoResponse( QHostInfo hostInfo);

  /**
   * Called to request the IP address of the sky lines tracking server.
   */
  void slotHostInfoRequest();

  /**
   * Called to send a ping to the sky lines server to verify the user data.
   */
  void slotSendPing();

  /**
   * Called, if an UDP diagram is available.
   */
  void slotReadPendingDatagrams();

  /**
   * Called, if retry timer expires to trigger a new sent request.
   */
  void slotRetry();

 private:

  QTimer* m_retryTimer;

  bool m_hostLookupIsRunning;

  /** IP Address of skylines.aero server. */
  QHostAddress m_serverIpAdress;

  /** User's live tracking key. */
  LiveTrackingKey m_liveTrackingKey;

  /** Live tracking key, consists of 16 hex digits.*/
  QString m_liveTrackingKeyString;

  /** UDP socket */
  QUdpSocket* m_udp;

  /** Packet identifier, starting with 1 at first call. */
  static PackageId m_packetId;

  /** Here is stored the last ping answer from the skyLines server. */
  qint32 m_lastPingAnswer;

  /**
   * UDP request queue. All FixPackets are stored in that queue.
   */
  QQueue<SkyLinesTracking::FixPacket> m_fixPacketQueue;

  /** counter for successfully fix package transfer to the server. */
  uint m_sentPackages;
};

SkyLinesTracker::PackageId SkyLinesTracker::m_packetId = 0;

#endif
