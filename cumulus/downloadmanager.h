/***********************************************************************
**
**   downloadmanager.h
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

#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <QtCore>

#include "httpclient.h"

class DownloadManager : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( DownloadManager )

 public:

  /**
   * Creates a download manager instance.
   */
  DownloadManager( QObject *parent = 0 );

  /**
   * Requests to download the passed url and to store the result under the
   * passed file destination.
   */
  bool downloadFile( QString &url, QString &destination );

  /**
   * Returns the free blocks of the file system for non root users.
   */
  long getFreeUserSpace( QString& path );

 private slots:

  /** Catch a finish signal with the downloaded url and the related result. */
  void slotFinished( QString &url, QNetworkReply::NetworkError code );

 private:

  HttpClient *client;

  /** Download working flag */
  bool downloadRunning;

  QSet<QString> urlSet; // Set of urls to be downloaded for fast checks

  /**
   * The download queue containing url and destination as string pair.
   */
  QQueue< QPair<QString, QString> > queue;
};

#endif /* DOWNLOAD_MANAGER_H */
