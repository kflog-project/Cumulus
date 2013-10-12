/***********************************************************************
 **
 **   preflightlivetrack24page.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
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

#include "generalconfig.h"
#include "layout.h"
#include "numberEditor.h"
#include "preflightlivetrack24page.h"

PreFlightLiveTrack24Page::PreFlightLiveTrack24Page(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("PreFlightLiveTrack24Page");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("PreFlight - LiveTrack24") );

  if( parent )
    {
      resize( parent->size() );
    }

  // Layout used by scroll area
  QHBoxLayout *sal = new QHBoxLayout;

  // new widget used as container for the dialog layout.
  QWidget* sw = new QWidget;

  // Scroll area
  QScrollArea* sa = new QScrollArea;
  sa->setWidgetResizable( true );
  sa->setFrameStyle( QFrame::NoFrame );
  sa->setWidget( sw );

#ifdef QSCROLLER
  QScroller::grabGesture( sa>viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // Add scroll area to its own layout
  sal->addWidget( sa );

  QHBoxLayout *contentLayout = new QHBoxLayout(this);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal, 10 );

  // Top layout's parent is the scroll widget
  QGridLayout *topLayout = new QGridLayout(sw);

  int row = 0;

  m_liveTrackEnabled = new QCheckBox(tr("LiveTrack24 on"));
  topLayout->addWidget(m_liveTrackEnabled, row, 0 );
  row++;

  QLabel *lbl = new QLabel(tr("Tracking interval (mm:ss):"));
  topLayout->addWidget(lbl, row, 0);

  QHBoxLayout* tiHbox = new QHBoxLayout;
  tiHbox->setMargin( 0 );
  topLayout->addLayout( tiHbox, row, 1 );
  row++;

  // Tracking interval minutes
  m_trackingIntervalMin = new NumberEditor;
  m_trackingIntervalMin->setDecimalVisible( false );
  m_trackingIntervalMin->setPmVisible( false );
  m_trackingIntervalMin->setRange( 0, 120 );
  m_trackingIntervalMin->setMaxLength(3);
  m_trackingIntervalMin->setSuffix( " min" );
  m_trackingIntervalMin->setValue( 0 );
  m_trackingIntervalMin->setTitle( tr("Tracking interval") );
  m_trackingIntervalMin->setTip( tr("0...120 minutes") );

  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,3})" ), this );
  m_trackingIntervalMin->setValidator( eValidator );

  int maw = QFontMetrics(font()).width("999 min") + 10;
  m_trackingIntervalMin->setMinimumWidth( maw );
  tiHbox->addWidget( m_trackingIntervalMin );
  tiHbox->addSpacing( 10 );
  tiHbox->addWidget( new QLabel(":") );
  tiHbox->addSpacing( 10 );

  // Tracking interval seconds
  m_trackingIntervalSec = new NumberEditor;
  m_trackingIntervalSec->setDecimalVisible( false );
  m_trackingIntervalSec->setPmVisible( false );
  m_trackingIntervalSec->setRange( 0, 60 );
  m_trackingIntervalSec->setMaxLength(2);
  m_trackingIntervalSec->setSuffix( " s" );
  m_trackingIntervalSec->setValue( 0 );
  m_trackingIntervalSec->setTitle( tr("Tracking interval") );
  m_trackingIntervalSec->setTip( tr("0...60 seconds") );

  eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,2})" ), this );
  m_trackingIntervalSec->setValidator( eValidator );

  maw = QFontMetrics(font()).width("99 s") + 10;
  m_trackingIntervalSec->setMinimumWidth( maw );
  tiHbox->addWidget( m_trackingIntervalSec );
  tiHbox->addStretch( 10 );

  lbl = new QLabel(tr("Airplane type:"));
  topLayout->addWidget(lbl, row, 0);
  m_airplaneType = new QComboBox;
  m_airplaneType->addItem( tr("Paraglider"), 1 );
  m_airplaneType->addItem( tr("Glider"), 8 );
  m_airplaneType->addItem( tr("Powered flight"), 64 );
  m_airplaneType->addItem( tr("Hot Air Balloon"), 128 );
  topLayout->addWidget(m_airplaneType, row, 1);
  row++;

  lbl = new QLabel(tr("Server:"));
  topLayout->addWidget(lbl, row, 0);
  m_server = new QComboBox;
  m_server->addItem( "www.livetrack24.com" );
  m_server->addItem( "test.livetrack24.com" );
  topLayout->addWidget(m_server, row, 1);
  row++;

  lbl = new QLabel(tr("User name:"));
  topLayout->addWidget(lbl, row, 0);

  m_username = new QLineEdit;
  topLayout->addWidget(m_username, row, 1);
  row++;

  lbl = new QLabel(tr("Password:"));
  topLayout->addWidget(lbl, row, 0);

  m_password = new QLineEdit;
  topLayout->addWidget(m_password, row, 1);
  row++;

  topLayout->setRowStretch(row, 10);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png"));

  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);
  load();
}

PreFlightLiveTrack24Page::~PreFlightLiveTrack24Page()
{
  // qDebug("PreFlightLiveTrack24Page::~PreFlightLiveTrack24Page()");
}

void PreFlightLiveTrack24Page::load()
{
  GeneralConfig* conf = GeneralConfig::instance();

  m_liveTrackEnabled->setChecked( conf->isLiveTrackOnOff() );
  m_trackingIntervalMin->setValue( conf->getLiveTrackInterval() / 60 );
  m_trackingIntervalSec->setValue( conf->getLiveTrackInterval() % 60 );
  m_username->setText( conf->getLiveTrackUserName() );
  m_password->setText( conf->getLiveTrackPassword() );

  int apt = conf->getLiveTrackAirplaneType();
  m_airplaneType->setCurrentIndex( m_airplaneType->findData( apt ) );

  QString server = conf->getLiveTrackServer();
  m_server->setCurrentIndex( m_server->findText( server ) );
}

void PreFlightLiveTrack24Page::save()
{
  GeneralConfig* conf = GeneralConfig::instance();
  bool oldOnOffState = conf->isLiveTrackOnOff();
  bool newOnOffState = m_liveTrackEnabled->isChecked();

  if( oldOnOffState != newOnOffState )
    {
      // Report new switch state before LiveTracking is switched off.
      // Otherwise an end request can not be sent to the server.
      emit onOffStateChanged( newOnOffState );
    }

  conf->setLiveTrackOnOff( newOnOffState );
  conf->setLiveTrackInterval(
      m_trackingIntervalMin->value() * 60 + m_trackingIntervalSec->value() );
  conf->setLiveTrackUserName( m_username->text().trimmed() );
  conf->setLiveTrackPassword( m_password->text().trimmed() );
  conf->setLiveTrackAirplaneType( m_airplaneType->itemData(m_airplaneType->currentIndex()).toInt() );
  conf->setLiveTrackServer( m_server->itemText(m_server->currentIndex()));
}

void PreFlightLiveTrack24Page::slotAccept()
{
  if( m_liveTrackEnabled->isChecked() &&
      ( m_username->text().trimmed().isEmpty() || m_password->text().trimmed().isEmpty() ))
    {
      // User name and password are required, when service is switched on!
      QString msg = QString(tr("<html>LiveTrack24 is switched on but "
                               "user name or password are missing!"
                               "<br><br>Please switch off service or add "
                               "the missing items.</html>"));

      QMessageBox mb( QMessageBox::Warning,
                      tr( "Login data missing" ),
                      msg,
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      mb.exec();
      return;
    }

  save();
  GeneralConfig::instance()->save();
  emit closingWidget();
  QWidget::close();
}

void PreFlightLiveTrack24Page::slotReject()
{
  emit closingWidget();
  QWidget::close();
}
