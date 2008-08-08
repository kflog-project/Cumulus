/***************************************************************************
    gpsnmea.cpp  -  Cumulus NMEA interpreter managing connection to the GPS
                    receiver resp. daemon process.
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by André Somers, 2008 by Axel Pauli
    email                : axel@kflog.org

    $Id$

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <cmath>

#include <QStringList>

#include "gpsnmea.h"
#include "mapmatrix.h"
#include "mapcalc.h"
#include "mapview.h"

#include "generalconfig.h"

// @AP: define timeout constants for gps fix supervision in milli seconds.
//      After this time an alarm is generated.

#define FIX_TO 25000

extern MapView   *_globalMapView;
extern MapMatrix *_globalMapMatrix;

// global gps pointer
GPSNMEA* gps = 0;

GPSNMEA::GPSNMEA(QObject* parent) : QObject(parent)
{
  // qDebug("GPSNMEA::GPSNMEA()");
  
  _status=notConnected;
  timeOutFix=new QTimer(this);
  connect (timeOutFix,SIGNAL(timeout()),this,SLOT(_slotTimeoutFix()));
  timeOutFix->start(FIX_TO); // fire every two-and-a-halfe seconds

  _lastMslAltitude=Altitude(0);
  _lastGNSSAltitude=Altitude(0);
  _lastStdAltitude=Altitude(0);
  _lastPressureAltitude=Altitude(0);
  _lastCoord=_globalMapMatrix->getMapCenter(false);
  _lastSpeed=Speed(-1.0);
  _lastHeading=-1;
  _lastDate=QDate::currentDate(); // set date to a valid value
  cntSIVSentence = 1;

  //init satellite info
  _lastSatInfo.fixValidity=1; //invalid
  _lastSatInfo.fixAccuracy=999; //not very accurate
  _lastSatInfo.satCount=0; //no satelites in view;
  _lastSatInfo.constellation="";
  _lastSatInfo.constellationTime=QTime::currentTime();
  _lastClockOffset = 0;

  // altitude reference delivered by GPS unit
  _deliveredAltitude = GPSNMEA::MSL;
  _userAltitudeCorrection.setMeters(0);

  _ignoreConnectionLost = false;

  gpsDevice = "";
  serial = 0;

#ifdef MAEMO
  gpsdConnection = 0;
#endif
  
  createGpsConnection();
}
   
void GPSNMEA::createGpsConnection()
{
  // qDebug("GPSNMEA::createGpsConnection()");
  
  QObject *gpsObject;
 
#ifndef MAEMO
 
  // We create only a GPSCon instance. The client process will be started later
  const char *callPath = ( GeneralConfig::instance()->getInstallRoot() + "/bin" ).toAscii().data();

  serial = new GPSCon(this, callPath);
  
  gpsObject = serial;
  
#else

  // Under Maemo we have to contact the Maemo gpsd process on its listen port.
  // If the device name is the nmea simulator the cumulus serial client is started.
  gpsDevice = GeneralConfig::instance()->getGpsDevice();

  // qDebug("GpsDevive=%s", gpsDevice.toLatin1().data() );
  
  if( gpsDevice == NMEASIM_DEVICE )
    {
      // We assume, that the nmea simulator shall be used and will start the gps client process
      const char *callPath = ( GeneralConfig::instance()->getInstallRoot() + "/bin" ).toAscii().data();

      serial = new GPSCon(this, callPath);
      gpsObject = serial;
    }
  else
    {
      // The Maemo GPS deamon shall be used.
      gpsdConnection = new GpsMaemo( this );
      gpsObject = gpsdConnection;
    }
  
#endif 

  connect (gpsObject, SIGNAL(newSentence(const QString&)),
            this, SLOT(slot_sentence(const QString&)) );
            
  connect (gpsObject, SIGNAL(newSentence(const QString&)),
            this, SIGNAL(newSentence(const QString&)) );
            
  connect (gpsObject, SIGNAL(gpsConnectionLost()),
            this, SLOT(_slotTimeout()) );
}


GPSNMEA::~GPSNMEA()
{
  // stop GPS client process
  if( serial )
  {
    delete serial;
    writeConfig();
  }

#ifdef MAEMO
  if( gpsdConnection )
  {
    delete gpsdConnection;
  }
#endif
}

/**
 * @Starts the GPS receiver client process and activates the receiver.
 */
void GPSNMEA::startGpsReceiver()
{
  // qDebug("GPSNMEA::startGpsReceiver()");
  
  if( serial )
    {
      serial->startClientProcess();
      serial->startGpsReceiving();
    }
  
#ifdef MAEMO
  if( gpsdConnection )
    {
      gpsdConnection->startGpsReceiving();
    }
#endif
}

/**
 * This slot is called by the GPSCon object when a new sentence has
 * arrived on the serial port. The argument contains the sentence to
 * analyze.
 */
void GPSNMEA::slot_sentence(const QString& sentenceIn)
{
  // qDebug("GPSNMEA::slot_sentence: %s", sentenceIn.toLatin1().data());
  
  if( QObject::signalsBlocked() )
    {
      // @AP: If the emitting of signals is blocked, we will ignore
      // this call. Otherwise this module can do internal state
      // changes, which will never distributed, e.g. Man->Gps mode
      // change. To ignore this causes fatal problems in cumulus.
      return;
    }

  QString sentence = sentenceIn;

  if(sentence[0] != '$')
    {
      qWarning("Invalid sentence! %s",sentence.toLatin1().data());
      return;
    }

  // qDebug("received sentence '%s'",sentence.toLatin1().data());

  sentence = sentence.replace( QRegExp("[\n\r]"), "" );

  int i = sentence.lastIndexOf('*');

  if (i != -1)
    { //we have found a marker for a checksum
      if (!checkCheckSum(i, sentence))
        {
          qWarning("Invalid checksum in sentence! %s",sentence.toLatin1().data());
          return; //the checksum did not match, disregard the sentence!
        }
    }

  // dump everyting behind the * and split sentence in parts for each
  // comma. The first part will contain the identifier, the rest the
  // arguments.  no reason to truncate. Checksum will be ignored.
  // sentence.truncate(i-1);

  QStringList slst = sentence.split( QRegExp("[,*:]"), QString::KeepEmptyParts );

  dataOK();

  if (slst[0]=="$GPRMC")
    { // RMC - Recommended Minimum Specific GNSS Data
      //qDebug("%s",slst[2].toLatin1().data());
      if (slst[2]!="V")
        { /*Data status OK, V is a warning*/
          fixOK();
          __ExtractTime(slst[1]);
          __ExtractDate(slst[9]);
          __ExtractKnotSpeed(slst[7]);
          __ExtractCoord(slst[3],slst[4],slst[5],slst[6]);
          __ExtractHeading(slst[8]);

          static bool updateClock = true;

          GeneralConfig *conf = GeneralConfig::instance();

          if( updateClock && conf->getGpsSyncSystemClock() )
            {
              // @AP: we make only one update to avoid confusing of running timers
              updateClock = false;
              QDateTime utc( _lastDate, _lastTime );
              setSystemClock(utc);
            }
        }
    }
  else
    if (slst[0]=="$GPGLL")
      { // GGL - Geographic Poistion - Latitude/Longitude
        if (slst.count()>=7)
          {
            if (slst[6]=="A")
              { /*Data status OK, V is a warning*/
                fixOK();
                __ExtractTime(slst[5]);
                __ExtractCoord(slst[1],slst[2],slst[3],slst[4]);
                // qDebug("GPGLL interpreted");
              }
          }
      }
    else
      if (slst[0]=="$GPGGA")
        { // GGA - Global Positioning System Fixed Data
          // qDebug ("sentence: %s", sentence.toLatin1().data());
          if (slst[6]!="0")
            { /*a value of 0 means invalid fix, and we don't need that one */
              fixOK();
              __ExtractTime(slst[1]);
              __ExtractCoord(slst[2],slst[3],slst[4],slst[5]);

              if( slst[11] == "" )
                {
                  slst[11] = "0";
                  slst[12] = "M";
                }

              __ExtractAltitude(slst[9],slst[10],slst[11],slst[12]);
              __ExtractSatcount(slst[7]);
            }
        }
      else
        if (slst[0]=="$PGRMZ")
          {
            /* Garmin proprietary sentence with altitude information */
            if (slst[3]=="3")
              { // 1=Altitude value, 3=GPS altitude, 2=user altitude (pressure?)
                __ExtractAltitude(slst[1],slst[2]);
              }
          }
        else
          if (slst[0]=="$GPGSA")
            { // GSA - GNSS DOP and Acitve Satellites
              __ExtractConstellation(slst);
            }
          else
            if (slst[0]=="$GPGSV")
              { // GSV - GNSS Satellites in View
                // qDebug("%s", sentenceIn.toLatin1().data());
                __ExtractSatsInView(slst);
              }
            else
              if (slst[0]=="$CLK") // maybe this is specific to Fortuna PocketTrack
                {
                  _lastClockOffset = slst[1].toInt();
                  qDebug ("Clock Offset: %d", _lastClockOffset);
                } else if (slst[0].startsWith("$Ack"))
                qDebug ("NMEA command ACK");
              else
                {
                  // qDebug ("Unsupported NMEA sentence: %s", sentence.toLatin1().data());
                }
}


/**
 * This function returns a QTime from the time encoded in a MNEA sentence.
 */
QTime GPSNMEA::__ExtractTime(const QString& timestring)
{
  QString hh (timestring.left(2));
  QString mm (timestring.mid(2,2));
  QString ss (timestring.mid(4,2));

  // @AP: newer CF Cards can also provide milli seconds. In this case the time
  // format is defined as hhmmss.sss. But we will not use it to avoid problems
  // with our fixes.

  QTime res = QTime( hh.toInt(), mm.toInt(), ss.toInt() );

  // @AP: don't overtake invalid times. They will cause invalid fixes!
  if( ! res.isValid() )
    {
      qWarning("GPSNMEA::__ExtractTime(): Invalid time %s! Ignoring it (%s, %d)",
               timestring.toLatin1().data(), __FILE__, __LINE__ );
      return res;
    }

  if (_lastTime!=res)
    {
      /**
       * The fixtime has been changed, now we are starting a new fix.
       * This means that all info from the previous fix has been send,
       * and can be analysed.
       */

      emit newFix();
      _lastTime=res;
    }

  return res;
}


/** This function returns a QDate from the date string encoded in a
    NWEA sentence as "ddmmyy". */
QDate GPSNMEA::__ExtractDate(const QString& datestring)
{
  QString dd (datestring.left(2));
  QString mm (datestring.mid(2,2));
  QString yy (datestring.right(2));

  /*we assume that we only use this after the year 2000, which is
    reasonable since this is made 2002 ...*/

  QDate res (yy.toInt() + 2000,mm.toInt(),dd.toInt());

  // @AP: don't overtake invalid dates
  if( res.isValid() )
    {
      _lastDate=res;
    }
  else
    {
      qWarning("GPSNMEA::__ExtractDate(): Invalid date %s! Ignoring it (%s, %d)",
               datestring.toLatin1().data(), __FILE__, __LINE__ );
    }

  return res;
}


/** This function returns a Speed from the speed encoded in knots */
Speed GPSNMEA::__ExtractKnotSpeed(const QString& speedstring)
{
  Speed res;
  res.setKnot(speedstring.toDouble());

  if (res!=_lastSpeed)
    {
      _lastSpeed=res;
      emit newSpeed();
    }
  return (res);
}


/** This function converts the coordinate data from the NMEA sentence to the internal QPoint format. */
QPoint GPSNMEA::__ExtractCoord(const QString& slat, const QString& slatNS, const QString& slon, const QString& slonEW)
{
  /* The internal KFLog format for coordinates represents coordinates in 10.000'st of a minute.
     So, one minute corresponds to 10.000, one degree to 600.000 and one second to 167.

     KFLogCoord = degrees * 600000 + minutes * 10000
  */

  int lat = 0;
  int lon = 0;
  float fLat = 0.0;
  float fLon = 0.0;
  
  lat  = slat.left(2).toInt();
  fLat = slat.mid(2).toFloat();
  
  // qDebug ("slat: %s", slat.toLatin1().data());
  // qDebug ("lat/fLat: %d/%f", lat, fLat);
  
  lon  = slon.left(3).toInt();
  fLon = slon.mid(3).toFloat();
  
  // qDebug ("slon: %s", slon.toLatin1().data());
  // qDebug ("lon/fLon: %d/%f", lon, fLon);

  int latmin = (int) rint(fLat * 1000);
  int lonmin = (int) rint(fLon * 1000);

  // convert to internal KFLog format
  int latTemp = lat * 600000 + latmin * 10;
  int lonTemp = lon * 600000 + lonmin * 10;

  // qDebug("latTemp=%d, lonTemp=%d", latTemp, lonTemp);

  if(slatNS == "S")
    latTemp = -latTemp;
  
  if(slonEW == "W")
    lonTemp = -lonTemp;

  QPoint res (latTemp, lonTemp);

  if( _lastCoord != res )
    {
      _lastCoord=res;
      emit newPosition();
    }
    
  return (_lastCoord);
}


/** Returns the time of the last fix. */
QTime GPSNMEA::getLastTime()
{
  return _lastTime;
}


/** Returns the date of the last fix. */
QDate GPSNMEA::getLastDate()
{
  return _lastDate;
}


/** Returns the last known speed. */
Speed GPSNMEA::getLastSpeed()
{
  return _lastSpeed;
}


/** Returns the last known coordinate in KFLog format (x=lat, y=lon). */
QPoint GPSNMEA::getLastCoord()
{
  return _lastCoord;
}


/** Returns the last know heading. */
double GPSNMEA::getLastHeading()
{
  return _lastHeading;
}


/** Returns the last know standard pressure altitude */
Altitude GPSNMEA::getLastStdAltitude()
{
  return _lastStdAltitude;
}


/** Returns the last know pressure altitude above Sea level */
Altitude GPSNMEA::getLastPressureAltitude()
{
  return _lastPressureAltitude;
}


/** Returns the last know altitude above Sea level */
Altitude GPSNMEA::getLastAltitude()
{
  return _lastMslAltitude;
}


/** Returns the last know altitude above WGS84 ellipsoid*/
Altitude GPSNMEA::getLastGNSSAltitude()
{
  return _lastGNSSAltitude;
}


/** Extract the heading from the NMEA sentence. */
double GPSNMEA::__ExtractHeading(const QString& headingstring)
{
  double res = headingstring.toDouble();
  if (res!=_lastHeading)
    {
      _lastHeading=res;
      emit newHeading();
    }

  return res;
}


/** Extracts the altitude from a NMEA GGA sentence */
Altitude GPSNMEA::__ExtractAltitude(const QString& altitude, const QString& unitAlt,
                                    const QString& heightOfGeoid, const QString& unitHeight)
{
  // qDebug("alt=%s, unit=%s, hae=%s, unit=%s",
  //  altitude.toLatin1().data(), unitAlt.toLatin1().data(), heightOfGeoid.toLatin1().data(), unitHeight.toLatin1().data() );

  Altitude res(0);
  double alt = altitude.toDouble();

  Altitude sep(0);
  double hae = heightOfGeoid.toDouble();

  // check for other unit as meters, meters is the default
  if( unitAlt.toLower() == "f" )
    res.setFeet(alt);
  else
    res.setMeters(alt);

  // check for other unit as meters, meters is the default
  if( unitHeight.toLower() == "f" )
    sep.setFeet(hae);
  else
    sep.setMeters(hae);

  if( _deliveredAltitude == GPSNMEA::HAE )
    {
      // @AP: GPS unit delivers Height above the WGS 84 ellipsoid. We must
      // adapt this to our internal variables.
      _lastGNSSAltitude.setMeters( res.getMeters() );

      // calculate MSL
      res.setMeters( res.getMeters() - sep.getMeters() );
    }
  else if( _deliveredAltitude == GPSNMEA::USER )
    {
      // @AP: GPS unit delivers Height above the WGS 84 ellipsoid. We must
      // adapt this to our internal variables.
      _lastGNSSAltitude.setMeters( res.getMeters() );

      // calculate MSL
      res.setMeters( res.getMeters() - _userAltitudeCorrection.getMeters() );
    }
  else // MSL comes in
    {
      // calculate HAE
      _lastGNSSAltitude.setMeters( res.getMeters() + sep.getMeters() );
    }

  if( _lastMslAltitude != res )
    {
      _lastMslAltitude = res;
      _lastStdAltitude = res;

      // @AP: calculate STD Altitude
      GeneralConfig *conf = GeneralConfig::instance();
      int qnhDiff = 1013 - conf->getQNH();

      if( qnhDiff != 0 )
        {
          // calculate altitude correction in meters from pressure difference
          int delta = (int) rint( qnhDiff * 8.6 );
          _lastStdAltitude.setMeters( res.getMeters() + delta );
        }

      emit newAltitude();
    }

  return res;
}


/** Extracts the altitude from a Garmin proprietary PGRMZ sentence */
Altitude GPSNMEA::__ExtractAltitude(const QString& number, const QString& unit)
{
  Altitude res(0);
  double num = number.toDouble();

  // check for other unit as meters, meters is the default
  if( unit.toLower() =="f" )
    res.setFeet(num);
  else
    res.setMeters(num);

  if( _lastMslAltitude != res )
    {
      _lastMslAltitude = res;  //store the new altitude
      emit newAltitude();      // let everyone know we have it!
    }

  return res;
}

/** Extracts the constellation from the NMEA sentence. */
QString GPSNMEA::__ExtractConstellation(const QStringList& sentence)
{
  QString result, tmp;

  if (sentence[2]!="1")
    {
      fixOK();
      _lastSatInfo.fixValidity=sentence[2].toInt();
    }

  for (int i=3; i<=14 && i < sentence.size() ; i++)
    {

      if( sentence[i] != QString::null && sentence[i] != QString("") )
        {
          tmp.sprintf("%02d",sentence[i].toInt());
          result+=tmp;
        }
    }

  if (result!=_lastSatInfo.constellation)
    {
      _lastSatInfo.constellationTime =_lastTime;
      _lastSatInfo.constellation=result;
      emit newSatConstellation();
    }

  return result;
}


/** Extracts the satcount from the NMEA sentence. */
void GPSNMEA::__ExtractSatcount(const QString& satcount)
{
  int count = satcount.toInt();
  if (count !=_lastSatInfo.satCount)
    {
      _lastSatInfo.satCount = count;
      emit newSatConstellation();
    }
}


/** Returns the last know satellite constellation string. */
SatInfo GPSNMEA::getLastSatInfo()
{
  return _lastSatInfo;
}


/** This slot is called by the internal timer to signal a timeout. If this timeout occurs, the connection is set to false, i.e., the connection has been lost. */
void GPSNMEA::_slotTimeout()
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
      qWarning("GPS CONNECTION SEEMS TO BE LOST!");
      _status=notConnected;
      emit statusChange(_status);

      _lastMslAltitude=Altitude(0);
      _lastGNSSAltitude=Altitude(0);
      _lastPressureAltitude=Altitude(0);
      _lastCoord=QPoint(0,0);
      //_lastDate=0;
      //_lastTime=0;
      _lastSpeed=Speed(-1.0);
      _lastHeading=-1;

      //init satalite info
      _lastSatInfo.fixValidity=1; //invalid
      _lastSatInfo.fixAccuracy=999; //not very accurate
      _lastSatInfo.satCount=0; //no satelites in view;
      _lastSatInfo.constellation="";
      _lastSatInfo.constellationTime=QTime::currentTime();
    }
}


/** This slot is called by the internal timer to signal a timeout. If this timeout occurs, the connection is set to false, i.e., the connection has been lost. */
void GPSNMEA::_slotTimeoutFix()
{
  if (_status == validFix)
    {
      _status=noFix;
      emit statusChange(_status);
      qWarning("GPS FIX LOST!");

      _lastMslAltitude=Altitude(0);
      _lastGNSSAltitude=Altitude(0);
      _lastPressureAltitude=Altitude(0);
      _lastCoord=QPoint(0,0);
      //_lastDate=0;
      //_lastTime=0;
      _lastSpeed=Speed(-1.0);
      _lastHeading=-1;

      //init satalite info
      _lastSatInfo.fixValidity=1; //invalid
      _lastSatInfo.fixAccuracy=999; //not very accurate
      _lastSatInfo.satCount=0; //no satelites in view;
      _lastSatInfo.constellation="";
      _lastSatInfo.constellationTime=QTime::currentTime();

      // stop timer, will be activated again with the next available fix
      timeOutFix->stop();
    }
}


/** This function is called to indicate that good data has been received. It resets the TimeOut timer and if necessary changes the connected status. */
void GPSNMEA::dataOK()
{

  if (_status==notConnected)
    {
      _status=noFix;
      emit statusChange(_status);
      emit connectedChange(_status != notConnected);
      qDebug("GPS CONNECTION ESTABLISHED!");
    }
}


void GPSNMEA::fixOK()
{

  // restart timer
  timeOutFix->start(FIX_TO);

  if (_status != validFix)
    {
      _status=validFix;
      emit statusChange(_status);
      qDebug("GPS FIX OBTAINED!");
    }
}


/**
 * This slot is called if the object needs to reset. It is used to stop the
 * serial connection and to opens a new one, to adjust to the new settings.
 */

void GPSNMEA::slot_reset()
{
  // qDebug("GPSNMEA::slot_reset()");

  GeneralConfig *conf = GeneralConfig::instance();

  QString oldDevice = gpsDevice;
  
  if( gpsDevice != conf->getGpsDevice() )
  {
    // GPS device has been changed by the user
    gpsDevice = conf->getGpsDevice();
    
    if( serial )
    {
      serial->stopGpsReceiving();
      delete serial;
      serial = 0;
    }

#ifdef MAEMO
    if( gpsdConnection )
    {
      if( oldDevice == gpsDevice )
      {
        // Ignore device modifications. Only switch between GPSD and Simulator is
        // of interest for Maemo.
        return;
      }

      gpsdConnection->stopGpsReceiving();
      delete gpsdConnection;
      gpsdConnection = 0;
    }
#endif
    
    // create a new connection
    createGpsConnection();
    // start the new connection
    startGpsReceiver();
    return;
  }  
  
  // no device modification, check serials baud rate
  if( serial )
    {
      if( serial->currentBautrate() != conf->getGpsSpeed() )
        {
          serial->stopGpsReceiving();
          serial->startGpsReceiving();
        }

      bool hard = conf->getGpsHardStart();
      bool soft = conf->getGpsSoftStart();

      sendLastFix (hard, soft);
    }
}


/** This slot is called to reset the gps device to factory settings */
void GPSNMEA::sendFactoryReset()
{
  if( serial ) serial->sendSentence ("$PSRF104,0.0,0.0,0,0,0,0,12,8");
}


/** This slot is called to switch debugging mode on/off */
void GPSNMEA::switchDebugging (bool on)
{
  QString cmd;
  cmd.sprintf ("$PSRF105,%d", on);
  if( serial ) serial->sendSentence (cmd);
}


/**
 * Send the data of last valid fix to the gps receiver. Use the current utc
 * time as basis for calculating gps week and timeof week. This function is
 * called only at initialization; we don't optimize to show the algorithm
 * clear.
 */
void GPSNMEA::sendLastFix (bool hard, bool soft)
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
    if( serial ) serial->sendSentence (cmd);
}


/** force a reset of the serial connection after a resume */
void GPSNMEA::forceReset()
{
  // qDebug("GPSNMEA::forceReset()");
  
  if( serial )
    {
      serial->stopGpsReceiving();
      serial->startGpsReceiving();
      slot_reset();
    }
}


/** This function calculates the checksum in the sentence.

NMEA-0183 Standard
The optional checksum field consists of a "*" and two hex digits
representing the exclusive OR of all characters between, but not
including, the "$" and "*".  A checksum is required on some sentences.
*/
uint GPSNMEA::calcCheckSum (int pos, const QString& sentence)
{
  uchar sum = 0;

  for( int i=1; i < pos; i++ )
    {
      uchar c = (sentence[i]).toAscii();

      if( c == '$' ) // Start sign will not be considered
        continue;

      if( c == '*' ) // End of sentence reached
        break;

      sum ^= c;
    }

  return sum;
}


/** This function checks if the checksum in the sentence matches the sentence. It retuns true if it matches, and false otherwise. */
bool GPSNMEA::checkCheckSum(int pos, const QString& sentence)
{
  uchar check = (uchar) sentence.right(2).toUShort(0, 16);
  return (check == calcCheckSum (pos, sentence));
}


/** Returns the current GPS connection status. True if connected, false if not. */
bool GPSNMEA::getConnected()
{
  return (_status != notConnected);
}


/** write config data to allow restore of last fix */
void GPSNMEA::writeConfig()
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


/** Set system date/time. Input is utc related. */
void GPSNMEA::setSystemClock( const QDateTime& utcDt )
{
  if( ! utcDt.isValid() )
    {
      return;
    }

  if( getuid() != 0 )
    {
      // we are not user root
      qWarning("Only the superuser can set the system clock!");
      return;
    }

  static char *noTZ = "TZ=";
  static char *utcTZ = "TZ=UTC";

  // save current TZ
  char *curTZ = getenv("TZ");

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

  if ( myTv.tv_sec != -1 )
    {
      settimeofday( &myTv, 0 );
    }

  timeOutFix->stop();

  // set hardware clock via hwclock tool, because the writeHWClock()
  // method will not do that in every case
  system("/sbin/hwclock --systohc");

  if( curTZ ) // restore old time zone
    {
      // decrement pointer to get the original string TZ=... getenv
      // returns only the assigned value.
      curTZ = curTZ-3;
    } else
    {
      curTZ = noTZ;
    }

  putenv( curTZ ); // restore old settings
  tzset(); // reactivate saved time zone

  // restart timer to avoid fix losts
  timeOutFix->start(FIX_TO);
  // set flag to avoid reporting of connection lost
  _ignoreConnectionLost = true;

  _globalMapView->message( tr("System clock synchronized") );
}


/** Extract Satelites In View (SIV) info from a NMEA sentence. */
void GPSNMEA::__ExtractSatsInView(const QStringList& sentence)
{
  //the GPGSV sentence can be split between multiple sentences.
  //qDebug("expecting: %d, found: %s",cntSIVSentence,sentence[2].toLatin1().data());
  //check if we were expecting this part of the info
  if (cntSIVSentence != (uint)sentence[2].toInt())
    return;

  if (cntSIVSentence == 1) //this is the first sentence of our series
    sivInfoInternal.clear();

  //extract info on the induvidual sats
  __ExtractSatsInView( sentence[4], sentence[5], sentence[6], sentence[7]);
  if (sentence.count()>11)
    __ExtractSatsInView( sentence[8], sentence[9], sentence[10], sentence[11]);
  if (sentence.count()>15)
    __ExtractSatsInView( sentence[12], sentence[13], sentence[14], sentence[15]);
  if (sentence.count()>19)
    __ExtractSatsInView( sentence[16], sentence[17], sentence[18], sentence[19]);

  cntSIVSentence++;
  if (cntSIVSentence > (uint)sentence[1].toInt()) //this was the last sentence in our series
    {
      cntSIVSentence = 1;
      sivInfo = sivInfoInternal;
      emit newSatInViewInfo();
      //qDebug("triggered new sivi signal");
    }
}


/** Extract Satelites In View (SIV) info from a NMEA sentence. */
void GPSNMEA::__ExtractSatsInView(const QString& id, const QString& elev, const QString& azimuth, const QString& snr)
{
  if (id.isEmpty())
    return;

  SIVInfo sivi;
  sivi.id = id.toInt();
  sivi.elevation = elev.toInt();
  sivi.azimuth = azimuth.toInt();
  if ( snr.isEmpty() )
    {
      sivi.db = -1;
    }
  else
    {
      sivi.db = snr.toInt();
    }

  sivInfoInternal.append(sivi);
  //qDebug("new sivi info (snr: %d", sivi->db);
}
