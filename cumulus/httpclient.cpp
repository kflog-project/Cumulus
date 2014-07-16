/***********************************************************************
**
**   httpclient.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2014 Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * This class is a simple HTTP download client.
 */
#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include <QtNetwork>

#include "httpclient.h"
#include "authdialog.h"
#include "generalconfig.h"

HttpClient::HttpClient( QObject *parent, const bool showProgressDialog ) :
  QObject(parent),
  m_progressDialog(0),
  m_manager(0),
  m_reply(0),
  m_tmpFile(0),
  m_userByteArray(0),
  m_url(""),
  m_destination(""),
  m_isBusy(false),
  m_timer(0)
 {
   if( showProgressDialog )
     {
       m_progressDialog = new QProgressDialog;
       connect( m_progressDialog, SIGNAL(canceled()), this, SLOT(slotCancelDownload()) );
     }

   m_manager = new QNetworkAccessManager(this);
   m_manager->setCookieJar ( new QNetworkCookieJar(this) );

   connect( m_manager, SIGNAL(authenticationRequired( QNetworkReply *, QAuthenticator * )),
            this, SLOT(slotAuthenticationRequired( QNetworkReply *, QAuthenticator * )) );

#ifndef QT_NO_OPENSSL
   connect( m_manager, SIGNAL(sslErrors( QNetworkReply *, const QList<QSslError> & )),
            this, SLOT(slotSslErrors( QNetworkReply *, const QList<QSslError> & )) );
#endif

   // timer to supervise connection.
   m_timer = new QTimer( this );
   m_timer->setInterval( 60000 ); // Timeout is 60s

   connect( m_timer, SIGNAL(timeout()), this, SLOT(slotCancelDownload()) );
}

HttpClient::~HttpClient()
{
  if( m_progressDialog )
    {
      delete m_progressDialog;
    }

  if( m_tmpFile )
    {
      if( m_tmpFile->isOpen() )
        {
          m_tmpFile->close();
        }

      m_tmpFile->remove();
      delete m_tmpFile;
    }

  m_timer->stop();
}

bool HttpClient::getData( QString &urlIn, QByteArray* userByteArray )
{
  if( m_isBusy == true )
    {
      qWarning( "HttpClient(%d): download is running!", __LINE__ );
      return false;
    }

  m_url = urlIn;
  m_userByteArray = userByteArray;

  QUrl url( urlIn );

  if( urlIn.isEmpty() || ! url.isValid() || userByteArray == 0 )
    {
      qWarning( "HttpClient(%d): Url or destination array are invalid!", __LINE__ );
      return false;
    }

  return sendRequest2Server();
}

bool HttpClient::downloadFile( QString &urlIn, QString &destinationIn )
{
  if( m_isBusy == true )
    {
      qWarning( "HttpClient(%d): download is running!", __LINE__ );
      return false;
    }

  m_url = urlIn;
  m_destination = destinationIn;

  QUrl url( urlIn );
  QFileInfo fileInfo( destinationIn );

  if( urlIn.isEmpty() || ! url.isValid() || fileInfo.fileName().isEmpty() )
    {
      qWarning( "HttpClient(%d): Url or destination file are invalid!", __LINE__ );
      return false;
    }

  m_tmpFile = new QFile( destinationIn + "." +
                       QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") );

  if( ! m_tmpFile->open( QIODevice::WriteOnly ) )
    {
      qWarning( "HttpClient(%d): Unable to open the file %s: %s",
                 __LINE__,
                 m_tmpFile->fileName ().toLatin1().data(),
                 m_tmpFile->errorString().toLatin1().data() );

      delete m_tmpFile;
      m_tmpFile = static_cast<QFile *> (0);
      return false;
    }

  if( sendRequest2Server() == false )
    {
      // Request to server has failed.
      return false;
    }

  if( m_progressDialog != static_cast<QProgressDialog *> (0) )
    {
      m_progressDialog->setWindowTitle( tr( "HTTP" ) );
      m_progressDialog->setLabelText( tr( "Downloading %1" ).arg( fileInfo.fileName() ) );
      m_progressDialog->show();
    }

  return true;
}

bool HttpClient::sendRequest2Server()
{
  // Check, if a proxy is defined. If yes, we use it.
  if( ! GeneralConfig::instance()->getProxy().isEmpty() )
    {
      QString hostName;
      quint16 port;

      if( parseProxy( GeneralConfig::instance()->getProxy(), hostName, port ) == true )
        {
          QNetworkProxy proxy;
          proxy.setType( QNetworkProxy::HttpProxy );
          proxy.setHostName( hostName );
          proxy.setPort( port );
          m_manager->setProxy( proxy );
        }
    }

  QString hw = " (Qt/Linux)";

#ifdef ANDROID
  hw = " (Qt/Android)";
#elif MAEMO4
  hw = " (Qt/Maemo4)";
#elif MAEMO5
  hw = " (Qt/Maemo5)";
#endif

  QString appl = QCoreApplication::applicationName() + "/" +
                 QCoreApplication::applicationVersion() + hw;

  QNetworkRequest request;
  request.setUrl( QUrl( m_url, QUrl::TolerantMode ));
  request.setRawHeader( "User-Agent", appl.toLatin1() );

  m_reply = m_manager->get(request);

  if( ! m_reply )
    {
      qWarning( "HttpClient(%d): Reply object is invalid!", __LINE__ );
      return false;
    }

  m_reply->setReadBufferSize(0);

  connect( m_reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );

  connect( m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
           this, SLOT(slotError(QNetworkReply::NetworkError)) );

  connect( m_reply, SIGNAL(finished()),
           this, SLOT(slotFinished()) );

  connect( m_reply, SIGNAL(downloadProgress(qint64, qint64)),
           this, SLOT(slotDownloadProgress( qint64, qint64 )) );

  m_isBusy = true;
  m_timer->start();
  return true;
}

/** User has canceled the download. */
void HttpClient::slotCancelDownload()
{
  qDebug( "HttpClient(%d): Operation canceled!", __LINE__ );

  m_timer->stop();

  if ( m_progressDialog != static_cast<QProgressDialog *> (0) )
    {
      m_progressDialog->reset();
      //_progressDialog->hide();
    }

  if( m_reply )
   {
      // That aborts the running download and sets the error state in the
      // reply object to QNetworkReply::OperationCanceledError. As next the
      // error signal and then the finish signal are emitted.
     m_reply->abort();
   }
}

/**
 * Network error occurred. Don't call abort() or close() in this method,
 * that leads to an endless loop!
 */
void HttpClient::slotError( QNetworkReply::NetworkError code )
{
  if( ! m_reply )
    {
      // Do ignore this call, because the reply object is already destroyed.
      // Do happen if the user don't close the message box.
      return;
    }

  qWarning( "HttpClient(%d): Network error %d, %s ",
           __LINE__, code, m_reply->errorString().toLatin1().data() );

  m_timer->start();

  if( code == QNetworkReply::NoError ||
      code == QNetworkReply::OperationCanceledError )
    {
      // Ignore these errors.
      return;
    }

  // If progress dialog is not activated, do not report anything more.
  if ( m_progressDialog != static_cast<QProgressDialog *> (0) )
    {
      m_progressDialog->reset();
      m_progressDialog->hide();

      QMessageBox::information( 0, QObject::tr("HTTP-%1").arg(code),
                               QObject::tr("Download failed with: %1")
                               .arg(m_reply->errorString() ));
    }
}

/** Report download progress to the user */
void HttpClient::slotDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  // qDebug() << "HttpClient::slotDownloadProgress" << bytesReceived << bytesTotal;

  if ( m_progressDialog != static_cast<QProgressDialog *> (0) )
    {
      // Report results to the progress dialog.
      m_progressDialog->setMaximum( bytesTotal );
      m_progressDialog->setValue( bytesReceived );
    }
  else
    {
      // Emit this signal to the outside, when no progress dialog is set up.
      emit downloadProgress( bytesReceived, bytesTotal );
    }

  m_timer->start();
}

void HttpClient::slotAuthenticationRequired( QNetworkReply * /* reply */,
                                             QAuthenticator *authenticator )
{
  m_timer->stop();
  getUserPassword( authenticator );
  m_timer->start();
}

void HttpClient::slotProxyAuthenticationRequired( const QNetworkProxy & /* proxy */,
                                                  QAuthenticator *authenticator )
{
  m_timer->stop();
  getUserPassword( authenticator );
  m_timer->start();
}

void HttpClient::getUserPassword( QAuthenticator *authenticator )
{
  QString user, password;
  QString title = QObject::tr( "%1 at %2" ).arg( authenticator->realm() ).arg( QUrl(m_url).host() );

  AuthDialog *dlg = new AuthDialog( user, password, title );
  dlg->adjustSize();

  if( dlg->exec() == QDialog::Accepted )
    {
      authenticator->setUser( user );
      authenticator->setPassword( password );
    }
}

#ifndef QT_NO_OPENSSL

void HttpClient::slotSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  QString errorString;

  for( int i = 0; i < errors.size(); i++ )
    {
      if( ! errorString.isEmpty() )
        {
          errorString += ", ";
        }

      errorString += errors.at(i).errorString();
    }

  qWarning() << "HTTP SSL Error:" << errorString << "-> will be ignored!";

  reply->ignoreSslErrors();
}

#endif

/** Download data for reading available. Put all of them into the opened
 *  temporary result file or in the user byte array.
 */
void HttpClient::slotReadyRead()
{
  if( m_reply && m_tmpFile )
    {
      QByteArray byteArray = m_reply->readAll();

      if( byteArray.size() > 0 )
        {
          m_tmpFile->write( byteArray );
        }
    }
  else if( m_reply && m_userByteArray )
    {
      m_userByteArray->append( m_reply->readAll() );
    }

  m_timer->start();
}

/**
 * Download is finished. Close destination file and reply instance.
 * The reply instance has to be deleted.
 */
void HttpClient::slotFinished()
{
  m_timer->stop();

  // Hide progress dialog.
  if ( m_progressDialog != static_cast<QProgressDialog *> (0) )
    {
      m_progressDialog->reset();
      m_progressDialog->hide();
    }

  if( m_reply )
    {
      // Read reply error status.
      enum QNetworkReply::NetworkError error = m_reply->error();

      // close opened IO device
      m_reply->close();

      if( m_tmpFile )
        {
          // File download has been done
          QString url;
          QString link = GeneralConfig::instance()->getOpenAipLink();

          if( m_url.contains(link) )
            {
              QStringList sl = m_url.split( "/");
              url = sl.last();
            }
          else
            {
              url = m_url;
            }

          qDebug( "Download %s finished with %d", url.toLatin1().data(), m_reply->error() );

          // Close temporary file.
          m_tmpFile->close();

          if( error != QNetworkReply::NoError )
            {
              // Request was aborted, tmp file must be removed.
              m_tmpFile->remove();
            }
          else
            {
              // Read last received bytes. Seems not to be necessary.
              // slotReadyRead();

              // Remove an old existing destination file before rename file.
              QFile::remove( m_destination );

              // Rename temporary file to destination file.
              m_tmpFile->rename( m_destination );
            }

          delete m_tmpFile;
          m_tmpFile = static_cast<QFile *> (0);
        }
      else if( m_userByteArray )
        {
          // Data have been read. Reset pointer to user's byte array.
          m_userByteArray = static_cast<QByteArray *> (0);;
        }

      // Destroy reply object
      m_reply->deleteLater();
      m_reply = static_cast<QNetworkReply *> (0);

      // Reset run flag before signal emit because signal finished can trigger
      // the next download.
      m_isBusy = false;

      // Inform about the result.
      emit finished( m_url, error );
    }
}

/**
 * Returns true, if proxy parameters are valid.
 */
bool HttpClient::parseProxy( QString proxyIn, QString& hostName, quint16& port )
{
  QStringList proxyParams = proxyIn.split(":", QString::SkipEmptyParts);

  if( proxyParams.size() != 2 )
    {
      // too less or too much parameters
      return false;
    }

  hostName = proxyParams.at(0).trimmed();

  bool ok = false;

  port = proxyParams.at(1).trimmed().toUShort(&ok);

  if( hostName.isEmpty() || ok == false )
    {
      return false;
    }

  return true;
}
