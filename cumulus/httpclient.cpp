/***********************************************************************
**
**   httpclient.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2010 Axel Pauli
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

#include <QtGui>
#include <QtNetwork>

#include "httpclient.h"
#include "authdialog.h"

 HttpClient::HttpClient( QObject *parent, const bool showProgressDialog ) :
   QObject(parent),
   _parent(parent),
   _progressDialog(0),
   reply(0),
   tmpFile(0),
   _url(""),
   _destination(""),
   httpRequestAborted(false),
   downloadRunning(false)
 {
   if( showProgressDialog )
     {
       _progressDialog = new QProgressDialog;
       connect( _progressDialog, SIGNAL(canceled()), this, SLOT(slotCancelDownload()) );
     }

   manager = new QNetworkAccessManager(this);
   manager->setCookieJar ( new QNetworkCookieJar(this) );

   connect( manager, SIGNAL(authenticationRequired( QNetworkReply *, QAuthenticator * )),
            this, SLOT(slotAuthenticationRequired( QNetworkReply *, QAuthenticator * )) );

#ifndef QT_NO_OPENSSL
   connect( manager, SIGNAL(sslErrors( QNetworkReply *, const QList<QSslError> & )),
            this, SLOT(slotSslErrors( QNetworkReply *, const QList<QSslError> & )) );
#endif

}

bool HttpClient::downloadFile( QString &urlIn, QString &destinationIn )
{
  qDebug("url=%s, dest=%s", urlIn.toLatin1().data(), destinationIn.toLatin1().data() );

  if( downloadRunning == true )
    {
      qWarning( "HttpClient(%d): download is running!", __LINE__ );
      return false;
    }

  _url = urlIn;
  _destination = destinationIn;

  QUrl url( urlIn );
  QFileInfo fileInfo( destinationIn );

  if( urlIn.isEmpty() || ! url.isValid() || fileInfo.fileName().isEmpty() )
    {
      qWarning( "HttpClient(%d): url or destination file are invalid!", __LINE__ );
      return false;
    }

  tmpFile = new QFile( destinationIn + "_tmp" );

  if( ! tmpFile->open( QIODevice::WriteOnly ) )
    {
      qWarning( "HttpClient(%d): Unable to save the file %s: %s",
                 __LINE__,
                 tmpFile->fileName ().toLatin1().data(),
                 tmpFile->errorString().toLatin1().data() );

      delete tmpFile;
      tmpFile = static_cast<QFile *> (0);
      return false;
    }

  QNetworkRequest request;

  request.setUrl( QUrl( _url, QUrl::TolerantMode ));
  request.setRawHeader("User-Agent", "Cumulus 2.4");

  reply = manager->get(request);

  if( ! reply )
    {
      qWarning("HttpClient(%d): Reply object is invalid!", __LINE__ );
      return false;
    }

  connect( reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );

  connect( reply, SIGNAL(error(QNetworkReply::NetworkError)),
           this, SLOT(slotError(QNetworkReply::NetworkError)) );

  connect( reply, SIGNAL(finished()),
           this, SLOT(slotFinished()) );

  connect( reply, SIGNAL(downloadProgress(qint64, qint64)),
           this, SLOT(slotDownloadProgress( qint64, qint64 )) );

  httpRequestAborted = false;
  downloadRunning = true;

  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      _progressDialog->setWindowTitle( tr( "HTTP" ) );
      _progressDialog->setLabelText( tr( "Downloading %1" ).arg( fileInfo.fileName() ) );
      _progressDialog->show();
    }

  return true;
}

/** User has canceled the download. */
void HttpClient::slotCancelDownload()
{
  qDebug( "HttpClient(%d): Download canceled!", __LINE__ );


  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      _progressDialog->reset();
      _progressDialog->hide();
    }

  if( reply )
   {
     reply->abort();
     httpRequestAborted = true;
   }
}

/** Network error occurred. */
void HttpClient::slotError( QNetworkReply::NetworkError code )
{
  qWarning( "HttpClient(%d): Network error %d, %s ",
           __LINE__, code, reply->errorString().toLatin1().data() );

  if( code == QNetworkReply::NoError )
    {
      return;
    }

  QMessageBox::information( 0, QObject::tr("HTTP"),
                           QObject::tr("Download failed with %1:%2")
                           .arg(code)
                           .arg(reply->errorString() ));

  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      _progressDialog->reset();
      _progressDialog->hide();
    }

  if( reply )
    {
      reply->abort();
      httpRequestAborted = true;
      downloadRunning = false;
    }
}

/** Report download progress to the user */
void HttpClient::slotDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  if( httpRequestAborted && ! downloadRunning )
    {
       return;
    }

  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      // Report results to the progress dialog.
      _progressDialog->setMaximum( bytesTotal );
      _progressDialog->setValue( bytesReceived );
    }
  else
    {
      // Emit this signal to the outside, when no progress dialog is set up.
      emit downloadProgress( bytesReceived, bytesTotal );
    }
}

void HttpClient::slotAuthenticationRequired( QNetworkReply * /* reply */,
                                             QAuthenticator *authenticator )
{
  return getUserPassword( authenticator );
}

void HttpClient::slotProxyAuthenticationRequired( const QNetworkProxy & /* proxy */,
                                                  QAuthenticator *authenticator )
{
  return getUserPassword( authenticator );
}

void HttpClient::getUserPassword( QAuthenticator *authenticator )
{
  QString user, password;
  QString title = QObject::tr( "%1 at %2" ).arg( authenticator->realm() ).arg( QUrl(_url).host() );

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

  if( QMessageBox::warning( 0, QObject::tr( "HTTP SSL Error" ),
      QObject::tr("One or more SSL errors has occurred: %1" ).arg( errorString ),
      QMessageBox::Ignore | QMessageBox::Abort ) == QMessageBox::Ignore )
    {
      reply->ignoreSslErrors();
    }
}

#endif

/** Downloaded ata for reading available. Put all of them into the opened
 *  temporary result file.
 */
void HttpClient::slotReadyRead()
{
  qDebug("HttpClient::slotReadyRead()");

  if( reply && tmpFile )
    {
      QByteArray byteArray = reply->readAll();

      qDebug("%d bytes read", byteArray.size());

      if( byteArray.size() > 0 )
        {
          tmpFile->write( byteArray );
          qDebug("%d bytes written into file", byteArray.size());
        }
    }
}

/**
 * Download is finished. Close destination file and reply instance.
 * The reply instance has to be deleted.
 */
void HttpClient::slotFinished()
{
  qDebug("HttpClient::slotFinished()");

  // Hide progress dialog.
  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      _progressDialog->reset();
      _progressDialog->hide();
    }

  if( reply && tmpFile )
    {
      if( httpRequestAborted )
        {
          // Request was aborted.
          reply->close();
          tmpFile->close();
          tmpFile->remove();

          // Inform about the result.
          emit finished( _url, false );
        }
      else
        {
          // Read last received bytes. Seems not to be necessary.
          // slotReadyRead();
          reply->close();

          // Remove an old existing destination file before rename file.
          QFile::remove( _destination );

          // Rename temporary file to destination file.
          tmpFile->rename( _destination );

          // Inform about the result.
          emit finished( _url, true );
        }

      delete tmpFile;
      tmpFile = static_cast<QFile *> (0);

      // Destroy reply object
      reply->deleteLater();
      reply = static_cast<QNetworkReply *> (0);
    }

  downloadRunning = false;
}
