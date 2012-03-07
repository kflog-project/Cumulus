/***************************************************************************
                          igclogger.cpp - creates an IGC logfile
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002      by André Somers
                               2008-2012 by Axel Pauli

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

#include <cstdlib>

#include <QtGui>

#include "igclogger.h"
#include "gpsnmea.h"
#include "hwinfo.h"

#include "target.h"
#include "generalconfig.h"
#include "mapcontents.h"
#include "flighttask.h"
#include "taskpoint.h"

// initialize static variables
IgcLogger* IgcLogger::_theInstance = static_cast<IgcLogger *> (0);

IgcLogger::IgcLogger(QObject* parent) :
  QObject(parent),
  closeTimer(0),
  _kRecordLogging(false),
  _backtrack( LimitedList<QStringList>(30) ),
  _flightMode( Calculator::unknown)
{
  if ( GeneralConfig::instance()->getLoggerAutostartMode() )
    {
      // auto logging mode is switched on by the user
      _logMode = standby;
    }
  else
    {
      // logger is switched off as default
      _logMode = off;
    }

  // load user configuration items
  _bRecordInterval = GeneralConfig::instance()->getBRecordInterval();
  _kRecordInterval = GeneralConfig::instance()->getKRecordInterval();

  lastLoggedBRecord = new QTime();
  lastLoggedFRecord = new QTime();
  lastLoggedKRecord = new QTime();

  resetTimer = new QTimer( this );
  connect( resetTimer, SIGNAL(timeout()), this, SLOT(slotResetLoggingTime()) );

  closeTimer = new QTimer(this);
  closeTimer->setSingleShot( true );
  connect( closeTimer, SIGNAL(timeout()), this, SLOT(slotCloseLogFile()) );
}

IgcLogger::~IgcLogger()
{
  if( _logMode == on )
    {
      CloseFile();
    }

  _theInstance = static_cast<IgcLogger *>(0);

  delete lastLoggedBRecord;
  delete lastLoggedFRecord;
  delete lastLoggedKRecord;
  delete resetTimer;
}

/** returns the existing singleton class */
IgcLogger* IgcLogger::instance()
{
  if( _theInstance == static_cast<IgcLogger *> (0) )
    {
      // first instance of this class is created
      _theInstance = new IgcLogger(0);
    }

  return _theInstance;
}

/**
 * This slot is called to read the logger configuration items after a modification.
 */
void IgcLogger::slotReadConfig()
{
  // qDebug( "IgcLogger::slotReadConfig()" );
  if( _logMode != on )
    {
      // Don't change mode, if logger is switch on
      if( GeneralConfig::instance()->getLoggerAutostartMode() )
        {
          // auto logging mode is switched on by the user
          _logMode = standby;
        }
      else
        {
          // logger is switched off as default
          _logMode = off;
        }
    }

  _bRecordInterval = GeneralConfig::instance()->getBRecordInterval();
  _kRecordInterval = GeneralConfig::instance()->getKRecordInterval();
}

/**
 * This slot is called to reset the logger interval after a modification.
 */
void IgcLogger::slotResetLoggingTime()
{
  _bRecordInterval = GeneralConfig::instance()->getBRecordInterval();
  _kRecordInterval = GeneralConfig::instance()->getKRecordInterval();
}

/** This slot is called by the calculator if a new flight sample is ready to
 *  make a log entry on a predefined interval into the IGC file.
 */
void IgcLogger::slotMakeFixEntry()
{
  // qDebug("IgcLogger::slotMakeFixEntry()");
  if ( _logMode == off || calculator->samplelist.count() == 0 )
    {
      // make sure logger is not off and and entries are in the sample list
      return;
    }

  const FlightSample &lastfix = calculator->samplelist.at(0);

  // check if we have to log a new B-Record
  if ( ! lastLoggedBRecord->isNull() &&
         lastLoggedBRecord->addSecs( _bRecordInterval ) > lastfix.time )
    {
      // write K-Record, if needed
      writeKRecord( lastfix.time );
      return;
    }

  *lastLoggedBRecord = lastfix.time;

  QString bRecord( "B" + formatTime(lastfix.time) + formatPosition(lastfix.position) + "A" +
                   formatAltitude(lastfix.altitude) + formatAltitude(lastfix.GNSSAltitude) +
                   QString("%1").arg(GpsNmea::gps->getLastSatInfo().fixAccuracy, 3, 10, QChar('0')) +
                   QString("%1").arg(GpsNmea::gps->getLastSatInfo().satsInUse, 2, 10, QChar('0')) );

  if ( _logMode == standby &&
       ( calculator->moving() == false ||
         _flightMode == Calculator::unknown ||
         _flightMode == Calculator::standstill ) )
    {
      // save B and F record and time in backtrack, if we are not in move
      QString fRecord = "F" +
                         formatTime( lastfix.time ) +
                         GpsNmea::gps->getLastSatInfo().constellation;
      QStringList list;
      list << bRecord << fRecord << QTime::currentTime ().toString("hhmmss");
      _backtrack.add( list );
      // qDebug( "_backtrack.size=%d", _backtrack.size() );

      // Set last F recording time from the oldest log entry. Looks a little bit
      // tricky but should work so. ;-)
      *lastLoggedFRecord = QTime::fromString( _backtrack.last().at(2), "hhmmss" );
      return;
    }

  if( isLogFileOpen() )
    {
      if( _logMode == standby || _backtrack.size() > 0 )
        {
          // There is a special case. The user can switch on the logger via toggle L
          // but the logger was before in state standby and the backtrack contains
          // entries. Such entries must be written out in the new opened log file
          // before start with normal logging. Otherwise the first B record is missing.
          _logMode = on;

          // set start date and time of logging
          startLogging = QDateTime::currentDateTime();

          // If log mode was before in standby we have to write out the backtrack entries.
          if( _backtrack.size() > 0 )
            {
              if( _backtrack.last().at( 0 ).startsWith( "B" ) )
                {
                  // The backtrack contains at the last position a B record but IGC log
                  // should start with a F record. Therefore we take the corresponding
                  // F record from the stored string list.
                  _stream << _backtrack.last().at( 1 ) << "\r\n";
                }

              for( int i = _backtrack.count() - 1; i >= 0; i-- )
                {
                  // qDebug( "backtrack %d: %s, %s", i,
                  // _backtrack.at(i).at(0).toLatin1().data(),
                  // _backtrack.at(i).at(1).toLatin1().data() );

                  _stream << _backtrack.at(i).at(0) << "\r\n";
                }

              _backtrack.clear(); // make sure we aren't leaving old data behind.
            }
          else
            {
              // If backtrack contains no entries we must write out a F record at first
              makeSatConstEntry( lastfix.time );
            }
        }

      /*
      qDebug("F-TimeCheck: time=%s, elapsed=%dms",
              lastLoggedFRecord->toString("hh:mm:ss").toLatin1().data(),
              lastLoggedFRecord->elapsed());
      */

      // Check if F record has to be written
      if( lastLoggedFRecord->elapsed() >= 5*60*1000 )
        {
          // According to the IGC specification after 5 minutes a F record has
          // to be logged. So we will do now.
          makeSatConstEntry( lastfix.time );
        }

      _stream << bRecord << "\r\n";

      // write K-Record
      writeKRecord( lastfix.time );

      emit madeEntry();
    }
}

/**
 * Writes a K-Record, if all conditions for that are true.
 */
void IgcLogger::writeKRecord( const QTime& timeFix )
{
  if( _kRecordLogging == false || ! _logfile.isOpen()  )
    {
      // 1. K-Record logging is switched off
      // 2. IGC logfile is not open.
      return;
    }

  // Check if we have to log a new K-Record.
  if ( ! lastLoggedKRecord->isNull() &&
         lastLoggedKRecord->addSecs( _kRecordInterval ) > timeFix )
    {
      // there is no log to do
      return;
    }

  *lastLoggedKRecord = timeFix;

  /**
   *  The additional five parameters are logged as K record.

      0         1            2           3         4
      1234567 890 123456 789 012 3456789 01234567890
      KHHMMSS hdt taskph wdi wsp -vat...
                                 +
      Example: K120000 090 100kph 270 020 -001200

      08-10 HDT, true heading as 3 numbers
      11-16 TAS, true airspeed as 3 numbers with unit kph
      17-19 WDI, wind direction as 3 numbers
      20-22 WSP, wind speed as 3 numbers in kph
      23-29 VAT, vario speed in meters as sign +/-, 3 numbers with 3 decimal numbers
   *
   */
  QString kRecord( "K" + formatTime(timeFix) +
                   QString("%1").arg( (int) rint(GpsNmea::gps->getLastHeading()), 3, 10, QChar('0')) +
                   QString("%1").arg( (int) rint(GpsNmea::gps->getLastTas().getKph()), 3, 10, QChar('0')) + "kph" +
                   QString("%1").arg( calculator->getlastWind().getAngleDeg(), 3, 10, QChar('0')) +
                   QString("%1").arg( (int) rint(calculator->getlastWind().getSpeed().getKph()), 3, 10, QChar('0')) +
                   formatVario(calculator->getlastVario()) );

  _stream << kRecord << "\r\n";
}

/** Call this slot, if a task sector has been touched to increase
 *  logger interval for a certain time.
 */

void IgcLogger::slotTaskSectorTouched()
{
  if ( _logMode != on )
    {
      return;
    }

  // activate timer to reset logger interval after 30s to default
  resetTimer->setSingleShot(true);
  resetTimer->start( 30*1000 );

  // activate a shorter logger interval
  _bRecordInterval = 1; // log every second up to now

  slotMakeFixEntry(); // save current position of touch
}

/**
 * Stop logging.
 */
void IgcLogger::Stop()
{
  if( _logMode == on )
    {
      CloseFile();
    }

  _logMode = off;
  _backtrack.clear();

  // Reset time classes to initial state
  delete lastLoggedBRecord;
  delete lastLoggedFRecord;
  delete lastLoggedKRecord;
  lastLoggedBRecord = new QTime();
  lastLoggedFRecord = new QTime();
  lastLoggedKRecord = new QTime();
  emit logging( getIsLogging() );
}

/**
 * Switches on the standby mode. If we are currently logging, the logfile will
 * be closed.
 */
void IgcLogger::Standby()
{
  if( _logMode == on )
    {
      CloseFile();
    }

  _logMode = standby;
  _backtrack.clear();

  // Reset time classes to initial state
  delete lastLoggedBRecord;
  delete lastLoggedFRecord;
  delete lastLoggedKRecord;
  lastLoggedBRecord = new QTime();
  lastLoggedFRecord = new QTime();
  lastLoggedKRecord = new QTime();
  emit logging( getIsLogging() );
}

/**
 * Creates a log file, if it not yet already exists and writes the header items
 * into it.
 *
 * Returns true if file is ready for further writing otherwise false.
 */
bool IgcLogger::isLogFileOpen()
{
#warning IGC Logfile is stored at User Data Directory

  if( _logfile.isOpen() )
    {
      // Logfile is already opened
      return true;
    }

  QString path(GeneralConfig::instance()->getUserDataDirectory());

  QString fname = createFileName(path);

  QDir dir(path);

  if ( ! dir.exists() )
    {
      dir.mkpath(path);
    }

  _logfile.setFileName( fname );

  if ( ! _logfile.open(QIODevice::WriteOnly) )
    {
      qWarning( "IGC-Logger: Cannot open file %s", fname.toLatin1().data() );
      return false;
    }

  // qDebug( "IGC-Logger: Created Logfile %s", fname.toLatin1().data() );

  _stream.setDevice(&_logfile);

  writeHeader();

  // As first create a F record
  slotConstellation( GpsNmea::gps->getLastSatInfo() );

  return true;
}

/** Closes the logfile. */
void IgcLogger::CloseFile()
{
  if( _logfile.isOpen() )
    {
      _logfile.close();
    }

  // reset logger start time
  startLogging = QDateTime();
}

/** This function writes the header of the IGC file into the logfile. */
void IgcLogger::writeHeader()
{
  GeneralConfig *conf = GeneralConfig::instance();

  QString pilot = conf->getSurname();
  QString date  = formatDate( GpsNmea::gps->getLastDate() );
  QString time  = formatTime( GpsNmea::gps->getLastTime() );

  QString coPilot            = "UNKNOWN";
  Glider::seat gliderSeats   = Glider::singleSeater;
  QString gliderType         = "UNKNOWN";
  QString gliderRegistration = "UNKNOWN";
  QString gliderCallSign     = "UNKNOWN";

  if( calculator->glider() )
    {
      // access glider items only if glider is defined
      coPilot            = calculator->glider()->coPilot();
      gliderSeats        = calculator->glider()->seats();
      gliderType         = calculator->glider()->type();
      gliderRegistration = calculator->glider()->registration();
      gliderCallSign     = calculator->glider()->callSign();
    }

  _stream << "AXXXCUM Cumulus soaring flight computer, Flight: " << flightNumber << "\r\n" ;
  _stream << "HFDTE" << date << "\r\n";
  _stream << "HFFXA500" << "\r\n";
  _stream << "HFPLTPILOTINCHARGE: " << (pilot.isEmpty() ? "Unknown" : pilot) << "\r\n";

  if( gliderSeats == Glider::doubleSeater )
    {
      if( coPilot == "" )
        {
          coPilot = tr( "Unknown" );
        }

      _stream << "HFCM2CREW2: " << coPilot << "\r\n";
    }

  _stream << "HFGTYGLIDERTYPE: " << gliderType << "\r\n";
  _stream << "HFGIDGLIDERID: " << gliderRegistration << "\r\n";
  _stream << "HFDTM100GPSDATUM: WSG-1984\r\n";
  _stream << "HFRFWFIRMWAREVERION: " << QCoreApplication::applicationVersion() << "\r\n";
  _stream << "HFRHWHARDWAREVERSION: " << HwInfo::instance()->getTypeString() << "\r\n" ;
  _stream << "HFFTYFRTYPE: Cumulus Version: " << QCoreApplication::applicationVersion()
          << ", Qt/X11 Version: " << qVersion() << "\r\n";
  _stream << "HFGPS: UNKNOWN\r\n";
  _stream << "HFPRSPRESSALTSENSOR: UNKNOWN\r\n";
  _stream << "HSCIDCOMPETITIONID: " << gliderCallSign << "\r\n";

  // GSP info lines committed for now
  // additional data (competition ID and class name) committed for now
  _stream << "I023638FXA3940SIU\r\n"; // Fix accuracy and sat count as add ons

  // Write J Record definitions, if extended logging is activated by the user.
  if( conf->getKRecordInterval() > 0 )
    {
      // Set extended logging flag used for writing of K record.
      _kRecordLogging = true;
      _stream << "J050810HDT1116TAS1719WDI2022WSP2329VAT" << "\r\n";
    }
  else
    {
      _kRecordLogging = false;
    }

  // Task support: C-Records
  extern MapContents* _globalMapContents;

  FlightTask* task = _globalMapContents->getCurrentTask();

  if ( ! task )
    {
      return; // no task active
    }

  QList<TaskPoint *> tpList = task->getTpList();

  if ( tpList.count() < 4 )
    {
      return; // too less task points
    }

  QString taskDate = formatDate( task->getDeclarationDateTime().date() );
  QString taskTime = formatTime( task->getDeclarationDateTime().time() );
  QString fnr; fnr = fnr.sprintf( "%04d", flightNumber );
  QString tpnr; tpnr = tpnr.sprintf( "%02d ", tpList.count() - 4 );
  QString taskId = task->getTaskTypeString();

  // date, time UTC is expected at first and second position
  _stream << "C"
          << taskDate
          << taskTime
          << QDate::currentDate().toString("ddMMyy")
          << fnr
          << tpnr
          << task->getTaskDistanceString() << " "
          << taskId
          << "\r\n";

  for ( int i=0; i < tpList.count(); i++ )
    {
      TaskPoint *tp = tpList.at(i);

      _stream << "C"
              << formatPosition( tp->origP )
              << tp->name << "\r\n";
    }
}

/** This function formats a date in the correct IGC format DDMMYY */
QString IgcLogger::formatDate(const QDate& date)
{
  QString result;
  result.sprintf("%02d%02d%02d",date.day(),date.month(),date.year()-2000);
  return result;
}

/** This slot is called to start or end a log. */
void IgcLogger::slotToggleLogging()
{
  // qDebug("toggle logging!");
  if ( _logMode == on )
    {
      QMessageBox mb( QMessageBox::Question,
                      tr( "Stop Logging?" ),
                      tr("<html>Are you sure you want<br>stop logging?</html>"),
                      QMessageBox::Yes | QMessageBox::No,
                      QApplication::desktop() );

      mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

      mb.show();
      QPoint pos = QApplication::desktop()->mapToGlobal( QPoint( QApplication::desktop()->width()/2 - mb.width()/2,
                                                                 QApplication::desktop()->height()/2 - mb.height()/2 ) );
      mb.move( pos );

#endif

      if( mb.exec() == QMessageBox::Yes )
        {
          // qDebug("Stopping logging...");
          Stop();
        }
    }
  else
    {
      // Logger is in mode standby or off
      int answer = QMessageBox::Yes;

      if( ! calculator->glider() )
        {
          QMessageBox mb( QMessageBox::Warning,
                          tr( "Start Logging?" ),
                          tr("<html>You should select a glider<br>before start logging.<br>Continue start logging?</html>"),
                          QMessageBox::Yes | QMessageBox::No,
                          QApplication::desktop() );

          mb.setDefaultButton( QMessageBox::No );

    #ifdef ANDROID

          mb.show();
          QPoint pos = QApplication::desktop()->mapToGlobal( QPoint( QApplication::desktop()->width()/2 - mb.width()/2,
                                                                     QApplication::desktop()->height()/2 - mb.height()/2 ) );
          mb.move( pos );

    #endif

          answer = mb.exec();
        }

      if( answer == QMessageBox::Yes )
        {
          _logMode = on;
        }
    }

  // emit the logging state in all cases to allow update of actions in MainWindow
  emit logging(getIsLogging());
}

/**
 * This slot is called, if a new task has been selected.
 */
void IgcLogger::slotNewTaskSelected()
{
  if( ! _logfile.isOpen() )
    {
      // Logger does not run, ignore this call.
      return;
    }

  QMessageBox mb( QMessageBox::Warning,
                  tr( "Restart Logging?" ),
                  tr("<html>A new flight task was selected.<br>Restart logging?</html>"),
                  QMessageBox::Yes | QMessageBox::No,
                  QApplication::desktop() );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = QApplication::desktop()->mapToGlobal( QPoint( QApplication::desktop()->width()/2 - mb.width()/2,
                                                             QApplication::desktop()->height()/2 - mb.height()/2 ) );
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::Yes )
    {
      // qDebug("Restarting logging...");
      Stop();
      _logMode = on;
      emit logging(getIsLogging());
    }
}

/** This slot is called to indicate that a new satellite constellation is now in use. */
void IgcLogger::slotConstellation( SatInfo& newConstellation )
{
  // qDebug("IgcLogger::slotConstellation()");
  makeSatConstEntry( newConstellation.constellationTime );
}

/** Makes a fix entry in the log file by using the passed time. */
void IgcLogger::makeSatConstEntry(const QTime &time)
{
  if( _logMode == on )
    {
      if( ! lastLoggedFRecord->isNull() && lastLoggedFRecord->elapsed() < 5*60*1000 )
        {
          // According to ICG Specification F-records should not updated at intervals
          // of less than 5 minutes.
          return;
        }

      QString entry = "F" +
                       formatTime( time ) +
                       GpsNmea::gps->getLastSatInfo().constellation;

      if( isLogFileOpen() )
        {
          _stream << entry << "\r\n";
          emit madeEntry();
        }

      lastLoggedFRecord->start();
    }
}

/** This function formats a QTime to the correct format for IGC files (HHMMSS) */
QString IgcLogger::formatTime(const QTime& time)
{
  QString result;
  result.sprintf("%02d%02d%02d",time.hour(),time.minute(),time.second());
  return result;
}

/**
 * This function formats the variometer speed value in meters with sign +/-,
 * 3 numbers with 3 decimal numbers.
 */
QString IgcLogger::formatVario(const Speed vSpeed)
{
  QString result;

  double mps = vSpeed.getMps();

  if( mps == 0.0 )
    {
      result += " ";
    }
  else if( mps > 0.0 )
    {
      result += "+";
    }

  // We need a number with 6 digits and without a decimal point. Therefore
  // the value meter per second is multiplied with 1000.
  result += QString("%1").arg( (int) rint(mps*1000.0), 6, 10, QChar('0') );

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

  calc = position.x(); // Latitude

  if( calc < 0 )
    {
      calc = -calc; // use positive values for now;
      latmark = "S";
    }
  else
    {
      latmark = "N";
    }

  latdeg = calc / 600000; // calculate degrees
  calc  -= latdeg * 600000; // subtract the whole degrees part
  latmin = calc / 10; // we need the minutes in 1000'st of a minute, not in 10.000'st.


  calc = position.y(); //longitude

  if( calc < 0 )
    {
      calc = -calc; //use positive values for now;
      lonmark = "W";
    }
  else
    {
      lonmark = "E";
    }

  londeg = calc / 600000; //calculate degrees
  calc -= londeg * 600000; //subtract the whole degrees part
  lonmin = calc / 10; //we need the minutes in 1000'st of a minute, not in 10.000'st.

  result.sprintf("%02d%05d%1s%03d%05d%1s",latdeg,latmin,latmark.toLatin1().data(),londeg,lonmin,lonmark.toLatin1().data());
  return result;
}

/** Creates a new filename for the IGC file according to the IGC standards (IGC GNSS FR Specification, may 2002, Section 2.5) YMDCXXXF.IGC */
QString IgcLogger::createFileName(const QString& path)
{
  int year=QDate::currentDate().year();
  year=year % 10;
  int month = QDate::currentDate().month();
  int day = QDate::currentDate().day();
  QString name = QString::number(year, 10) + QString::number(month, 13) + QString::number(day, 32);
  name += "X000"; // @AP: one X has to be added for unknown manufacture

  int i=1;
  QString result=name + QString::number(i, 36);

  while ( QFile(path + "/" + result.toUpper() + ".IGC").exists() )
    {
      i++;
      result = name + QString::number(i, 36);
    }

  flightNumber = i; //store the resulting number so we can use it in the logfile itself

  return path + "/" + result.toUpper() + ".IGC";
}

void IgcLogger::slotFlightModeChanged( Calculator::FlightMode newFlightMode )
{
  _flightMode = newFlightMode;

  if( GeneralConfig::instance()->getLoggerAutostartMode() == false )
    {
      // Logger auto start mode not active.
      return;
    }

  if( newFlightMode == Calculator::standstill || newFlightMode == Calculator::unknown )
    {
      // Close logfile after 90s still stand or unknown mode.
      closeTimer->start(90000);
    }
  else
    {
      closeTimer->stop();
    }
}

void IgcLogger::slotCloseLogFile()
{
  if( GeneralConfig::instance()->getLoggerAutostartMode() )
    {
      Standby();
    }
}
