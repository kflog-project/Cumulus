/***********************************************************************
 **
 **   preflightlivetrack24page.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2013-2014 by Axel Pauli
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
#include "mainwindow.h"
#include "numberEditor.h"
#include "preflightlivetrack24page.h"

PreFlightLiveTrack24Page::PreFlightLiveTrack24Page(QWidget *parent) :
  QWidget(parent),
  m_updateTimer(0)
{
  setObjectName("PreFlightLiveTrack24Page");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("PreFlight - LiveTracking") );

  if( parent )
    {
      resize( parent->size() );
    }

  // HTTP Client for login test in LiveTrack24 server.
  m_httpClient = new HttpClient( this, false );

  connect( m_httpClient, SIGNAL( finished(QString &, QNetworkReply::NetworkError) ),
           this, SLOT( slotHttpResponse(QString &, QNetworkReply::NetworkError) ));

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

  m_liveTrackEnabled = new QCheckBox(tr("LiveTrack on"));
  topLayout->addWidget(m_liveTrackEnabled, row, 0 );
  row++;

  QLabel *lbl = new QLabel(tr("Track interval:"));
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

  const QStringList& srvList = GeneralConfig::getLiveTrackServerList();

  for( int i = 0; i < srvList.size(); i++ )
    {
      int pos = 0;

      if( srvList.at(i).startsWith("http://") )
        {
          pos = 7;
        }
      else
        {
          pos = 8;
        }

      m_server->addItem( srvList.at(i).mid(pos), srvList.at(i) );
    }

  topLayout->addWidget(m_server, row, 1);
  row++;

  connect( m_server, SIGNAL(currentIndexChanged(int)),
           SLOT(slotCurrentIndexChanged(int)) );

  lbl = new QLabel(tr("User name:"));
  topLayout->addWidget(lbl, row, 0);

  Qt::InputMethodHints imh = Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText;

  m_username = new QLineEdit;
  m_username->setInputMethodHints(imh | m_username->inputMethodHints() );
  topLayout->addWidget(m_username, row, 1);
  row++;

  lbl = new QLabel(tr("Password:"));
  topLayout->addWidget(lbl, row, 0);

  m_password = new QLineEdit;
  m_password->setInputMethodHints(imh | m_password->inputMethodHints() );
  topLayout->addWidget(m_password, row, 1);
  row++;

  m_loginTestButton = new QPushButton( tr("Login Test") );
  topLayout->addWidget(m_loginTestButton, row, 0);
  row++;

  connect( m_loginTestButton, SIGNAL(pressed()), this, SLOT(slotLoginTest()));

  topLayout->setRowMinimumHeight(row++, 20);
  topLayout->setRowStretch(row++, 10);

  m_sessionDisplay = new QLabel;
  topLayout->addWidget(m_sessionDisplay, row, 0, 1, 2);
  row++;

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

  // Start an update timer to update session data periodically.
  m_updateTimer = new QTimer(this);
  m_updateTimer->setSingleShot(false);
  connect( m_updateTimer, SIGNAL(timeout()), SLOT(showSessionData()) );
  m_updateTimer->start( 2400 );
}

PreFlightLiveTrack24Page::~PreFlightLiveTrack24Page()
{
  // qDebug("PreFlightLiveTrack24Page::~PreFlightLiveTrack24Page()");
}

void PreFlightLiveTrack24Page::showSessionData()
{
  LiveTrack24Logger* ltl = MainWindow::mainWindow()->getLiveTrack24Logger();

  QString session = ltl->sessionStatus() ? tr("on") : tr("off");

  uint cached, sent = 0;
  ltl->getPackageStatistics( cached, sent );

  QString txt = tr("Session: %1, Packages: cached: %2 sent: %3")
                .arg(session).arg(cached).arg(sent);

  m_sessionDisplay->setText( txt );
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
  m_server->setCurrentIndex( conf->getLiveTrackIndex() );

  showSessionData();
}

void PreFlightLiveTrack24Page::slotCurrentIndexChanged( int index )
{
  QString data[3];

  GeneralConfig::instance()->getLiveTrackAccountData( index, data );

  m_username->setText( data[1] );
  m_password->setText( data[2] );
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
  conf->setLiveTrackInterval( m_trackingIntervalMin->value() * 60 + m_trackingIntervalSec->value() );
  conf->setLiveTrackAirplaneType( m_airplaneType->itemData(m_airplaneType->currentIndex()).toInt() );
  conf->setLiveTrackIndex( m_server->currentIndex() );
  conf->setLiveTrackAccountData( m_server->currentIndex(),
                                 m_username->text().trimmed(),
                                 m_password->text().trimmed() );
}

void PreFlightLiveTrack24Page::slotAccept()
{
  if( m_liveTrackEnabled->isChecked() &&
      ( m_username->text().trimmed().isEmpty() ||
        ( m_server->itemText(m_server->currentIndex()).contains("live") &&
          m_password->text().trimmed().isEmpty() )))
    {
      // User name and password are required, when service is switched on!
      QString msg = QString(tr("<html>LiveTracking is switched on but "
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

void PreFlightLiveTrack24Page::slotLoginTest()
{
  if( m_username->text().trimmed().isEmpty() ||
      ( m_server->itemText(m_server->currentIndex()).contains("live") &&
        m_password->text().trimmed().isEmpty() ))
    {
      // User name and password are required for the login test
      QString msg = QString(tr("User name or password are missing!"));

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

  QString server = m_server->itemData(m_server->currentIndex()).toString();

  QString loginUrl = server +
                     QString( "/client.php?op=login&user=%1&pass=%2")
                              .arg(m_username->text().trimmed())
                              .arg(m_password->text().trimmed() );

  m_httpResultBuffer.clear();

  bool ok = m_httpClient->getData( loginUrl, &m_httpResultBuffer );

  if( ! ok )
    {
      QString msg = tr("<html>Network error!<br><br>Does exist an Internet connection?</html>");

      QMessageBox mb( QMessageBox::Information,
                      tr("Login Test failed"),
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

  // Disable test button
  m_loginTestButton->setEnabled( false );
}

void PreFlightLiveTrack24Page::slotHttpResponse( QString &urlIn,
                                                 QNetworkReply::NetworkError codeIn )
{
  Q_UNUSED(urlIn)

  m_loginTestButton->setEnabled( true );

  QString msg;

  if( codeIn != QNetworkReply::NoError )
    {
      if( codeIn > 0 && codeIn < 100 )
        {
          // There was a problem on the network.
          msg = tr("<html>Network error!<br><br>Does exist an Internet connection?</html>");
        }
      else
        {
          msg = QString(tr("<html>Server login failed!"
                           "<br><br>Check user name and password.</html>"));
        }
    }
  else
    {
      // Check the returned user identifier. For a successful login it
      // must be greater than zero.
      bool ok;
      uint userId = m_httpResultBuffer.toUInt( &ok );

      qDebug() << "LiveTrack Login Test: UserId=" << m_httpResultBuffer;

      if( ! ok || userId == 0 )
        {
          msg = QString(tr("<html>Server login failed!"
                           "<br><br>Check user name and password.</html>"));
        }
      else
        {
          msg = QString(tr("<html>Server login succeeded!</html>"));
        }
    }

  QMessageBox mb( QMessageBox::Information,
                  tr("Login Test result"),
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
}
