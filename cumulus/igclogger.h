/***************************************************************************
                          igclogger.h  -  creates an IGC logfile
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002      by André Somers
                               2008-2012 by Axel Pauli

    email                : kflog.cumulus@gmail.com

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

/**
 * \class IgcLogger
 *
 * \author André Somers, Axel Pauli
 *
 * \brief IGC logger
 *
 * This class provides the IGC logging facilities, using the
 * parsed data from the GPS NMEA object.
 *
 * \date 2002-2012
 *
 * \version $Id$
 */

#ifndef IGC_LOGGER_H
#define IGC_LOGGER_H

/* info about flight logger */
#define FL_CODE X
#define FL_LCODE XXX
#define FL_NAME Cumulus
#define FL_ID ATS

#define FL_HWV UNKNOWN /* the version number of Cumulus is used as firmware version */

/* info about GPS, may in the future be used from GPS */
#define GPS_MAN UNKNOWN
#define GPS_MODEL UNKNOWN
#define GPS_CHAN UNKNOWN
#define GPS_MAXALT UNKNOWN

#include <QDateTime>
#include <QFile>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTime>

#include "altitude.h"
#include "calculator.h"
#include "limitedlist.h"

class QMutex;

class IgcLogger : public QObject
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( IgcLogger )

public:

  /**
    * Used to describe the current logging mode:
    * -off: no logging going on
    * -on: logging enabled
    * -standby: logging will be turned on when the flight status changes
    */
  enum LogMode{ off=0, standby=1, on=2 };

  /**
   * Used to store the basic data of a flight.
   */
  struct FlightData
  {
      QDateTime takeoff;
      QDateTime landing;
      QTime flightTime;
      QString pilot1;
      QString pilot2;
      QString gliderType;
      QString gliderReg;
  };

public:

  /** returns the existing singleton class */
  static IgcLogger* instance();

  /**
   * Destructor
   */
  virtual ~IgcLogger();

  /**
   * Stop logging
   */
  void Stop();

  /**
   * Switches to standby mode. If we are currently logging, the logfile will
   * be closed.
   */
  void Standby();

  /**
   * @return true if we are currently logging
   */
  bool getIsLogging() const
  {
    return ( _logMode == on );
  };

  /**
   * @return true if we are currently standby
   */
  bool getIsStandby() const
  {
    return ( _logMode == standby );
  };

  /**
   * @return Date and time object containing logger start time, if the logger
   *         works in auto start mode. If the logger is not running or the
   *         auto start mode is disabled, the returned object is invalid.
   */
  QDateTime& loggerStart()
  {
    return startLogging;
  }

  /**
   * Returns the content of the flight logbook as string list. All entries are
   * appended to the passed string list.
   *
   * \param logbook The current flight logbook as string list.
   */
  void getLogbook( QStringList& logbook );

  /**
   * Writes a flight logbook as file.
   */
  bool writeLogbook( QStringList& logbook );

public slots:
  /**
   * This slot is used internally by the timer to make a log entry on
   * an interval, but can also be used from outside the class to
   * make sure a specific point is being logged (i.e., to respond to
   * a user command to log).
   */
  void slotMakeFixEntry();

  /** Call this slot, if a task sector has been touched to increase
   *  logger interval for a certain time.
   */
  void slotTaskSectorTouched();

  /**
   * This slot is called to indicate that a new satellite constellation is
   * now in use.
   */
  void slotConstellation( SatInfo& newConstellation );

  /**
   * This slot is called to start or end a log.
   */
  void slotToggleLogging();

  /**
   * This slot is called, if a new task has been selected.
   */
  void slotNewTaskSelected();

  /**
   * This slot is called to read the logger configuration items after a modification.
   */
  void slotReadConfig();

  /**
   * This slot is called, if the flight mode has been changed.
   */
  void slotFlightModeChanged( Calculator::FlightMode newFlightMode );

private slots:

  /**
   * This slot is called to reset the logger interval after a modification.
   */
  void slotResetLoggingTime();

  /**
   * This slot is called to close the logfile after a certain timeout.
   */
  void slotCloseLogFile();

  /**
   * This slot is called when a takeoff is recognized in auto logger mode.
   * The reported date and time uses the local time zone.
   *
   * \param dt Date and time of local time zone
   */
  void slotTakeoff( QDateTime& dt );

  /**
   * This slot is called when a landing is recognized in auto logger mode.
   * The reported date and time uses the local time zone.
   *
   * \param dt Date and time of local time zone.
   */
  void slotLanded( QDateTime& dt );

signals: // Signals
  /**
   * This signal is send to indicate that logging has started
   * (isLogging is true) or has stopped (isLogging is false).
   */
  void logging(bool isLogging);

  /**
   * This signal is emitted for every fix entry that has been
   * entered in the log. It can be used to display a visual
   * indication that an entry has been made.
   */
  void madeEntry();

  /**
   * Reports take off time, if auto logger mode is enabled.
   */
  void takeoffTime( QDateTime& dt );

  /**
   * Reports landing time, if auto logger mode is enabled.
   */
  void landingTime( QDateTime& dt );

private:

  /**
   * Constructor is private because this is a singleton class.
   */
  IgcLogger(QObject* parent = static_cast<QObject *>(0) );

  /**
   * Writes a K-Record, if all conditions are true.
   */
  void writeKRecord( const QTime& timeFix );

  /**
   * Creates a log file, if it not yet already exists and writes the header items
   * into it.
   *
   * Returns true if file is ready for further writing otherwise false.
   */
  bool isLogFileOpen();

  /**
   * Closes the logfile.
   */
  void CloseFile();

  /**
   * This function formats a date in the correct igc format DDMMYY
   */
  QString formatDate(const QDate& date);

  /**
   * This function writes the header for the IGC file into the logfile.
   */
  void writeHeader();

  /**
   * Makes a fix entry in the logfile.
   */
  void makeSatConstEntry(const QTime &time);

  /**
   * This function formats an Altitude to the correct format
   * for IGC files (XXXXX)
   */
  QString formatAltitude(Altitude altitude);

  /**
   * This function formats a QTime to the correct format for igc
   * files (HHMMSS)
   */
  QString formatTime(const QTime& time);

  /**
   * This function formats the position to the correct format for
   * IGC files. Latitude and Longitude are encoded as DDMMmmmADDDMMmmmO,
   * with A=N or S and O=E or W.
   */
  QString formatPosition(const QPoint& position);

  /**
   * This function formats the variometer speed value in meters with sign +/-,
   * 3 numbers with 3 decimal numbers.
   */
  QString formatVario(const Speed vSpeed);

  /**
   * Creates a new filename for the IGC file according to the IGC
   * standards (IGC GNSS FR Specification, may 2002, Section 2.5)
   */
  QString createFileName(const QString& path);

  /**
   * Writes a new entry into the flight logbook.
   */
  bool writeLogbookEntry();

  /**
   * Creates a new header entry for the flight logbook.
   */
  QString createLogbookHeader();

  /**
   * A pointer to the singleton existing instance.
   */
  static IgcLogger* _theInstance;

  /** A timer to reset the logger interval to the default value after a modification. */
  QTimer* resetTimer;

  /** A timer for closing the logfile after a certain timeout.*/
  QTimer* closeTimer;

  /** The text stream object to write our data to. */
  QTextStream _stream;

  /** This is our log file. It is being used via the _stream object. */
  QFile _logfile;

  /** Contains the current active logging mode. */
  LogMode _logMode;

  /** B-Record logger time interval in seconds. */
  int _bRecordInterval;

  /** K-Record logger time interval in seconds. */
  int _kRecordInterval;

  /** Enable K-Record logging. */
  bool _kRecordLogging;

  /** Time stamp of the last logged B record */
  QTime* lastLoggedBRecord;

  /** Time stamp of the last logged F record */
  QTime* lastLoggedFRecord;

  /** Time stamp of the last logged K record */
  QTime* lastLoggedKRecord;

  /** Date and time of logging start. */
  QDateTime startLogging;

  /** List of last would-be log entries.
    * This list is filled when in standby mode with strings that would be
    * in the log were logging enabled. When a change in flight mode is detected
    * and logging is triggered, the list is used to write out some older events
    * to the log. This way, we can be sure that the complete start sequence is
    * available in the log. */
  LimitedList<QStringList> _backtrack;

  /** Stores the flight number for this day */
  int flightNumber;

  /** Stores the last known flight mode. */
  Calculator::FlightMode _flightMode;

  /** Stores the basic data of a flight. */
  FlightData _flightData;

  /** Mutex used for load and save of logbook file. */
  static QMutex mutex;
};

#endif
