/***********************************************************************
**
**   downloadmanager.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * This class is a HTTP download manager. It processes download requests
 * in their incoming order one after another, not in parallel.
 */

#include <sys/statvfs.h>

#include "downloadmanager.h"
#include "calculator.h"
#include "gpsnmea.h"

const ulong DownloadManager::MinFsSpace = 25*1024*1024; // 25MB

/**
 * Creates a download manager instance.
 */
DownloadManager::DownloadManager( QObject *parent ) :
  QObject(parent),
  client(0),
  downloadRunning(false)
{
  client = new HttpClient(this);

  connect( client, SIGNAL( finished(QString &, QNetworkReply::NetworkError) ),
           this, SLOT( slotFinished(QString &, QNetworkReply::NetworkError) ));
}

/**
 * Requests to download the passed url and to store the result under the
 * passed file destination. Destination must consist of a full path.
 */
bool DownloadManager::downloadRequest( QString &url, QString &destination )
{
  mutex.lock();

  // Check input parameters. If url was already requested the download
  // is rejected.
  if( url.isEmpty() || urlSet.contains(url) || destination.isEmpty() )
    {
      mutex.unlock();
      return false;
    }

  extern Calculator* calculator;

  if( GpsNmea::gps->getConnected() && calculator->moving() )
    {
      // We have a GPS connection and are in move. That does not allow
      // to make any downloads.
      mutex.unlock();
      return false;
    }

  QString destDir = QFileInfo(destination).dir().absolutePath();

  // Check free size of destination file system. If size is less than 25MB the
  // download is rejected.
  if( getFreeUserSpace( destDir ) < MinFsSpace )
    {
      qWarning( "DownloadManager(%d): Free space on %s less than 25MB!",
                __LINE__, destDir.toLatin1().data() );

      mutex.unlock();
      return false;
    }

  if( downloadRunning == false )
    {
      // No download is running, do start the next one
      if( client->downloadFile( url, destination ) == false )
        {
          qWarning( "DownloadManager(%d): Download of '%s' failed!",
                     __LINE__, url.toLatin1().data() );
          // Start of download failed.
          mutex.unlock();
          return false;
        }

      downloadRunning = true;
    }

  // Insert request in queue.
  urlSet.insert( url );
  QPair<QString, QString> pair( url, destination );
  queue.enqueue( pair );

  mutex.unlock();
  return true;
}

/**
 * Catches a finish signal with the downloaded url and the related result
 * from the HTTP client.
 */
void DownloadManager::slotFinished( QString &urlIn, QNetworkReply::NetworkError codeIn )
{
  mutex.lock();

  if( codeIn != QNetworkReply::NoError )
    {
      // Error already reported by the HTTP client
    }

  // Remove last done request from the queue and from the url set.
  if( ! queue.isEmpty() )
    {
      queue.removeFirst();
    }

  urlSet.remove( urlIn );

  if( queue.isEmpty() )
    {
      // No more entries in queue. All downloads are finished.
      downloadRunning = false;
      emit finished();
      mutex.unlock();
      return;
    }

  extern Calculator* calculator;

  if( GpsNmea::gps->getConnected() && calculator->moving() )
    {
      // We have a GPS connection and are in move. That does not allow
      // to make any downloads. Remove all requests.
      qWarning( "DownloadManager(%d): We are moving, all requests are removed!",
                __LINE__ );

      queue.clear();
      urlSet.clear();
      downloadRunning = false;
      emit finished();
      mutex.unlock();
      return;
    }

  // Get next request from the queue
  QPair<QString, QString> pair = queue.head();

  QString url = pair.first;
  QString destination = pair.second;

  QString destDir = QFileInfo(destination).dir().absolutePath();

  // Check free size of destination file system. If size is less than 25MB the
  // download is not executed.
  if( getFreeUserSpace( destDir ) < MinFsSpace )
    {
      qWarning( "DownloadManager(%d): Free space on %s less than 25MB!",
                __LINE__, destDir.toLatin1().data() );

      mutex.unlock();

      // We simulate an operation cancel error, if the file system has
      // not enough space available.
      slotFinished( url, QNetworkReply::OperationCanceledError );
      return;
    }

  // Start the next download.
  if( client->downloadFile( url, destination ) == false )
    {
      qWarning( "DownloadManager(%d): Download of '%s' failed!",
                 __LINE__, url.toLatin1().data() );
      // Start of download failed.
      mutex.unlock();

      // We simulate an operation cancel error, if download
      // could not be started.
      slotFinished( url, QNetworkReply::OperationCanceledError );
      return;
    }

  mutex.unlock();
  return;
}

/**
 * Returns the free size of the file system in bytes for non root users.
 */
ulong DownloadManager::getFreeUserSpace( QString& path )
{
  struct statvfs buf;
  int res;

  res = statvfs( path.toLatin1().data(), &buf );

  if( res )
    {
      return 0;
    }

  qDebug() << "FSBlockSize=" << buf.f_bsize
           << "FSSizeInBlocks=" << buf.f_blocks
           << "FreeAvail=" << buf.f_bfree
           << "FreeAvailNonRoot=" << buf.f_bavail;

  // free size available to non-superuser in bytes
  return buf.f_bavail * buf.f_bsize;
}
