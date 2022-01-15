/***********************************************************************
 **
 **   preflightlivetrack24page.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2013-2021 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <unistd.h>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#ifdef ANDROID
#include "jnisupport.h"
#endif

#include "generalconfig.h"
#include "layout.h"
#include <HelpBrowser.h>
#include "MainWindow.h"
#include "numberEditor.h"
#include "preflightlivetrack24page.h"
#include "skylines/SkyLinesTracker.h"

PreFlightLiveTrack24Page::PreFlightLiveTrack24Page(QWidget *parent) :
  QWidget(parent),
  m_passwordIsHidden(true),
  m_updateTimer(0),
  m_slt(0)
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
  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );
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

  int maw = QFontMetrics(font()).horizontalAdvance("999 min") + 10;
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

  maw = QFontMetrics(font()).horizontalAdvance("99 s") + 10;
  m_trackingIntervalSec->setMinimumWidth( maw );
  tiHbox->addWidget( m_trackingIntervalSec );
  tiHbox->addStretch( 10 );

  lbl = new QLabel(tr("Airplane type:"));
  topLayout->addWidget(lbl, row, 0);
  m_airplaneType = new QComboBox;
  m_airplaneType->addItem( tr("Paraglider"), 1 );
  m_airplaneType->addItem( tr("Glider"), 8 );
  m_airplaneType->addItem( tr("Paramotor"), 16 );
  m_airplaneType->addItem( tr("Trike"), 32 );
  m_airplaneType->addItem( tr("Powered flight"), 64 );
  m_airplaneType->addItem( tr("Hot Air Balloon"), 128 );
  m_airplaneType->addItem( tr("Car"), 17100 );
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
      else if( srvList.at(i).startsWith("https://") )
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

  connect( m_username, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  topLayout->addWidget(m_username, row, 1);
  row++;

  lbl = new QLabel(tr("Password:"));
  topLayout->addWidget(lbl, row, 0);

  QHBoxLayout* hbox = new QHBoxLayout();
  hbox->setMargin( 0 );

  m_password = new QLineEdit;
  m_password->setInputMethodHints( imh | m_password->inputMethodHints() );
  m_password->setEchoMode( QLineEdit::Password );
  hbox->addWidget( m_password );

  connect( m_password, SIGNAL( returnPressed() ), MainWindow::mainWindow(),
           SLOT( slotCloseSip() ) );

  connect( m_password, SIGNAL( returnPressed() ), MainWindow::mainWindow(),
           SLOT( slotCloseSip() ) );

  m_logglePassword = new QPushButton( tr( "Show" ) );

  connect( m_logglePassword, SIGNAL( clicked() ), SLOT( slotTogglePassword() ) );

  hbox->addWidget( m_logglePassword );
  topLayout->addLayout( hbox, row, 1 );
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

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

  connect(help, SIGNAL(pressed()), this, SLOT(slotHelp()));
  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(help, 1);
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

  int oldLiveTrackIndex = conf->getLiveTrackIndex();
  int newLiveTrackIndex = m_server->currentIndex();

  conf->setLiveTrackOnOff( newOnOffState );
  conf->setLiveTrackInterval( m_trackingIntervalMin->value() * 60 + m_trackingIntervalSec->value() );
  conf->setLiveTrackAirplaneType( m_airplaneType->itemData(m_airplaneType->currentIndex()).toInt() );
  conf->setLiveTrackIndex( m_server->currentIndex() );
  conf->setLiveTrackAccountData( m_server->currentIndex(),
                                 m_username->text().trimmed(),
                                 m_password->text().trimmed() );

  if( oldLiveTrackIndex != newLiveTrackIndex )
    {
      emit liveTrackingServerChanged();
    }
}

void PreFlightLiveTrack24Page::slotHelp()
{
  QString file = "cumulus-preflight-settings-livetracking.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void PreFlightLiveTrack24Page::slotAccept()
{
  if( m_liveTrackEnabled->isChecked() )
    {
      if ( m_username->text().trimmed().isEmpty() ||
          ( m_server->itemText(m_server->currentIndex()).contains("live") &&
              m_password->text().trimmed().isEmpty() ))
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

      if( m_server->itemText(m_server->currentIndex() ) ==
          SkyLinesTracker::getServerName() )
        {
          // Check SkyLines user key. Must be 8 hex digits.
          bool ok;
          m_username->text().trimmed().toULongLong(&ok, 16);

          if( ok == false )
            {
              // User name and password are required, when service is switched on!
              QString msg = QString(tr("<html>LiveTracking is switched on but "
                  "your user key is invalid!"
                  "<br><br>Please switch off service or correct "
                  "your user key (8 hex numbers).</html>"));

              QMessageBox mb( QMessageBox::Warning,
                              tr( "User key invalid" ),
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
        }
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
  QString server = m_server->itemData(m_server->currentIndex()).toString();

  if( server.startsWith("http") == true )
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

      QString loginUrl = server +
                         QString( "/client.php?op=login&user=%1&pass=%2")
                                  .arg(m_username->text().trimmed())
                                  .arg(m_password->text().trimmed() );

      m_httpResultBuffer.clear();

      qDebug() << "loginUrl=" << loginUrl;

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
      return;
    }

  if( server == SkyLinesTracker::getServerName() )
    {
      // Check SkyLines user key. Must be a hex digit string.
      bool ok;
      m_username->text().trimmed().toULongLong(&ok, 16);

      if( ok == false )
        {
          // User name and password are required, when service is switched on!
          QString msg = QString(tr("<html>LiveTracking is switched on but "
              "your user key is invalid!"
              "<br><br>Please switch off service or correct "
              "your user key (8 hex numbers).</html>"));

          QMessageBox mb( QMessageBox::Warning,
                          tr( "User key invalid" ),
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

      // Store current user data, will be used by ping test.
      GeneralConfig::instance()->setLiveTrackAccountData( m_server->currentIndex(),
                                                          m_username->text().trimmed(),
                                                          m_password->text().trimmed() );

      // Check login to SkyLines
      m_slt = new SkyLinesTracker( this, true );

      connect( m_slt, SIGNAL(connectionFailed()),
               SLOT(slotSkyLinesConnectionFailed()) );
      connect( m_slt, SIGNAL(pingResult(quint32)),
               SLOT(slotSkyLinesPingResult(quint32)) );

      // Disable test button
      m_loginTestButton->setEnabled( false );
      m_slt->startTracking();
      return;
   }
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

  showLoginTestResult( msg );
}

void PreFlightLiveTrack24Page::showLoginTestResult( QString& msg )
{
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

void PreFlightLiveTrack24Page::slotSkyLinesConnectionFailed()
{
  // There was a problem on the network.
  QString msg = tr("<html>Network error!<br><br>Does exist an Internet connection?</html>");
  showLoginTestResult( msg );
  m_loginTestButton->setEnabled( true );
  m_slt->deleteLater();
}

/** Called to report the ping result. */
void PreFlightLiveTrack24Page::slotSkyLinesPingResult( quint32 result )
{
  QString msg = QString(tr("<html>SkyLines server login test succeeded!</html>"));

  if( result == 1 )
    {
      msg = QString(tr("<html>SkyLines login test failed!"
                       "<br><br>Do check your Live Tracking Key.</html>"));

    }

  showLoginTestResult( msg );
  m_loginTestButton->setEnabled( true );
  m_slt->deleteLater();
}

/**
 * Called, if the password toggle button is pressed.
 */
void PreFlightLiveTrack24Page::slotTogglePassword()
{
  if( m_passwordIsHidden == true )
    {
      m_passwordIsHidden = false;
      m_password->setEchoMode( QLineEdit::Normal );
      m_logglePassword->setText( tr( "Hide" ) );
    }
  else
    {
      m_passwordIsHidden = true;
      m_password->setEchoMode( QLineEdit::Password );
      m_logglePassword->setText( tr( "Show" ) );
    }
}
