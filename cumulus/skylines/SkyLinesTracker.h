/***********************************************************************
**
**  SkyLinesTracker.h
**
**  This file is part of Cumulus.
**
************************************************************************
**
**  Copyright (c): 2018 Axel Pauli
**
**  This file is distributed under the terms of the General Public
**  License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SkyLinesTracker
 *
 * \author Axel Pauli
 *
 * \brief API for a skylines.aero live tracking client.
 *
 * This class implements the API used by the SkylinesTracking server at
 * skylines.aero. The protocol uses UDP as transport medium.
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

#include "generalconfig.h"
#include "LiveTrackBase.h"
#include "skyLinesTrackingProtocol.h"
#include "Udp.h"

class SkyLinesTracker : public LiveTrackBase
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SkyLinesTracker )

 public:

  /**
   * Creates an instance of that class.
   *
   * \param parent Object of parent class
   * \param test Flag for verifying user's livetrack key. It enables some calls
   *             even SkyLines Tracking is not activated by the user.
   */
  SkyLinesTracker(QObject* parent, bool testing=false);

  virtual ~SkyLinesTracker();

  /** User's live tracking key, a hex string */
  typedef quint64 LiveTrackingKey;

  /** package identifier as sequence number. */
  typedef quint16 PackageId;

  /*
   * Returns the default UDP server port from SykLines.aero as integer.
   */
  static unsigned getDefaultPort()
  {
    return 5597;
  }

  /*
   * Returns the default UDP server port from SykLines.aero as string.
   */
  static const char* getDefaultPortAsString()
  {
    return "5597";
  }

  /**
   * Returns the SkyLines.aero server name.
   */
  static QString getServerName()
  {
    return "skylines.aero";
  }

  /**
   * Returns the SkyLines.aero IP V4 address.
   */
  static QString getServerIpAddress()
  {
    return "95.128.34.172";
  }

  /**
   * Requests the IP address of the skylines server and after that
   * send out a ping to the server to verify the user's live track key.
   *
   * \return True on success otherwise false.
   */
  virtual bool startTracking();

  /**
   * Sends a "tracking point" packet to the skylines.aero server.
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
   * Finishes the sending of tracking packages to the server.
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
   * \return false, if fix packet queue is empty.
   *         Otherwise true is returned.
   */
  virtual bool livetrackWorkingState()
  {
    return ( m_fixPacketQueue.isEmpty() == false );
  };

  /**
   * Returns the first found local IP V4 address as string.
   */
  QString getMyIpAddress();

 private:

  /**
   * Checks, if the service is requested by the user.
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
   * Puts a fix packet into the queue and activates the sending to the server.
   *
   * \param fixPaket A fix packet to be enqueued.
   */
  bool enqueueRequest( QByteArray& fixPaket );

  /**
   * Checks if the queue limit is reached to avoid a memory problem. If the
   * queue is full, the oldest route point is removed from the queue.
   */
  void checkQueueLimit();

  /**
   * Sends the next fix point from the queue to the server.
   *
   * \return True in case of success otherwise false.
   */
  bool sendNextFixpoint();

  /**
   * Generates a package identifier for the ping request. Every call increments
   * the internal counter.
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

  /**
   * Emitted when the connection is not possible to the server.
   */
  void connectionFailed();

  /**
   * Emits the ping result.
   */
  void pingResult( quint32 result );

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
   * Called to send a ping to the sky lines server to verify the connection and
   * the user's live track key.
   */
  void slotSendPing();

  /**
   * Called, if an UDP diagram is available from the sky lines server.
   */
  void slotReadPendingDatagrams();

  /**
   * Called if UDP datagram was written to the network.
   */
  void slotBytesWritten();

  /**
   * Called, if the retry timer expires to trigger the sending of the next
   * fix packet after a sent problem.
   */
  void slotRetry();

 private:

  /*
   * Flag to enable a ping test to the server for external callers.
   */
  bool m_testing;

  QTimer* m_retryTimer;

  /** Flag to signal a host lookup is running or not. */
  bool m_hostLookupIsRunning;

  /** IP Address of skylines.aero server. */
  QHostAddress m_serverIpAdress;

  /** User's SkyLines tracking key as 64 bit unsigned integer. */
  LiveTrackingKey m_liveTrackingKey;

  /** User's SkyLines tracking key as hex string. */
  QString m_liveTrackingKeyString;

  /** UDP socket to communicate with the Server. */
  Udp* m_udp;

  /** Packet identifier, starting with 1 at the first call. */
  static PackageId m_packetId;

  /** Here is stored the last ping answer from the skyLines server. */
  quint32 m_lastPingAnswer;

  /** Start day of tracking in ms UTC. */
  quint64 m_startDay;

  /**
   * UDP request queue. All fix packets are stored in that queue.
   */
  QQueue<QByteArray> m_fixPacketQueue;

  /** Counter for successfully sent UDP datagrams to the server. */
  uint m_sentPackages;
};

#endif
