/***********************************************************************
**
**   preflightflarmpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreflightFlarmPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for pre-flight Flarm settings.
 *
 * A widget for pre-flight Flarm settings.
 *
 * \date 2012
 *
 * \version $Id$
 */

#ifndef PREFLIGHT_FLARM_PAGE_H_
#define PREFLIGHT_FLARM_PAGE_H_

#include <QWidget>

#include "flarm.h"

class QComboBox;
class QLabel;
class QLineEdit;
class FlightTask;

class PreFlightFlarmPage : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightFlarmPage )

 public:

  PreFlightFlarmPage(FlightTask* ftask=0, QWidget *parent=0);

  virtual ~PreFlightFlarmPage();

 private slots:

 /** Requests the Flarm data from the device.*/
 void slotRequestFlarmData();

 /** Sends all IGC data to the Flarm. */
 void slotWriteFlarmData();

 /** Sets all IGC data as defined in Cumulus. */
 void slotSetIgcData();

 /** Called to update version info. */
  void slotUpdateVersions( const Flarm::FlarmVersion& info );

  /** Called to update error info. */
  void slotUpdateErrors( const Flarm::FlarmError& info );

  /** Called to update configuration info. */
  void slotUpdateConfiguration( const QStringList& info );

 private:

  /** Loads the available Flarm data into the label displays. */
  void loadFlarmData();

  QLabel*    hwVersion;
  QLabel*    swVersion;
  QLabel*    obstVersion;
  QLabel*    flarmId;
  QLabel*    errSeverity;
  QLabel*    errCode;
  QComboBox* logInt;
  QLineEdit* pilot;
  QLineEdit* copil;
  QLineEdit* gliderId;
  QLineEdit* gliderType;
  QLineEdit* compId;
  QLineEdit* compClass;
  QLabel*    task;

  FlightTask* m_ftask;
};

#endif
