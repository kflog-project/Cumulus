/***********************************************************************
**
**   downloadmanager.cpp
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
 * This class is a HTTP download manager.
 */

#include <sys/vfs.h>

#include "downloadmanager.h"

DownloadManager::DownloadManager( QObject *parent ) :
  QObject(parent),
  client(0),
  downloadRunning(false)
{
  client = new HttpClient(this);
}

/**
 * Requests to download the passed url and to store the result under the
 * passed file destination.
 */
bool DownloadManager::downloadFile( QString &url, QString &destination )
{
  return true;
}

/**
 * Returns the free blocks of the file system for non root users.
 */
long DownloadManager::getFreeUserSpace( QString& path )
{
  struct statfs buf;
  int res;

  res = statfs( path.toLatin1().data(), &buf );

  if( res )
    {
      return -1;
    }

  qDebug() << "TotalBlocks=" << buf.f_blocks
           << "FreeBlocksFs=" << buf.f_bfree
           << "FreeAvail=" << buf.f_bavail;

  return buf.f_bavail; /* free blocks avail to non-superuser */
}
