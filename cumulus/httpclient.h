/***********************************************************************
**
**   httpclient.h
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

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QList>
#include <QFile>
#include <QProgressDialog>
#include <QString>
#include <QNetworkReply>
#include <QAuthenticator>

class HttpClient : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( HttpClient )

 public:

  /**
   * Pass false via showProgressDialog to suppress the progress dialog.
   */
  HttpClient( QObject *parent = 0, const bool showProgressDialog = true );

  /**
   * Requests to download the passed url and to store the result under the
   * passed file destination.
   */
  bool downloadFile( QString &url, QString &destination );

  /**
   * Returns true, if a download is running otherwise false.
   */
  bool isDownloadRunning() const
  {
    return downloadRunning;
  };

 signals:

  /** Sent a finish signal with the downloaded url and the related result. */
  void finished( QString &url, bool result );

  /** Forwarded signal from QNetworkReply. Is sent out, when no progress dialog
   *  is set up.
   */
  void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );

 private slots:

  /** User request, to cancel a running download */
  void slotCancelDownload();

  // Slots for Signal emitted by QNetworkAccessManager
  void slotAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator );
  void slotProxyAuthenticationRequired( const QNetworkProxy &proxy, QAuthenticator *authenticator );

#ifndef QT_NO_OPENSSL
  void slotSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif

  // Slots for Signal emitted by QNetworkReply
  void slotDownloadProgress ( qint64 bytesReceived, qint64 bytesTotal );
  void slotError( QNetworkReply::NetworkError code );
  void slotFinished();

  /** Downloaded data for reading available. */
  void slotReadyRead();

 private:

  /** Opens a user password dialog on server request. */
  void getUserPassword( QAuthenticator *authenticator );

  QObject               *_parent;
  QProgressDialog       *_progressDialog;
  QNetworkAccessManager *manager;
  QNetworkReply         *reply;
  QFile                 *tmpFile;

  QString               _url;
  QString               _destination;

  bool                  httpRequestAborted;
  bool                  downloadRunning;
};

#endif /* HTTP_CLIENT_H */
