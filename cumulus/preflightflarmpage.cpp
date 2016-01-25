/***********************************************************************
 **
 **   preflightflarmpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2012-2016 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "calculator.h"
#include "CuLabel.h"
#include "flarmbase.h"
#include "flighttask.h"
#include "gpsnmea.h"
#include "generalconfig.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "preflightflarmpage.h"
#include "preflighttaskpage.h"
#include "taskfilemanager.h"
#include "varspinbox.h"

extern MapContents* _globalMapContents;
extern Calculator*  calculator;

// Timeout in ms for waiting for response
#define RESP_TO 10000

// Define a retry value for command sending after error or timeout
#define RETRY_AFTER_ERROR 3

PreFlightFlarmPage::PreFlightFlarmPage(QWidget *parent) :
  QWidget(parent),
  m_cmdIdx(0),
  m_errorReportCounter(0),
  m_taskUploadRunning(false)
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
  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // Add scroll area to an own layout
  sal->addWidget( sa );

  // Pass scroll area layout to the parent widget.
  setLayout( sal );

  QVBoxLayout *allLayout  = new QVBoxLayout(form);
  QGridLayout* gridLayout = new QGridLayout;

  gridLayout->addWidget( new QLabel(tr("SN:")), 0, 0 );
  serial = new QLabel("???");
  gridLayout->addWidget( serial, 0, 1 );
  gridLayout->setColumnMinimumWidth( 2, 20 );

  gridLayout->addWidget( new QLabel(tr("RId:")), 0, 3 );
  radioId = new QLabel("???");
  gridLayout->addWidget( radioId, 0, 4 );
  gridLayout->setColumnMinimumWidth( 5, 20 );

  gridLayout->addWidget( new QLabel(tr("Sv:")), 0, 6);
  errSeverity = new QLabel("???", this);
  gridLayout->addWidget( errSeverity, 0, 7 );
  gridLayout->setColumnMinimumWidth( 8, 20 );

  gridLayout->addWidget( new QLabel(tr("Err:")), 0, 9);
  errCode = new CuLabel("???", this);
  errCode->setFrameStyle(QFrame::Box | QFrame::Panel);
  errCode->setLineWidth(3);
  gridLayout->addWidget( errCode, 0, 10 );

  connect( errCode, SIGNAL(mousePress()), SLOT(slotShowErrorText()) );

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("ODB:")), 1, 0);
  obstVersion = new QLabel("???");
  gridLayout->addWidget( obstVersion, 1, 1 );
  gridLayout->setColumnMinimumWidth( 2, 20 );

  gridLayout->addWidget( new QLabel(tr("Hw:")), 1, 3);
  hwVersion = new QLabel("???");
  gridLayout->addWidget( hwVersion, 1, 4 );
  gridLayout->setColumnMinimumWidth( 5, 20 );

  gridLayout->addWidget( new QLabel(tr("Sw:")), 1, 6);
  swVersion = new QLabel("???");
  gridLayout->addWidget( swVersion, 1, 7 );
  gridLayout->setColumnMinimumWidth( 8, 20 );

  gridLayout->addWidget( new QLabel(tr("Igc:")), 1, 9);
  igcVersion = new QLabel("???");
  gridLayout->addWidget( igcVersion, 1, 10 );

  gridLayout->setColumnStretch( 11, 10 );

  QGroupBox *dataBox = new QGroupBox(tr("Flarm Data"));
  dataBox->setLayout( gridLayout );

  allLayout->addWidget(dataBox );
  allLayout->addSpacing( 10 );

  //----------------------------------------------------------------------------

  QHBoxLayout* lineLayout = new QHBoxLayout;
  lineLayout->setSpacing( 10 );

  lineLayout->addWidget( new QLabel(tr("LogInt:")));
  logInt = new QSpinBox;
  logInt->setRange(0, 8);
  logInt->setSingleStep(1);
  logInt->setSuffix("s");
  logInt->setSpecialValueText("?");
  logInt->setValue(0);
  VarSpinBox* vli = new VarSpinBox(logInt);
  lineLayout->addWidget( vli );

  lineLayout->addWidget( new QLabel(tr("Priv:")));
  priv = new QPushButton("?");
  lineLayout->addWidget( priv );

  connect( priv, SIGNAL(pressed()), SLOT(slotChangePrivMode()) );

  lineLayout->addWidget( new QLabel(tr("NoTrk:")));
  notrack = new QPushButton("?");
  lineLayout->addWidget( notrack );

  connect( notrack, SIGNAL(pressed()), SLOT(slotChangeNotrackMode()) );

  lineLayout->addStretch( 10 );

  allLayout->addLayout(lineLayout );
  allLayout->addSpacing( 10 );

  //----------------------------------------------------------------------------
  Qt::InputMethodHints imh;
  gridLayout = new QGridLayout;
  int row = 0;

  gridLayout->addWidget( new QLabel(tr("Pilot:")), row, 0);
  pilot = new QLineEdit;
  imh = Qt::ImhNoPredictiveText;
  pilot->setInputMethodHints(imh);

  connect( pilot, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  gridLayout->addWidget( pilot, row, 1 );

  gridLayout->addWidget( new QLabel(tr("Co-Pilot:")), row, 2);
  copil = new QLineEdit;
  copil->setInputMethodHints(imh);

  connect( copil, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  gridLayout->addWidget( copil, row, 3 );
  row++;

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("Glider Id:")), row, 0);
  gliderId = new QLineEdit;
  gliderId->setInputMethodHints(imh);

  connect( gliderId, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  gridLayout->addWidget( gliderId, row, 1 );

  gridLayout->addWidget( new QLabel(tr("Glider Ty:")), row, 2);
  gliderType = new QLineEdit;
  gliderType->setInputMethodHints(imh);

  connect( gliderType, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  gridLayout->addWidget( gliderType, row, 3 );
  row++;

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("Comp Id:")), row, 0 );
  compId = new QLineEdit;
  compId->setInputMethodHints(imh);

  connect( compId, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  gridLayout->addWidget( compId, row, 1 );

  gridLayout->addWidget( new QLabel(tr("Comp Cl:")), row, 2);
  compClass = new QLineEdit;
  compClass->setInputMethodHints(imh);

  connect( compClass, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  gridLayout->addWidget( compClass, row, 3 );
  row++;

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("Task:")), row, 0);
  flarmTask = new QLabel;

  gridLayout->addWidget( flarmTask, row, 1 );

  taskBox = new QComboBox;
  taskBox->setEditable(false);
  taskBox->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);

#ifdef ANDROID
  QAbstractItemView* iv = taskBox->view();
  QScrollBar* ivsb = iv->verticalScrollBar();
  ivsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

  gridLayout->addWidget( taskBox, row, 2, 1, 2 );

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

  connect( Flarm::instance(), SIGNAL(flarmConfigurationInfo( QStringList&)),
            this, SLOT(slotUpdateConfiguration( QStringList&)) );

  connect( Flarm::instance(), SIGNAL(flarmError( QStringList&)),
            this, SLOT(slotReportError( QStringList&)) );

  // Timer for command time supervision
  m_timer = new QTimer( this );
  m_timer->setSingleShot( true );
  m_timer->setInterval( RESP_TO );

  connect( m_timer, SIGNAL(timeout()), SLOT(slotTimeout()));

  // Load available Flarm data.
  loadFlarmData();

  // Load task selection box with available tasks
  TaskFileManager tfm;
  QStringList sl = tfm.getTaskListNames();

  if( sl.isEmpty() )
    {
      // Don't show a box, if no data are available.
      taskBox->hide();
    }
  else
    {
      // Add an empty field at the beginning of the list.
      sl.prepend("");
      taskBox->addItems( sl );

      // Select the last saved task.
      QString lastTask = GeneralConfig::instance()->getCurrentTaskName();

      int index = taskBox->findText(lastTask );

      if( index != -1 )
	{
	  taskBox->setCurrentIndex( index );
	}
    }

  // Check, if a Gps is connected. We try to consider that as a Flarm device.
  // But that must not be true!
  if( GpsNmea::gps->getConnected() == true )
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
  version.igcVersion.isEmpty() ? igcVersion->setText("???") : igcVersion->setText(version.igcVersion);
  version.serial.isEmpty() ? serial->setText("???") : serial->setText(version.serial);
  version.radioId.isEmpty() ? radioId->setText("???") : radioId->setText(version.radioId);

  error.severity.isEmpty() ? errSeverity->setText("???") : errSeverity->setText(error.severity);
  error.errorCode.isEmpty() ? errCode->setText("???") : errCode->setText(error.errorCode);
}

void PreFlightFlarmPage::slotSetIgcData()
{
  GeneralConfig *conf = GeneralConfig::instance();

  pilot->setText( conf->getSurname() );

  Glider* glider = calculator->glider();

  if( glider == static_cast<Glider *> (0) )
    {
      return;
    }

  copil->setText( glider->coPilot() );
  gliderId->setText( glider->registration() );
  gliderType->setText( glider->type() );
  compId->setText( glider->callSign() );

  // Select the last saved task.
  QString lastTask = conf->getCurrentTaskName();

  int index = taskBox->findText( lastTask );

  if( index != -1 )
    {
      taskBox->setCurrentIndex( index );
    }
  else
    {
      if( taskBox->count() > 0 )
	{
	  taskBox->setCurrentIndex( 0 );
	}
    }
}

void PreFlightFlarmPage::slotClearIgcData()
{
  clearUserInputFields();
  flarmTask->clear();

  if( taskBox->count() > 0 )
    {
      taskBox->setCurrentIndex( 0 );
    }
}

void PreFlightFlarmPage::clearUserInputFields()
{
  logInt->setValue( 0 );
  priv->setText( "?" );
  notrack->setText( "?" );
  pilot->clear();
  copil->clear();
  gliderId->clear();
  gliderType->clear();
  compId->clear();
  compClass->clear();
}

void PreFlightFlarmPage::slotRequestFlarmData()
{
  // It is not clear, if a Flarm device is connected. We send out
  // some requests to check that. If no answer is reported in a certain
  // time the supervision timer will reset this request.
  // It seems there are some new Flarm commands available, which are not listed
  // in the current DataPortSpec paper 6.00.
  // http://www.flarm.com/support/manual/developer/dataport_spec_6.00_addendum.txt
  //
  // Check, if a Gps is connected, we try to consider it as a Flarm device.
  // But that must not be always true!
  if( GpsNmea::gps->getConnected() != true )
    {
      QString text0 = tr("Flarm device not reachable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  // Disable button pressing.
  enableButtons( false );

  // Clear all user input fields to see if something is coming in.
  clearUserInputFields();
  m_cmdIdx = 0;
  m_cmdList.clear();
  m_errorReportCounter = 0;

  // Here we set NMEA output and a range of 25500m. All other set items are
  // untouched.
  m_cmdList << "$PFLAC,S,NMEAOUT,71"
	    << "$PFLAC,S,RANGE,25500"
	    << "$PFLAE,R"
            << "$PFLAV,R"
            << "$PFLAC,R,RADIOID"
            << "$PFLAC,R,IGCSER"
            << "$PFLAC,R,SER"
            << "$PFLAC,R,LOGINT"
            << "$PFLAC,R,PRIV"
	    << "$PFLAC,R,NOTRACK"
            << "$PFLAC,R,PILOT"
            << "$PFLAC,R,COPIL"
            << "$PFLAC,R,GLIDERID"
            << "$PFLAC,R,GLIDERTYPE"
            << "$PFLAC,R,COMPID"
            << "$PFLAC,R,COMPCLASS";

  nextFlarmCommand();
}

void PreFlightFlarmPage::nextFlarmCommand()
{
   if( m_cmdIdx >= m_cmdList.size() )
    {
       bool noticeUser = m_taskUploadRunning;

       closeFlarmDataTransfer();

      // nothing more to send
      if( noticeUser == true )
	{
	  QApplication::restoreOverrideCursor();
	  m_taskUploadRunning = false;

	  // Ask the user for reboot.
	  ask4RebootFlarm();
	}

      return;
    }

   QByteArray ba = FlarmBase::replaceUmlauts(m_cmdList.at(m_cmdIdx).toLatin1());

   qDebug() << "Flarm $Command:" << ba;

   bool res = GpsNmea::gps->sendSentence( ba );

   m_cmdIdx++;
   m_timer->start();

  if( res == false )
    {
      closeFlarmDataTransfer();
      QString text0 = tr("Flarm device not reachable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
    }
}

/**
 * The Flarm firmware update 6.x generates this new error. It seems, that the
 * Classic Flarm device has problems with the UART receiver performance, so that
 * UART checksum errors occur.
 */
void PreFlightFlarmPage::slotReportError( QStringList& info )
{
  if( info[0] != "ERROR" && info.size() < 2 )
    {
      return;
    }

  qWarning() << "Flarm $ERROR: " << info.join(",");

  if( info[1].trimmed().isEmpty() == false )
    {
      m_errorReportCounter++;

      if( m_errorReportCounter < RETRY_AFTER_ERROR && m_cmdIdx > 0 )
        {
          // Retry to send the last command after error three times.
          m_cmdIdx--;
          nextFlarmCommand();
          return;
        }

      QApplication::restoreOverrideCursor();
      enableButtons( true );
      m_timer->stop();

      // Clear the queued commands in the command list
      m_cmdList.clear();
      m_cmdIdx = 0;
      m_errorReportCounter = 0;

      QString text0 = tr("Flarm Problem");
      QString text1 = "<html>" + text0 + "<br><br>" + info.join(",") + "</html>";
      messageBox( QMessageBox::Warning, text1, text0 );
      qWarning() << "FLARM Error:" << info.join(",");
      return;
    }
}

void PreFlightFlarmPage::slotUpdateVersions( const Flarm::FlarmVersion& info )
{
  hwVersion->setText( info.hwVersion);
  swVersion->setText( info.swVersion);
  obstVersion->setText( info.obstVersion);
  nextFlarmCommand();
}

void PreFlightFlarmPage::slotUpdateErrors( const Flarm::FlarmError& info )
{
  errSeverity->setText( info.severity);
  errCode->setText( info.errorCode);
  nextFlarmCommand();
}

void PreFlightFlarmPage::slotUpdateConfiguration( QStringList& info )
{
  qDebug() << "Flarm $Response:" << info;

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

  if( info[2].startsWith( "ERROR(unknown command)" ) )
    {
      // Our command was unknown, we do ignore that and go on.
      nextFlarmCommand();
      return;
    }

  if( info[2].startsWith( "ERROR" ) || info[2].startsWith( "WARNING" ))
    {
      closeFlarmDataTransfer();
      QString text0 = tr("Flarm Problem");
      QString text1 = "<html>" + text0 + "<br><br>" + info.join(",") + "</html>";
      messageBox( QMessageBox::Warning, text1, text0 );
      qWarning() << "$PFLAC error!" << info.join(",");
      return;
    }

  if( info.size() < 4 )
    {
      qWarning() << "$PFLAC too less parameters!" << info.join(",");
      closeFlarmDataTransfer();
      return;
    }

  // Reset error report counter after a positive answer.
  m_errorReportCounter = 0;

  if( info[2] == "NMEAOUT" || info[2] == "RANGE" )
    {
      nextFlarmCommand();
      return;
    }

  if( info[2] == "RADIOID" && info.size() >= 5 )
    {
      // $PFLAC,A,RADIOID,1,A832ED*    [ICAO ID]
      // $PFLAC,A,RADIOID,2,DE4123*    [FLARM ID]
      radioId->setText( info[4] );
      Flarm::getFlarmVersion().radioId = info[4];
      nextFlarmCommand();
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

      nextFlarmCommand();
      return;
    }

  if( info[2] == "PRIV" )
    {
      priv->setText( info[3] );
      nextFlarmCommand();
      return;
    }

  if( info[2] == "NOTRACK" )
    {
      notrack->setText( info[3] );
      nextFlarmCommand();
      return;
    }

  if( info[2] == "PILOT" )
    {
      pilot->setText( info[3] );
      nextFlarmCommand();
      return;
    }

  if( info[2] == "COPIL" )
    {
      copil->setText( info[3] );
      nextFlarmCommand();
      return;
    }

  if( info[2] == "GLIDERID" )
    {
      gliderId->setText( info[3] );
      nextFlarmCommand();
      return;
    }

  if( info[2] == "GLIDERTYPE" )
    {
      gliderType->setText( info[3] );
      nextFlarmCommand();
      return;
    }

  if( info[2] == "COMPID" )
    {
      compId->setText( info[3] );
      nextFlarmCommand();
      return;
    }

  if( info[2] == "COMPCLASS" )
    {
      compClass->setText( info[3] );
      nextFlarmCommand();
      return;
    }

  if( info[2] == "IGCSER" )
    {
      // $PFLAC,A,IGCSER,7JK*
      // $PFLAC,A,IGCSER,*               [non-IGC device]
      igcVersion->setText( info[3] );
      Flarm::getFlarmVersion().igcVersion = info[3];
      nextFlarmCommand();
      return;
    }

  if( info[2] == "SER" )
    {
      // $PFLAC,R,SER
      // Returns the device's serial number (32 bit unsigned integer).
      // Example:
      // $PFLAC,A,SER,1342*
      // $PFLAC,A,SER,1828342834*
      serial->setText( info[3] );
      Flarm::getFlarmVersion().serial = info[3];
      nextFlarmCommand();
      return;
    }

  if( info[2] == "NEWTASK" )
    {
      flarmTask->setText( info[3] );
      nextFlarmCommand();
      return;
    }

  if( info[2] ==  "ADDWP" )
    {

      nextFlarmCommand();
      return;
    }

  qWarning() << "PFFP::slotUpdateConfiguration:"
              << info.join(",")
              << "not processed!";
}

/** Sends all IGC data to the Flarm. */
void PreFlightFlarmPage::slotWriteFlarmData()
{
  // Check, if a Gps is connected, we try to consider it as a Flarm device.
  // But that must not be true!
  if( GpsNmea::gps->getConnected() != true )
    {
      QString text0 = tr("Flarm device not reachable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  enableButtons( false );
  m_cmdIdx = 0;
  m_cmdList.clear();
  m_errorReportCounter = 0;
  m_taskUploadRunning = true;

  if( logInt->value() > 0 )
    {
      m_cmdList << ("$PFLAC,S,LOGINT," + QString::number(logInt->value()));
    }

  if( priv->text() != "?" )
    {
      m_cmdList << "$PFLAC,S,PRIV," + priv->text();
    }

  if( notrack->text() != "?" )
    {
      m_cmdList << "$PFLAC,S,NOTRACK," + notrack->text();
    }

  m_cmdList << "$PFLAC,S,PILOT," + FlarmBase::replaceUmlauts( pilot->text().trimmed().toLatin1() )
            << "$PFLAC,S,COPIL," + FlarmBase::replaceUmlauts( copil->text().trimmed().toLatin1() )
            << "$PFLAC,S,GLIDERID," + FlarmBase::replaceUmlauts( gliderId->text().trimmed().toLatin1() )
            << "$PFLAC,S,GLIDERTYPE," + FlarmBase::replaceUmlauts( gliderType->text().trimmed().toLatin1() )
            << "$PFLAC,S,COMPID," + FlarmBase::replaceUmlauts( compId->text().trimmed().toLatin1() )
            << "$PFLAC,S,COMPCLASS," + FlarmBase::replaceUmlauts( compClass->text().trimmed().toLatin1() );

  if( taskBox->count() <= 1 || taskBox->isVisible() == false )
    {
      m_cmdList << "$PFLAC,S,NEWTASK,";
      nextFlarmCommand();
      return;
    }

  // A flight task is selected in the task box.
  QString tpName = taskBox->currentText().left(50);

  // Load the flight task
  TaskFileManager tfm;
  FlightTask* ft = tfm.loadTask( tpName );

  if( GeneralConfig::instance()->getCurrentTaskName() != tpName )
    {
      GeneralConfig::instance()->setCurrentTaskName( tpName );
      _globalMapContents->setCurrentTask( ft );
      emit newTaskSelected();
    }

  m_cmdList << "$PFLAC,S,NEWTASK," + tpName;

  if( ft == static_cast<FlightTask *>(0) )
    {
      nextFlarmCommand();
      return;
    }

  // Write Flarm Task file in Cumulus's data directory for control
  createFlarmTaskList( ft );

  // Flarm limits tasks in its length to 192 characters. We do check and
  // adapt that here.
  QList<TaskPoint *>& tpList = ft->getTpList();
  int left = -1;

  while( true )
    {
      int sizeDescr = ft->getTaskName().size();

      for( int i = 0; i < tpList.count(); i++ )
        {
          sizeDescr += tpList.at(i)->getWPName().left(left).size();
        }

      int total = 7 + (tpList.size() * 9) + sizeDescr;

      if( total <= 192 )
        {
          break;
        }

      if( left == -1 )
        {
          left = 8;
        }
      else
        {
          left--;

          if( left == -1 )
            {
              // Shorter is not possible, we must abort here.
              QApplication::restoreOverrideCursor();
              enableButtons( true );

              QString text0 = tr("Task Error");
              QString text1 = "<html>" + tr("Task") + " " +
                               ft->getTaskName() + " " + tr("is too long!") + "</html>";
              messageBox( QMessageBox::Warning, text1, text0 );
              qWarning() << "Task" << ft->getTaskName()
                          << "is longer as 192 characters";
              return;
            }
        }
    }

  // Takeoff point as dummy point
  m_cmdList << "$PFLAC,S,ADDWP,0000000N,00000000E,Takeoff";

  for( int i = 0; i < tpList.count(); i++ )
    {
      // $PFLAC,S,ADDWP,4647900N,01252700E,Lienz Ni
      TaskPoint* tp = tpList.at(i);

      int degree, intMin;
      double min;

      WGSPoint::calcPos( tp->getWGSPosition().x(), degree, min );

      // Minute is expected as 1/1000
      intMin = static_cast<int> (rint(min * 1000));

      QString lat = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 2, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("S") : QString("N") );

      WGSPoint::calcPos( tp->getWGSPosition().y(), degree, min );

      intMin = static_cast<int> (rint(min * 1000));

      QString lon = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 3, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("W") : QString("E") );

      QString cmd = "$PFLAC,S,ADDWP,"
                    + lat
                    + "," + lon + ","
                    + tp->getWPName().left(left).trimmed();

      m_cmdList <<  cmd;
    }

  // Landing point as dummy point
  m_cmdList << "$PFLAC,S,ADDWP,0000000N,00000000E,Landing";

  nextFlarmCommand();
}

void PreFlightFlarmPage::slotTimeout()
{
  qDebug() << "PreFlightFlarmPage::slotTimeout(): m_errorReportCounter="
           << m_errorReportCounter
           << "m_cmdIdx=" << m_cmdIdx;

  m_errorReportCounter++;

  if( m_errorReportCounter < RETRY_AFTER_ERROR && m_cmdIdx > 0 )
    {
      // Retry to send the last command after timeout three times.
      m_cmdIdx--;
      nextFlarmCommand();
      return;
    }

  closeFlarmDataTransfer();

  QString text0 = tr("Flarm device not reachable!");
  QString text1 = tr("Error");
  messageBox( QMessageBox::Warning, text0, text1 );
}

void PreFlightFlarmPage::closeFlarmDataTransfer()
{
  QApplication::restoreOverrideCursor();
  enableButtons( true );

  // Note, this method is also called in case on no timeout to enable the
  // buttons and to restore the cursor. Therefore a running timer must be
  // stopped too.
  if( m_timer->isActive() )
    {
      m_timer->stop();
    }

  // Clear the queued commands in the command list
  m_cmdList.clear();
  m_cmdIdx = 0;
  m_errorReportCounter = 0;
  m_taskUploadRunning = false;
}

void PreFlightFlarmPage::slotClose()
{
  QApplication::restoreOverrideCursor();
  m_timer->stop();
  emit closingWidget();
  close();
}

void PreFlightFlarmPage::slotChangePrivMode()
{
  if( priv->text() == "?" )
    {
      // Private mode was not set to a real value, ignore call.
      return;
    }

  // Toggle button text
  if( priv->text() == "0" )
    {
      priv->setText( "1" );
    }
  else
    {
      priv->setText( "0" );
    }
}

void PreFlightFlarmPage::slotChangeNotrackMode()
{
  if( notrack->text() == "?" )
    {
      // Notrack mode was not set to a real value, ignore call.
      return;
    }

  // Toggle button text
  if( notrack->text() == "0" )
    {
      notrack->setText( "1" );
    }
  else
    {
      notrack->setText( "0" );
    }
}

void PreFlightFlarmPage::slotShowErrorText()
{
  const Flarm::FlarmError& error = Flarm::instance()->getFlarmError();

  if( error.errorText.isEmpty() )
    {
      return;
    }

  QString title = QString(tr("Code %1 means")).arg(error.errorCode);

  QString text = "<html>" + title + ":<br><br>" + error.errorText + "</html>";

  messageBox( QMessageBox::Information, text, title );
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

void PreFlightFlarmPage::ask4RebootFlarm()
{
  QMessageBox mb( QMessageBox::Question,
		  tr( "Reboot Flarm?"),
		  tr( "To activate the new task, the Flarm must be rebooted!") +
		  QString("<p>") + tr("Execute reboot now?") + "</p>",
		  QMessageBox::Yes | QMessageBox::No,
		  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::No )
    {
      return;
    }

  // Flarm reset command
  QByteArray ba("$PFLAR,0");

  GpsNmea::gps->sendSentence( ba );
}

/** Creates a flarmTask definition file in Flarm format. */
bool PreFlightFlarmPage::createFlarmTaskList( FlightTask* flightTask )
{
  if( ! flightTask )
    {
      return false;
    }

  QList<TaskPoint *>& tpList = flightTask->getTpList();

  if( tpList.isEmpty() || tpList.size() < 2 )
    {
      return false;
    }

  QString fn = GeneralConfig::instance()->getUserDataDirectory() + "/cumulus-flarm.tsk";

  // Save one backup copy.
  if( QFileInfo(fn).exists() )
    {
      QFile::remove( fn + ".bak" );
      QFile::rename( fn, fn + ".bak" );
    }

  QFile f(fn);

  if( !f.open( QIODevice::WriteOnly ) )
    {
      qWarning( "Could not write to flarmTask-file %s", f.fileName().toLatin1().data() );
      return false;
    }

  QTextStream stream( &f );
  stream.setCodec( "ISO 8859-15" );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "// Flarm task declaration created at "
         << dtStr
         << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl;

  stream << "$PFLAC,S,NEWTASK,"
         << FlarmBase::replaceUmlauts( flightTask->getTaskName().toLatin1() )
         << endl;

  // Takeoff point as dummy point
  stream << "$PFLAC,S,ADDWP,0000000N,00000000E,Takeoff dummy" << endl;

  for( int i = 0; i < tpList.count(); i++ )
    {
      // $PFLAC,S,ADDWP,4647900N,01252700E,Lienz Ni
      TaskPoint* tp = tpList.at(i);

      int degree, intMin;
      double min;

      WGSPoint::calcPos( tp->getWGSPosition().x(), degree, min );

      // Minute is expected as 1/1000
      intMin = static_cast<int> (rint(min * 1000));

      QString lat = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 2, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("S") : QString("N") );

      WGSPoint::calcPos( tp->getWGSPosition().y(), degree, min );

      intMin = static_cast<int> (rint(min * 1000));

      QString lon = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 3, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("W") : QString("E") );

      stream << "$PFLAC,S,ADDWP,"
             << lat
             << "," << lon << ","
             << FlarmBase::replaceUmlauts( tp->getWPName().toLatin1() )
             << endl;
    }

  // Landing point as dummy point
  stream << "$PFLAC,S,ADDWP,0000000N,00000000E,Landing dummy" << endl;

  stream << endl;
  f.close();

  return true;
}
