/***********************************************************************
**
**   DownloadManagerAndroid.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <unistd.h>
#include <sys/vfs.h>

#include <QtCore>
#include <QtNetwork>

#include "DownloadManagerAndroid.h"
#include "calculator.h"
#include "gpsnmea.h"
#include "jnisupport.h"

/** Initializes static number. */
short DownloadManager::m_runningDownloads = 0;

/** Initializes static flag. */
bool DownloadManager::m_stop = false;

/** Initializes static mutex. */
QMutex DownloadManager::m_mutexStatic;

DownloadManager::DownloadManager( QObject *parent ) :
  QObject(parent),
  m_dlmThread(0),
  m_downloadRunning(false),
  requests(0),
  errors(0),
  MinFsSpaceInMB(25)
{
  setObjectName("DownloadManager");
}

DownloadManager::~DownloadManager()
{
  if( m_dlmThread != 0 )
    {
      qWarning() << "~DownloadManager(): Download thread is running!";
    }
}

/**
 * Requests to download the passed url and to store the result under the
 * passed file destination. Destination must consist of a full path.
 */
bool DownloadManager::downloadRequest( QString &url,
                                       QString &destination,
                                       bool movingCheck )
{
  if( stopFlag() == true )
    {
      return false;
    }

  QMutexLocker locker( &m_mutex );

  // Check input parameters. If url was already requested the download
  // is rejected.
  if( url.isEmpty() || urlSet.contains(url) || destination.isEmpty() )
    {
      return false;
    }

  extern Calculator* calculator;

  if( movingCheck == true && GpsNmea::gps->getConnected() && calculator->moving() )
    {
      // We have a GPS connection and are in move. That does not allow
      // to make any downloads.
      return false;
    }

  QString destDir = QFileInfo(destination).absolutePath();

  // Check free size of destination file system. If size is less than 25MB the
  // download is rejected.
  if( getFreeUserSpace( destDir ) < MinFsSpaceInMB )
    {
      qWarning( "DownloadManager(%d): Free space on %s less than %.1fMB!",
                __LINE__, destDir.toLatin1().data(), MinFsSpaceInMB );

      return false;
    }

  if( m_downloadRunning == false )
    {
      // No download is running, do start the next one
      m_dlmThread = new DownloadManagerThread( this, url, destination );

      connect( m_dlmThread, SIGNAL( finished( QString, int )),
               this, SLOT(slotFinished( QString, int )) );

      m_dlmThread->start();

      QString destFile = QFileInfo(destination).fileName();
      emit status( tr("downloading ") + destFile );
      m_downloadRunning = true;
    }

  // Insert request in queue.
  urlSet.insert( url );
  QPair<QString, QString> pair( url, destination );
  queue.enqueue( pair );
  requests++;

  return true;
}

void DownloadManager::slotFinished( QString urlIn, int codeIn )
{
  qDebug() <<"DownloadManager::slotFinished: URL=" << urlIn << "Res=" << codeIn;

  QMutexLocker locker( &m_mutex );

  m_dlmThread = 0;

  if( stopFlag() == true )
    {
      queue.clear();
      urlSet.clear();
      m_downloadRunning = false;
      return;
    }

  if( codeIn != QNetworkReply::NoError )
    {
      // Error already reported by the HTTP client
      errors++;
    }

  if( codeIn != QNetworkReply::NoError && codeIn != QNetworkReply::ContentNotFoundError )
    {
      // There was a fatal problem on the network. We do abort all further downloads
      // to avoid an error avalanche.
      qWarning( "DownloadManager(%d): Network problem occurred, canceling of all downloads!",
                __LINE__ );

      queue.clear();
      urlSet.clear();
      m_downloadRunning = false;
      emit networkError();
      return;
    }

  // Remove the last done request from the queue and from the url set.
  if( ! queue.isEmpty() )
    {
      QPair<QString, QString> pair = queue.dequeue();

      if( codeIn == QNetworkReply::NoError )
        {
          // Emit the successfully download.
          emit fileDownloaded( pair.second );
        }
    }

  // Remove last done request from the url set.
  urlSet.remove( urlIn );

  if( queue.isEmpty() )
    {
      // No more entries in queue. All downloads are finished.
      m_downloadRunning = false;
      emit status( tr("Downloads finished") );
      emit finished( requests, errors );
      requests = 0;
      errors   = 0;
      return;
    }

#if 0

  // @AP 24.09.2013: Restriction removed to continue all started downloads.
  extern Calculator* calculator;

  if( GpsNmea::gps->getConnected() && calculator->moving() )
    {
      // We have a GPS connection and are in move. That does not allow
      // to make any downloads. Remove all requests.
      qWarning( "DownloadManager(%d): We are moving, all requests are removed!",
                __LINE__ );

      queue.clear();
      urlSet.clear();
      m_downloadRunning = false;
      emit finished( requests, errors );
      requests = 0;
      errors   = 0;
      return;
    }

#endif

  // Get next request from the queue
  QPair<QString, QString> pair = queue.head();

  QString url = pair.first;
  QString destination = pair.second;

  QString destDir = QFileInfo(destination).absolutePath();

  // Check free size of destination file system. If size is less than 25MB the
  // download is not executed.
  if( getFreeUserSpace( destDir ) < MinFsSpaceInMB )
    {
      qWarning( "DownloadManager(%d): Free space on %s less than %.1fMB!",
                __LINE__, destDir.toLatin1().data(), MinFsSpaceInMB );

      // We simulate an operation cancel error, if the file system has
      // not enough space available.
      slotFinished( url, QNetworkReply::OperationCanceledError );
      return;
    }

  // Start the next download.
  // No download is running, do start the next one
  m_dlmThread = new DownloadManagerThread( this, url, destination );

  connect( m_dlmThread, SIGNAL( finished( QString, int )),
           this, SLOT(slotFinished( QString, int )) );

  m_dlmThread->start();

  QString destFile = QFileInfo(destination).fileName();
  emit status( tr("downloading ") + destFile );

  return;
}

/**
 * Returns the free size of the file system in bytes for non root users.
 * The passed path must be exist otherwise the call will fail!
 */
double DownloadManager::getFreeUserSpace( QString& path )
{
  struct statfs buf;
  int res;

  res = statfs( path.toLatin1().data(), &buf );

  if( res )
    {
      qWarning( "DownloadManager(%d): Free space check failed for %s!",
                __LINE__, path.toLatin1().data() );

      perror("GetFreeUserSpace");
      return 0.0;
    }

#if 0
    qDebug() << "Path=" << path
             << "FSBlockSize=" << buf.f_bsize
             << "FSSizeInBlocks=" << buf.f_blocks
             << "FreeAvail=" << buf.f_bfree
             << "FreeAvailNonRoot=" << buf.f_bavail
             << "FreeAvailNonRoot=" << ( double (buf.f_bavail) * double (buf.f_bsize) / double(1024 * 1024) )
             << "MB";
#endif

    // Free size available to non-superuser in megabytes. We have to use doubles
    // to avoid a number overflow.
    return ( double (buf.f_bavail) * double (buf.f_bsize) / double(1024 * 1024) );
}


/*-------------------------DownloadManagerThread------------------------------*/

#include <csignal>

DownloadManagerThread::DownloadManagerThread( QObject *parent,
                                              QString& url,
                                              QString& destination ) :
  QThread( parent ),
  m_url(url),
  m_destination(destination)
{
  setObjectName( "DownloadManagerThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

DownloadManagerThread::~DownloadManagerThread()
{
}

void DownloadManagerThread::run()
{
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  // Check is signal is connected to a slot.
  if( receivers( SIGNAL( finished( QString, int )) ) == 0 )
    {
      qWarning() << "DownloadManagerThread: No Slot connection to Signal finished";
      return;
    }

  // This call starts the download at activity side.
  int result = jniDownloadFile( m_url, m_destination );

  emit finished( m_url, result );
}
