/***************************************************************************
    gpsnmea.cpp  -  Cumulus NMEA parser and decoder
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002      by André Somers,
                               2008-2012 by Axel Pauli

    email                : axel@kflog.org

    $Id$

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 **************************************************************************/

/*
 * This class parses and decodes the NMEA sentences and provides access
 * to the last know data. Furthermore it is managing the connection to a GPS
 * receiver connected by RS232, USB or to a GPS daemon process on Maemo. Due to
 * different APIs under Maemo4 and Maemo5 different conditional defines are used
 * for special adoptions. The following defines are in use:
 *
 * MAEMO  for Maemo in general
 * MAEMO4 for Maemo 4, OS2008, devices N8x0
 * MAEMO5 for Maemo 5, OS2009, devices N900
 *
 * ANDROID for Android-Lighthouse Qt port (skipping all code referring to
 * external devices, using system location service via JNI)
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctime>
#include <cmath>

#include <QtCore>

#include "generalconfig.h"
#include "gpsnmea.h"
#include "mapmatrix.h"
#include "mapcalc.h"
#include "mapview.h"

#ifdef ANDROID
#include "androidevents.h"
#include "jnisupport.h"
#endif

#ifdef FLARM
#include "flarm.h"
#endif

// @AP: define timeout constants for GPS fix supervision in milli seconds.
//      After this time an alarm is generated.
#define FIX_TO 25000

extern MapView   *_globalMapView;

// global GPS object pointer
GpsNmea *GpsNmea::gps = static_cast<GpsNmea *> (0);

// number of created class instances
short GpsNmea::instances = 0;

// Dictionary with known sentence keywords
QHash<QString, short> GpsNmea::gpsHash;

// Mutex for thread synchronization
QMutex GpsNmea::mutex;

GpsNmea::GpsNmea(QObject* parent) : QObject(parent)
{
  if( instances > 0 )
    {
      // There exists already a class instance as singleton.
      return;
    }

  instances++;

  _enableGpsDataProcessing = true;

  resetDataObjects();

  // Load desired GPS sentence identifier into the hash filter.
  getGpsMessageKeys(gpsHash);

  // GPS fix supervision, is started after the first fix was received
  timeOutFix = new QTimer(this);
  connect (timeOutFix, SIGNAL(timeout()), this, SLOT(_slotTimeoutFix()));

  // Load last used device
  gpsDevice   = GeneralConfig::instance()->getGpsDevice();

#ifndef ANDROID
  serial = static_cast<GpsCon *> (0);
#else
  serial = static_cast<QObject *> (0);
#endif

  nmeaLogFile = static_cast<QFile *> (0);

  if( GeneralConfig::instance()->getGpsNmeaLogState() == true )
    {
      slot_openNmeaLogFile();
    }

  createGpsConnection();
}

GpsNmea::~GpsNmea()
{
  // decrement instance counter
  instances--;

  // stop GPS client process
  if ( serial )
    {
      delete serial;
      writeConfig();
    }

  if( nmeaLogFile )
    {
      if( nmeaLogFile->isOpen() )
        {
          nmeaLogFile->close();
        }

      delete nmeaLogFile;
    }
}

void GpsNmea::getGpsMessageKeys( QHash<QString, short>& gpsKeys)
{
  mutex.lock();
  gpsKeys.clear();

  // Load all desired GPS sentence identifier into the hash table
  gpsKeys.insert( "$GPRMC", 0);
  gpsKeys.insert( "$GPGLL", 1);
  gpsKeys.insert( "$GPGGA", 2);
  gpsKeys.insert( "$GPGSA", 3);
  gpsKeys.insert( "$GPGSV", 4);
  gpsKeys.insert( "$PGRMZ", 5);
  gpsKeys.insert( "$PCAID", 6);
  gpsKeys.insert( "!w",     7);
  gpsKeys.insert( "$PGCS",  8);
  gpsKeys.insert( "$LXWP0", 9);
  gpsKeys.insert( "$LXWP2", 10);
  gpsKeys.insert( "$GPDTM", 11);

#ifdef FLARM
  gpsKeys.insert( "$PFLAA", 20);
  gpsKeys.insert( "$PFLAU", 21);
  gpsKeys.insert( "$PFLAV", 22);
  gpsKeys.insert( "$PFLAE", 23);
  gpsKeys.insert( "$PFLAC", 24);
#endif

#ifdef MAEMO5
  gpsKeys.insert( "$MAEMO0", 30);
  gpsKeys.insert( "$MAEMO1", 31);
#endif

  gpsKeys.squeeze();
  mutex.unlock();
}

/** Resets all data objects to their initial values. This is called
 *  at startup, at restart and if the GPS fix has been lost. */
void GpsNmea::resetDataObjects()
{
  _status = notConnected;
  _lastMslAltitude = Altitude(0);
  _lastGNSSAltitude = Altitude(0);
  calcStdAltitude( Altitude(0) );
  _lastPressureAltitude = Altitude(0);
  _reportAltitude = true;
  _lastCoord = QPoint(0,0);
  _lastSpeed = Speed(-1.0);
  _lastHeading = -1;
  _lastDate = QDate::currentDate(); // set date to a valid value
  cntSIVSentence = 1;

  // initialize satellite info
  _lastSatInfo.fixValidity       = 1; //invalid
  _lastSatInfo.fixAccuracy       = 999; //not very accurate
  _lastSatInfo.satsInView        = 0; //no satellites in view;
  _lastSatInfo.satsInUse         = 0; //no satellites in use;
  _lastSatInfo.constellation     = "";
  _lastSatInfo.constellationTime = QTime::currentTime();
  _lastClockOffset = 0;

  // altitude reference delivered by GPS unit
  _deliveredAltitude = static_cast<GpsNmea::DeliveredAltitude> (GeneralConfig::instance()->getGpsAltitude());
  _userAltitudeCorrection = GeneralConfig::instance()->getGpsUserAltitudeCorrection();

  // special logger items
  _lastWindDirection = 0;
  _lastWindSpeed = Speed(0);
  _lastWindAge = 0;
  _lastQnh = 0;
  _lastVariometer = Speed(0);
  _lastMc = Speed(0);
  _lastTas = Speed(0);
  _lastUtc = QDateTime();

  _ignoreConnectionLost = false;
  _gprmcSeen = false;
  _baroAltitudeSeen = false;

  reportedUnknownKeys.clear();

#ifdef FLARM
  pflaaIsReceiving = false;
  Flarm::reset();
  emit newFlarmCount( -1 );
#endif
}

void GpsNmea::createGpsConnection()
{
#ifndef ANDROID

  // qDebug("GpsNmea::createGpsConnection()");
  QObject *gpsObject = 0;

  // We create only a GpsCon instance. The GPS daemon process will be started
  // later. This is also valid hence Maemo5.
  QString callPath = GeneralConfig::instance()->getAppRoot() + "/bin";

  serial = new GpsCon(this, callPath.toAscii().data());

  gpsObject = serial;

  connect (gpsObject, SIGNAL(newSentence(const QString&)),
           this, SLOT(slot_sentence(const QString&)) );

  // Broadcasts the new NMEA sentence
  connect (gpsObject, SIGNAL(newSentence(const QString&)),
           this, SIGNAL(newSentence(const QString&)) );

  // The connection to the GPS receiver or daemon has been lost
  connect (gpsObject, SIGNAL(gpsConnectionOff()),
           this, SLOT( _slotGpsConnectionOff()) );

  // The connection to the GPS receiver or daemon has been established
  connect (gpsObject, SIGNAL(gpsConnectionOn()),
           this, SLOT( _slotGpsConnectionOn()) );
#endif
}

/**
 * Enables or disables the notifications from the GPS receiver socket. Can
 * be used to stop GPS data receiving for a certain time to prevent data loss.
 * But be careful to prevent a receiver socket buffer overflow!
 */
void GpsNmea::enableReceiving( bool enable )
{
  // qDebug("GpsNmea::enableReceiving(%s)", enable ? "true" : "false");

  _enableGpsDataProcessing = enable;

#ifndef ANDROID

  if ( serial )
    {
      QSocketNotifier* notifier = static_cast<QSocketNotifier *> (0);

      notifier = serial->getDaemonNotifier();

      if( notifier )
        {
          notifier->setEnabled( enable );
        }
    }

#endif
}

/**
 * Starts the GPS client process and activates the GPS receiver.
 */
void GpsNmea::startGpsReceiver()
{
#ifndef ANDROID

  if ( serial )
    {
      serial->startClientProcess();
    }

#endif
}

/**
 * This slot is called by the GpsCon object when a new sentence has
 * arrived from the GPS receiver. The argument contains the sentence to
 * analyze.
 *
 * @AP 2009-03-02: There was added support for a Cambrigde device. This device
 * emits proprietary sentences $PCAID and !w. It can also deliver altitudes
 * (MSL and STD) derived from a pressure sensor. If these values should be
 * used as base altitude instead of the normal GPS altitude, the user option
 * altitude must be set to PRESSURE.
 *
 * @AP 2010-03-16: There was added special support for Maemo. Two proprietary
 * sentences $MAEMO[0/1] are used to transfer the GPS data from the daemon process
 * to Cumulus.
 *
 * @AP 2010-07-31: There was added support for FLARM devices. The sentences
 * $PFLAU, $PFLAA and $PGRMZ are processed.
 *
 * @AP 2010-08-12: The GPS sentence checksum is checked now in the receiver
 * function. Only positive verified sentences are forwarded to this slot.
 */
void GpsNmea::slot_sentence(const QString& sentenceIn)
{
  // qDebug("GpsNmea::slot_sentence: %s", sentenceIn.toLatin1().data());
  if( nmeaLogFile && nmeaLogFile->isOpen() )
    {
      // Write sentence into log file
      nmeaLogFile->write(sentenceIn.toAscii().data());
    }

  if( sentenceIn.isEmpty() )
    {
      return;
    }

  if ( QObject::signalsBlocked() )
    {
      // @AP: If the emitting of signals is blocked, we will ignore
      // this call. Otherwise this module can do internal state
      // changes, which will never distributed, e.g. Man->Gps mode
      // change. To ignore this causes fatal problems in Cumulus.
      return;
    }

  // Split sentence in single parts for each comma and the checksum. The first
  // part will contain the identifier, the rest the arguments.
  QStringList slst = sentenceIn.split( QRegExp("[,*]"), QString::KeepEmptyParts );

  if( ! gpsHash.contains(slst[0]) )
    {
      if( ! reportedUnknownKeys.contains(slst[0]) )
        {
          qWarning() << "GpsNmea::slot_sentence: No Id found for" << slst[0];
          reportedUnknownKeys.insert(slst[0]);
        }

      return;
    }

  dataOK();

#ifdef FLARM

  if( slst[0] == "$PFLAA" )
    {
      // PFLAA receiving starts
      pflaaIsReceiving = true;
      // qDebug() << "PFLAA receiving started";
    }
  else if( pflaaIsReceiving == true )
    {
      // PFLAA receiving is finished
      pflaaIsReceiving = false;
      // qDebug() << "PFLAA receiving finished";
      Flarm::instance()->collectPflaaFinished();
    }

#endif

#if 0
//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

  if( slst[0] == "$GPRMC" )
    {
      QString pflau ="$PFLAU,9,1,2,1,3,20,2,-139,2073,DD8452*";

      uint sum = calcCheckSum( pflau.size(), pflau );

      QString sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflau + sumStr );

      //-------------------------------------------------------------
      QString pflaa = "$PFLAA,3,-242,40,-139,2,DD8452,270,,21,0.9,1*";

      sum = calcCheckSum( pflaa.size(), pflaa );

      sumStr = QString("%1").arg( sum, 2, 16, QChar('0') );

      slot_sentence( pflaa + sumStr );

      //---------------------------------------------------------------
      pflaa = "$PFLAA,2,-700,-700,100,2,222222,0,,30,-0.2,1*";

      sum = calcCheckSum( pflaa.size(), pflaa );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      //-------------------------------------------------------------
      pflaa = "$PFLAA,0,-2000,2000,100,2,333333,90,,30,-0.3,1*";

      sum = calcCheckSum( pflaa.size(), pflaa );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      //---------------------------------------------------------------
      pflaa = "$PFLAA,0,-1547,69,444,2,444444,180,,30,1.4,1*";

      sum = calcCheckSum( pflaa.size(), pflaa );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,1,347,-1669,1555,2,555555,270,,30,1.5,1*";

      sum = calcCheckSum( pflaa.size(), pflaa );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,2,347,-69,2666,2,666666,66,,30,-1.6,1*";

      sum = calcCheckSum( pflaa.size(), pflaa );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,0,-2747,3669,-77,2,777777,359,,30,1.7,1*";

      sum = calcCheckSum( pflaa.size(), pflaa );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,0,-3,-5000,400,2,888888,199,,30,-1.8,1*";

      sum = calcCheckSum( pflaa.size(), pflaa );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,0,4747,-2,999,2,999999,245,,30,2.9,1*";

      sum = calcCheckSum( pflaa.size(), pflaa );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

    }

//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
#endif

  // Call the decode methods for the known sentences
  switch( gpsHash.value( slst[0] ) )
  {
    case 0: // GPRMC
      __ExtractGprmc( slst );
      return;
    case 1: // GPGLL
      __ExtractGpgll( slst );
      return;
    case 2: // GPGGA
      __ExtractGpgga( slst );
      return;
    case 3: // GPGSA
      __ExtractConstellation( slst );
      return;
    case 4: // GPGSV
      __ExtractSatsInView( slst );
      return;
    case 5: // PGRMZ
      __ExtractPgrmz( slst );
      return;
    case 6: // PCAID
      __ExtractPcaid( slst );
      return;
    case 7: // !w
      __ExtractCambridgeW( slst );
      return;
    case 8: // $PGCS
      __ExtractPgcs( slst );
      return;
    case 9: // $LXWP0
      __ExtractLxwp0( slst );
      return;
    case 10: // $LXWP2
      __ExtractLxwp2( slst );
      return;
    case 11: // $GPDTM
      __ExtractGpdtm( slst );
      return;

#ifdef FLARM

    case 20: // $PFLAA
      {
        Flarm::FlarmAcft aircraft;
        Flarm::instance()->extractPflaa( slst, aircraft );
        return;
      }
    case 21: // $PFLAU
      __ExtractPflau( slst );
      return;

    case 22: // $PFLAV
      Flarm::instance()->extractPflav( slst );
      return;

    case 23: // $PFLAE
      Flarm::instance()->extractPflae( slst );
      return;

    case 24: // $PFLAC
      Flarm::instance()->extractPflac( slst );
      return;

#endif

#ifdef MAEMO5

    case 50:
      // Handle sentences created by GPS Maemo Client process. These sentenceIns
      // contain no checksum items.
      __ExtractMaemo0( slst );
      return;

    case 51:
      // Handle sentences created by GPS Maemo Client process. These sentenceIns
      // contain no checksum items.
      __ExtractMaemo1( slst );
      return;

#endif

    default:

      qWarning() << "Unknown GPS sentence:" << sentenceIn;
      return;
  }
}

/**

  See http://www.nmea.de/nmea0183datensaetze.html

  RMC - Recommended Minimum Navigation Information

                                                               12
          1         2 3       4 5        6 7   8   9     10  11 | 13
          |         | |       | |        | |   |   |      |   | | |
   $--RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a,a*hh<CR><LF>

   $GPRMC,132217.00,A,5228.19856,N,01408.32249,E,47.100,267.38,300710,,,A*58

   Field Number:
    1) UTC Time
    2) Status, A = valid position, V = Navigation receiver warning
    3) Latitude ddmm.mmm
    4) Latitude hemisphere, N or S
    5) Longitude dddmm.mm
    6) Longitude hemisphere, E or W
    7) Speed over ground, knots 0.0 ... 999.9
    8) Course over ground, degrees true
    9) UTC date of position fix, ddmmyy format
   10) Magnetic Variation, degrees
   11) Magnetic Variation direction, E or W
   12) Signal integrity, A=Autonomous mode
   13) Checksum, hh
*/
void GpsNmea::__ExtractGprmc( const QStringList& slst )
{
  if( slst.size() < 10 )
    {
      qWarning( "$GPRMC contains too less parameters!" );
      return;
    }

  _gprmcSeen = true;

  //qDebug("%s",slst[2].toLatin1().data());
  if( slst[2] == "A" )
    { /* Data status A=OK, V=warning */
      fixOK( "RMC" );

      __ExtractTime(slst[1]);
      __ExtractDate(slst[9]);
      __ExtractKnotSpeed(slst[7]);
      __ExtractCoord(slst[3],slst[4],slst[5],slst[6]);
      __ExtractHeading(slst[8]);

      if( _lastTime.isValid() && _lastDate.isValid() )
        {
          QDateTime utc( _lastDate, _lastTime, Qt::UTC );

          if( utc != _lastUtc )
            {
              _lastUtc = utc;
            }

          static bool updateClock = true;

          GeneralConfig *conf = GeneralConfig::instance();

          if( updateClock && conf->getGpsSyncSystemClock() )
            {
              // @AP: we make only one update to avoid confusing of running timers
              updateClock = false;
              setSystemClock( utc );
            }
        }

      if( _lastTime != _lastRmcTime )
        {
          /**
           * The fix time has been changed and that is reported now.
           * We do check the fix time only here in the $GPRMC sentence.
           */
          _lastRmcTime = _lastTime;
          emit newFix( _lastRmcTime );
        }
    }
  else
    {
      fixNOK( "RMC" );

      QTime time = __ExtractTime( slst[1] );
      QDate date = __ExtractDate( slst[9] );

      if( time.isValid() && date.isValid() )
        {
          QDateTime utc( _lastDate, _lastTime, Qt::UTC );

          if( utc != _lastUtc )
            {
              // save date and time as UTC
              _lastUtc = utc;
            }
        }
    }
}

/**
  GLL - Geographic Position - Latitude/Longitude

         1       2 3        4 5         6 7
         |       | |        | |         | |
  $--GLL,llll.ll,a,yyyyy.yy,a,hhmmss.ss,A*hh<CR><LF>

   Field Number:
    1) Latitude
    2) N or S (North or South)
    3) Longitude
    4) E or W (East or West)
    5) Universal Time Coordinated (UTC)
    6) Status A - Data Valid, V - Data Invalid
    7) Checksum
*/
void GpsNmea::__ExtractGpgll( const QStringList& slst )
{
  if ( slst.size() < 7 )
    {
      qWarning("$GPGLL contains too less parameters!");
      return;
    }

  if (slst[6] == "A")
    {
      fixOK( "GGL" );
      __ExtractTime(slst[5]);
      __ExtractCoord(slst[1],slst[2],slst[3],slst[4]);
    }
  else
    {
      fixNOK( "GGL" );
    }
}

/**
  GGA - Global Positioning System Fix Data, Time, Position and fix related data for a GPS receiver.

          1         2       3 4        5 6 7  8   9  10 11 12 13  14   15
          |         |       | |        | | |  |   |   | |   | |   |    |
   $--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh<CR><LF>

   Field Number:
    1) Universal Time Coordinated (UTC) of position fix, hhmmss format
    2) Latitude, ddmm.mmmm format (leading zeros will be transmitted)
    3) Latitude hemisphere, N or S
    4) Longitude, dddmm.mmmm format (leading zeros will be transmitted)
    5) Longitude hemisphere, E or W
    6) GPS Quality Indicator,
       0 - fix not available,
       1 - GPS fix,
       2 - Differential GPS fix
       6 - Estimated
    7) Number of satellites in view, 00 - 12
    8) Horizontal Dilution of precision 0.5...99.9
    9) Antenna Altitude above/below mean-sea-level (geoid)
   10) Units of antenna altitude, meters
   11) Geoidal separation, the difference between the WGS-84 earth
       ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level
       below ellipsoid
   12) Units of geoidal separation, meters
   13) Age of differential GPS data, time in seconds since last SC104
       type 1 or 9 update, null field when DGPS is not used
   14) Differential reference station ID, 0000-1023
   15) Checksum
*/
void GpsNmea::__ExtractGpgga( const QStringList& slst )
{
  if ( slst.size() < 15 )
    {
      qWarning("$GPGGA contains too less parameters!");
      return;
    }

  if ( slst[6] != "0" && ! slst[6].isEmpty() )
    {
      /*a value of 0 means invalid fix and we don't need that one */
      if( _gprmcSeen == false )
        {
          // Report fix info only, if GPRMC was not received. I saw
          // sometimes different reportings. GGA said OK, RMC said NOK.
          fixOK( "GGA" );
        }

      __ExtractTime(slst[1]);
      __ExtractCoord(slst[2],slst[3],slst[4],slst[5]);
      __ExtractAltitude(slst[9],slst[10]);
      __ExtractSatsInView(slst[7]);
    }
  else if( slst[6] == "0" )
    {
      if( _gprmcSeen == false )
        {
          // Report fix info only, if GPRMC was not received. I saw
          // sometimes different reportings. GGA said OK, RMC said NOK.
          fixNOK( "GGA" );
        }
    }
}

/**
  Used by Garmin and Flarm devices
  $PGRMZ,93,f,3*21
         93,f         Altitude in feet
         3            Position fix dimensions 2 = FLARM barometric altitude
                                              3 = GPS altitude
*/
void GpsNmea::__ExtractPgrmz( const QStringList& slst )
{
  if ( slst.size() < 4 )
    {
      qWarning("$PGRMZ contains too less parameters!");
      return;
    }

  /* Garmin proprietary sentence with altitude information */
  if ( slst[3] == "2" ) // pressure altitude
    {
      bool ok;
      double num = slst[1].toDouble( &ok );

      if( ok )
        {
          _baroAltitudeSeen = true;

          Altitude altitude;

          altitude.setFeet( num );

          if( _lastPressureAltitude != altitude || _reportAltitude == true )
            {
              _reportAltitude = false;
              _lastPressureAltitude = altitude; // store the new pressure altitude

              if( _deliveredAltitude == GpsNmea::PRESSURE )
                {
                  // Set these altitudes too, when pressure is selected.
                  _lastMslAltitude.setMeters( altitude.getMeters() + _userAltitudeCorrection.getMeters() );

                  // calculate STD altitude
                  calcStdAltitude( _lastMslAltitude );
                }

              emit newAltitude( _lastMslAltitude, _lastStdAltitude, _lastGNSSAltitude );
            }
          }

        return;
      }

  if ( slst[3] == "3" ) // 3=GPS altitude
    {
      __ExtractAltitude(slst[1], slst[2]);
      return;
    }
}

/**
  Used by Cambridge devices. The PCAID sentence format is:

  $PCAID,<1>,<2>,<3>,<4>*hh<CR><LF>
  $PCAID,N,-00084,000,128*0B

  <1>     Logged 'L' Last point Logged 'N' Last Point not logged
  <2>     Barometer Altitude in meters (Leading zeros will be transmitted), Hendrik said: STD
  <3>     Engine Noise Level
  <4>     Log Flags
  *hh     Checksum, XOR of all bytes of the sentence after the `$' and before the '!'
*/
void GpsNmea::__ExtractPcaid( const QStringList& slst )
{
  if ( slst.size() < 5 )
    {
      qWarning("$PCAID contains too less parameters!");
      return;
    }

  Altitude res(0);
  double num = slst[2].toDouble();
  res.setMeters( num );

  if ( _lastStdAltitude != res && _deliveredAltitude == GpsNmea::PRESSURE )
    {
      // Store this altitude as STD, if the user has pressure selected.
      // In the other case the STD is derived from the GPS altitude.
      _lastStdAltitude = res;
      // This altitude must not be notified as new value because
      // the Cambridge device delivers also MSL in its !w sentence.
    }
}

/**
  Used by Garrecht Volkslogger devices. The PGCS1 sentence format is:

  $PGCS,<1>,<2>,<3>,<4>,<5>*CS
  $PGCS,1,0EBC,0003,06A6,03*1F

  Volkslogger pressure and pressure altitude information

  0  - PGCS - Sentence ID
  1  - 1 - gcs-sentence no. 1
  2  - 0EBC - (pressure * 4096) / 1100: value of pressure-sensor (hex coded)
  3  - 0003 - pressure altitude [m] (hex coded)
  4  - 06A6 - reserved for further use?
  5  - 03 - reserved for further use?
  CS - 1F - checksum of total sentence
*/
void GpsNmea::__ExtractPgcs( const QStringList& slst )
{
  if ( slst.size() < 6 )
    {
      qWarning("$PGCS contains too less parameters!");
      return;
    }

  Altitude res(0);
  bool ok;
  int num = slst[3].toInt(&ok, 16);

  if (!ok)
    {
      qWarning("$PGCS contains corrupt pressure altitude!");
      return;
    }

  if (num > 32768)
    {
      num -= 65536;  // FFFF = -1, FFFE = -2, etc.
    }

  res.setMeters( num );

  if( ( _lastStdAltitude != res || _reportAltitude == true ) &&
      _deliveredAltitude == GpsNmea::PRESSURE )
    {
      _reportAltitude = false;
      // Store this altitude as STD, if the user has pressure selected.
      // In the other case the STD is derived from the GPS altitude.
      _lastStdAltitude = res;
      // We only get a STD altitude from the Volkslogger, so we calculate
      // the MSL altitude using the QNH provided by the user
      calcMslAltitude( res );
      // Since in GpsNmea::PRESSURE mode _lastPressureAltitude is returned,
      // we set it, too.
      _lastPressureAltitude = _lastMslAltitude;
      emit newAltitude( _lastMslAltitude, _lastStdAltitude, _lastGNSSAltitude );
    }
}

#ifdef FLARM

  /**
  $PFLAU,<RX>,<TX>,<GPS>,<Power>,<AlarmLevel>,<RelativeBearing>,<AlarmType>,
  <RelativeVertical>,<RelativeDistance>,<ID>
  */
void GpsNmea::__ExtractPflau( const QStringList& slst )
{
  bool res = Flarm::instance()->extractPflau( slst );

  if( res )
    {
      static QTime lastReporting;

      // Check the GPS fix state reported by Flarm.
      const Flarm::FlarmStatus& status = Flarm::instance()->getFlarmStatus();

      if( status.Gps == Flarm::NoFix )
        {
          fixNOK( "PFLAU" );
        }
      else
        {
          fixOK( "PFLAU" );
        }

      if( lastReporting.elapsed() >= 5000 )
        {
          // To reduce load, Flarm count is reported only after 5s.
          // We do send always a new state independently of a change
          // in the mean time.
          lastReporting  = QTime::currentTime();

          emit newFlarmCount( status.RX );
        }
    }
}

#endif

/**
  DTM - Datum Reference

         1   2 3       4 5       6 7 8  9
         |   | |       | |       | | |  |
  $--DTM,xxx,x,xx.xxxx,x,xx.xxxx,x,,xxx*hh<CR><LF>

  Field Number:
  1) Local datum code
      W84 - WGS84
      W72 - WGS72
      S85 - SGS85
      P90 - PE90
      999 - User defined
      IHO datum code
  2) Local datum sub code
  3) Latitude offset (minute)
  4) Latitude offset mark (N: +, S: -)
  5) Longitude offset (minute)
  6) Longitude offset mark (E: +, W: -)
  7) Altitude offset (m) Always null
  8) Datum
      W84 - WGS84
      W72 - WGS72
      S85 - SGS85
      P90 - PE90
      ...
  9) Checksum
*/
void GpsNmea::__ExtractGpdtm( const QStringList& slst )
{
  if ( slst.size() < 9 )
    {
      qWarning("$GPDTM contains too less parameters!");
      return;
    }

  _mapDatum = slst[8];
}

/**
 * @Returns the last know GPS altitude depending on user
 * selection MSL or Pressure. MSL is the default.
 */
Altitude GpsNmea::getLastAltitude() const
{
  if ( GeneralConfig::instance()->getGpsAltitude() == GpsNmea::PRESSURE )
    {
      return _lastPressureAltitude;
    }

  // MSL is the default in all other cases, when the user did
  // not select pressure.
  return _lastMslAltitude;
};

/**
  Used by Cambridge devices. The !w proprietary  sentence format is:

  !w,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>,<13>*hh<CR><LF>
  !w,000,000,0000,500,01032,01027,00053,200,200,200,000,000,100*51

  <1>    Vector wind direction in degrees
  <2>    Vector wind speed in 10ths of meters per second
  <3>    Vector wind age in seconds
  <4>    Component wind in 10ths of Meters per second + 500 (500 = 0, 495 = 0.5 m/s
         tailwind)
  <5>    True altitude in Meters + 1000 (pressure altitude related to MSL)
  <6>    Instrument QNH setting
  <7>    True airspeed in 100ths of Meters per second
  <8>    Variometer reading in 10ths of knots + 200
  <9>    Averager reading in 10ths of knots + 200
  <10>   Relative variometer reading in 10ths of knots + 200
  <11>   Instrument MacCready setting in 10ths of knots
  <12>   Instrument Ballast setting in percent of capacity
  <13>   Instrument Bug setting

  *hh   Checksum, XOR of all bytes of the sentence after the `!' and before the '*'

  Extracts wind, QNH and vario data from Cambridge's !w sentence.
*/
void GpsNmea::__ExtractCambridgeW( const QStringList& stringList )
{
  bool ok, ok1;
  Speed speed(0);
  double num = 0.0;

  if ( stringList.size() < 14 )
    {
      qWarning("!w contains too less parameters!");
      return;
    }

  // extract wind direction in degrees
  int windDir = stringList[1].toInt( &ok );

  // calculate inverse angle
  windDir = windDir < 180 ? windDir+180 : windDir-180;

  // wind speed in 10ths of meters per second
  num = stringList[2].toDouble( &ok1 );
  speed.setMps( num/10. );

  // Wind is only emitted if a valid position fix is available.
  if ( ok && ok1 && _status == validFix &&
       (_lastWindDirection != windDir || _lastWindSpeed != speed ))
    {
      _lastWindDirection = windDir;
      _lastWindSpeed = speed;
      emit newWind( _lastWindSpeed, _lastWindDirection ); // notify change
    }

  // extract true altitude in Meters + 1000 (Hendrik said: pressure altitude related to MSL)
  Altitude res(0);
  num = stringList[5].toDouble( &ok ) -1000.0;
  res.setMeters( num );

  if( ok )
    {
      if( _lastPressureAltitude != res || _reportAltitude == true )
      {
        _reportAltitude = false;
        _lastPressureAltitude = res; // store the new pressure altitude

        if ( _deliveredAltitude == GpsNmea::PRESSURE )
          {
            // set these altitudes too, when pressure is selected
            _lastMslAltitude.setMeters( res.getMeters() + _userAltitudeCorrection.getMeters() );
            // STD altitude is delivered by Cambrigde via $PCAID record
            // calcStdAltitude( res );
          }

        emit newAltitude( _lastMslAltitude, _lastStdAltitude, _lastGNSSAltitude );
      }
    }

  // extract QNH
  ushort qnh = stringList[6].toUShort( &ok );

  if( ok && _lastQnh != qnh )
    {
      // update the QNH in GeneralConfig
      GeneralConfig::instance()->setQNH( qnh );
    }

  // True airspeed in 100ths of Meters per second
  if( ! stringList[7].isEmpty() )
    {
      num = stringList[7].toDouble( &ok ) / 100.0;

      if( ok )
        {
          speed.setMps( num );

          if( _lastTas != speed )
            {
              _lastTas = speed;
              emit newTas( _lastTas );
            }
        }
    }

  // extract variometer, reading in 10ths of knots + 200
  num = (stringList[8].toDouble( &ok ) - 200.0) / 10.0;
  speed.setKnot( num );

  if ( ok && _lastVariometer != speed )
    {
      _lastVariometer = speed;
      emit newVario( _lastVariometer ); // notify change
    }

  // extract MacCready, reading in 10ths of knots
  num = stringList[11].toDouble( &ok ) / 10.0;
  speed.setKnot( num );

  if ( ok && _lastMc != speed )
    {
      _lastMc = speed;
      emit newMc( _lastMc ); // notify change
    }
}

/**
 * Extracts speed, altitude, vario, heading, wind data from LX Navigation $LXWP0
 * sentence.

    Used by LX Navigation devices. The LXWP0 sentence format is:

    $LXWP0,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>,*CS
    LXWP0,Y,222.3,1665.5,1.71,,,,,,239,174,10.1

    LX Navigation vario, pressure altitude, airspeed, wind information

    0 - LXWP0 - Sentence ID
    1 - 'Y' or 'N' logger stored
    2 - air speed in km/h as double
    3 - pressure altitude in meters as double
    4...9 - vario values in m/s, last 6 measurements in last second or only one value as double
    10 - heading degree of plane as integer
    11 - wind direction as degree
    12 - wind speed km/h as double
    CS - checksum of total sentence
*/

void GpsNmea::__ExtractLxwp0( const QStringList& stringList )
{
  bool ok, ok1;
  Speed speed(0);
  double num = 0.0;

  if ( stringList.size() < 13 )
    {
      qWarning("$LXWP0 contains too less parameters!");
      return;
    }

  // airspeed TAS in km/h
  if( ! stringList[2].isEmpty() )
    {
      num = stringList[2].toDouble( &ok );

      if( ok )
        {
          speed.setKph( num );

          if( _lastTas != speed )
            {
              _lastTas = speed;
              emit newTas( _lastTas );
            }
        }
    }

  // pressure altitude in meters
  if( ! stringList[3].isEmpty() )
    {
      num = stringList[3].toDouble( &ok );

      if( ok )
        {
          _baroAltitudeSeen = true;

          Altitude altitude( num );

          if( _lastPressureAltitude != altitude || _reportAltitude == true )
            {
              _reportAltitude = false;
              _lastPressureAltitude = altitude; // store the new pressure altitude

              if( _deliveredAltitude == GpsNmea::PRESSURE )
                {
                  // set these altitudes too, when pressure is selected
                  _lastMslAltitude.setMeters( altitude.getMeters() + _userAltitudeCorrection.getMeters() );
                  // calculate STD altitude
                  calcStdAltitude( altitude );
                }

              emit newAltitude( _lastMslAltitude, _lastStdAltitude, _lastGNSSAltitude );
            }
        }
    }

  // Variometer values 1...6 in m/s. Can be also zero or only one value available.
  // If more than one value contained the average value is computed.
  int varioValues = 0;
  double varioTotal = 0.0;

  for( int i = 4; i < 10; i++ )
    {
      if( ! stringList[i].isEmpty() )
        {
          num = stringList[i].toDouble( &ok );

            if( ok )
              {
                varioTotal += num;
                varioValues++;
              }
          }
    }

  if( varioValues > 0 )
    {
      // compute variometer average value
      speed.setMps( varioTotal / (double) varioValues );

      if ( _lastVariometer != speed )
        {
          _lastVariometer = speed;
          emit newVario( _lastVariometer ); // notify change
        }
    }

  // heading degree of plane
  num = __ExtractHeading( stringList[10] );

  // extract wind direction in degrees
  int windDir = static_cast<int> (rint(stringList[11].toDouble( &ok )));

  // wind speed in km/h
  num = stringList[12].toDouble( &ok1 );
  speed.setKph( num );

  // Wind is only emitted if a valid position fix is available.
  if ( ok && ok1 && _status == validFix &&
      ( _lastWindDirection != windDir || _lastWindSpeed != speed ))
    {
      _lastWindDirection = windDir;
      _lastWindSpeed = speed;
      emit newWind( _lastWindSpeed, _lastWindDirection ); // notify change
    }
}

/**
    Used by LX Navigation devices. The LXWP2 sentence format is:

    $LXWP2,<1>,<2>,<3>,<4>,<5>,<6>,<7>,*CS
    0 - LXWP2 - Sentence ID
    1 - McCready float in m/s
    2 - Bugs 0...100%
    ...

   Extracts McCready data from LX Navigation $LXWP2 sentence.
*/
void GpsNmea::__ExtractLxwp2( const QStringList& stringList )
{
  bool ok;
  Speed speed(0);
  double num = 0.0;

  if ( stringList.size() < 8 )
    {
      qWarning("$LXWP2 contains too less parameters!");
      return;
    }

  // extract MacCready in m/s
  if( ! stringList[1].isEmpty() )
    {
      num = stringList[1].toDouble( &ok );
      speed.setMps( num );

      if ( ok && _lastMc != speed )
        {
          _lastMc = speed;
          emit newMc( _lastMc ); // notify change
        }
    }
}

/**
 * This function returns a QTime from the time encoded in a MNEA sentence.
 */
QTime GpsNmea::__ExtractTime(const QString& timeString)
{
  if( timeString.isEmpty() && timeString.size() < 6 )
    {
      // qWarning("Invalid GPS time %s", timeString.toLatin1().data());
      return QTime();
    }

  QString hh (timeString.left(2));
  QString mm (timeString.mid(2,2));
  QString ss (timeString.mid(4,2));

  // @AP: newer CF Cards can also provide milliseconds. In this case the time
  // format is defined as hhmmss.sss. But we will not use it to avoid problems
  // with our fixes.
  QTime res = QTime( hh.toInt(), mm.toInt(), ss.toInt() );

  // @AP: don't overtake invalid times. They will cause invalid fixes!
  if ( ! res.isValid() )
    {
      qWarning("GpsNmea::__ExtractTime(): Invalid time %s! Ignoring it (%s, %d)",
               timeString.toLatin1().data(), __FILE__, __LINE__ );
      return QTime();
    }

  _lastTime = res;

  return res;
}

/** This function returns a QDate from the date string encoded in a
    NWEA sentence as "ddmmyy". */
QDate GpsNmea::__ExtractDate(const QString& dateString)
{
  if( dateString.isEmpty() && dateString.size() != 6 )
    {
      // qWarning("Invalid GPS date %s", dateString.toLatin1().data());
      return QDate();
    }

  QString dd (dateString.left(2));
  QString mm (dateString.mid(2,2));
  QString yy (dateString.right(2));

  /*we assume that we only use this after the year 2000, which is
    reasonable since this is made 2002 ...*/

  bool ok1, ok2, ok3;

  QDate res (yy.toInt(&ok1) + 2000, mm.toInt(&ok2), dd.toInt(&ok3) );

  // @AP: don't take over invalid dates
  if ( ok1 && ok2 && ok3 && res.isValid() )
    {
      _lastDate = res;
    }
  else
    {
      qWarning("GpsNmea::__ExtractDate(): Invalid date %s! Ignoring it (%s, %d)",
               dateString.toLatin1().data(), __FILE__, __LINE__ );
    }

  return res;
}

/** This function returns a Speed from the speed encoded in knots */
Speed GpsNmea::__ExtractKnotSpeed(const QString& speedString)
{
  Speed res;

  if( speedString.isEmpty() )
    {
      return res;
    }

  bool ok;
  double speed = speedString.toDouble( &ok );

  if( ok == false )
    {
      return res;
    }

  res.setKnot( speed );

  if( res != _lastSpeed )
    {
      _lastSpeed = res;
      emit newSpeed( _lastSpeed );
    }

  return res;
}

/** This function converts the coordinate data from the NMEA sentence to the internal QPoint format. */
QPoint GpsNmea::__ExtractCoord(const QString& slat, const QString& slatNS,
                               const QString& slon, const QString& slonEW)
{
  /* The internal KFLog format for coordinates represents coordinates in 10.000'st of a minute.
     So, one minute corresponds to 10.000, one degree to 600.000 and one second to 167.
     KFLogCoord = degrees * 600000 + minutes * 10000
  */

  if( slat.isEmpty() || slatNS.isEmpty() ||
      slon.isEmpty() || slonEW.isEmpty() )
      {
        return QPoint();
      }

  int lat = 0;
  int lon = 0;
  float fLat = 0.0;
  float fLon = 0.0;

  bool ok1, ok2, ok3, ok4;

  lat  = slat.left(2).toInt(&ok1);
  fLat = slat.mid(2).toFloat(&ok2);

  // qDebug ("slat: %s", slat.toLatin1().data());
  // qDebug ("lat/fLat: %d/%f", lat, fLat);

  lon  = slon.left(3).toInt(&ok3);
  fLon = slon.mid(3).toFloat(&ok4);

  if( !ok1 || !ok2 || !ok3 || !ok4 )
    {
      return QPoint();
    }

  // qDebug ("slon: %s", slon.toLatin1().data());
  // qDebug ("lon/fLon: %d/%f", lon, fLon);

  int latMin = (int) rint(fLat * 10000);
  int lonMin = (int) rint(fLon * 10000);

  // convert to internal KFLog format
  int latTemp = lat * 600000 + latMin;
  int lonTemp = lon * 600000 + lonMin;

  // qDebug("latTemp=%d, lonTemp=%d", latTemp, lonTemp);

  if (slatNS == "S")
    {
      latTemp = -latTemp;
    }

  if (slonEW == "W")
    {
      lonTemp = -lonTemp;
    }

  QPoint res (latTemp, lonTemp);

  if ( _lastCoord != res )
    {
      _lastCoord=res;
      emit newPosition( _lastCoord );
    }

  return _lastCoord;
}

/** Extract the heading from the NMEA sentence. */
double GpsNmea::__ExtractHeading(const QString& headingstring)
{
  if( headingstring.isEmpty() )
    {
      return 0.0;
    }

  bool ok;

  double heading = headingstring.toDouble(&ok);

  if( ok == false )
    {
      return 0.0;
    }

  if ( heading != _lastHeading )
    {
      _lastHeading = heading;
      emit newHeading( _lastHeading );
    }

  return heading;
}

/**
 * Extracts the altitude from a NMEA GGA sentence or from a Garmin/Flarm
 * proprietary PGRMZ sentence.
 */
Altitude GpsNmea::__ExtractAltitude( const QString& altitude, const QString& unit )
{
  // qDebug("alt=%s, unit=%s", altitude.toLatin1().data(), unitAlt.toLatin1().data() );
  bool ok;

  Altitude res(0);
  double alt = altitude.toDouble(&ok);

  if( ok == false )
    {
      return res;
    }

  // Check for other unit as meters, meters is the default.
  // Consider user's altitude correction
  if ( unit.toLower() == "f" )
    {
      res.setFeet( alt + _userAltitudeCorrection.getFeet() );
    }
  else
    {
      res.setMeters( alt + _userAltitudeCorrection.getMeters() );
    }

  _lastGNSSAltitude = res;

  if( ( _lastMslAltitude != res || _reportAltitude == true ) &&
      _deliveredAltitude != GpsNmea::PRESSURE )
    {
      _reportAltitude = false;
      // set these altitudes only, when pressure is not selected
      _lastMslAltitude = res;
      calcStdAltitude( res );
      emit newAltitude( _lastMslAltitude, _lastStdAltitude, _lastGNSSAltitude );
    }

  return res;
}

/**
  GSA - GPS DOP and active satellites

          1 2 3                    14 15  16  17  18
          | | |                    |  |   |   |   |
   $--GSA,a,a,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x.x,x.x,x.x*hh<CR><LF>

   Field Number:
    1) Selection mode
    2) Mode, 1=no-Fix, 2=2D-Fix, 3=3D-Fix
    3) ID of 1st satellite used for fix
    4) ID of 2nd satellite used for fix
    ...
    14) ID of 12th satellite used for fix
    15) PDOP in meters
    16) HDOP in meters
    17) VDOP in meters
    18) checksum

  Extracts the constellation from the NMEA sentence.
*/
QString GpsNmea::__ExtractConstellation(const QStringList& sentence)
{
  if ( sentence.size() < 18 )
    {
      qWarning("$GPGSA contains too less parameters!");
      return "";
    }

  QString result, tmp;

  if( sentence[2] != "" )
    {
      bool ok;

      int fix = sentence[2].toInt(&ok);

      if( ok )
        {
          _lastSatInfo.fixValidity = fix;

          if( _gprmcSeen == false )
            {
              // Report fix info only, if GPRMC was not received. I saw
              // sometimes different reportings. GSA said OK, RMC said NOK.
              if( fix == 1 )
                {
                  fixNOK( "GSA" );
                }
              else
                {
                  fixOK( "GSA" );
                }
            }
        }
    }

  int lastSatsInUse = _lastSatInfo.satsInUse;
  _lastSatInfo.satsInUse = 0;

  for( int i = 3; i <= 14 && i < sentence.size(); i++ )
    {
      if( sentence[i] != "" )
        {
          _lastSatInfo.satsInUse++;
          tmp.sprintf( "%02d", sentence[i].toInt() );
          result += tmp;
        }
    }

  if( lastSatsInUse != _lastSatInfo.satsInUse )
    {
      emit newSatCount( _lastSatInfo );
    }

  if( sentence[15] != "" )
    {
      // PDOP in meters
      bool ok;
      double pdop = sentence[15].toDouble( &ok );

      if( ok == true && _lastSatInfo.fixAccuracy != pdop )
        {
          _lastSatInfo.fixAccuracy = static_cast<int>(rint(pdop));
        }
    }

  // Store receive time of constellation in every case.
  _lastSatInfo.constellationTime = _lastTime;

  if( result != _lastSatInfo.constellation )
    {
      _lastSatInfo.constellation = result;
      emit newSatConstellation( _lastSatInfo );
    }

  return result;
}

/** Extracts the satellite count in view from the NMEA sentence. */
bool GpsNmea::__ExtractSatsInView(const QString& satcount)
{
  bool ok;

  int count = satcount.toInt(&ok);

  if( ! ok )
    {
      return false;
    }

  if( ok && count != _lastSatInfo.satsInView )
    {
      _lastSatInfo.satsInView = count;
      emit newSatCount( _lastSatInfo );
    }

  return true;
}

#ifdef MAEMO5

/**
 * Extract proprietary sentence $MAEMO0. It is created by the GPS Maemo Client
 * and not all positions are always set. In such a case they are empty.
 */
void GpsNmea::__ExtractMaemo0(const QStringList& slist)
{
  /**
   * Definition of proprietary sentence $MAEMO0.
   *
   *  0) $MAEMO
   *  1) Mode
   *  2) Time stamp as unsigned integer, local time (Maemo4)
   *  3) Ept
   *  4) Latitude  in KFLog degrees
   *  5) Longitude in KFLog degrees
   *  6) Eph in m
   *  7) Speed in m/s (Maemo4), km/h (Maemo5)
   *  8) Eps
   *  9) Track in degree 0...359
   * 10) Epd
   * 11) Altitude in meters
   * 12) Epv
   * 13) Climb
   * 14) Epc
   */

  bool ok, ok1;

  /*
  Extract mode. Possible values can be:
  LOCATION_GPS_DEVICE_MODE_NOT_SEEN The device has not seen a satellite yet. (Not used by Cumulus)
  LOCATION_GPS_DEVICE_MODE_NO_FIX   The device has no fix.
  LOCATION_GPS_DEVICE_MODE_2D       The device has latitude and longitude fix.
  LOCATION_GPS_DEVICE_MODE_3D       The device has latitude, longitude, and altitude.
  */

#if 0

  // @AP: This fix information stands sometimes in contradiction to the reported
  // fix information in the $MAEMO1 sentence. Therefore I do ignore the information
  // here. Otherwise we get a ping pong setting of fix ok or nok.
  if( ! slist[1].isEmpty() )
    {
      int mode = slist[1].toInt( &ok );

      if( ok )
        {
          _lastSatInfo.fixValidity = mode;

          if( mode == LOCATION_GPS_DEVICE_MODE_3D )
            {
              fixOK( "Maemo0" );
            }
          else
            {
              fixNOK( "Maemo0" );
            }
        }
    }

#endif

  // Extract time info. It is encoded in local seconds and is converted to UTC
  if( ! slist[2].isEmpty() )
    {
      uint uintTime = slist[2].toUInt( &ok );

      if( ok )
        {
          QDateTime local;
          local.setTime_t( uintTime );

          QDateTime utc;
          utc = local.toUTC();

          if( utc.isValid() )
            {
              _lastUtc = utc;
              _lastTime = utc.time();
              _lastDate = utc.date();
              // qDebug() << _lastUtc.toString();
            }
        }
    }

  // Extract Latitude and Longitude. They are encoded in KFLog format.
  if( ! slist[4].isEmpty() && ! slist[5].isEmpty() )
    {
      int lat = slist[4].toInt( &ok );
      int lon = slist[5].toInt( &ok1 );

      if( ok && ok1 )
        {
          QPoint res(lat, lon);

          if( _lastCoord != res )
            {
              _lastCoord = res;
              emit newPosition( _lastCoord );
            }
        }
    }

  // Extract Eph, horizontal position accuracy, encoded in m.
  if( ! slist[6].isEmpty() )
    {
      // PDOP is handled in meters
      double pdop = slist[6].toDouble( &ok );

      if( ok == true && _lastSatInfo.fixAccuracy != pdop )
        {
          _lastSatInfo.fixAccuracy = static_cast<int>(rint(pdop));
        }
    }

  // Extract Speed, encoded in m/s under Maemo4 in Km/h under Maemo5
  if( ! slist[7].isEmpty() )
    {
      double dSpeed = slist[7].toDouble( &ok );

      if( ok )
        {
          Speed speed;

#ifdef MAEMO4
          speed.setMps( dSpeed ); // m/s under Maemo4
#else
          speed.setKph( dSpeed );
#endif

          if( speed != _lastSpeed )
            {
              _lastSpeed = speed;
              emit newSpeed( _lastSpeed );
            }
        }
    }

  // Extract track (heading) info.Track is encoded in degree 0...359.
  if( ! slist[9].isEmpty() )
    {
      __ExtractHeading( slist[9] );
    }

  // Extract altitude info. Altitude is encoded in meters
  if( ! slist[11].isEmpty() )
    {
      __ExtractAltitude( slist[11], "M" );
    }

  // Climb info not extracted at the moment!

  // Do report a new fix, if the time has been changed. That must be the last
  // action after the end of extraction.
  if( _lastTime != _lastRmcTime )
    {
      /**
       * The fix time has been changed and that is reported now.
       * We do check the fix time only once in the $GPRMC sentence.
       */
      _lastRmcTime = _lastTime;
      emit newFix( _lastRmcTime );
    }
}

/**
 * Extract proprietary sentence $MAEMO1.
 */
void GpsNmea::__ExtractMaemo1(const QStringList& slist)
{
  /**
   * Definition of proprietary sentence $MAEMO1.
   *
   *  0) $MAEMO1
   *  1) Status
   *  2) Satellites in view
   *  3) Satellites in use
   *  4) Satellite number
   *  5) Elevation
   *  6) Azimuth
   *  7) Signal strength
   *  8) Satellite in use
   *  9) Repetition of 4-8 according to Satellites in view
   */

  // Store receive time of constellation in every case.
  _lastSatInfo.constellationTime = _lastTime;

  bool ok;

  /**
  Extract status, possible values are:
  LOCATION_GPS_DEVICE_STATUS_NO_FIX   The device does not have a fix.
  LOCATION_GPS_DEVICE_STATUS_FIX      The device has a fix.
  */
  if( ! slist[1].isEmpty() )
    {
      int status = slist[1].toInt( &ok );

      if( ok )
        {
          if( status == LOCATION_GPS_DEVICE_STATUS_FIX  )
            {
              fixOK( "Maemo1" );
            }
          else
            {
              _lastSatInfo.fixValidity = LOCATION_GPS_DEVICE_MODE_NO_FIX;
              fixNOK( "Maemo1" ) ;
            }
        }
    }

  // Extracts Satellites in view
  if( ! slist[2].isEmpty() )
    {
      if( __ExtractSatsInView( slist[2] ) )
        {
          QString satsForFix;
          sivInfoInternal.clear();

          for( int i = 4; i <= slist.size() - 5; i +=5 )
            {
              __ExtractSatsInView( slist[i], slist[i+1], slist[i+2], slist[i+3] );

              if( ! slist[i+4].isEmpty() && slist[i+4] != "0" )
                {
                  // Satellite is used for fix
                  int sat =  slist[i].toInt( &ok );
                  satsForFix += QString("%1").arg( sat, 2, 10, QChar('0') );
                }
            }

          sivInfo = sivInfoInternal;

          if( satsForFix != _lastSatInfo.constellation )
            {
              _lastSatInfo.constellation = satsForFix;
              emit newSatConstellation( _lastSatInfo );
            }

          emit newSatInViewInfo( sivInfo );

        }
    }

  // Extracts Satellites in use
  if( ! slist[3].isEmpty() )
    {
      int satsInUse = slist[3].toInt( &ok );

    if( ok )
      {
        _lastSatInfo.satsInUse = satsInUse;
        emit newSatCount( _lastSatInfo );
      }
    }
}

#endif

/** This slot is called by the external GPS receiver process to signal
 *  a connection lost to the GPS receiver or daemon. */
void GpsNmea::_slotGpsConnectionOff()
{
  if( _ignoreConnectionLost )
    {
      // Ignore a connection lost state. That must be done after a system
      // clock update to avoid senseless reporting to other modules.
      _ignoreConnectionLost = false;
      return;
    }

  if( _status != notConnected )
    {
      qWarning( "GPS CONNECTION SEEMS TO BE LOST!" );
      resetDataObjects();
      emit statusChange( _status );
    }
}

/** This slot is called by the external GPS receiver process to signal
 *  a established connection to the GPS receiver or daemon.
 */
void GpsNmea::_slotGpsConnectionOn()
{
  dataOK();
}

/** This slot is called by the internal timer to signal a timeout.
 *  This timeout occurs if no valid position fix has been received since
 *  the last fix for the given time. The cause can be bad receiver conditions,
 *  view to the sky is not clear, a.s.o.
 */
void GpsNmea::_slotTimeoutFix()
{
  if( _status == validFix )
    {
      _status = noFix;
      emit statusChange( _status );
      qWarning( "TO: GPS FIX LOST!" );
      // stop timer, will be activated again with the next available fix
      timeOutFix->stop();

#ifdef FLARM

      pflaaIsReceiving = false;
      Flarm::reset();
      emit newFlarmCount( -1 );

#endif

    }
}

/** This function is called to indicate that valid data (checksum ok)
 *  has been received. If necessary it changes the connection status.
 */
void GpsNmea::dataOK()
{
  if ( _status == notConnected )
    {
      // reset altitudes, will set in manual mode to 1000m
      _lastMslAltitude = Altitude(0);
      _lastGNSSAltitude = Altitude(0);
      calcStdAltitude( Altitude(0) );
      _lastPressureAltitude = Altitude(0);
      emit newAltitude( _lastMslAltitude, _lastStdAltitude, _lastGNSSAltitude );

      _status = noFix;
      emit statusChange(_status);
      emit connectedChange( _status != notConnected );
      qDebug("GPS CONNECTION ESTABLISHED!");
    }
}

/** This function is called to indicate that a valid fix has been received.
 *  It resets/restarts the FIX TimeOut timer and if necessary updates
 *  the connection status.
 */
void GpsNmea::fixOK( const char* who )
{
  // restart timer for FIX supervision
  timeOutFix->start( FIX_TO );

  if( _status != validFix )
    {
      _status = validFix;
      emit statusChange( _status );
      qDebug() << who << "GPS FIX OBTAINED!";
    }
}

/** This function is called to indicate that a valid fix has been lost.
 *  It necessary updates the connection status.
 */
void GpsNmea::fixNOK( const char* who )
{
  // stop timer, will be activated again with the next available fix
  timeOutFix->stop();

  if( _status == validFix )
    {
      _status = noFix;
      emit statusChange( _status );
      qDebug() << who << "GPS FIX Lost!";
    }
}

/**
 * This slot is called if the GPS needs to reset or update. It is used to stop
 * the GPS receiver connection and to opens a new one to adjust the
 * new settings.
 */
void GpsNmea::slot_reset()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // altitude reference delivered by GPS unit
  _deliveredAltitude = static_cast<GpsNmea::DeliveredAltitude> (conf->getGpsAltitude());
  _userAltitudeCorrection = conf->getGpsUserAltitudeCorrection();
  _reportAltitude = true;

#ifndef ANDROID

  QString oldDevice = gpsDevice;

  if ( gpsDevice != conf->getGpsDevice() )
    {
      // qDebug() << "slot_reset(): GPS Device changed";
      // GPS device has been changed by the user
      gpsDevice = conf->getGpsDevice();

      if ( serial )
        {
          serial->stopGpsReceiving();
          delete serial;
          serial = 0;
        }

      // reset data objects
      resetDataObjects();
      // create a new connection
      createGpsConnection();
      // start the new connection
      startGpsReceiver();
      return;
    }

  // no device modification, check serials baud rate
  if ( serial )
    {
      if ( serial->currentBautrate() != conf->getGpsSpeed() )
        {
          // qDebug() << "slot_reset(): GPS Baudrate changed";
          serial->stopGpsReceiving();
          serial->startGpsReceiving();
        }

      // @AP: Must we do that?
      // bool hard = conf->getGpsHardStart();
      // bool soft = conf->getGpsSoftStart();
      // sendLastFix (hard, soft);
    }

#endif

}

bool GpsNmea::sendSentence(const QString command)
{
#ifndef ANDROID

  if( serial )
    {
      // Only a serial can forward commands to the GPS.
      return serial->sendSentence( command );
    }


  return false;

#else

  // We have to add the checksum and cr lf to the command.
  uint csum = calcCheckSum( command.toAscii().data() );
  QString check;
  check.sprintf ("*%02X\r\n", csum);
  QString cmd (command + check);

  // Forward command to the java part via jni
  return jniGpsCmd( cmd );

#endif
}

#if 0
/** This slot is called to reset the gps device to factory settings */
void GpsNmea::sendFactoryReset()
{
  if ( serial ) serial->sendSentence ("$PSRF104,0.0,0.0,0,0,0,0,12,8");
}


/** This slot is called to switch debugging mode on/off */
void GpsNmea::switchDebugging (bool on)
{
  QString cmd;
  cmd.sprintf ("$PSRF105,%d", on);
  if ( serial ) serial->sendSentence (cmd);
}

/**
 * Send the data of last valid fix to the gps receiver. Use the current utc
 * time as basis for calculating gps week and time of week. This function is
 * called only at initialization; we don't optimize to show the algorithm
 * clear.
 */
void GpsNmea::sendLastFix (bool hard, bool soft)
{
  switchDebugging (true);

  GeneralConfig *conf = GeneralConfig::instance();
  int ilat = conf->getGpsLastFixLat();
  int ilon = conf->getGpsLastFixLon();
  int alt  = conf->getGpsLastFixAlt();
  _lastClockOffset = conf->getGpsLastFixClk();

  int latdeg, latmin, latsec;
  int londeg, lonmin, lonsec;
  WGSPoint::calcPos (ilat, latdeg, latmin, latsec);
  WGSPoint::calcPos (ilon, londeg, lonmin, lonsec);
  double lat = latdeg + (double)latmin/60 + (double)latsec/3600;
  double lon = londeg + (double)lonmin/60 + (double)lonsec/3600;

  // get current time
  time_t currentTime;
  time (&currentTime);

  // gps weeks count from 0 to 1023, starting at Jan 5 1980.
  // the last gps week number 0 was Aug 22 1999
  // the next will be Apr 7 2019
  struct tm reftime;
  reftime.tm_year = 1999 - 1990; // unix years start at 1990
  reftime.tm_mon = 8 - 1;        // months start at 0
  reftime.tm_mday = 22;           // that's right - days start at 1
  reftime.tm_hour = 0;
  reftime.tm_min = 0;
  reftime.tm_sec = 0;
  reftime.tm_isdst = 0;          // no daylight saving time
  time_t reference = mktime (&reftime);
  time_t diff = currentTime - reference;
  const uint seconds_per_week = 60*60*24*7;
  // We don't expect this program to run after 2019, but you never know ...
  uint gps_week = (diff / seconds_per_week) % 1024;
  uint gps_seconds = diff % seconds_per_week;

#define SATS  12
#define HARD_RESET 4
#define SOFT_RESET 3

  QString cmd;
  // if we have no valid fix, do a hard reset
  if ((ilat == 0) && (ilon ==0) && hard)
    cmd.sprintf ("$PSRF104,%02.4f,%03.4f,%d,%d,%d,%d,%d,%d",
                 0.0,0.0,0,_lastClockOffset,gps_seconds,gps_week,SATS,HARD_RESET);
  else if (soft)
    cmd.sprintf ("$PSRF104,%02.4f,%03.4f,%d,%d,%d,%d,%d,%d",
                 lat,lon,alt,_lastClockOffset,gps_seconds,gps_week,SATS,SOFT_RESET);

  if (!cmd.isEmpty())
    if ( serial ) serial->sendSentence (cmd);
}
#endif


/** force a reset of the serial connection after a resume */
void GpsNmea::forceReset()
{
  qDebug("GpsNmea::forceReset()");

#ifndef ANDROID

  if ( serial )
    {
      serial->stopGpsReceiving();
      serial->startGpsReceiving();
      slot_reset();
    }

#endif

}

/**

This function calculates the checksum of the sentence.

NMEA-0183 Standard
The optional checksum field consists of a "*" and two hex digits
representing the exclusive OR of all characters between, but not
including, the "$" and "*".  A checksum is required on some sentences.
*/
uchar GpsNmea::calcCheckSum( const char *sentence )
{
  uchar sum = 0;

  for( uint i = 1; i < strlen( sentence ); i++ )
    {
      uchar c = (uchar) sentence[i];

      if( c == '$' || c == '!' ) // Start sign will not to be considered
        {
          continue;
        }

      if( c == '*' ) // End of sentence reached
        {
          break;
        }

      sum ^= c;
    }

  return sum;
}

/** This function calculates the STD altitude from the passed MSL altitude. */
void GpsNmea::calcStdAltitude(const Altitude& altitude)
{
  GeneralConfig *conf = GeneralConfig::instance();

  int qnhDiff = 1013 - conf->getQNH();

  if( qnhDiff != 0 )
    {
      // Calculate altitude correction in meters from pressure difference.
      // The common approach is to expect a pressure difference of 1 hPa per
      // 30ft until 18.000ft. 30ft are 9.1437m
      int delta = (int) rint( qnhDiff * 9.1437 );
      _lastStdAltitude.setMeters( altitude.getMeters() + delta );
    }
  else
    {
      _lastStdAltitude = altitude;
    }
}

/** This function calculates the MSL altitude from the passed STD altitude. */
void GpsNmea::calcMslAltitude(const Altitude& altitude)
{
  GeneralConfig *conf = GeneralConfig::instance();

  int qnhDiff = 1013 - conf->getQNH();

  if( qnhDiff != 0 )
    {
      // Calculate altitude correction in meters from pressure difference.
      // The common approach is to expect a pressure difference of 1 hPa per
      // 30ft until 18.000ft. 30ft are 9.1437m
      int delta = (int) rint( qnhDiff * 9.1437 );
      _lastMslAltitude.setMeters( altitude.getMeters() - delta );
    }
  else
    {
      _lastMslAltitude = altitude;
    }
}

/** write configuration data to allow restore of last fix */
void GpsNmea::writeConfig()
{
  if (_status == validFix)
    {
      GeneralConfig *conf = GeneralConfig::instance();
      conf->setGpsLastFixLat(_lastCoord.x());
      conf->setGpsLastFixLon(_lastCoord.y());
      conf->setGpsLastFixAlt( (int) rint(_lastMslAltitude.getMeters()) );
      conf->setGpsLastFixClk(_lastClockOffset);
      conf->save();
    }
}

/** Set system date/time. Input is UTC related. */
void GpsNmea::setSystemClock( const QDateTime& utcDt )
{
  if( !utcDt.isValid() )
    {
      return;
    }

  if( getuid() != 0 )
    {
      // we are not user root
      qWarning( "Only the superuser can set the system clock!" );
      return;
    }

  static char *noTZ;
  static char *utcTZ;
  noTZ = strdup( "TZ=" );
  utcTZ = strdup( "TZ=UTC" );

  // save current TZ
  char *curTZ = getenv( "TZ" );

  // set TZ to UTC
  putenv( utcTZ );
  tzset();

  struct tm tms;

  tms.tm_sec = utcDt.time().second();
  tms.tm_min = utcDt.time().minute();
  tms.tm_hour = utcDt.time().hour();
  tms.tm_mday = utcDt.date().day();
  tms.tm_mon = utcDt.date().month() - 1; // 0-11 instead of 1-12
  tms.tm_year = utcDt.date().year() - 1900; // year - 1900
  tms.tm_wday = -1;
  tms.tm_yday = -1;
  tms.tm_isdst = 0;

  time_t utcSeconds = mktime( &tms );

  struct timeval myTv;
  myTv.tv_sec = utcSeconds;
  myTv.tv_usec = 0;

  if( myTv.tv_sec != -1 )
    {
      settimeofday( &myTv, 0 );
    }

  timeOutFix->stop();

  // set hardware clock via hwclock tool, because the writeHWClock()
  // method will not do that in every case
  system( "/sbin/hwclock --systohc" );

  if( curTZ ) // restore old time zone
    {
      // decrement pointer to get the original string TZ=... getenv
      // returns only the assigned value.
      curTZ = curTZ - 3;
    }
  else
    {
      curTZ = noTZ;
    }

  putenv( curTZ ); // restore old settings
  tzset(); // reactivate saved time zone

  // restart timer to avoid fix losts
  timeOutFix->start( FIX_TO );
  // set flag to avoid reporting of connection lost
  _ignoreConnectionLost = true;

  _globalMapView->message( tr( "System clock synchronized" ) );

  free( noTZ );
  free( utcTZ );
}

/**
  GSV - Satellites in view

          1 2 3 4 5 6 7     n
          | | | | | | |     |
   $--GSV,x,x,x,x,x,x,x,...*hh<CR><LF>

   Field Number:
    1) total number of messages
    2) message number
    3) satellites in view
    4) satellite number
    5) elevation in degrees
    6) azimuth in degrees to true
    7) SNR in dB
    more satellite infos like 4)-7)
    n) checksum

  Extract Satellites In View (SIV) info from a NMEA sentence.
*/
void GpsNmea::__ExtractSatsInView(const QStringList& sentence)
{
  //the GPGSV sentence can be split between multiple sentences.
  //qDebug("expecting: %d, found: %s",cntSIVSentence,sentence[2].toLatin1().data());
  //check if we were expecting this part of the info
  if( cntSIVSentence != sentence[2].toUInt() )
    {
      return;
    }

  if( cntSIVSentence == 1 ) //this is the first sentence of our series
    {
      sivInfoInternal.clear();
    }

  // extract info on the individual sats
  __ExtractSatsInView( sentence[4], sentence[5], sentence[6], sentence[7] );

  if( sentence.count() > 11 )
    {
      __ExtractSatsInView( sentence[8], sentence[9], sentence[10], sentence[11] );
    }
  if( sentence.count() > 15 )
    {
      __ExtractSatsInView( sentence[12], sentence[13], sentence[14], sentence[15] );
    }
  if( sentence.count() > 19 )
    {
      __ExtractSatsInView( sentence[16], sentence[17], sentence[18], sentence[19] );
    }

  cntSIVSentence++;

  if( cntSIVSentence > sentence[1].toUInt() ) //this was the last sentence in our series
    {
      cntSIVSentence = 1;
      sivInfo = sivInfoInternal;
      emit newSatInViewInfo( sivInfo );
      //qDebug("triggered new sivi signal");
    }
}

/** Extract Satellites In View (SIV) info from a NMEA sentence. */
void GpsNmea::__ExtractSatsInView( const QString& id,
                                       const QString& elev,
                                       const QString& azimuth,
                                       const QString& snr)
{
  if( id.isEmpty() || elev.isEmpty() || azimuth.isEmpty() || snr.isEmpty() )
    {
      // ignore empty data
      return;
    }

  SIVInfo sivi;
  sivi.id = id.toInt();
  sivi.elevation = elev.toInt();
  sivi.azimuth = azimuth.toInt();

  if( snr.isEmpty() )
    {
      sivi.db = -1;
    }
  else
    {
      sivi.db = snr.toInt();
    }

  sivInfoInternal.append( sivi );
  //qDebug("new sivi info (snr: %d", sivi->db);
}

/**
 * called to open the NMEA log file.
 */
void GpsNmea::slot_openNmeaLogFile()
{
  QString fname = GeneralConfig::instance()->getUserDataDirectory() + "/CumulusNmea.log";

  if( nmeaLogFile == 0 )
    {
      QFileInfo fi(fname);

      if( fi.exists() && fi.size() > 0 )
        {
          QFile::remove( fname + ".old" );
          QFile::rename ( fname, fname + ".old" );
        }

      nmeaLogFile = new QFile(fname);
    }

  if( !nmeaLogFile->isOpen() )
    {
      bool ok = nmeaLogFile->open( QIODevice::WriteOnly | QIODevice::Text );

      if( ! ok )
        {
          qWarning() << "Cannot open file" << fname;
        }
    }
}

/**
 * called to close the NMEA log file.
 */
void GpsNmea::slot_closeNmeaLogFile()
{
  if( nmeaLogFile )
    {
      if( nmeaLogFile->isOpen() )
        {
          nmeaLogFile->close();
        }

      delete nmeaLogFile;
      nmeaLogFile = 0;
    }
}

#ifdef ANDROID

/**
 * Handler for custom QEvents posted by the native JNI functions in
 * jnisupport.cpp. GPS data handling duplicated from other NMEA functions.
 */
bool GpsNmea::event(QEvent *event)
  {
    if( ! _enableGpsDataProcessing )
      {
        return true;
      }

    // Handles NMEA sentences forwarded by the Android system
    if( event->type() == QEvent::User + 2 )
      {
        GpsNmeaEvent *gpsNmeaEvent = static_cast<GpsNmeaEvent *>(event);
        slot_sentence( gpsNmeaEvent->sentence() );
        emit newSentence( gpsNmeaEvent->sentence() );
        return true;
      }

    if( event->type() == QEvent::User )
      {
        static ulong called = 0;

        called++;

        if( called == 10 )
          {
            // After 10 calls we assume, that no MNEA row data are delivered by
            // Android. To enable wind calculation, we must reset the minimal
            // sat count. That is a bad hack -:(
            GeneralConfig::instance()->setMinSatCount( 0 );
          }

        // Report status change otherwise the glider symbol is not activated on the map.
        dataOK();
        fixOK( "ALC");

        QString msg = "$ANDROID,";

        GpsFixEvent *gpsFixEvent = static_cast<GpsFixEvent *>(event);

        // Handle altitude
        Altitude newAlt(0);
        double alt = gpsFixEvent->altitude();

        // Consider user's altitude correction
        newAlt.setMeters( alt + _userAltitudeCorrection.getMeters() );

        _lastGNSSAltitude = newAlt;

        if ( _lastMslAltitude != newAlt )
          {
            _lastMslAltitude = newAlt;
            calcStdAltitude( newAlt );
            emit newAltitude( _lastMslAltitude, _lastStdAltitude, _lastGNSSAltitude );
          }

        msg += newAlt.getText( true, 0 );

        // Handle position
        int lat = (int) rint(gpsFixEvent->latitude()  * 600000.0);
        int lon = (int) rint(gpsFixEvent->longitude() * 600000.0);

        QPoint newPoint (lat, lon);

        if ( _lastCoord != newPoint )
          {
            _lastCoord = newPoint;
            emit newPosition( _lastCoord );
          }

        msg += "," + QString::number( gpsFixEvent->latitude(), 'f' );
        msg += "," + QString::number( gpsFixEvent->longitude(), 'f' );

        // Handle speed
        Speed newSpeedFix;
        newSpeedFix.setMps( (double) gpsFixEvent->speed() );

        if ( newSpeedFix != _lastSpeed && fabs((newSpeedFix - _lastSpeed).getMps()) > 0.3 )
          {
            // report speed change only if the difference is greater than 0.3m/s, 1.08Km/h
            _lastSpeed = newSpeedFix;
            emit newSpeed( _lastSpeed );
          }

        msg += "," + newSpeedFix.getHorizontalText( true, 1 );

        // Handle heading
        double newHeadingFix = gpsFixEvent->heading();

        if ( newHeadingFix != _lastHeading )
          {
            _lastHeading = newHeadingFix;
            emit newHeading( _lastHeading );
          }

        msg += "," + QString::number( (int) rint(newHeadingFix) );

        // Handle time
        QDateTime fix_utc;
        fix_utc.setMSecsSinceEpoch( gpsFixEvent->time() );

        fix_utc = fix_utc.toUTC();

        if( fix_utc.time() != _lastRmcTime )
          {
            _lastRmcTime = fix_utc.time();
            emit newFix( _lastRmcTime );
          }

        msg += "," + fix_utc.toString(Qt::ISODate);

        // emits the new message that something is to see in GPS status widget
        emit newSentence( msg );

        if( nmeaLogFile && nmeaLogFile->isOpen() )
          {
            // Write message into log file
            nmeaLogFile->write(msg.toAscii().data());
          }

        // Handle accuracy
        return true;
      }

    // Handles reported status changes of the Android GPS receiver
    if( event->type() == QEvent::User + 1 )
      {
        GpsStatusEvent *gpsStatusEvent = static_cast<GpsStatusEvent *>(event);

        GpsStatus status = static_cast<GpsNmea::GpsStatus>(gpsStatusEvent->status());

        if( status == notConnected )
          {
            _slotGpsConnectionOff();
          }
        else
          {
            _slotGpsConnectionOn();
          }

        return true;
      }

  // call the default event processing
  return QObject::event(event);
}

#endif /* ANDROID */
