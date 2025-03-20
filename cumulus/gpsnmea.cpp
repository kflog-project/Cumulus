/***************************************************************************
    gpsnmea.cpp  -  Cumulus NMEA parser and decoder
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002      by André Somers,
                               2008-2025 by Axel Pauli

    email                : kflog.cumulus@gmail.com

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
 * receiver connected by RS232, USB, BT or WiFi.
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctime>
#include <cmath>

#include <QtCore>

#include "altitude.h"
#include "Atmosphere.h"
#include "generalconfig.h"
#include "gpsnmea.h"
#include "mapmatrix.h"
#include "mapcalc.h"
#include "mapview.h"
#include "speed.h"

#ifdef FLARM
#include "flarm.h"
#endif

// @AP: define timeout constants for GPS fix supervision in milli seconds.
//      After this time an alarm is generated.
#define FIX_TO 25000

// global GPS object pointer
GpsNmea *GpsNmea::gps = static_cast<GpsNmea *> (0);

// number of created class instances
short GpsNmea::instances = 0;

// Dictionary with known sentence keywords
QHash<QString, short> GpsNmea::gpsHash;

// Mutex for thread synchronization
QMutex GpsNmea::mutex;

// Flarm NMEAOUT initialization command for protocol version 9.
#define FLARM_NMEAOUT_INIT_CMD "$PFLAC,S,NMEAOUT,91"

// Flarm device type query
#define FLARM_DEVTYPE_CMD "$PFLAC,R,DEVTYPE"

GpsNmea::GpsNmea(QObject* parent) :
  QObject(parent),
  flarmNmeaOutInitDone(false)
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
  gpsDevice = GeneralConfig::instance()->getGpsDevice();

  // Connector to the GPS data provider
  connector = 0;

  /** BT Device name and address */
  btDevice = GeneralConfig::instance()->getGpsBtDevice();

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
  if ( connector )
    {
      delete connector;
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

  // Load all desired GPS sentence identifiers into the hash table NMEA Talkers
  // BD = Beidou Sat
  // GP = GPS Sat
  // GA = GALILEO Sat
  // GL = GLONASS Sat
  // GN = All GPS systems
  gpsKeys.insert( "$BDRMC", 0);
  gpsKeys.insert( "$GPRMC", 0);
  gpsKeys.insert( "$GARMC", 0);
  gpsKeys.insert( "$GLRMC", 0);
  gpsKeys.insert( "$GNRMC", 0);

  gpsKeys.insert( "$BDGLL", 1);
  gpsKeys.insert( "$GPGLL", 1);
  gpsKeys.insert( "$GAGLL", 1);
  gpsKeys.insert( "$GLGLL", 1);
  gpsKeys.insert( "$GNGLL", 1);

  gpsKeys.insert( "$BDGGA", 2);
  gpsKeys.insert( "$GPGGA", 2);
  gpsKeys.insert( "$GAGGA", 2);
  gpsKeys.insert( "$GLGGA", 2);
  gpsKeys.insert( "$GNGGA", 2);

  gpsKeys.insert( "$BDGSA", 3);
  gpsKeys.insert( "$GPGSA", 3);
  gpsKeys.insert( "$GAGSA", 3);
  gpsKeys.insert( "$GLGSA", 3);
  gpsKeys.insert( "$GNGSA", 3);

  gpsKeys.insert( "$BDGSV", 4);
  gpsKeys.insert( "$GPGSV", 4);
  gpsKeys.insert( "$GAGSV", 4);
  gpsKeys.insert( "$GLGSV", 4);
  gpsKeys.insert( "$GNGSV", 4);

  gpsKeys.insert( "$PGRMZ", 5);
  gpsKeys.insert( "$PCAID", 6);
  gpsKeys.insert( "!w",     7);
  gpsKeys.insert( "$PGCS",  8);
  gpsKeys.insert( "$LXWP0", 9);
  gpsKeys.insert( "$LXWP2", 10);
  gpsKeys.insert( "$GPDTM", 11);

  gpsKeys.insert( "$GNGNS", 12);

  // OpenVario
  gpsKeys.insert( "$POV", 13 );

  // XCVario
  gpsKeys.insert( "$PXCV", 14 );

  gpsKeys.insert( "$GPVTG", 15 );
  gpsKeys.insert( "$GNVTG", 15 );

  // Magnetic heading from XCVario
  gpsKeys.insert( "$HCHDM", 16 );

  // Magnetic true heading from XCVario
  gpsKeys.insert( "$HCHDT", 17 );

  #ifdef FLARM
  gpsKeys.insert( "$PFLAA", 20);
  gpsKeys.insert( "$PFLAU", 21);
  gpsKeys.insert( "$PFLAV", 22);
  gpsKeys.insert( "$PFLAE", 23);
  gpsKeys.insert( "$PFLAC", 24);
  gpsKeys.insert( "$PFLAR", 25);
  gpsKeys.insert( "$PFLAI", 26);
  gpsKeys.insert( "$PFLAO", 27);
  gpsKeys.insert( "$PFLAQ", 28);
  gpsKeys.insert( "$PFLAX", 29);
  gpsKeys.insert( "$ERROR", 30);
#endif

  // Only for test purposes
  gpsKeys.insert( "$PTAS", 42);

  gpsKeys.squeeze();
  mutex.unlock();
}

/** Resets all data objects to their initial values. This is called
 *  at startup, at restart and if the GPS fix has been lost. */
void GpsNmea::resetDataObjects()
{
  _status = notConnected;
  _lastGNSSAltitude = Altitude(0);
  _lastPressureAltitude = Altitude(0);
  _reportAltitude = true;
  _lastCoord = QPoint(0,0);
  _lastSpeed = Speed(-1.0);
  _lastHeading = -1.;
  _lastMagneticHeading = -1.;
  _lastMagneticTrueHeading = -1.;
  _lastTemperature = -300.0;
  _lastBugs = 0;
  _lastStaticPressure = -1.0;
  _lastDynamicPressure = -1.0;

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

  // altitude type (GPS or Baro) expected by the user
  _userExpectedAltitude = static_cast<GpsNmea::DeliveredAltitude> (GeneralConfig::instance()->getGpsAltitude());

  // GPS source to be used
  _gpsSource = GeneralConfig::instance()->getGpsSource().left(3);

  // Pressure device to be used.
  _pressureDevice = GeneralConfig::instance()->getPressureDevice();

  // special logger items
  _lastWindDirection = 0;
  _lastWindSpeed = Speed(0);
  _lastWindAge = 0;
  _lastQnh = 0;
  _lastVariometer = Speed(0);
  _lastMc = Speed(0);
  _lastTas = Speed(0);
  _lastUtc = QDateTime::currentDateTimeUtc();

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
  // qDebug("GpsNmea::createGpsConnection()");
  QObject *gpsObject = 0;

  // We create only a GpsCon instance. The GPS daemon process will be started
  // later.
  QString callPath = GeneralConfig::instance()->getAppRoot() + "/bin";

  connector = new GpsCon(this, callPath.toLatin1().data());

  gpsObject = connector;

  connect (gpsObject, SIGNAL(newSentence(const QString&)),
           this, SLOT(slot_sentence(const QString&)) );

  // Broadcasts the new NMEA sentence
  connect (gpsObject, SIGNAL(newSentence(const QString&)),
           this, SIGNAL(newSentence(const QString&)) );

  // Broadcasts that a new Flarm flight list is available
  connect (gpsObject, SIGNAL(newFlarmFlightList(const QString&)),
           this, SIGNAL(newFlarmFlightList(const QString&)) );

  // Broadcasts that a new Flarm flight download info is available
  connect (gpsObject, SIGNAL(newFlarmFlightDownloadInfo(const QString&)),
           this, SIGNAL(newFlarmFlightDownloadInfo(const QString&)) );

  // Broadcasts that a new Flarm flight download progress is available
  connect (gpsObject, SIGNAL(newFlarmFlightDownloadProgress(const int, const int)),
           this, SIGNAL(newFlarmFlightDownloadProgress(const int, const int)) );

  // The connection to the GPS receiver or daemon has been lost
  connect (gpsObject, SIGNAL(gpsConnectionOff()),
           this, SLOT( _slotGpsConnectionOff()) );

  // The connection to the GPS receiver or daemon has been established
  connect (gpsObject, SIGNAL(gpsConnectionOn()),
           this, SLOT( _slotGpsConnectionOn()) );

  connect (gpsObject, SIGNAL(gpsConnectionOn()),
           this, SLOT( _slotGpsConnectionOn()) );

  // Broadcasts an device error report from the GPS client
  connect (gpsObject, SIGNAL(deviceReport( const QString&, int ) ),
           this, SIGNAL( deviceReport( const QString&, int ) ) );
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

  if ( connector )
    {
      QSocketNotifier* notifier = static_cast<QSocketNotifier *> (0);

      notifier = connector->getDaemonNotifier();

      if( notifier )
        {
          notifier->setEnabled( enable );
        }
    }
}

/**
 * Starts the GPS client process and activates the GPS receiver.
 */
void GpsNmea::startGpsReceiver()
{
  if( GeneralConfig::instance()->getGpsSwitchState() == true && connector != 0 )
    {
      connector->startClientProcess();
    }
}

/**
 * This slot is called by the GpsCon object when a new sentence has
 * arrived from the GPS receiver. The argument contains the sentence to
 * analyze.
 */
void GpsNmea::slot_sentence( const QString& sentenceIn )
{
  if( flarmNmeaOutInitDone == false && sentenceIn.startsWith( "$GPRMC" ) )
    {
      flarmNmeaOutInitDone = true;

      // Send out the Flarm NMEAOUT initialization command, that
      // a connected Flarm device is working in the expected mode.
      // Note, it is not checked before, if the connected device
      // is a Flarm. That maybe cause trouble.
      sendSentence( FLARM_NMEAOUT_INIT_CMD );
      sleep(1);
      // Ask the Flarm device for its type.
      sendSentence( FLARM_DEVTYPE_CMD );
    }

  if( nmeaLogFile && nmeaLogFile->isOpen() )
    {
      // Write sentence into log file
      nmeaLogFile->write(sentenceIn.toLatin1().data());
    }

  if( sentenceIn.isEmpty() )
    {
      return;
    }

  if( QObject::signalsBlocked() )
    {
      // @AP: If the emitting of signals is blocked, we will ignore
      // this call. Otherwise this module can do internal state
      // changes, which will never distributed, e.g. Man->Gps mode
      // change. To ignore this causes fatal problems in Cumulus.
      return;
    }

  QString sentence;

  // Remove the checksum *hh<CR><LF> from the sentence. It was already checked
  // in the receiver method.
  int idx = sentenceIn.lastIndexOf( QChar('*') );

  if( idx != -1 )
    {
      sentence = sentenceIn.left( idx );
    }
  else
    {
      sentence = sentenceIn;
    }

  // Split sentence in single parts for each comma and the checksum. The first
  // part will contain the identifier, the rest the arguments.
  QStringList slst = sentence.split( ",", Qt::KeepEmptyParts );

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

  // If we receive NMEA sentences, we assume, that the Flarm works in text mode.
  // PowerFlarm changes after a timeout of 60s automatically back from the
  // binary mode into the text mode. That is stored in the Flarm class.
  FlarmBase::setProtocolMode( FlarmBase::text );

#endif

#if 0
//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

  if( slst[0] == "$GPRMC" )
    {
      /**
       *   1     2    3    4      5         6            7                8
        $PFLAU,<RX>,<TX>,<GPS>,<Power>,<AlarmLevel>,<RelativeBearing>,<AlarmType>,
           9                     10            11
        <RelativeVertical>,<RelativeDistance>,<ID>
      */
      //                  1   2 3 4 5 6 7  8   9   10    11
      QString pflau = "$PFLAU,9,1,2,1,2,20,4,-10,30,DD8451*";

      uint sum = calcCheckSum( pflau.toLatin1().data() );

      QString sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      static int i = 0;

      if( (++i % 15) == 0 )
        {
          slot_sentence( pflau + sumStr );
        }

      //-------------------------------------------------------------
      QString pflao = "$PFLAO,1,1,471122335,85577812,2000,100,4550,1432832400,DF4738,2,41*";

      sum = calcCheckSum( pflao.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      static int j = 0;

      if( (++j % 30) == 0 )
        {
      	  slot_sentence( pflao + sumStr );
      	}

      //-------------------------------------------------------------
      QString pflaa = "$PFLAA,3,0,50,-10,2,123456,20,,21,4.1,6*";

      sum = calcCheckSum( pflaa.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16, QChar('0') );

      slot_sentence( pflaa + sumStr );

      //---------------------------------------------------------------
      pflaa = "$PFLAA,2,0,-100,10,2,654321,20,,30,-0.2,1*";

      sum = calcCheckSum( pflaa.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      //-------------------------------------------------------------
      pflaa = "$PFLAA,0,-2000,2000,100,2,333333,90,,30,-0.3,1*";

      sum = calcCheckSum( pflaa.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      //---------------------------------------------------------------
      pflaa = "$PFLAA,0,-1547,69,444,2,444444,180,,30,1.4,1*";

      sum = calcCheckSum( pflaa.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,1,347,-1669,1555,2,555555,270,,30,1.5,1*";

      sum = calcCheckSum( pflaa.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,2,347,-69,2666,2,666666,66,,30,-1.6,1*";

      sum = calcCheckSum( pflaa.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,0,-2747,3669,-77,2,777777,359,,30,1.7,1*";

      sum = calcCheckSum( pflaa.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,0,-3,-5000,400,2,888888,199,,30,-1.8,1*";

      sum = calcCheckSum( pflaa.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );

      pflaa = "$PFLAA,0,4747,-2,999,2,999999,245,,30,2.9,1*";

      sum = calcCheckSum( pflaa.toLatin1().data() );

      sumStr = QString("%1").arg( sum, 2, 16,  QChar('0') );

      slot_sentence( pflaa + sumStr );
    }

//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
#endif

  // Call the decode methods for the known sentences
  switch( gpsHash.value( slst[0] ) )
  {
    case 0: // GPRMC
      if( slst[0].startsWith(_gpsSource) )
          {
            __ExtractGprmc( slst );
          }
      return;

    case 1: // GPGLL
      if( slst[0].startsWith(_gpsSource) )
          {
            __ExtractGpgll( slst );
          }
      return;

    case 2: // GPGGA
      if( slst[0].startsWith(_gpsSource) )
          {
            __ExtractGpgga( slst );
          }
      return;

    case 3: // GPGSA
      if( slst[0].startsWith(_gpsSource) )
          {
            __ExtractConstellation( slst );
          }
      return;

    case 4: // GPGSV or GLGSV
      // __ExtractSatsInView( slst );
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

    case 12: // $GNGNS
      if( slst[0].startsWith(_gpsSource) )
          {
            __ExtractGngns( slst );
          }
      return;

    case 13: // $POV,  OpenVario
      __ExtractPov( slst );
      return;

    case 14: // $PXCV, XCVario
      __ExtractPxcv( slst );
      return;

    case 15: // $GPVTG, $GNVTG
      __ExtractGpvtg( slst );
      return;

    case 16: // $HCHDM compass magnetic heading
      __ExtractHchdm( slst );
      return;

    case 17: // $HCHDT compass true magnetic heading
      __ExtractHchdt( slst );
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

    case 25: // $PFLAR
      Flarm::instance()->extractPflar( slst );
      return;

    case 26: // $PFLAI
      Flarm::instance()->extractPflai( slst );
      return;

    case 27: // $PFLAO
      Flarm::instance()->extractPflao( slst );
      return;

    case 28: // $PFLAQ
      Flarm::instance()->extractPflaq( slst );
      return;

    case 29: // $PFLAX
      Flarm::instance()->extractPflax( slst );
      return;

    case 30: // $ERROR
      Flarm::instance()->extractError( slst );
      return;

#endif

    case 42:
       // // Only for test purposes for simulating TAS in km/h
       __ExtractTas( slst );
       return;

    default:

      qWarning() << "Unknown GPS sentence:" << sentenceIn;
      return;
  }
}

/**

  See http://www.nmea.de/nmea0183datensaetze.html

  [GP|GN]RMC - Recommended Minimum Navigation Information

                                                               12
          1         2 3       4 5        6 7   8   9     10  11 | 13
          |         | |       | |        | |   |   |      |   | | |
   $--RMC,hhmmss.sss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a,a*hh<CR><LF>

   $GPRMC,132217.000,A,5228.19856,N,01408.32249,E,47.100,267.38,300710,,,A*58

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
  if( slst.size() < 13 )
    {
      // Checksum has to be ignored in counting.
      qWarning() << slst[0] << "contains too less parameters!";
      return;
    }

  _gprmcSeen = true;

  if( slst[2] == "A" )
    { /* Data status A=OK, V=warning */
      fixOK( "RMC" );

      static QTime lastUtcTime;

      QTime utcTime = __ExtractTime(slst[1]);

      if( lastUtcTime == utcTime )
        {
          // Only every second the RMC sentence is processed.
          return;
        }

      lastUtcTime = utcTime;

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

          if( _lastRmcUtc != utc )
            {
              /**
               * The fix date and time have been changed and that is reported now.
               * We do check the fix time only here in the $GPRMC sentence.
               */
              _lastRmcUtc = utc;
              emit newFix( _lastRmcUtc );
            }
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
  [GP|GN]GLL - Geographic Position - Latitude/Longitude

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
  if( slst.size() < 7 )
    {
      // Checksum has to be ignored in counting.
      qWarning() << slst[0] << "contains too less parameters!";
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
      // Checksum has to be ignored in counting.
      qWarning() << slst[0] << "contains too less parameters!";
      return;
    }

  if ( slst[6] != "0" && ! slst[6].isEmpty() )
    {
      /* a value of 0 means invalid fix and we don't need that one */
      if( _gprmcSeen == false )
        {
          // Report fix info only, if GPRMC was not received. I saw
          // sometimes different reportings. GGA said OK, RMC said NOK.
          fixOK( "GGA" );
        }

      static QTime lastUtcTime;

      QTime utcTime = __ExtractTime(slst[1]);

      if( lastUtcTime == utcTime )
        {
          // Only every second the GGA sentence is processed.
          return;
        }

      lastUtcTime = utcTime;

      __ExtractCoord(slst[2], slst[3], slst[4], slst[5]);
      __ExtractAltitude(slst[9], slst[10]);
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
       1        2           3 4            5 6  7  8   9    10  11 12 13
       |        |           | |            | |  |  |   |    |    | |  |
$GNGNS,124524.0,5227.153706,N,01335.903142,E,AA,15,0.7,37.5,43.0, ,  *54<CR><LF>

   Field Number:
    1) Universal Time Coordinated (UTC) of position fix, hhmmss format
    2) Latitude, ddmm.mmmm format (leading zeros will be transmitted)
    3) Latitude hemisphere, N or S
    4) Longitude, dddmm.mmmm format (leading zeros will be transmitted)
    5) Longitude hemisphere, E or W
    6) Mode indicator per GPS System to be processed
       N = No fix
       A = Autonomous mode
       D = Differential mode
       P = Precise mode is used to compute position fix
       R = Real Time Kinematic
       F = Float RTKE = Estimated (dead reckoning) mode
       M = Manual input mode
       S = Simulator mode.
    7) total number of satellites in use, 00 - 99
    8) Horizontal Dilution of precision 0.5...99.9
    9) Antenna Altitude above/below mean-sea-level (geoid)
   10) Geoidal separation, the difference between the WGS-84 earth
       ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level
       below ellipsoid
   11) Age of differential GPS data, time in seconds since last SC104
       type 1 or 9 update, null field when DGPS is not used
   12) Differential reference station ID, 0000-1023
   13) Checksum
 */
void GpsNmea::__ExtractGngns( const QStringList& slst )
{
  if( slst.size() < 13 )
    {
      // Checksum has to be ignored in counting.
      qWarning() << slst[0] << "contains too less parameters!";
      return;
    }

  if( slst[6].contains( "N") == false )
    {
      static QTime lastUtcTime;

      QTime utcTime = __ExtractTime(slst[1]);

      if( lastUtcTime == utcTime )
        {
          // Only every second the GNS sentence is processed.
          return;
        }

      lastUtcTime = utcTime;

      __ExtractCoord(slst[2], slst[3], slst[4], slst[5]);
      __ExtractAltitude(slst[9], QString("M"));
      __ExtractSatsInView(slst[7]);
    }
}

/**
Track made good and speed over ground

$GPVTG,147.6,T,147.6,M,2.5,N,4.7,K,D*22

0 $GPVTG, Message ID
1 Track made good (degrees true)
2 T: track made good is relative to true north
3 Track made good (degrees magnetic)
4 M: track made good is relative to magnetic north
5 Speed Kn, Speed over ground, knots
6 N: speed is measured in knots
7 Speed over ground in kilometers/hour (kph)
8 K: speed over ground is measured in kph
9 mode ind, Positioning system mode indicator,
   see Table: NMEA Positioning System Mode Indicator
10 *xx, Check sum
11 [CR][LF], Sentence terminator

NMEA Positioning System Mode Indicator
Mode Indicator

A    Autonomous
D    Differential
E    Estimated (dead reckoning) mode
M    Manual input
N    Data not valid

*/
void GpsNmea::__ExtractGpvtg( const QStringList& slst )
{
  if ( slst.size() < 9 )
    {
      // Checksum has to be ignored in counting.
      qWarning("$PGVTG contains too less parameters!");
      return;
    }

  if( slst[2] == "T" )
    {
      __ExtractHeading( slst[1] );
    }

  if( slst[4] == "M" )
    {
      bool ok = false;

      double mh = slst[3].toDouble( &ok );

      if( ok == true )
        {
          _lastMagneticHeading = mh;
          emit newMagneticHeading( mh );
        }
    }

  if( slst[6] == "N" )
    {
      __ExtractKnotSpeed( slst[5] );
    }
}

/**
 * Compass sentence
 *
 * $HCHDM,238.5,M*xx<CR><LF>
 *
 * 0 HCHDM
 * 1 Heading Degrees, magnetic
 * 2 M, Magnetic track indicator
 * 3 *xx Check sum
 * 4 [CR][LF], Sentence terminator
 *
 * Extracts HCHDM sentence, magnetic compass with magnetic heading
 * message.
 */
void GpsNmea::__ExtractHchdm( const QStringList& slst )
{
  if ( slst.size() < 3 )
    {
      // Checksum has to be ignored in counting.
      qWarning("HCHDM contains too less parameters!");
      return;
    }

  bool ok = false;

  if( slst[2] != "M" )
    {
      // No magnetic heading, ignore it
      return;
    }

  double mh = slst[1].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  _lastMagneticHeading = mh;
  emit newMagneticHeading( mh );
}

/**
 * Compass sentence
 *
 * $HCHDT,238.5,T*xx<CR><LF>
 *
 * 0 HCHDT
 * 1 Heading Degrees, true
 * 2 T, True track indicator
 * 3 *xx Check sum
 * 4 [CR][LF], Sentence terminator
 *
 * Extracts HCHDM sentence, magnetic compass with magnetic heading
 * message.
 */
void GpsNmea::__ExtractHchdt( const QStringList& slst )
{
  if ( slst.size() < 3 )
    {
      // Checksum has to be ignored in counting.
      qWarning("HCHDT contains too less parameters!");
      return;
    }

  bool ok = false;

  if( slst[2] != "T" )
    {
      // No true magnetic heading, ignore it
      return;
    }

  double mh = slst[1].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  _lastMagneticTrueHeading = mh;
  emit newMagneticTrueHeading( mh );
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
      // Checksum has to be ignored in counting.
      qWarning("$PGRMZ contains too less parameters!");
      return;
    }

  /*
   * Garmin or Flarm proprietary sentence with pressure altitude information.
   * Only considered, if pressure device is Garmin or FLARM.
   */
  if ( slst[3] == "2" &&
      (_pressureDevice == "Garmin" || _pressureDevice == "Flarm") )
    {
      bool ok;
      double num = slst[1].toDouble( &ok );

      if( ok )
        {
          Altitude altitude;
          altitude.setFeet( num );

          if( _lastPressureAltitude != altitude || _reportAltitude == true )
            {
              _reportAltitude = false;
              _baroAltitudeSeen = true;

              // If we have pressure altitude, the barometer sensor is
              // normally calibrated to 1013.25hPa and that is the standard
              // pressure altitude.
              _lastPressureAltitude = altitude;

              // report new pressure altitude
              emit newPressureAltitude( _lastPressureAltitude );
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
  Used by XC and OpenVario devices:

  https://www.openvario.org/doku.php?id=projects:series_00:software:nmea

  $POV,P,<baro>,Q,<dp>,E,<te>,T,<temp>“

  <baro>: static pressure in hPa, Example: 1018.35

  <dp>: dynamic pressure in Pa, Example: 23.3

  <te>: TE vario in m/s, Example: 2.15

  <temp>: temperature in deg C, Example: 23.52

  $POV,P,+949.30,Q,24.57*7D
 */
void GpsNmea::__ExtractPov( const QStringList& slst )
{
  if ( slst.size() < 3 )
    {
      qWarning("$POV contains too less parameters!");
      return;
    }

  if( _pressureDevice != "OpenVario" )
    {
      // OpenVario is not selected as pressure device.
      return;
    }

  for( int i=1; i < slst.size() - 1; i += 2 )
    {
      if( slst[i] == "P" )
        {
          // P: static pressure in hPa
          // Example: $POV,P,1018.35*39
          bool ok = false;
          double p = slst[i+1].toDouble( &ok );

          if( ok == false )
            {
              // abort all in error case
              return;
            }

          if( _lastStaticPressure != p )
            {
              _lastStaticPressure = p;
              emit newStaticPressure( p );
            }

          Altitude altitude( Atmosphere::calcAltitude( p ) );

          if( _lastPressureAltitude != altitude || _reportAltitude == true )
            {
              _baroAltitudeSeen = true;
              _reportAltitude = false;

              // If we have pressure altitude, the barometer sensor is
              // normally calibrated to 1013.25hPa and that is the standard
              // pressure altitude.
              _lastPressureAltitude = altitude;

              // report new pressure altitude
              emit newPressureAltitude( _lastPressureAltitude );
            }
        }
      else if( slst[i] == "Q" )
        {
          // Q: dynamic pressure in Pa, Example: 23.3
          // Example: $POV,Q,23.3*39
          bool ok = false;
          double q = slst[i+1].toDouble( &ok );

          if( ok == false )
            {
              // abort all in error case
              return;
            }

          if( _lastDynamicPressure != q )
            {
              _lastDynamicPressure = q;
              emit newDynamicPressure( q );
            }
        }
      else if( slst[i] == "E" )
        {
          // E: TE vario in m/s
          // Example: $POV,E,2.15*14
          bool ok = false;
          double vs = slst[i+1].toDouble( &ok );

          if( ok == false )
            {
              // abort all in error case
              return;
            }

          Speed speed( vs );

          if ( _lastVariometer != speed )
            {
              _lastVariometer = speed;
              emit newVario( _lastVariometer ); // notify change
            }
        }
      else if( slst[i] == "S" )
        {
          // S: true airspeed in km/h
          // Example: $POV,S,123.45*05
          bool ok = false;
          double tas = slst[i+1].toDouble( &ok );

          if( ok == false )
            {
              // abort all in error case
              return;
            }

          Speed speed;
          speed.setKph( tas );

          if( _lastTas != speed )
            {
              _lastTas = speed;
              emit newTas( _lastTas );
            }
        }
      else if( slst[i] == "T" )
        {
          // T: temperature in deg C
          // Example: $POV,T,23.52*35
          bool ok = false;
          double temperature = slst[i+1].toDouble( &ok );

          if( ok == false )
            {
              // abort all in error case
              return;
            }

          if( _lastTemperature != temperature )
            {
              _lastTemperature = temperature;
              emit newTemperature(_lastTemperature );
            }
        }
    }
}

/**
  Used by XCVario devices:

  The sentence has following format:

   0 - $PXCV,
   1 - BBB.B, ​​ // Vario, -30 to +30 m/s, negative sign for sink
   2 - C.C,  ​​ ​​​​ // MacCready 0 to 10 m/s
   3 - EE,  ​​ ​​ ​​​​ // Bugs degradation, 0 = clean to 30 %
   4 - F.FF,  ​​​​ // Ballast 1.00 to 1.60
   5 - G,  ​​ ​​ ​​ ​​​​ // 0 in climb, 1 in cruise
   6 - HH.H,  ​​​​ // Outside airtemp in degrees celcius ( may have leading negative sign )
   7 - QQQQ.Q, // QNH e.g. 1013.2
   8 - PPPP.P, // Static pressure in hPa
   9 - QQQQ.Q, // Dynamic pressure in Pa
  10 - RRR.R, ​​ // Roll angle
  11 - III.I, ​​ // Pitch angle
  12 - X.XX,  ​​​​ // Acceleration in X-Axis
  13 - Y.YY,  ​​​​ // Acceleration in Y-Axis
  14 - Z.ZZ,  ​​​​ // Acceleration in Z-Axis
  15   *CHK = standard NMEA checksum
       <CR><LF>

  Note! Items 10, 11, 12, 13, 14 are empty, if the AHRS feature is not switched
  on by a valid license key.

  Example:

  $PXCV,0.0,1.0,0,1.00,0,20.9,1024.5,1016.4, 0.0,0.3,20.4,0.41,0.01,1.09*3F
*/
void GpsNmea::__ExtractPxcv( const QStringList& slst )
{
  if ( slst.size() < 15 )
    {
      // Checksum has to be ignored in counting.
      qWarning("$PXCV contains too less parameters!");
      return;
    }

  if( _pressureDevice != "XCVario" )
    {
      // XCVario is not selected as pressure device.
      return;
    }

  bool ok = false;

  // 1 - BBB.B, ​​Vario, -30 to +30 m/s, negative sign for sink
  double vs = slst[1].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  Speed vSpeed( vs );

  if ( _lastVariometer != vSpeed )
    {
      _lastVariometer = vSpeed;
      emit newVario( _lastVariometer ); // notify change
    }

  // 2 - C.C, MacCready 0 to 10 m/s
  double mc = slst[2].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  Speed mcSpeed( mc );

  if ( _lastMc != mcSpeed )
    {
      _lastMc = mcSpeed;
      emit newMc( _lastMc ); // notify change
    }

  // 3 - EE, Bugs degradation, 0 = clean to 30 %
  unsigned short bugs = slst[3].toUShort( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  if ( bugs <= 100 && _lastBugs != bugs )
    {
      _lastBugs = bugs;
      emit newBugs( _lastBugs ); // notify change
    }

  // 6 - HH.H, Outside airtemp in degrees celcius ( may have leading negative sign )
  double temperature = slst[6].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  if( _lastTemperature != temperature )
    {
      _lastTemperature = temperature;
      emit newTemperature(_lastTemperature );
    }

  // 8 - PPPP.P, Static pressure in hPa
  double p = slst[8].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  if( _lastStaticPressure != p )
    {
      _lastStaticPressure = p;
      emit newStaticPressure( p );
    }

  Altitude altitude( Altitude::altitudeFromPressure( p ) );

  if( _lastPressureAltitude != altitude || _reportAltitude == true )
    {
      _baroAltitudeSeen = true;
      _reportAltitude = false;

      // If we have pressure altitude, the barometer sensor is
      // normally calibrated to 1013.25hPa and that is the standard
      // pressure altitude.
      _lastPressureAltitude = altitude;

      // report new pressure altitude
      emit newPressureAltitude( _lastPressureAltitude );
    }

  // 9 - QQQQ.Q, Dynamic pressure in Pa
  double q = slst[9].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  if( _lastDynamicPressure != q )
    {
      _lastDynamicPressure = q;
      emit newDynamicPressure( q );
    }

  for( int i = 10; i <= 14; i++ )
    {
      // Check, if AHRS data is available. If items are missing don't process
      // AHRS data.
      if( slst[i].isEmpty() == true )
        {
          return;
        }
    }

  // 10 - RRR.R, ​​Roll angle
  double ra = slst[10].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  // 11 - III.I, Pitch angle
  double pa = slst[11].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  // 12 - X.XX, Acceleration in X-Axis
  double ax = slst[12].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  // 13 - Y.YY,  ​​​Acceleration in Y-Axis
  double ay = slst[13].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  // 14 - Z.ZZ, Acceleration in Z-Axis
  double az = slst[14].toDouble( &ok );

  if( ok == false )
    {
      // abort all in error case
      return;
    }

  emit newAHRSInfo( ra, pa, ax, ay, az );
}

/**
  Used by Cambridge devices. The PCAID sentence format is:

  $PCAID,<1>,<2>,<3>,<4>*hh<CR><LF>
  $PCAID,N,-00084,000,128*0B

  <1>     Logged 'L' Last point Logged 'N' Last Point not logged
  <2>     Barometer Altitude in meters (Leading zeros will be transmitted), Hendrik said: STD
  <3>     Engine Noise Level
  <4>     Log Flags
  <5>     *hh Checksum, XOR of all bytes of the sentence after the `$' and before the '!'
*/
void GpsNmea::__ExtractPcaid( const QStringList& slst )
{
  if ( slst.size() < 5 )
    {
      // Checksum has to be ignored in counting.
      qWarning("$PCAID contains too less parameters!");
      return;
    }

  if( _pressureDevice != "Cambridge" )
    {
      // Cambridge is not selected as pressure device.
      return;
    }

  Altitude res(0);
  double num = slst[2].toDouble();
  res.setMeters( num );

  if (_lastPressureAltitude != res || _reportAltitude == true )
    {
      _reportAltitude = false;
      _baroAltitudeSeen = true;
      _lastPressureAltitude = res;
      // This altitude must not be notified as new value because
      // the Cambridge device delivers also MSL in its !w sentence.
      // report new pressure altitude
      // emit newPressureAltitude( _lastPressureAltitude );
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
      // Checksum has to be ignored in counting.
      qWarning("$PGCS contains too less parameters!");
      return;
    }

  if( _pressureDevice != "Volkslogger" )
    {
      // Volkslogger is not selected as pressure device.
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

  _baroAltitudeSeen = true;

  if (num > 32768)
    {
      num -= 65536;  // FFFF = -1, FFFE = -2, etc.
    }

  res.setMeters( num );

  if( _lastPressureAltitude != res || _reportAltitude == true )
    {
      _reportAltitude = false;

      // Store this value as pressure altitude.
      _lastPressureAltitude = res;

      emit newPressureAltitude( _lastPressureAltitude );
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

      static QElapsedTimer lastReporting;

      if( lastReporting.isValid() == false || lastReporting.elapsed() >= 5000 )
        {
          // To reduce load, Flarm count is reported only after 5s.
          // We do send always a new state independently of a change
          // in the mean time.
          lastReporting.start();

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
      // Checksum has to be ignored in counting.
      qWarning("$GPDTM contains too less parameters!");
      return;
    }

  _mapDatum = slst[8];
}

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
      // Checksum has to be ignored in counting.
      qWarning("!w contains too less parameters!");
      return;
    }

  if( _pressureDevice != "Cambridge" )
    {
      // Cambridge is not selected as pressure device.
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
        _baroAltitudeSeen = true;
        _reportAltitude = false;
        _lastPressureAltitude = res; // store the new pressure altitude

        emit newPressureAltitude( _lastPressureAltitude );
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
      // Checksum has to be ignored in counting.
      qWarning("$LXWP0 contains too less parameters!");
      return;
    }

  if( _pressureDevice != "LX" )
    {
      // LX is not selected as pressure device.
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

              // store the new pressure altitude
              _lastPressureAltitude = altitude;

              emit newPressureAltitude( _lastPressureAltitude );
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
      // Checksum has to be ignored in counting.
      qWarning("$LXWP2 contains too less parameters!");
      return;
    }

  if( _pressureDevice != "LX" )
    {
      // LX is not selected as pressure device.
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

  // extract bugs in percent
  if( ! stringList[2].isEmpty() )
    {
      unsigned short bugs = stringList[2].toUShort( &ok );

      if ( ok && bugs <= 100 && _lastBugs != bugs )
        {
          _lastBugs = bugs;
          emit newBugs( _lastBugs ); // notify change
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
  _lastSpeed = res;

  emit newSpeed( _lastSpeed );
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

  lon  = slon.left(3).toInt(&ok3);
  fLon = slon.mid(3).toFloat(&ok4);

  if( !ok1 || !ok2 || !ok3 || !ok4 )
    {
      return QPoint();
    }

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

  _lastHeading = heading;
  emit newHeading( _lastHeading );

  return heading;
}

/**
 * Extract proprietary test sentence $PTAS,<TAS>*
 * TAS in km/h
 */
void GpsNmea::__ExtractTas( const QStringList& data )
{
  if( data.isEmpty() or data.size() < 2 )
    {
      qWarning("$PTAS contains too less parameters!");
      return;
    }

  bool ok;

  double tas = data[1].toDouble(&ok);

  if( ok == false )
    {
      return;
    }

  Speed speed;
  speed.setKph( tas );

   _lastTas = speed;
   emit newTas( _lastTas );
}

/**
 * Extracts the altitude from a NMEA GGA sentence.
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
      res.setFeet( alt );
    }
  else
    {
      res.setMeters( alt );
    }

  if( _lastGNSSAltitude != res )
    {
      _lastGNSSAltitude = res;
      emit newGNSSAltitude( _lastGNSSAltitude );
    }

  return res;
}

/**
  [GP|GN]GSA - GPS DOP and active satellites

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
      // Checksum has to be ignored in counting.
      qWarning() << sentence[0] << "contains too less parameters!";
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
          tmp = QString( "%1" ).arg( sentence[i], 2, QChar('0') );
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
      _lastGNSSAltitude = Altitude(0);
      _lastPressureAltitude = Altitude(0);

      emit newGNSSAltitude( _lastGNSSAltitude );
      emit newPressureAltitude( _lastPressureAltitude );

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
 * Called, if the user wants to switch on/off the GPS data processing.
 */
void GpsNmea::slot_userGpsSwitchRequest()
{
  bool gpsSwitchState = GeneralConfig::instance()->getGpsSwitchState();

  // qDebug() << "GpsNmea::slot_userGpsSwitchRequest()" << gpsSwitchState;

  if( connector != 0 )
    {
      if( gpsSwitchState == true )
        {
          connector->startGpsReceiving();
        }
      else
        {
          connector->stopGpsReceiving();

          // Report connection stop after 3s to garantee that all GPS
          // data have been read from the queue to avoid a reconnect.
          // The report changes to GPS ststus to manuelly.
          QTimer::singleShot( 3000, this, SLOT( _slotGpsReportConnectionOff() ) );
        }
    }
}

/**
 * Called to report a switched off GPS connection by the user.
 */
void GpsNmea::_slotGpsReportConnectionOff()
{
  resetDataObjects();
  emit statusChange( _status );
  emit deviceReport( tr( "GPS connection switched off by user" ), 5000 );
  qDebug() << "User has switched off GPS connection";
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
  _userExpectedAltitude = static_cast<GpsNmea::DeliveredAltitude> (conf->getGpsAltitude());
  _reportAltitude = true;
  flarmNmeaOutInitDone = false;
  _gpsSource = conf->getGpsSource().left(3);

  bool gpsSwitchState = conf->getGpsSwitchState();
  QString oldDevice = gpsDevice;

  if( gpsDevice != conf->getGpsDevice() )
    {
      // qDebug() << "slot_reset(): GPS Device changed";
      // GPS device has been changed by the user
      gpsDevice = conf->getGpsDevice();

      if( connector )
        {
          connector->stopGpsReceiving();
          delete connector;
          connector = 0;
        }

      // reset data objects
      resetDataObjects();
      // create a new connection
      createGpsConnection();
      // start the new connection
      startGpsReceiver();
      return;
    }

  if( connector != 0 )
    {
      // No device modification
      if( gpsDevice.startsWith( "/dev/tty" ) )
        {
          // check serials baud rate
          if( connector->currentBautrate() != conf->getGpsSpeed() )
            {
              // restart connector
              connector->stopGpsReceiving();

              if( gpsSwitchState == true )
                {
                  connector->startGpsReceiving();
                }
            }
        }
      else if( gpsDevice.startsWith( "WiFi" ) )
        {
          // Check, if WiFi data have been changed.
          if( wifi_1_Ip != conf->getGpsWlanIp1() ||
              wifi_1_Port != conf->getGpsWlanPort1() ||
              wifi_1_active != conf->getGpsWlanCB1() ||
              wifi_2_Ip != conf->getGpsWlanIp2() ||
              wifi_2_Port != conf->getGpsWlanPort2() ||
              wifi_2_active != conf->getGpsWlanCB2())
            {
              // Take over new WiFi data from configuration
              wifi_1_Ip = conf->getGpsWlanIp1();
              wifi_1_Port = conf->getGpsWlanPort1();
              wifi_1_active = conf->getGpsWlanCB1();
              wifi_2_Ip = conf->getGpsWlanIp2();
              wifi_2_Port = conf->getGpsWlanPort2();
              wifi_2_active = conf->getGpsWlanCB2();

              // restart connector
              connector->stopGpsReceiving();

              if( gpsSwitchState == true )
                {
                  connector->startGpsReceiving();
                }
            }
        }

#ifdef BLUEZ
     else if( gpsDevice == BT_ADAPTER )
        {
         // Bluetooth adapter is selected, check if the device was changed.
         QPair<QString, QString> confDevice =
             conf->getGpsBtDevice();

         if( btDevice != confDevice )
           {
             // the user has changed the device
             btDevice = confDevice;

             // restart connector
             connector->stopGpsReceiving();

             if( gpsSwitchState == true )
               {
                 connector->startGpsReceiving();
               }
           }
        }
#endif
    }
}

bool GpsNmea::sendSentence(const QString command)
{
  if( connector )
    {
      // Only a connector can forward commands to the GPS.
      return connector->sendSentence( command );
    }

  return false;
}

//------------------------------------------------------------------------------

#ifdef FLARM

/** Requests a flight list from a Flarm device. */
bool GpsNmea::getFlarmFlightList()
{
  if( connector )
    {
      // No GPS data are available, if flights are downloaded from Flarm.
      // The timer must be stopped to prevent TO actions.
      timeOutFix->stop();

      // Only a connector can request a Flarm fight list.
      return connector->getFlarmFlightList();
    }

  return false;
}

bool GpsNmea::getFlarmIgcFiles( QString& flightData )
{
  if( connector )
    {
      // No GPS data are available, if flights are downloaded from Flarm.
      // The timer must be stopped to prevent TO actions.
      timeOutFix->stop();

      /// Only a connector can request a Flarm file download.
      return connector->getFlarmIgcFiles( flightData );
    }

  return false;
}

bool GpsNmea::flarmReset()
{
  if( connector )
    {
      // No GPS data are available, if flights are downloaded from Flarm.
      // The timer must be stopped to prevent TO actions.
      timeOutFix->stop();

      /// Only a connector can request a Flarm reset.
      return connector->flarmReset();
    }

  return false;
}

#endif // FLARM

//------------------------------------------------------------------------------

/** force a reset of the connector connection after a resume */
void GpsNmea::forceReset()
{
  qDebug("GpsNmea::forceReset()");

  if( connector )
    {
      connector->stopGpsReceiving();

      if( GeneralConfig::instance()->getGpsSwitchState() )
        {
          connector->startGpsReceiving();
        }

      slot_reset();
    }
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

  emit deviceReport( tr( "System clock synchronized" ), 3000 );

  free( noTZ );
  free( utcTZ );
}

/**
  GPGSV - GPS Satellites in view
  GLGSV - GLONASS Satellites in view

          1 2 3 4 5 6 7     n
          | | | | | | |     |
   $GPGSV,x,x,x,x,x,x,x,...*hh<CR><LF>
   $GLGSV,x,x,x,x,x,x,x,...*hh<CR><LF>

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
  if( sentence.size() < 8 )
    {
      // Checksum has to be ignored in counting.
      qWarning( "$GPGSV contains too less parameters!" );
      return;
    }

  // The GPGSV sentence can be split into multiple sentences.
  // qDebug("expecting: %d, found: %s",cntSIVSentence,sentence[2].toLatin1().data());
  // Check if we were expecting this part of the info.
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
                                   const QString& snr )
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
  sivi.db = snr.toInt();

  sivInfoInternal.append( sivi );
  //qDebug("new sivi info (snr: %d", sivi->db);
}

/**
 * called to open the NMEA log file.
 */
void GpsNmea::slot_openNmeaLogFile()
{
  QString logStart = QDateTime::currentDateTime().toString( Qt::ISODate );
  QString fname = GeneralConfig::instance()->getUserDataDirectory() +
                  "/CumulusNmea_" + logStart + ".log";

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
