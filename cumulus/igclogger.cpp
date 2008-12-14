/***************************************************************************
                          igclogger.cpp  -  description
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by André Somers, 2008 Axel Pauli
    email                : axel@kflog.org

    This file is part of Cumulus

    $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#include <QtGlobal>
#include <QMessageBox>
#include <QDir>

#include "igclogger.h"
#include "gpsnmea.h"
#include "hwinfo.h"

#include "target.h"
#include "generalconfig.h"
#include "mapcontents.h"
#include "flighttask.h"
#include "waypoint.h"

IgcLogger * IgcLogger::_theInstance = 0;


IgcLogger::IgcLogger(QObject* parent)
  : QObject(parent),
    _backtrack(LimitedList<QString> (15))
{
  timer=new QTimer(this);
  connect( timer, SIGNAL(timeout()), this, SLOT(slotMakeFixEntry()) );
  _logMode=off;
}


IgcLogger * IgcLogger::instance()
{
  // First user will create it
  if( _theInstance == 0 )
    _theInstance = new IgcLogger(0);
  return( _theInstance );
};


IgcLogger::~IgcLogger()
{
  if (_logMode==on)
    CloseFile();
  _theInstance=0;
}


/** Starts logging of data */
void IgcLogger::Start()
{
  // load logger record time interval, time im ms
  int interval = GeneralConfig::instance()->getLoggerInterval() * 1000;
  timer->setSingleShot(false);
  timer->start( interval);
}


/** Stop logging */
void IgcLogger::Stop()
{
  timer->stop();
  _logMode=off;
}


/** This slot is used internaly by the timer to make a log entry on an
    interval, but can also be used from outside the class to make sure
    a specific point is being logged (ie., to respond to a usercommand
    to log). */
void IgcLogger::slotMakeFixEntry()
{
  if ( _logMode == off ) {
    return;
  }

  if( fasterLoggingTime.elapsed() > 30000 ) {
    // reset logging to normal mode after 30 seconds
     timer->setInterval( GeneralConfig::instance()->getLoggerInterval() * 1000 );
  }

  if( calculator->samplelist.count() == 0 ) {
    return; // make sure there is an entry in the list!
  }

  const flightSample &lastfix = calculator->samplelist.at(0);

  //check if we have a new fix to log
  if (lastfix.time == lastLoggedFix) {
    qWarning("No log entry made: no recent fix.");
    //we can add a warning here if there has not been a recent fix for more than a predefined time.
    return;
  }

  lastLoggedFix = lastfix.time;

  //#warning: FIXME: The 'A' represents a valid 3D fix. This should be taken from the GPS!
  QString entry("B" + formatTime(lastfix.time) + formatPosition(lastfix.position) + "A"
                + formatAltitude(lastfix.altitude) + formatAltitude(lastfix.GNSSAltitude));

  if( _logMode==standby ) {
    _backtrack.add(entry);
  } else {
    _stream << entry << "\n";
    emit madeEntry();
    _logfile.flush(); //make sure the file is flushed, so we will not lose data if something goes wrong
  }
}

/** Call this slot, if a task sector has been touched to increase
 *  logger interval for a certain time.
 */

void IgcLogger::slotTaskSectorTouched()
{
  if ( _logMode == off ) {
    return;
  }

  slotMakeFixEntry(); // save current position of touch

  // activate a shorter logger interval
  fasterLoggingTime.start();
  timer->setInterval( 1000 ); // log every second up to now
}

void IgcLogger::Standby()
{
  if (_logMode==on) {
    CloseFile();
  }
  _logMode=standby;
  _backtrack.clear();
  Start();
  emit logging(getisLogging());
}


/** Creates a log file */
void IgcLogger::CreateLogfile()
{
#warning IGC Logfile is stored at User Data Directory

  QString path(GeneralConfig::instance()->getUserDataDirectory());

  QString fname = createFileName(path);

  QDir dir(path);

  if( ! dir.exists() )
    {
      dir.mkpath(path);
    }

  _logfile.setFileName(fname);

  if( ! _logfile.open(QIODevice::WriteOnly | QIODevice::Text ) )
    {
      qWarning( "IGC-Logger: Cannot open file %s",
                fname.toLatin1().data() );
    }
  else
    {
      qDebug( "IGC-Logger: Created Logfile %s", fname.toLatin1().data() );
    }

  _stream.setDevice(&_logfile);
  writeHeaders();
  _logMode=on;
  makeSatConstEntry();
  slotMakeFixEntry();
  // DocLnk lnk(fname);
  // lnk.setType("application/x-igc");
  // lnk.writeLink();

  Start();
}


/** Closes the logfile. */
void IgcLogger::CloseFile()
{
  Stop();
  _logfile.close();
}


/** This function writes the headers for the logfile to the logfile. */
void IgcLogger::writeHeaders()
{
  GeneralConfig *conf = GeneralConfig::instance();

  QString surName = conf->getSurname();
  QString date = formatDate(QDate::currentDate());
  QString time = formatTime(QTime::currentTime());

  _stream << "AXXXCUZ Cumulus Soaring Flightcomputer, flightnumber " << flightnumber << "\n" ;
  _stream << "HFDTE" << date << "\n";
  _stream << "HFFXA500" << "\n";
  _stream << "HFPLTPILOT: " << (surName.isEmpty() ? "Unknown" : surName.toLatin1().data()) << "\n";
  _stream << "HFGTYGLIDERTYPE: " << calculator->glider()->type() << "\n";
  _stream << "HFGIDGLIDERID: " << calculator->glider()->registration() << "\n";
  _stream << "HFDTM100GPSDATUM: WSG-1984\n";
  _stream << "HFRFWFIRMWAREVERION: " << CU_VERSION << "\n";
  _stream << "HFRHWHARDWAREVERSION: " << HwInfo::instance()->getTypeString() << "\n" ;
  _stream << "HFFTYFRTYPE: Cumulus Version: " << CU_VERSION << ", Qt/X11 Version: " << qVersion() << "\n";
  _stream << "HSCIDCOMPETITIONID: " << calculator->glider()->callsign() << "\n";

  //GSP info lines ommitted for now
  //additional data (competion ID and classname) ommitted for now

  _stream << "I013640FXA\n"; //only the default fix extention for now
  _logfile.flush();

  // no J record for now

  // task support: C records

  extern MapContents * _globalMapContents;
  FlightTask* task = (FlightTask *) _globalMapContents->getCurrentTask();

  if( ! task ) {
    return; // no task active
  }

  QList<wayPoint*> wpList = task->getWPList();

  if( wpList.count() < 4 ) {
    return; // too less waypoints
  }

  QString fnr;
  fnr.sprintf( "%04d", flightnumber );
  QString tpnr;
  tpnr.sprintf( "%02d", wpList.count() - 4 );
  QString taskId = task->getTaskTypeString();

  // date/time normally UTC is expected
  _stream << "C"
          << date.toLatin1().data()
          << time.toLatin1().data()
          << date.toLatin1().data()
          << fnr.toLatin1().data()
          << tpnr.toLatin1().data()
          << taskId.toLatin1().data()
          << "\n";

  for( int i=0; i < wpList.count(); i++ ) {
    wayPoint *wp = wpList.at(i);

    _stream << "C"
            << formatPosition( wp->origP ).toLatin1().data()
            << wp->name.toLatin1().data() << "\n";
  }

  _logfile.flush();
}


/** This function formats a date in the correct igc format DDMMYY */
QString IgcLogger::formatDate(const QDate& date)
{
  QString result;
  result.sprintf("%02d%02d%02d",date.day(),date.month(),date.year()-2000);    //We don't expect this to be used a century from now....
  return result;
}


/** Read property of bool isLogging. */
const bool IgcLogger::getisLogging()
{
  return (_logMode==on);
}


/** Return true if we are in standby mode. */
const bool IgcLogger::getisStandby()
{
  return (_logMode==standby);
}


/** This slot is called to start or end a log. */
void IgcLogger::slotToggleLogging()
{
  qDebug("toggle logging!");

  if (_logMode==on) {
    // Trying to ask a question causes segfault?!
    int answer= QMessageBox::question(0,tr("Stop Logging?"),
                                     tr("Are you sure you want\nto close the logfile\nand stop logging?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Escape | QMessageBox::Default);

    if (answer==QMessageBox::Yes) {
      qDebug("Stopping logging...");
      _logMode=off;
      CloseFile();
    }
  } else if (_logMode==standby) {
    int answer= QMessageBox::question(0,tr("Stop Logging?"),
                                     tr("Are you sure you want\nto stop listening\nfor events to autostart\nlogging?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Escape | QMessageBox::Default);

    if (answer==QMessageBox::Yes) {
      qDebug("Logger standby mode turned off.");
      _logMode=off;
    }
  } else {
    if (calculator->glider()) {
      CreateLogfile();
      _logMode=on;
      //    emit logging(isLogging);
    } else {
      QMessageBox::information(0,tr("Can't start log"),
                               tr("You need to select a glider\nbefore starting logging."),
                               QMessageBox::Ok);
    }
  }
  // emit the logging state in all cases to allow update of actions in cumulusapp
  emit logging(getisLogging());
}


/** This slot is called to indicate that a new satellite constellation is now in use. */
void IgcLogger::slotConstellation()
{
  makeSatConstEntry();
}


/** Makes a fix entry in the logfile. */
void IgcLogger::makeSatConstEntry()
{

  if (_logMode>off) {
    QString entry = "F" + formatTime(gps->getLastSatInfo().constellationTime) +
      gps->getLastSatInfo().constellation;

    if (_logMode==standby) {
      _backtrack.add(entry);
    } else {
      _stream << entry << "\n";
      emit madeEntry();
      _logfile.flush(); //make sure the file is flushed, so we will not lose data if something goes wrong
    }
  }
}


/** This function formats a QTime to the correct format for igc files (HHMMSS) */
QString IgcLogger::formatTime(const QTime& time)
{
  QString result;
  result.sprintf("%02d%02d%02d",time.hour(),time.minute(),time.second());
  return result;
}


/** This function formats an Altitude to the correct format for IGC files (XXXXX) */
QString IgcLogger::formatAltitude(Altitude altitude)
{
  QString result;
  result.sprintf("%05.0f",altitude.getMeters());
  return result;

}


/** This function formats the position to the correct format for igc files. Latitude and Longitude are encoded as DDMMmmmADDDMMmmmO, with A=N or S and O=E or W. */
QString IgcLogger::formatPosition(const QPoint& position)
{
  /*The internal KFLog format for coordinates represents coordinates in 10.000'st of a minute.
    So, one minute corresponds to 10.000, one degree to 600.000 and one second to 167.

    KFLogCoord = degrees * 600000 + minutes * 10000
  */

  int latdeg, londeg, calc, latmin, lonmin;
  QString result, latmark, lonmark;

  calc = position.x(); //lattitude
  if (calc<0) {
    calc=-calc; //use positive values for now;
    latmark="S";
  } else
    latmark="N";
  latdeg=calc/600000;  //calculate degrees
  calc-=latdeg*600000;  //substract the whole degrees part
  latmin=calc/10; //we need the minutes in 1000'st of a minute, not in 10.000'st.


  calc = position.y(); //longitude
  if (calc<0) {
    calc=-calc; //use positive values for now;
    lonmark="W";
  } else
    lonmark="E";
  londeg=calc/600000;  //calculate degrees
  calc-=londeg*600000;  //substract the whole degrees part
  lonmin=calc/10; //we need the minutes in 1000'st of a minute, not in 10.000'st.

  result.sprintf("%02d%05d%1s%03d%05d%1s",latdeg,latmin,latmark.toLatin1().data(),londeg,lonmin,lonmark.toLatin1().data());
  return result;
}


/** Creates a new filename for the IGC file according to the IGC standards (IGC GNSS FR Specification, may 2002, Section 2.5) YMDCXXXF.IGC */
QString IgcLogger::createFileName(const QString& path)
{
  int year=QDate::currentDate().year();
  year=year % 10;
  int month=QDate::currentDate().month();
  int day=QDate::currentDate().day();
  QString name=QString::number(year,10) + QString::number(month,13) + QString::number(day,32);
  name+="X000"; // @AP: one x have to be added for unknown manufacture

  int i=1;
  QString result=name + QString::number(i,36);
  while (QFile(path + "/" + result.toUpper() + ".IGC").exists()) {
    i++;
    result=name + QString::number(i,36);
  }
  flightnumber=i; //store the resulting number so we can use it in the logfile itself

  return path + "/" + result.toUpper() + ".IGC";
}


void IgcLogger::slotFlightMode(Calculator::flightmode mode)
{
  if ((_logMode==standby) && (mode!=Calculator::standstill)) {
    _logMode=on;
    CreateLogfile();
    for (int i = _backtrack.count()-1; i>=0; i--) {
      qDebug("backtrack %d: %s",i,_backtrack.at(i).toLatin1().data());
      _stream << _backtrack.at(i).toLatin1().data()<<"\n";
    }
    _backtrack.clear(); //make sure we aren't leaving old data behind.
  }
}

