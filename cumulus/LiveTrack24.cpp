/***********************************************************************
**
**   LiveTrack24.cpp
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

#include <cstdlib>
#include <ctime>

#include <QtGui>
#include <QtNetwork>

#include "calculator.h"
#include "generalconfig.h"
#include "LiveTrack24.h"
#include "mainwindow.h"

LiveTrack24::LiveTrack24( QObject *parent ) :
  QObject(parent),
  m_parent(parent),
  m_httpClient(0),
  m_retryTimer(0),
  m_userId(0),
  m_sessionId(0),
  m_packetId(0)
{
  m_httpClient = new HttpClient( this, false );

  connect( m_httpClient, SIGNAL( finished(QString &, QNetworkReply::NetworkError) ),
           this, SLOT( slotHttpResponse(QString &, QNetworkReply::NetworkError) ));

  m_retryTimer = new QTimer( this );
  m_retryTimer->setInterval( 60000 ); // Timeout is 60s
  m_retryTimer->setSingleShot( true );

  connect( m_retryTimer, SIGNAL(timeout()), this, SLOT(slotRetry()) );
}

LiveTrack24::~LiveTrack24()
{
  m_retryTimer->stop();
}

bool LiveTrack24::startTracking()
{
  const char* method = "LiveTrack24::startTracking():";

  GeneralConfig* conf = GeneralConfig::instance();

  // Check if LiveTracking is switched on
  if( conf->isLiveTrackOnOff() == false )
    {
      qDebug() << method << "LiveTracking is switched off!";
      return true;
    }

  // Check, if user name and password are defined by the user
  const QString& userName = conf->getLiveTrackUserName();
  QString  password = conf->getLiveTrackPassword();

  if( userName.isEmpty() )
    {
      qWarning() << method << "LiveTracking user name is missing!";
    }

  if( password.isEmpty() )
    {
      qWarning() << method << "LiveTracking password is missing!";
    }

  // This starts a new tracking session. First it is checked, if we need a login.
  if( m_userId == 0 )
    {
      // /client.php?op=login&user=username&pass=pass
      //
      // There is no user identifier defined, login is needed as first.
      QString loginUrl = "http://" +
                          conf->getLiveTrackServer() +
                         "/client.php?op=login&user=%1&pass=%2";
      queueRequest( qMakePair( QString("Login"), loginUrl) );
    }

  // /track.php?leolive=2&sid=42664778&pid=1&client=YourProgramName&v=1&user=yourusername&pass=yourpass
  // &phone=Nokia 2600c&gps=BT GPS&trk1=4&vtype=16388&vname=vehicle name and model

  QString gliderType;
  QString gliderRegistration;

  if( calculator->glider() )
    {
      gliderType = calculator->glider()->type();
      gliderRegistration = calculator->glider()->registration();
    }

  QString startUrl = "http://" +
                     conf->getLiveTrackServer() +
                     "/track.php?leolive=2&sid=%1&pid=%2&client=Cumulus&v=" +
                     QCoreApplication::applicationVersion() +
                     "&user=%3&pass=%4&phone=" + "Android" +
                     // Get Phone
                     "&gps=" + "internal GPS" +
                     // Get GPS
                     "&trk1=%4&vtype=" + QString::number(conf->getLiveTrackAirplaneType()) +
                     "&vname=" + gliderType;

  if( gliderRegistration.isEmpty() == false )
    {
      startUrl += " " + gliderRegistration;
    }

  return queueRequest( qMakePair( QString("Start"), startUrl) );
}

bool LiveTrack24::routeTracking( const QPoint& position,
                                 const int altitude,
                                 const uint groundSpeed,
                                 const uint course,
                                 qint64 utcTimeStamp )
{
  GeneralConfig* conf = GeneralConfig::instance();

  // Check if LiveTracking is switched on
  if( conf->isLiveTrackOnOff() == false )
    {
      qDebug() << "LiveTrack24::routeTracking(): LiveTracking is switched off!";
      return true;
    }

  QString routeUrl = "http://" +
                     conf->getLiveTrackServer() +
                     "/track.php?leolive=4&sid=%1&pid=%2" +
                     "&lat=" + QString::number(float(position.x()) / 600000.0) +
                     "&lon=" + QString::number(float(position.y()) / 600000.0) +
                     "&alt=" + QString::number( altitude ) +
                     "&sog=" + QString::number( groundSpeed ) +
                     "&cog=" + QString::number( course ) +
                     "&tm=" + QString::number( utcTimeStamp );

  return queueRequest( qMakePair( QString("Route"), routeUrl) );
}

bool LiveTrack24::endTracking()
{
  GeneralConfig* conf = GeneralConfig::instance();

  // Check if LiveTracking is switched on
  if( conf->isLiveTrackOnOff() == false )
    {
      qDebug() << "LiveTrack24::endTracking(): LiveTracking is switched off!";
      return true;
    }

  QString endUrl = "http://" +
                    conf->getLiveTrackServer() +
                   "/track.php?leolive=3&sid=%1&pid=%2&prid=0";

  return queueRequest( qMakePair( QString("End"), endUrl) );
}

void LiveTrack24::slotRetry()
{
  // Data sending was not possible and the retry timer has expired.
  // As next we check if LiveTracking is still switched on.
  // If not, we stop here the data sending and clear the request queue.
  if( GeneralConfig::instance()->isLiveTrackOnOff() == false )
    {
      qDebug() << "LiveTrack24::slotRetry: LiveTracking is switched off, clear data queue!";
      m_requestQueue.clear();
      return;
    }

  // Take the next request from the queue and try to send it to the server.
  sendHttpRequest();
}

bool LiveTrack24::queueRequest( QPair<QString, QString> keyAndUrl )
{
  m_requestQueue.enqueue( keyAndUrl );
  return sendHttpRequest();
}

bool LiveTrack24::sendHttpRequest()
{
  GeneralConfig* conf = GeneralConfig::instance();

  if( m_requestQueue.isEmpty() )
    {
      // Queue is empty.
      return true;
    }

  if( m_httpClient->isBusy() )
    {
      // Another request is just in work, do nothing more.
      return true;
    }

  // Check, if user name and password are defined by the user
  const QString& userName = conf->getLiveTrackUserName();
  QString  password = conf->getLiveTrackPassword();

  // Get the next element to be sent from the queue
  QPair<QString, QString>& keyAndUrl = m_requestQueue.head();

  // Clear the HTTP result buffer
  m_httpResultBuffer.clear();

  if( keyAndUrl.first == "Login" )
    {
      // A login has to be executed. The user identifier is returned or
      // zero in error case, if user name or password are wrong.
      QString url = keyAndUrl.second.arg(userName).arg(password);

      bool ok = m_httpClient->getData( url, &m_httpResultBuffer );

      if( ! ok )
        {
          m_retryTimer->start();
        }

      return ok;
    }

  QString url;

  if( keyAndUrl.first == "Start" )
    {
      // A start package has to be sent to the server. That requires to
      // set the package identifier to one and to create a new session identifier.
      m_packetId = 1;
      m_sessionId = generateSessionId( m_userId );

      if( m_userId == 0 )
        {
          // TODO Was tun in diesem Fall? Sollte nie passieren.
          qWarning() << "LiveTrack24: User identifier is zero!";
        }

      url = keyAndUrl.second.arg(m_sessionId).arg(m_packetId).arg(userName).arg(password);
      m_packetId++;
    }
  else
    {
      // Complete URL with session and package identifier
      url = keyAndUrl.second.arg(m_sessionId).arg(m_packetId);
      m_packetId++;
    }

  qDebug() << "<--URL=" << url;

  bool ok = m_httpClient->getData( url, &m_httpResultBuffer );

  if( ! ok )
    {
      m_retryTimer->start();
    }

  return ok;
}

/** Generates a random session id */
LiveTrack24::SessionId LiveTrack24::generateSessionId()
{
  srand( time( 0 ) );
  int rnd = rand();
  return (rnd & 0x7F000000) | 0x80000000;
}

/** Generates a random session id containing the given user identifier */
LiveTrack24::SessionId LiveTrack24::generateSessionId( const LiveTrack24::UserId userId )
{
  return generateSessionId() | (userId & 0x00ffffff);
}

void LiveTrack24::slotHttpResponse( QString &urlIn, QNetworkReply::NetworkError codeIn )
{
  Q_UNUSED(urlIn)

  qDebug() << "LiveTrack24::slotHttpResponse:" << m_httpResultBuffer << "ErrCode=" << codeIn;

  if( codeIn != QNetworkReply::NoError )
    {
      // There was a problem on the network. We make a retry after a certain time.
      m_retryTimer->start();
      return;
    }

  // Remove the last done request from the queue.
  if( ! m_requestQueue.isEmpty() )
    {
      QPair<QString, QString> keyAndUrl = m_requestQueue.dequeue();

      if( keyAndUrl.first == "Login" )
        {
          // Check the returned user identifier. For a successful login it
          // must be greater than zero.
          bool ok;
          m_userId = m_httpResultBuffer.toUInt( &ok );

          if( ! ok || m_userId == 0 )
            {
              // Login failed. We disable further live tracking.
              GeneralConfig::instance()->setLiveTrackOnOff( false );
              m_retryTimer->stop();
              m_requestQueue.clear();

              // Inform the user about our decision.
              QString msg = QString(tr("<html>LiveTrack24 login failed!<br><br>Switching off service.</html>"));

              QMessageBox mb( QMessageBox::Critical,
                              tr("Login Error"),
                              msg,
                              QMessageBox::Ok,
                              MainWindow::mainWindow() );

#ifdef ANDROID

              mb.show();
              QPoint pos = MainWindow::mainWindow()->mapToGlobal(QPoint( MainWindow::mainWindow()->width()/2  - mb.width()/2,
                                                                         MainWindow::mainWindow()->height()/2 - mb.height()/2 ));
              mb.move( pos );

#endif
              mb.exec();
              return;
            }
        }

      // Send the next request from the queue.
      sendHttpRequest();
    }
}
