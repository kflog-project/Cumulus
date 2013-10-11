/***********************************************************************
**
**   httpclient.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class HttpClient
 *
 * \author Axel Pauli
 *
 * \brief This class is a simple HTTP download client.
 *
 * \date 2010-2013
 *
 * \version $Id$
 */

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <QByteArray>
#include <QString>
#include <QFile>
#include <QTimer>
#include <QList>

#include <QProgressDialog>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QNetworkAccessManager>

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

  virtual ~HttpClient();

  /**
   * Requests to download the passed url and to store the result under the
   * passed file destination.
   */
  bool downloadFile( QString &url, QString &destination );

  /**
   * Get data from the passed URL and return it into a byte array.
   *
   * \param url URL for get request
   *
   * \param userByteArray Byte array for fetched data
   */
  bool getData( QString &url, QByteArray* userByteArray );

  /**
   * Returns the network manager to be used by the HTTP client.
   */
  QNetworkAccessManager *networkManager() const
  {
    return m_manager;
  };

  /**
   * Returns true, if a download is running otherwise false.
   */
  bool isDownloadRunning() const
  {
    return m_downloadRunning;
  };

  /**
   * Returns true, if proxy parameters are valid.
   */
  static bool parseProxy( QString proxyIn, QString& hostName, quint16& port );

 signals:

  /** Sent a finish signal with the downloaded url and the related result. */
  void finished( QString &url, QNetworkReply::NetworkError code );

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

  /** Send URL request to the HTTP server. */
  bool sendRequest2Server();

  /** Opens a user password dialog on server request. */
  void getUserPassword( QAuthenticator *authenticator );

  QObject               *m_parent;
  QProgressDialog       *m_progressDialog;
  QNetworkAccessManager *m_manager;
  QNetworkReply         *m_reply;
  QFile                 *m_tmpFile;
  QByteArray            *m_userByteArray;
  QString               m_url;
  QString               m_destination;
  bool                  m_downloadRunning;
  QTimer                *m_timer;
};

#endif /* HTTP_CLIENT_H */
