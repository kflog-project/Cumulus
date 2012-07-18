/***********************************************************************
 **
 **   preflightflarmpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "calculator.h"
#include "gpsnmea.h"
#include "generalconfig.h"
#include "mainwindow.h"
#include "preflightflarmpage.h"

PreFlightFlarmPage::PreFlightFlarmPage(FlightTask* ftask, QWidget *parent) :
  QWidget(parent),
  m_ftask(ftask)
{
  setObjectName("PreFlightFlarmPage");
  setWindowTitle(tr("Flarm IGC Setup"));
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( _globalMainWindow )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  QVBoxLayout *allLayout  = new QVBoxLayout(this);
  QGridLayout* gridLayout = new QGridLayout;

  gridLayout->addWidget( new QLabel(tr("Id:")), 0, 0 );
  flarmId = new QLabel("???");
  gridLayout->addWidget( flarmId, 0, 1 );
  gridLayout->setColumnMinimumWidth( 2, 20 );

  gridLayout->addWidget( new QLabel(tr("Status:")), 0, 3);
  errSeverity = new QLabel("???");
  gridLayout->addWidget( errSeverity, 0, 4 );
  gridLayout->setColumnMinimumWidth( 5, 20 );

  gridLayout->addWidget( new QLabel(tr("Error:")), 0, 6);
  errCode = new QLabel("???");
  gridLayout->addWidget( errCode, 0, 7 );

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("Hw:")), 1, 0);
  hwVersion = new QLabel("???");
  gridLayout->addWidget( hwVersion, 1, 1 );
  gridLayout->setColumnMinimumWidth( 2, 20 );

  gridLayout->addWidget( new QLabel(tr("Sw:")), 1, 3);
  swVersion = new QLabel("???");
  gridLayout->addWidget( swVersion, 1, 4 );
  gridLayout->setColumnMinimumWidth( 5, 20 );

  gridLayout->addWidget( new QLabel(tr("Db:")), 1, 6);
  obstVersion = new QLabel("???");
  gridLayout->addWidget( obstVersion, 1, 7 );

  gridLayout->setColumnStretch( 8, 10 );
  allLayout->addLayout(gridLayout );

  //----------------------------------------------------------------------------

  QHBoxLayout* lineLayout = new QHBoxLayout;

  lineLayout->addWidget( new QLabel(tr("Log Interval:")));
  logInt = new QComboBox;
  QStringList list;
  list << "?" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8";
  logInt->addItems(list);
  logInt->setCurrentIndex ( 0 );
  lineLayout->addWidget( logInt );
  lineLayout->addStretch( 10 );

  allLayout->addLayout(lineLayout );

  //----------------------------------------------------------------------------

  gridLayout = new QGridLayout;
  int row = 0;

  gridLayout->addWidget( new QLabel(tr("Pilot:")), row, 0);
  pilot = new QLineEdit;
  gridLayout->addWidget( pilot, row, 1 );

  gridLayout->addWidget( new QLabel(tr("Co-Pilot:")), row, 2);
  copil = new QLineEdit;
  gridLayout->addWidget( copil, row, 3 );
  row++;

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("Glider Id:")), row, 0);
  gliderId = new QLineEdit;
  gridLayout->addWidget( gliderId, row, 1 );

  gridLayout->addWidget( new QLabel(tr("Glider Type:")), row, 2);
  gliderType = new QLineEdit;
  gridLayout->addWidget( gliderType, row, 3 );
  row++;

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("Comp Id:")), row, 0 );
  compId = new QLineEdit;
  gridLayout->addWidget( compId, row, 1 );

  gridLayout->addWidget( new QLabel(tr("Comp Class:")), row, 2);
  compClass = new QLineEdit;
  gridLayout->addWidget( compClass, row, 3 );
  row++;

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("Task:")), row, 0);
  task = new QLabel;
  gridLayout->addWidget( task, row, 1 );

  if( m_ftask )
    {
      task->setText( m_ftask->getTaskName() );
    }
  else
    {
      task->setText( tr("none") );
    }

  allLayout->addLayout(gridLayout );
  allLayout->addStretch( 10 );

  //----------------------------------------------------------------------------

  // All buttons are put into a hbox.
  QHBoxLayout* hbbox = new QHBoxLayout;

  QPushButton* cmd = new QPushButton(tr("Read"), this);
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), this, SLOT(slotRequestFlarmData()));

  hbbox->addSpacing( 10 );

  cmd = new QPushButton(tr("Set"), this);
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), this, SLOT(slotSetIgcData()));

  hbbox->addSpacing( 10 );

  cmd = new QPushButton(tr("Write"), this);
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), this, SLOT(slotWriteFlarmData()));

  hbbox->addSpacing( 10 );

  cmd = new QPushButton(tr("Close"), this);
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), this, SLOT(close()));

  allLayout->addLayout( hbbox );
  //----------------------------------------------------------------------------

  connect( Flarm::instance(), SIGNAL(flarmVersionInfo(const Flarm::FlarmVersion&)),
            this, SLOT(slotUpdateVersions(const Flarm::FlarmVersion&)) );

  connect( Flarm::instance(), SIGNAL(flarmErrorInfo( const Flarm::FlarmError&)),
            this, SLOT(slotUpdateErrors(const Flarm::FlarmError&)) );

  connect( Flarm::instance(), SIGNAL(flarmConfigurationInfo( const QStringList&)),
            this, SLOT(slotUpdateConfiguration( const QStringList&)) );

  loadFlarmData();
  slotRequestFlarmData();
}

PreFlightFlarmPage::~PreFlightFlarmPage()
{
}

void PreFlightFlarmPage::loadFlarmData()
{
  const Flarm::FlarmVersion& version = Flarm::instance()->getFlarmVersion();
  const Flarm::FlarmError&     error = Flarm::instance()->getFlarmError();

  version.hwVersion.isEmpty() ? hwVersion->setText("???") : hwVersion->setText(version.hwVersion);
  version.swVersion.isEmpty() ? swVersion->setText("???") : swVersion->setText(version.swVersion);
  version.obstVersion.isEmpty() ? obstVersion->setText("???") : obstVersion->setText(version.obstVersion);

  error.severity.isEmpty() ? errSeverity->setText("???") : errSeverity->setText(error.severity);
  error.errorCode.isEmpty() ? errCode->setText("???") : errCode->setText(error.errorCode);
}

void PreFlightFlarmPage::slotSetIgcData()
{
  GeneralConfig *conf = GeneralConfig::instance();

  pilot->setText( conf->getSurname() );

  extern Calculator* calculator;

  Glider* glider = calculator->glider();

  if( glider == static_cast<Glider *> (0) )
    {
      return;
    }

  copil->setText( glider->coPilot() );
  gliderId->setText( glider->registration() );
  gliderType->setText( glider->type() );
  compId->setText( glider->callSign() );
}

void PreFlightFlarmPage::slotRequestFlarmData()
{
  // Check, if Flarm is connected. In this case there should be available
  // some Flarm status data.
  const Flarm::FlarmStatus& status = Flarm::instance()->getFlarmStatus();

  if( status.valid != true )
    {
      qDebug() << "PFFP::slotRequestFlarmData: No Flarm device is connected!";
      return;
    }

  // Flarm is asked to report some data. The results are reported via signals.
  GpsNmea::gps->sendSentence( "$PFLAE,R" ); // Error status
  GpsNmea::gps->sendSentence( "$PFLAV,R" ); // Versions

  // Different configuration items.
  GpsNmea::gps->sendSentence( "$PFLAC,R,ID" );
  GpsNmea::gps->sendSentence( "$PFLAC,R,LOGINT" );
  GpsNmea::gps->sendSentence( "$PFLAC,R,PILOT" );
  GpsNmea::gps->sendSentence( "$PFLAC,R,COPIL" );
  GpsNmea::gps->sendSentence( "$PFLAC,R,GLIDERID" );
  GpsNmea::gps->sendSentence( "$PFLAC,R,GLIDERTYPE" );
  GpsNmea::gps->sendSentence( "$PFLAC,R,COMPID" );
  GpsNmea::gps->sendSentence( "$PFLAC,R,COMPCLASS" );
}

void PreFlightFlarmPage::slotUpdateVersions( const Flarm::FlarmVersion& info )
{
  hwVersion->setText( info.hwVersion);
  swVersion->setText( info.swVersion);
  obstVersion->setText( info.obstVersion);
}

void PreFlightFlarmPage::slotUpdateErrors( const Flarm::FlarmError& info )
{
  errSeverity->setText( info.severity);
  errCode->setText( info.errorCode);
}

void PreFlightFlarmPage::slotUpdateConfiguration( const QStringList& info )
{
  /**
   * The complete received $PFLAC sentence is the input here.
   *
   * PFLAC,<QueryType>,<Key>,<Value>
   * Attention, response can be "$PFLAC,A,ERROR*"
   */

  if( info[2] == "ERROR" )
    {
      qWarning() << "$PFLAC reported an Error!" << info.join(",");
      return;
    }

  if( info[2] == "ID" )
    {
      flarmId->setText( info[3] );
      return;
    }

  if( info[2] == "LOGINT" )
    {
      bool ok;
      int idx = info[3].toInt(&ok);

      if( ok && idx > 0 && idx < 9 )
        {
          logInt->setCurrentIndex( idx );
        }
      else
        {
          logInt->setCurrentIndex( 0 );
        }

      return;
    }

  if( info[2] == "PILOT" )
    {
      pilot->setText( info[3] );
      return;
    }

  if( info[2] == "COPIL" )
    {
      copil->setText( info[3] );
      return;
    }

  if( info[2] == "GLIDERID" )
    {
      gliderId->setText( info[3] );
      return;
    }

  if( info[2] == "GLIDERTYPE" )
    {
      gliderType->setText( info[3] );
      return;
    }

  if( info[2] == "COMPID" )
    {
      compId->setText( info[3] );
      return;
    }

  if( info[2] == "COMPCLASS" )
    {
      compClass->setText( info[3] );
      return;
    }

  qWarning() << "PFFP::slotUpdateConfiguration:"
              << "Key=" << info[2]
              << "Value=" << info[3]
              << "not processed!";
}

/** Sends all IGC data to the Flarm. */
void PreFlightFlarmPage::slotWriteFlarmData()
{
  // Check, if Flarm is connected. In this case there should be available
  // some Flarm status data.
  const Flarm::FlarmStatus& status = Flarm::instance()->getFlarmStatus();

  if( status.valid != true )
    {
      qDebug() << "PFFP::slotWriteFlarmData: no Flarm device is connected!";
      return;
    }

  if( logInt->currentIndex() > 0 )
    {
      GpsNmea::gps->sendSentence( "$PFLAC,S,LOGINT," + logInt->currentText() );
    }

  GpsNmea::gps->sendSentence( "$PFLAC,S,PILOT," + pilot->text() );
  GpsNmea::gps->sendSentence( "$PFLAC,S,COPIL," + copil->text() );
  GpsNmea::gps->sendSentence( "$PFLAC,S,GLIDERID," + gliderId->text() );
  GpsNmea::gps->sendSentence( "$PFLAC,S,GLIDERTYPE," + gliderType->text() );
  GpsNmea::gps->sendSentence( "$PFLAC,S,COMPID," + compId->text() );
  GpsNmea::gps->sendSentence( "$PFLAC,S,COMPCLASS," + compClass->text() );

  if( m_ftask == 0 )
    {
      GpsNmea::gps->sendSentence( "$PFLAC,S,NEWTASK," );
      return;
    }

  QList<TaskPoint *>& tpList = m_ftask->getTpList();

  if( tpList.isEmpty() )
    {
      GpsNmea::gps->sendSentence( "$PFLAC,S,NEWTASK," );
      return;
    }

  GpsNmea::gps->sendSentence( "$PFLAC,S,NEWTASK," + m_ftask->getTaskName());

  for( int i = 0; i < tpList.count(); i++ )
    {
      // $PFLAC,S,ADDWP,4647900N,01252700E,Lienz Ni
      TaskPoint* tp = tpList.at(i);

      int degree, intMin;
      double min;

      WGSPoint::calcPos( tp->origP.x(), degree, min );

      // Minute is expected as 1/1000
      intMin = static_cast<int> (rint(min * 1000));

      QString lat = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 2, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("S") : QString("N") );

      WGSPoint::calcPos( tp->origP.y(), degree, min );

      intMin = static_cast<int> (rint(min * 1000));

      QString lon = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 3, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("W") : QString("E") );

      QString cmd = "$PFLAC,S,ADDWP,"
                    + lat
                    + "," + lon + ","
                    + tp->name + " - " + tp->description;

      GpsNmea::gps->sendSentence( cmd );
    }
}
