/***********************************************************************
**
**   LiveTrack24.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013-2018 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

// See http://www.livetrack24.com/wiki/LiveTracking%20API for more details.

#include <cstdlib>
#include <ctime>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include <QtGlobal>
#include <QtNetwork>

#include "calculator.h"
#include "generalconfig.h"
#include "hwinfo.h"
#include "LiveTrack24.h"
#include "mainwindow.h"

#ifdef ANDROID
#include "jnisupport.h"
#endif

// Define control keys for URL caching
const uchar LiveTrack24::Login = 'L';
const uchar LiveTrack24::Start = 'S';
const uchar LiveTrack24::Route = 'R';
const uchar LiveTrack24::End   = 'E';

// Define a maximum queue length to limit the data amount. That limit is reached
// after an hour, if all 5s an entry is made.
#define MaxQueueLen 750

LiveTrack24::LiveTrack24( QObject *parent ) :
  LiveTrackBase(parent),
  m_httpClient(0),
  m_retryTimer(0),
  m_userId(0),
  m_sessionId(0),
  m_sessionUrl(""),
  m_packetId(0),
  m_sentPackages(0)
{
  setSessionServer();

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
      return false;
    }

  // Reset package counter
  m_sentPackages = 0;

  // Set the session server URL to the configured item.
  setSessionServer();

  // Check, if user name and password are defined by the user
  const QString& userName = conf->getLiveTrackUserName();
  QString  password       = conf->getLiveTrackPassword();

  if( userName.isEmpty() )
    {
      qWarning() << __LINE__ << method << "LiveTracking user name is missing!";
    }

  if( password.isEmpty() )
    {
      qWarning() << __LINE__ << method << "LiveTracking password is missing!";
    }

  // A new tracking session has to be opened. First is checked, if we need a login.
  if( m_userId == 0 )
    {
      // All previous cached data are cleared because there was no login to the
      // server in the past and in this case only the current session data
      // are stored.
      m_requestQueue.clear();

      // /client.php?op=login&user=username&pass=pass
      //
      // There is no user identifier defined, login is needed as first.
      QString loginUrl = "%1/client.php?op=login&user=%2&pass=%3";
      queueRequest( qMakePair( Login, loginUrl) );
    }

  // /track.php?leolive=2&sid=42664778&pid=1&client=YourProgramName&v=1
  // &user=yourusername&pass=yourpass&phone=Nokia 2600c&gps=BT GPS&trk1=4
  // &vtype=16388&vname=vehicle name and model
  QString gliderType = "unknown";
  QString gliderRegistration;

  if( calculator->glider() )
    {
      gliderType = calculator->glider()->type();
      gliderRegistration = calculator->glider()->registration();
    }

  QString phoneModel;

#ifdef ANDROID

  phoneModel = jniGetBuildData().value("BRAND") + ", " + jniGetBuildData().value("MODEL");

#else

  phoneModel = HwInfo::instance()->getType() + " " +
               HwInfo::instance()->getSubType();

#endif

  if( phoneModel.isEmpty() )
    {
      phoneModel="unknown";
    }

  QString startUrl = "%1=2&sid=%2&pid=%3&client=Cumulus&v=" +
                     QCoreApplication::applicationVersion() +
                     "&user=%4&pass=%5&phone=" + phoneModel +
                     "&gps=" + "internal/external GPS" +
                     "&trk1=" + QString::number(conf->getLiveTrackInterval()) +
                     "&vtype=" + QString::number(conf->getLiveTrackAirplaneType()) +
                     "&vname=" + gliderType;

  if( gliderRegistration.isEmpty() == false )
    {
      startUrl += " " + gliderRegistration;
    }

  return queueRequest( qMakePair( Start, startUrl) );
}

bool LiveTrack24::routeTracking( const QPoint& position,
                                 const int altitude,
                                 const uint groundSpeed,
                                 const uint course,
                                 const double /* vario */,
                                 qint64 utcTimeStamp )
{
  GeneralConfig* conf = GeneralConfig::instance();

  // Check if LiveTracking is switched on
  if( conf->isLiveTrackOnOff() == false )
    {
      return true;
    }

  QString routeUrl = QString("%1=4&sid=%2&pid=%3") +
                     "&lat=" + QString::number(float(position.x()) / 600000.0) +
                     "&lon=" + QString::number(float(position.y()) / 600000.0) +
                     "&alt=" + QString::number( altitude ) +
                     "&sog=" + QString::number( groundSpeed ) +
                     "&cog=" + QString::number( course ) +
                     "&tm=" + QString::number( utcTimeStamp );

  return queueRequest( qMakePair( Route, routeUrl) );
}

bool LiveTrack24::endTracking()
{
  GeneralConfig* conf = GeneralConfig::instance();

  // Check if LiveTracking is switched on
  if( conf->isLiveTrackOnOff() == false )
    {
      return true;
    }

  QString endUrl = "%1=3&sid=%2&pid=%3&prid=0";

  return queueRequest( qMakePair( End, endUrl) );
}

void LiveTrack24::slotRetry()
{
  // Data sending was not possible and the retry timer has expired.
  // As next we check if LiveTracking is still switched on.
  // If not, we stop here the data sending and clear the request queue.
  if( GeneralConfig::instance()->isLiveTrackOnOff() == false )
    {
      m_requestQueue.clear();
      return;
    }

  // Take the next request from the queue and try to send it to the server.
  sendHttpRequest();
}

bool LiveTrack24::queueRequest( QPair<uchar, QString> keyAndUrl )
{
  checkQueueLimit();
  m_requestQueue.enqueue( keyAndUrl );
  return sendHttpRequest();
}

void LiveTrack24::checkQueueLimit()
{
  if( m_requestQueue.size() <= MaxQueueLen )
    {
      return;
    }

  // The maximum queue length is reached. In this case the oldest route point
  // element is removed.
  for( int i = 0; i < m_requestQueue.size(); i++ )
    {
      if( m_requestQueue.at(i).first == Route )
        {
          // hau wech
          m_requestQueue.removeAt( i );
          break;
        }
    }
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

  // Get user name and password.
  const QString& userName = conf->getLiveTrackUserName();
  QString  password = conf->getLiveTrackPassword();

  // Get the next element to be sent from the queue.
  QPair<uchar, QString>& keyAndUrl = m_requestQueue.head();

  // Clear the HTTP result buffer.
  m_httpResultBuffer.clear();

  if( keyAndUrl.first == Login )
    {
      // A login has to be executed. The user identifier is returned or
      // zero in error case, if user name or password are wrong.
      QString url = keyAndUrl.second.arg(getSessionServer()).arg(userName).arg(password);

      bool ok = m_httpClient->getData( url, &m_httpResultBuffer );

      if( ! ok )
        {
          m_retryTimer->start();
        }

      return ok;
    }

  QString urlBegin = getSessionServer() + "/track.php?leolive";
  QString url;

  if( keyAndUrl.first == Start )
    {
      // A start package has to be sent to the server. That requires to
      // set the package identifier to one and to create a new session identifier.
      m_packetId = 1;
      m_sessionId = generateSessionId( m_userId );

      if( m_userId == 0 )
        {
          qWarning() << __LINE__ << "LiveTrack24::sendHttpRequest(): User identifier is zero!";
        }

      url = keyAndUrl.second.arg(urlBegin).arg(m_sessionId).arg(m_packetId).arg(userName).arg(password);
      m_packetId++;
    }
  else
    {
      // Complete URL with session and package identifier.
      url = keyAndUrl.second.arg(urlBegin).arg(m_sessionId).arg(m_packetId);
      m_packetId++;
    }

  bool ok = m_httpClient->getData( url, &m_httpResultBuffer );

  // qDebug() << "<--URL=" << url << "HTTP-Res=" << ok;

  if( ! ok )
    {
      m_retryTimer->start();
    }

  return ok;
}

LiveTrack24::SessionId LiveTrack24::generateSessionId()
{
  srand( time( 0 ) );
  int rnd = rand();
  return (rnd & 0x7F000000) | 0x80000000;
}

LiveTrack24::SessionId LiveTrack24::generateSessionId( const LiveTrack24::UserId userId )
{
  return generateSessionId() | (userId & 0x00ffffff);
}

void LiveTrack24::slotHttpResponse( QString &urlIn, QNetworkReply::NetworkError codeIn )
{
  Q_UNUSED(urlIn)

  // qDebug() << "LiveTrack24::slotHttpResponse:" << m_httpResultBuffer << "ErrCode=" << codeIn;

  if( codeIn > 0 && codeIn < 100 )
    {
      // There was a problem on the network. We make a retry after a certain time.
      m_retryTimer->start();
      return;
    }

  if( codeIn != 0 )
    {
      // any other error
      if( ! m_requestQueue.isEmpty() )
        {
          QPair<uchar, QString> keyAndUrl = m_requestQueue.head();

          if( keyAndUrl.first == Login )
            {
              // It seems to be a login problem.
              stopLiveTracking();
              return;
            }

          // UnknownContentError = 299
          if( codeIn == 299 )
            {
              // It seems something wrong in the sent URL. WE remove that URL
              // in this case to avoid a dead lock.
              m_requestQueue.removeFirst();
            }
        }

      m_retryTimer->start();
      return;
    }

  m_sentPackages++;

  // Remove the last executed request from the queue.
  if( ! m_requestQueue.isEmpty() )
    {
      QPair<uchar, QString> keyAndUrl = m_requestQueue.dequeue();

      if( keyAndUrl.first == Login )
        {
          // Check the returned user identifier. For a successful login it
          // must be greater than zero.
          bool ok;
          m_userId = m_httpResultBuffer.toUInt( &ok );

          if( ! ok || m_userId == 0 )
            {
              stopLiveTracking();
              return;
            }
        }

      // Send the next request from the queue.
      sendHttpRequest();
    }
}

void LiveTrack24::stopLiveTracking()
{
  // Login failed. We disable further live tracking.
  GeneralConfig::instance()->setLiveTrackOnOff( false );
  m_retryTimer->stop();
  m_requestQueue.clear();

  // Inform the user about our decision.
  QString msg = QString(tr("<html>LiveTrack login failed!<br><br>Switching off service.</html>"));

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
}
