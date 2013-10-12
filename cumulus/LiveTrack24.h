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
 * \brief API for the LiveTrack24.com server.
 *
 * Procedure:
 *
 * - Generate a Session Id
 *   (including User Id if available)
 *
 * - Send Start-of-Track packet
 *   (on flight start or application start in midair)
 *
 * - Send GPS-Point packet(s)
 *
 * - Send End-of-Track packet
 *   (on landing or application close)
 *
 * @see http://www.livetrack24.com/wiki/en/Leonardo%20Live%20Tracking%20API
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

  typedef quint32 UserId;
  typedef quint32 SessionId;

  /** Sends the "start of track" packet to the tracking server */
  bool startTracking();

  /**
   * Sends a "GPS point" packet to the tracking server
   *
   * @param ground_speed Speed over ground in km/h
   */
  bool routeTracking( const QPoint& position,
                      const int altitude,
                      const uint groundSpeed,
                      const uint course,
                      qint64 utcTimeStamp );

  /** Sends the "end of track" packet to the tracking server */
  bool endTracking();

 private:

  /**
   * Puts a HTTP request into the queue and activates the sending to the server.
   *
   * \param keyAndUrl a pair consisting of a key and an URL
   */
  bool queueRequest( QPair<QString, QString> keyAndUrl );

  /**
   * Sends the next request from the request queue to the server.
   *
   * @return true in case of success otherwise false.
   */
  bool sendHttpRequest();

  /** Generates a random session id */
  SessionId generateSessionId();

  /** Generates a random session id containing the given user identifier */
  SessionId generateSessionId( const UserId userId );

  /**
   * \return The set server address without http://
   */
  const QString& getServer()
  {
    return GeneralConfig::instance()->getLiveTrackServer();
  };

 signals:

 private slots:

   /** Called, if the HTTP request is finished. */
   void slotHttpResponse( QString &url, QNetworkReply::NetworkError code );

   /** Called, if retry timer expires to trigger a new sent request. */
   void slotRetry();

 private:

  QObject*    m_parent;
  HttpClient* m_httpClient;
  QTimer*     m_retryTimer;

  UserId    m_userId;
  SessionId m_sessionId;

  /** Packet identifier, starts with 1 at tracking start. */
  uint m_packetId;

  /** Result buffer for HTTP requests. */
  QByteArray m_httpResultBuffer;

  /** HTTP request queue. */
  QQueue<QPair<QString, QString> > m_requestQueue;
};

#endif
