/***********************************************************************
**
**   DownloadManagerAndroid.h
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

/**
 * \class DownloadManager
 *
 * \author Axel Pauli
 *
 * \brief Manager for HTTP file download handling using the Android activity.
 *
 * This class handles the HTTP download requests of Cumulus. Downloads
 * of different files can be requested. The requests are executed by the Java
 * part of the Android activity in a serial order.
 *
 * \date 2014
 *
 * \version 1.0
 */

#ifndef DownloadManager_h
#define DownloadManager_h

#include <QtCore>
#include <QThread>

class DownloadManagerThread;

class DownloadManager : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( DownloadManager )

 public:

  /**
   * Creates a download manager instance.
   */
  DownloadManager( QObject *parent );

  virtual ~DownloadManager();

  /**
   * Requests to download the passed url and to store the result under the
   * passed file destination. The destination must be consist of a full path.
   *
   * \param url URL to be downloaded
   *
   * \param destination Download destination as filename with full path
   *
   * \param movingCheck Rejects downloads during move, if set to true.
   *                    That is the default setting.
   *
   * \return True on success otherwise false
   */
  bool downloadRequest( QString &url, QString &destination, bool movingCheck=true );

  /**
   * Increment global counter m_runningDownloads.
   */
  static void incrementRunningDownloads()
  {
    m_mutexStatic.lock();
    m_runningDownloads++;
    m_mutexStatic.unlock();
  }

  /**
   * Decrement global counter m_runningDownloads.
   */
  static void decrementRunningDownloads()
  {
    m_mutexStatic.lock();
    m_runningDownloads--;
    m_mutexStatic.unlock();
  }

  /**
   * \return Value of m_runningDownloads.
   */
  static short runningDownloads()
  {
    m_mutexStatic.lock();
    short result = m_runningDownloads;
    m_mutexStatic.unlock();
    return result;
  }

  /**
   * Sets the stop flag to the given value.
   *
   *  \param state New state of the stop flag.
   */
  static bool setStopFlag( const bool state )
  {
    m_mutexStatic.lock();
    bool oldState = m_stop;
    m_stop = state;
    m_mutexStatic.unlock();
    return oldState;
  }

  /**
   * Gets the current value of the stop flag.
   *
   *  \return Current state of the stop flag.
   */
  static bool stopFlag()
  {
    m_mutexStatic.lock();
    bool result = m_stop;
    m_mutexStatic.unlock();
    return result;
  }

  signals:

   /** Sends out a status message. */
   void status( const QString& msg );

   /** Sends a finish signal if all requested downloads are done. */
   void finished( int requests, int errors );

   /** Sends a network error signal, if such problem occurred. */
   void networkError();

   /**
    * Sends a signal if a file has been downloaded successfully.
    *
    * \file The downloaded file.
    */
   void fileDownloaded( QString& file );

 private:

  /**
   * Returns the free size of the file system in bytes for non root users.
   */
  double getFreeUserSpace( QString& path );

 private slots:

  /**
   * Slot to catch a finish signal with the downloaded url and the related
   * result.
   */
  void slotFinished( QString url, int resultCode );

 private:

  /**
   * Total number of currently running downloads.
   */
  static short m_runningDownloads;

  /**
   * Flag to signal, that no new download actions shall be started. A just
   * running download action shall be finished.
   */
  static bool m_stop;

  /**
   * Mutex to protect m_stop flag.
   */
  static QMutex m_mutexStatic;

  DownloadManagerThread* m_dlmThread;

  /** Download working flag */
  bool m_downloadRunning;

  /** Set of urls to be downloaded, used for fast checks */
  QSet<QString> urlSet;

  /**
   * The download queue containing url and destination as string pair.
   */
  QQueue< QPair<QString, QString> > queue;

  /** Mutex to protect data accesses. */
  QMutex m_mutex;

  /** Counter for download request. */
  int requests;

  /** Counter for download errors. */
  int errors;

  /**
   * Required minimum space in MB on file system destination to
   * execute the download request.
   */
  const double MinFsSpaceInMB;
};

//------------------------------------------------------------------------------

class DownloadManagerThread : public QThread
{
  Q_OBJECT

 public:

  DownloadManagerThread( QObject *parent,
                         QString& url,
                         QString& destination );

  virtual ~DownloadManagerThread();

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();

 signals:

  /**
   * Sent a finish signal with the downloaded url and the related result.
   */
  void finished( QString url, int resultCode );

 private:

  QString m_url;
  QString m_destination;
};

#endif /* DownloadManager_h */
