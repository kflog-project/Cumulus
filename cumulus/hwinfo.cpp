/***********************************************************************
 **
 **   hwinfo.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2004 by      Eckhard Völlm
 **                   2008-2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

// @AP: qDebug does not work on Opie, therefore debug must be done via
// iostream in this class

using namespace std;

#include <iostream>
#include <malloc.h>
#include <stdio.h>
#include <mntent.h>

#include <QtCore>

#include "hwinfo.h"

HwInfo* HwInfo::theInstance = 0;

HwInfo::HwInfo()
{
  // Readout Hardware information and initialize variables
  _hwType    = unknown;
  _hwSubType = other;
  _hwString  = QString("");
  _fbRot     = 0;
  _fbDepth   = 16;

  // @AP: Due to a bug in Qt4 it is not possible to read from special
  // file devices without problems. Old good C solution will work fine :-))
  FILE *in = fopen( PATH_PROC_CPUINFO, "r" );

  if ( in )
    {
      char buf[256];

      while ( fgets( buf, sizeof( buf ) -1, in ) )
        {
          QString line(buf);

          if ( line.toLower().startsWith( "hardware" ) ||
               line.toLower().startsWith( "model name" ) )
            {
              _hwString = line.mid(line.indexOf(":") + 1).trimmed();

              cerr << "HW-String: '" << _hwString.toLatin1().data() << "'" << endl;

              if ( line.toLower().contains( "nokia" ) )
                {
                  _hwType = nokia;

                  if ( line.toUpper().contains( "RX-34" ) )
                    {
                      _hwSubType = n800;
                    }
                  else if ( line.toUpper().contains( "RX-44" ) )
                    {
                      _hwSubType = n810;
                    }
                  else if ( line.toUpper().contains( "RX-51" ) )
                    {
                      _hwSubType = n900;
                    }
                }

              break;
            }

          if ( line.toLower().startsWith( "processor" ) )
            {
              cerr << "HwInfo: " << line.simplified().toLatin1().data() << endl;
            }
        }

      fclose(in);
    }
  else
    {
      cerr << "HwInfo: Can't open path '" << PATH_PROC_CPUINFO << "'" << endl;
    }

  if ( _hwType == unknown )
    {
      cerr << "HwInfo: unknown PDA hardware - using default: desktop" << endl;
      _hwType = desktop;

      if (_hwString.isEmpty())
        {
          _hwString = "UNKNOWN";
        }
    }
}

HwInfo::~HwInfo()
{
  // Singleton lives till power off...
}

int HwInfo::getFreeMemory()
{
  int a=0, res=0, posStart=0, posEnd=0;

  // @AP: Due to a bug in Qt4 it is not possible to read from special
  // file devices without problems. Old good C solution will work fine :-))
  FILE *in = fopen( PATH_PROC_MEMINFO, "r" );

  if ( in )
    {
      char buf[256];

      while ( fgets( buf, sizeof( buf ) -1, in ) )
        {
          QString line(buf);
          // qDebug("LINE=%s", line.toLatin1().data() );

          if ( line.startsWith( "MemFree" ) || line.startsWith( "Buffers" ) || line.startsWith( "Cached" ) )
            {
              posStart = line.indexOf(':');
              posEnd = line.indexOf(" kB");
              a = line.mid(posStart+1,posEnd-posStart).trimmed().toInt();
              // qDebug( "Usable memory: '%d' (found in line '%s')", a, line.toLatin1().data() );
              res += a;
            }
        }

      fclose( in );
    }
  else
    {
      qWarning( "- can't open '%s' ", PATH_PROC_MEMINFO  );
    }

  if ( res == 0 )
    {
      qWarning( "No usable memory info found, assuming 1 MB free." );
      res = 1024;
    }
  else
    {
      // qDebug("Usable memory found: %d kB", res);
    }

  //get free heap space
  struct mallinfo m = mallinfo();
  //qDebug ("  fordblks: %d (%d KB)\n  uordblks: %d\n  hblkhd: %d\nTotal used: %d; total allocated: %d", m.fordblks, m.fordblks/1024,m.uordblks, m.hblkhd, m.uordblks+m.hblkhd,  m.uordblks+m.hblkhd+m.fordblks);
  //add the free space on the heap to the total free space, minus the fragmentation factor
  int heapfree = m.fordblks/1024;

  // qDebug("free heap space: %d KB", heapfree);
  if (heapfree > HEAP_FRAGMENTATION_FACTOR)
    {
      res += heapfree - HEAP_FRAGMENTATION_FACTOR;
    }

  // qDebug("Free memory=%d KB", res);
  return res;
}

const QString HwInfo::getCfDevice( void )
{
  qDebug("Detecting serial device for CF card...");
  int posStart;
  QString res("/dev/ttyS3");
  QFile f( PATH_CF_INFO );

  if ( ! f.exists() )
    {
      f.setFileName( PATH_CF_INFO1);
    }

  if ( f.open( QIODevice::ReadOnly ) )
    {
      QTextStream s( &f );

      while ( !s.atEnd() )
        {
          QString line ( s.readLine() );
          if ( line.startsWith( "0" ) )
            {
              posStart=line.indexOf("ttyS");
              res="/dev/" + line.mid(posStart,6).trimmed();
              qDebug("CF-Card device %s found.", res.toLatin1().data());
            }
        }
      f.close();
    }
  else
    {
      qWarning( "HwInfo::getCfDevice: Can't open '%s', reverting to default", f.fileName().toLatin1().data() );
    }
  return res;
}

/**
 * Checks if an active mount does exist.
 *
 * @param mountPoint Path to be checked for mounting.
 *
 * @returns true in case of success otherwise false.
 */
bool HwInfo::isMounted( const QString& mountPoint )
{
  /*
    /proc/mounts
    This file provides a list of all mounts in use by the system:

    rootfs / rootfs rw 0 0
    /dev/mmcblk0p1 /media/mmc2 vfat rw,nosuid,nodev,noexec,noatime,nodiratime,uid=299
    ...
  */

#ifdef ANDROID
  // Availability of data directory was checked initially
  return true;
#else
  bool result = false;

  struct mntent *fs;

  FILE *fp = setmntent("/etc/mtab", "r");

  while( (fs = getmntent(fp)) !=  static_cast<struct mntent *> (0) )
    {
      // printf ("%s %s %s %d %d\n", fs->mnt_fsname, fs->mnt_dir, fs->mnt_type, fs->mnt_opts, fs->mnt_freq,  fs->mnt_passno);
      if( QString( fs->mnt_dir ) == mountPoint )
        {
          result = true;
          break;
        }
    }

  endmntent( fp );

  return result;
#endif
}
