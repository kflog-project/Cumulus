/***********************************************************************
**
**   LiveTrack24.h
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
 * \class LiveTrack24
 *
 * \author Axel Pauli
 *
 * \brief API for the LiveTrack24 server at www.livetrack24.com
 *
 * This class implements the API used by the LiveTrack24 server at
 * www.livetrack24.com. It can also be used for the LiveTracking of SkyLines
 * project at www.skylines-project.org.
 *
 * Implemented methods:
 *
 * - Generate a Session Identifier including the User's Identifier
 *
 * - Send Start-of-Track packet on flight start or application start in midair
 *
 * - Send GPS Route Point packet(s)
 *
 * - Send End-of-Track packet on landing or application close
 *
 * \see http://www.livetrack24.com/wiki/LiveTracking%20API
 * \see https://www.skylines-project.org/tracking/info
 *
 * \date 2013
 *
 * \version $Id$
 */

#ifndef LiveTrack24_h
#define LiveTrack24_h

#include <QByteArray>
#include <QObject>
#include <QPair>
#include <QQueue>
#include <QString>
#include <QTimer>

#include "generalconfig.h"
#include "httpclient.h"
#include "wgspoint.h"

class LiveTrack24 : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( LiveTrack24 )

 public:

  LiveTrack24( QObject* parent = 0 );

  virtual ~LiveTrack24();

  /**
   * LiveTracking values for vehicle types
   */
  enum VehicleTypes {
    PARAGLIdER = 1,
    FLEX_WING_FAI1 = 2,
    RIGId_WING_FAI5 = 4,
    GLIdER = 8,
    PARAMOTOR = 16,
    TRIKE = 32,
    POWERED_AIRCRAFT = 64,
    HOT_AIR_BALLOON = 128,

    WALK = 16385,
    RUN = 16386,
    BIKE = 16388,

    HIKE = 16400,
    CYCLE = 16401,
    MOUNTAIN_BIKE = 16402,
    MOTORCYCLE = 16403,

    WINDSURF = 16500,
    KITESURF = 16501,
    SAILING = 16502,

    SNOWBOARD = 16600,
    SKI = 16601,
    SNOWKITE = 16602,

    CAR = 17100,
    CAR_4X4 = 17101,
  };

  /** User identifier data type. */
  typedef quint32 UserId;

  /** Session identifier data type. */
  typedef quint32 SessionId;

  /**
   * Sends the "start of track" packet to the tracking server.
   *
   * \return True on success otherwise false.
   */
  bool startTracking();

  /**
   * Sends a "GPS route point" packet to the tracking server
   *
   * \param position Coordinates as WGS84 in KFLog format
   * \param altitude Altitude in meters above MSL
   * \param groundSpeed Speed over ground in km/h
   * \param course Course over ground 0...360 degrees
   * \param utcTimeStamp UTC seconds since 1970
   *
   * \return True on success otherwise false.
   */
  bool routeTracking( const QPoint& position,
                      const int altitude,
                      const uint groundSpeed,
                      const uint course,
                      qint64 utcTimeStamp );

  /**
   * Sends the "end of track" packet to the tracking server.
   *
   * \return True on success otherwise false.
   */
  bool endTracking();

  /**
   * Provides a package statistics about the current session.
   *
   * \param cachedPkgs Package number in cache waiting for sending
   *
   * \param sentPkgs Package number transfered to the server
   */
  void getPackageStatistics( uint& cachedPkgs, uint& sentPkgs )
  {
    cachedPkgs = m_requestQueue.size();
    sentPkgs   = m_sentPackages;
  };

 private:

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
  bool sendHttpRequest();

  /**
   * Generates a random session identifier.
   *
   * \return A random session identifier.
   */
  SessionId generateSessionId();

  /**
   * Generates a random session identifier containing the given user identifier
   *
   * \param UserId User identifier to be included in session identifier.
   *
   * \return Calculated random session identifier
   */
  SessionId generateSessionId( const UserId userId );

  /**
   * Stores the session server with the right protocol prefix at the variable
   * m_sessionUrl.
   */
  void setSessionServer()
  {
    const QString& server = GeneralConfig::instance()->getLiveTrackServer();

    if( server.contains("livetrack24") )
      {
        m_sessionUrl = "http://" + server;
      }
    else if( server.contains("skylines") )
      {
        m_sessionUrl = "https://" + server;
      }
  };

  /**
   * \return The session server address with the right protocol prefix for the
   *         currently active session.
   */
  const QString& getSessionServer()
  {
    return m_sessionUrl;
  };

  /**
   * Stops the live tracking and informs the user about that fact.
   */
  void stopLiveTracking();

 signals:

 private slots:

   /**
    * Called, if the last sent HTTP request is finished.
    *
    * \param url URL of the executed request
    *
    * \param code Result code
    */
   void slotHttpResponse( QString &url, QNetworkReply::NetworkError code );

   /** Called, if retry timer expires to trigger a new sent request. */
   void slotRetry();

 private:

  HttpClient* m_httpClient;
  QTimer*     m_retryTimer;

  /** User identifier returned during login to server. */
  UserId m_userId;

  /**
   * Session identifier, generated with method generateSessionId.
   * The user identifier is the base for the session identifier.
   */
  SessionId m_sessionId;

  /**
   * URL used for the current active live tracking session.
   */
  QString m_sessionUrl;

  /** Packet identifier, starting with 1 at tracking start. */
  uint m_packetId;

  /** Result buffer for HTTP request. */
  QByteArray m_httpResultBuffer;

  /**
   * HTTP request queue. Every entry consists of a QPair, containing a key
   * and the related URL. The following keys are defined:
   *
   * Login 'L'
   * Start 'S'
   * Route 'R'
   * End   'E'
   */
  QQueue<QPair<uchar, QString> > m_requestQueue;

  /** Key identifier for the queue m_requestQueue. */
  static const uchar Login;
  static const uchar Start;
  static const uchar Route;
  static const uchar End;

  /** counter for successfully package transfer to the server. */
  uint m_sentPackages;
};

#endif
