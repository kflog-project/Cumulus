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
#include "varspinbox.h"

// Timeout in ms for waiting for response
#define RESP_TO 10000

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

  /*
    * AP: It was a longer fight to get running the widget in a scroll area. The
    * trick is, to put the scroll area into an own layout and add this layout
    * to the parent widget. The dialog form itself must be put into an own,
    * widget which is passed to the scroll area widget. That ensures that the
    * scroll area fills out the parent widget. A good example was to find here:
    *
    * http://wiki.forum.nokia.com/index.php/Shows_the_use_of_QScrollArea
    */

  // Layout used by scroll area
  QHBoxLayout *sal = new QHBoxLayout;

  // new widget as container for the dialog layout.
  QWidget* form = new QWidget(this);

  QScrollArea* sa = new QScrollArea( this );
  sa->setWidgetResizable( true );
  sa->setFrameStyle( QFrame::NoFrame );
  sa->setWidget( form );
#ifdef QSCROLLER
  QScroller::grabGesture(sa, QScroller::LeftMouseButtonGesture);
#endif

  // Add scroll area to an own layout
  sal->addWidget( sa );

  // Pass scroll area layout to the parent widget.
  setLayout( sal );

  QVBoxLayout *allLayout  = new QVBoxLayout(form);
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

  QGroupBox *dataBox = new QGroupBox(tr("Flarm Data"));
  dataBox->setLayout( gridLayout );

  allLayout->addWidget(dataBox );
  allLayout->addSpacing( 10 );

  //----------------------------------------------------------------------------

  QHBoxLayout* lineLayout = new QHBoxLayout;

  lineLayout->addWidget( new QLabel(tr("Log Interval:")));
  logInt = new QSpinBox(this);
  logInt->setRange(0, 8);
  logInt->setSingleStep(1);
  logInt->setSuffix(" s");
  logInt->setSpecialValueText("?");
  logInt->setValue(0);
  VarSpinBox* vli = new VarSpinBox(logInt);
  lineLayout->addWidget( vli );
  lineLayout->addStretch( 10 );

  allLayout->addLayout(lineLayout );
  allLayout->addSpacing( 10 );

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
  task = new QLineEdit;
  task->setReadOnly( true );
  gridLayout->addWidget( task, row, 1 );

  if( m_ftask )
    {
      task->setText( m_ftask->getTaskName() );
    }

  allLayout->addLayout(gridLayout );
  allLayout->addStretch( 10 );

  //----------------------------------------------------------------------------

  // All buttons are put into a hbox.
  QHBoxLayout* hbbox = new QHBoxLayout;

  QPushButton* cmd = new QPushButton(tr("Read"), this);
#ifndef ANDROID
  cmd->setToolTip( tr("Read data from Flarm") );
#endif
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotRequestFlarmData()));
  readButton = cmd;

  hbbox->addSpacing( 10 );

  cmd = new QPushButton(tr("Set"), this);
#ifndef ANDROID
  cmd->setToolTip( tr("Set Cumulus data") );
#endif
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotSetIgcData()));
  setButton = cmd;

  hbbox->addSpacing( 10 );

  cmd = new QPushButton(tr("Clear"), this);
#ifndef ANDROID
  cmd->setToolTip( tr("Clear input fields") );
#endif
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotClearIgcData()));
  clearButton = cmd;

  hbbox->addSpacing( 10 );
  cmd = new QPushButton(tr("Write"), this);
#ifndef ANDROID
  cmd->setToolTip( tr("Write data to Flarm") );
#endif
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotWriteFlarmData()));
  writeButton = cmd;

  hbbox->addSpacing( 10 );

  cmd = new QPushButton(tr("Close"), this);
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotClose()));

  allLayout->addLayout( hbbox );

  //----------------------------------------------------------------------------

  // Add Flarm signals to our slots to get Flarm data.
  connect( Flarm::instance(), SIGNAL(flarmVersionInfo(const Flarm::FlarmVersion&)),
            this, SLOT(slotUpdateVersions(const Flarm::FlarmVersion&)) );

  connect( Flarm::instance(), SIGNAL(flarmErrorInfo( const Flarm::FlarmError&)),
            this, SLOT(slotUpdateErrors(const Flarm::FlarmError&)) );

  connect( Flarm::instance(), SIGNAL(flarmConfigurationInfo( const QStringList&)),
            this, SLOT(slotUpdateConfiguration( const QStringList&)) );

  // Timer for command time supervision
  m_timer = new QTimer( this );
  m_timer->setSingleShot( true );
  m_timer->setInterval( RESP_TO );

  connect( m_timer, SIGNAL(timeout()), SLOT(slotTimeout()));

  // Load available Flarm data
  loadFlarmData();

  const Flarm::FlarmStatus& status = Flarm::instance()->getFlarmStatus();

  if( status.valid == true )
    {
      // Request Flarm data, if Flarm device was recognized.
      slotRequestFlarmData();
    }
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

  if( m_ftask )
    {
      task->setText( m_ftask->getTaskName() );
    }
}

void PreFlightFlarmPage::slotClearIgcData()
{
  logInt->setValue( 0 );
  pilot->clear();
  copil->clear();
  gliderId->clear();
  gliderType->clear();
  compId->clear();
  compClass->clear();
  task->clear();
}

void PreFlightFlarmPage::slotRequestFlarmData()
{
  // It is not clear, if a Flarm device is connected. We send out
  // some requests to check that. If no answer is reported in a certain
  // time the supervision timer will reset this request.
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  // Disable button pressing.
  enableButtons( false );

  bool res = true;

  // Flarm is asked to report some data. The results are reported via signals.
  res &= GpsNmea::gps->sendSentence( "$PFLAE,R" ); // Error status
  res &= GpsNmea::gps->sendSentence( "$PFLAV,R" ); // Versions

  // Activate NMEA data sending
  res &= GpsNmea::gps->sendSentence( "$PFLAC,S,NMEAOUT,1" );

  // set range to 25500m
  res &= GpsNmea::gps->sendSentence( "$PFLAC,S,RANGE,25500" );

  // Request different configuration items.
  res &= GpsNmea::gps->sendSentence( "$PFLAC,R,ID" );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,R,LOGINT" );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,R,PILOT" );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,R,COPIL" );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,R,GLIDERID" );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,R,GLIDERTYPE" );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,R,COMPID" );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,R,COMPCLASS" );

  m_timer->start();

  if( res == false )
    {
      slotTimeout();
      QString text0 = tr("Flarm device not reachable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
    }
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
  if( info[1] != "A" )
    {
      qWarning() << "PFFP::sUC: Missing query type A!"
                 << info.join(",");
      return;
    }

  if( info[2] == "ERROR" )
    {
      slotTimeout();
      QString text0 = tr("Flarm Error");
      QString text1 = "<html>" + text0 + "<br><br>" + info.join(",") + "</html>";
      messageBox( QMessageBox::Warning, text0, text1 );
      qWarning() << "$PFLAC error!" << info.join(",");
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
      int val = info[3].toInt(&ok);

      if( ok && val > 0 && val < 9 )
        {
          logInt->setValue( val );
        }
      else
        {
          logInt->setValue( 0 );
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
      // If this item is reported, we assume, that the connection to the
      // Flarm device was possible and data have been delivered.
      compClass->setText( info[3] );
      slotTimeout();
      return;
    }

  qDebug() << "PFFP::slotUpdateConfiguration:"
           << info.join(",")
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
      QString text0 = tr("No connection to the Flarm device!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );

      qDebug() << "PFFP::slotWriteFlarmData:" << text0;
      return;
    }

  if( logInt->value() > 0 )
    {
      GpsNmea::gps->sendSentence( "$PFLAC,S,LOGINT," + QString::number(logInt->value()) );
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  enableButtons( false );

  bool res = true;

  res &= GpsNmea::gps->sendSentence( "$PFLAC,S,PILOT," + pilot->text().trimmed() );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,S,COPIL," + copil->text().trimmed() );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,S,GLIDERID," + gliderId->text().trimmed() );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,S,GLIDERTYPE," + gliderType->text().trimmed() );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,S,COMPID," + compId->text().trimmed() );
  res &= GpsNmea::gps->sendSentence( "$PFLAC,S,COMPCLASS," + compClass->text().trimmed() );

  if( m_ftask == 0 )
    {
      res &= GpsNmea::gps->sendSentence( "$PFLAC,S,NEWTASK," );
      return;
    }

  QList<TaskPoint *>& tpList = m_ftask->getTpList();

  if( tpList.isEmpty() )
    {
      res &= GpsNmea::gps->sendSentence( "$PFLAC,S,NEWTASK," );
      return;
    }

  res &= GpsNmea::gps->sendSentence( "$PFLAC,S,NEWTASK," + m_ftask->getTaskName());

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

      res &= GpsNmea::gps->sendSentence( cmd );
    }

  m_timer->start();

  if( res == false )
    {
      slotTimeout();
      QString text0 = tr("Flarm device not reachable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
    }
}

void PreFlightFlarmPage::slotTimeout()
{
  QApplication::restoreOverrideCursor();
  enableButtons( true );

  // Note, this method is also called in case on no timeout to enable the
  // buttons and to restore the cursor. Therefore a running timer must be
  // stooped too.
  if( m_timer->isActive() )
    {
      m_timer->stop();
    }
  else
    {
      QString text0 = tr("No response from Flarm device!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
    }
}

void PreFlightFlarmPage::slotClose()
{
  QApplication::restoreOverrideCursor();
  m_timer->stop();
  close();
}

void PreFlightFlarmPage::enableButtons( const bool toggle )
{
  readButton->setEnabled( toggle );
  writeButton->setEnabled( toggle );
  setButton->setEnabled( toggle );
  clearButton->setEnabled( toggle );
}

/** Shows a popup message box to the user. */
void PreFlightFlarmPage::messageBox(  QMessageBox::Icon icon,
                                           QString message,
                                           QString title )
{
  QMessageBox mb( icon,
                  title,
                  message,
                  QMessageBox::Ok,
                  this );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  mb.exec();
}
