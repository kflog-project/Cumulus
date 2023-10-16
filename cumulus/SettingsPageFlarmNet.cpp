/***********************************************************************
**
**   SettingsPageFlarmNet.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2023 by Axel Pauli (kflog.cumulus@gmail.com)
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

#include <FlarmNet.h>
#include <SettingsPageFlarmNet.h>
#include "generalconfig.h"
#include "helpbrowser.h"
#include "layout.h"
#include "mainwindow.h"
#include "whatsthat.h"

SettingsPageFlarmNet::SettingsPageFlarmNet( QWidget *parent ) :
  QWidget(parent),
  fnUseStart(false),
  m_downloadManger(nullptr),
  m_downloadIsRunning(false),
  m_downloadDone(false),
  m_first(true)
{
  setObjectName("SettingsPageFlarmNet");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - FlarmNet") );

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

#ifdef ANDROID
  QScrollBar* lvsb = sa->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // Add scroll area to its own layout
  sal->addWidget( sa );

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal, 10 );

  // The parent of the layout is the scroll widget
  QGridLayout *topLayout = new QGridLayout(sw);

  topLayout->setHorizontalSpacing(20 * Layout::getIntScaledDensity() );
  topLayout->setVerticalSpacing(10 * Layout::getIntScaledDensity() );

  int row=0;

  useFlarmNet = new QCheckBox( tr("use FlarmNet data"), this );
  useFlarmNet->setToolTip( tr("Check it for FlarmNet usage") );
  topLayout->addWidget( useFlarmNet, row, 0 );
  row++;

  topLayout->addWidget(new QLabel(tr("FlarmNet URL:"), this), row, 0);
  row++;

  Qt::InputMethodHints imh;
  editFnFile = new QLineEdit( this );
  imh = (editFnFile->inputMethodHints() | Qt::ImhNoPredictiveText);
  editFnFile->setInputMethodHints(imh);
  topLayout->addWidget( editFnFile, row, 0, 1, 2 );
  row++;

  topLayout->addWidget(new QLabel(tr("FlarmNet Filter:"), this), row, 0);
  row++;

  Qt::InputMethodHints imh1;
  editFnFilter = new QLineEdit( this );
  imh1 = (editFnFilter->inputMethodHints() | Qt::ImhNoPredictiveText);
  editFnFilter->setInputMethodHints(imh1);
  topLayout->addWidget( editFnFilter, row, 0, 1, 2 );
  row++;

  buttonDownload = new QPushButton( tr("Download"), this );
  topLayout->addWidget( buttonDownload, row, 0 );

  info = new QLabel( this );
  topLayout->addWidget( info, row, 1 );
  row++;

  buttonReset = new QPushButton( tr("Defaults"), this );
  buttonReset->setToolTip( tr("Reset all to defaults") );
  topLayout->addWidget( buttonReset, row, 0 );
  row++;

  topLayout->setRowStretch ( row, 10 );
  topLayout->setColumnStretch( 1, 10 );

  connect( buttonDownload, SIGNAL(clicked()), SLOT(slotDownload()) );
  connect( buttonReset, SIGNAL(clicked()), SLOT(slotSetFactoryDefault()) );

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
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png"));

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
}

SettingsPageFlarmNet::~SettingsPageFlarmNet()
{
}

void SettingsPageFlarmNet::slotHelp()
{
  QString file = "cumulus-settings-flarmnet.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void SettingsPageFlarmNet::slotAccept()
{
  if( m_downloadIsRunning == true )
    {
      return;
    }

  if( fnFileStart != editFnFile->text().trimmed() ||
      fnFilterStart != editFnFilter->text().trimmed() ||
      fnUseStart != useFlarmNet->isChecked() ||
      m_downloadDone == true )
    {
      save();
      GeneralConfig::instance()->save();
      emit settingsChanged();
    }

  if( useFlarmNet->isChecked() == true &&
      ( fnUseStart != useFlarmNet->isChecked() ||
        fnFilterStart != editFnFilter->text().trimmed() ||
        m_downloadDone == true ) )
    {
      // reload database from file.
      FlarmNetThread* thread = new FlarmNetThread( 0 );
      thread->start();
    }
  else if( useFlarmNet->isChecked() == false )
    {
      FlarmNet::unloadData();
    }

  QWidget::close();
}

void SettingsPageFlarmNet::slotReject()
{
  if( m_downloadIsRunning == true )
    {
      return;
    }

  QWidget::close();
}

void SettingsPageFlarmNet::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  editFnFile->setText( conf->getFlarmNetUrl() );
  editFnFilter->setText( conf->getFlarmNetFilter() );
  useFlarmNet->setChecked( conf->useFlarmNet() );

  if( m_first == true )
    {
      m_first = false;
      fnFileStart = conf->getFlarmNetUrl().trimmed();
      fnFilterStart = conf->getFlarmNetFilter().trimmed();
      fnUseStart = conf->useFlarmNet();
    }
}

void SettingsPageFlarmNet::save()
{
  GeneralConfig *conf = GeneralConfig::instance();
  conf->setFlarmNetUrl( editFnFile->text().trimmed() );
  conf->setFlarmNetFilter( editFnFilter->text().trimmed() );
  conf->setUseFlarmNet( useFlarmNet->isChecked() );
}

void SettingsPageFlarmNet::slotSetFactoryDefault()
{
  GeneralConfig *conf = GeneralConfig::instance();
  conf->setFlarmNetUrl( FLARM_NET_URL );
  conf->setFlarmNetFilter( "" );
  conf->setUseFlarmNet( false );
  load();
  info->clear();
}

void SettingsPageFlarmNet::slotDownload()
{
  QString url = editFnFile->text().trimmed();

  if( url.size() == 0 || url.startsWith( "https:") == false )
    {
      return;
    }

  if( m_downloadIsRunning == true )
    {
      // Do not allow multiple calls, if download is already running.
      return;
    }

  // set download marker, disable download button usage
  m_downloadIsRunning = true;
  buttonDownload->setEnabled( false );

  if( m_downloadManger == nullptr )
    {
      m_downloadManger = new DownloadManager(this);

      connect( m_downloadManger, SIGNAL(finished( int, int )),
               this, SLOT(slotDownloadsFinished( int, int )) );

      connect( m_downloadManger, SIGNAL(networkError()),
               this, SLOT(slotNetworkError()) );

      connect( m_downloadManger, SIGNAL(fileDownloaded(QString&)),
               this, SLOT(slotFileDownloaded(QString&)) );
    }

  // Create download destination directories
  QDir dir( GeneralConfig::instance()->getUserDataDirectory() );
  dir.mkdir( "flarmNet");
  QString fileDb = QFileInfo( url.mid( 6 ) ).fileName();
  QString destDb = dir.absolutePath() + "/flarmNet/" + fileDb;

  info->setText( tr("Running") );

  m_downloadManger->downloadRequest( url, destDb, true );
}

void SettingsPageFlarmNet::slotFileDownloaded( QString& /* file */)
{
  // we inform about the database file download, if database usage is desired.
  if( useFlarmNet->isChecked() == true )
    {
      m_downloadDone = true;
    }

  info->setText( tr("Finished") );
}

void SettingsPageFlarmNet::slotNetworkError()
{
  // A network error has occurred. We delete the download manager to get faster
  // a new connection.
  m_downloadManger->deleteLater();
  m_downloadManger = nullptr;

  QString msg = QString(tr("Network error. Check Internet connection!"));
  info->setText( msg );

  m_downloadIsRunning = false;
  buttonDownload->setEnabled( true );
}

void SettingsPageFlarmNet::slotDownloadsFinished( int /* requests */,
                                                  int errors )
{
  if( errors != 0 )
    {
      QString msg = QString(tr("Download error, check URL!"));
      info->setText( msg );
    }

  m_downloadIsRunning = false;
  buttonDownload->setEnabled( true );
}
