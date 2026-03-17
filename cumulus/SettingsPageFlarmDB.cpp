/***********************************************************************
**
**   SettingsPageFlarmDB.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2023-2026 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <SettingsPageFlarmDB.h>
#include <QtWidgets>

#include "calculator.h"
#include "FlarmDB.h"
#include "generalconfig.h"
#include "HelpBrowser.h"
#include "layout.h"
#include "MainWindow.h"

SettingsPageFlarmDB::SettingsPageFlarmDB( QWidget *parent ) :
  QWidget(parent),
  fnUseStart(false),
  m_downloadManger(nullptr),
  m_downloadIsRunning(false),
  m_downloadDone(false),
  m_first(true)
{
  setObjectName("SettingsPageFlarmDB");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Flarm Database") );

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
  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );

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

  useFlarmDB = new QCheckBox( tr("use Flarm Database"), this );
  useFlarmDB->setToolTip( tr("Check it for Flarm Database usage") );
  topLayout->addWidget( useFlarmDB, row, 0, 1, 3 );
  row++;

  buttonDownloadFN = new QPushButton( tr("Download"), this );
  topLayout->addWidget( buttonDownloadFN, row, 0 );

  topLayout->addWidget(new QLabel(tr("FlarmNet URL:"), this), row, 1);
  row++;

  Qt::InputMethodHints imh;
  editFnFile = new QLineEdit( this );
  imh = (editFnFile->inputMethodHints() | Qt::ImhNoPredictiveText);
  editFnFile->setInputMethodHints(imh);
  topLayout->addWidget( editFnFile, row, 0, 1, 3 );
  row++;

  topLayout->setRowMinimumHeight( row, 20 * Layout::getIntScaledDensity() );
  row++;

  buttonDownloadOGN = new QPushButton( tr("Download"), this );
  topLayout->addWidget( buttonDownloadOGN, row, 0 );

  topLayout->addWidget(new QLabel(tr("OGN URL:"), this), row, 1);
  row++;

  editOGNFile = new QLineEdit( this );
  imh = (editOGNFile->inputMethodHints() | Qt::ImhNoPredictiveText);
  editOGNFile->setInputMethodHints(imh);
  topLayout->addWidget( editOGNFile, row, 0, 1, 3 );
  row++;

  topLayout->setRowMinimumHeight( row, 20 * Layout::getIntScaledDensity() );
  row++;

  dbFilterLabel = new QLabel( this );
  topLayout->addWidget( dbFilterLabel, row, 0, 1, 2  );
  count = new QPushButton( tr("Count"), this );
  topLayout->addWidget( count, row, 2 );
  row++;

  connect( count, SIGNAL(clicked()), SLOT(slotCount()) );

  Qt::InputMethodHints imh1;
  editDBFilter = new QLineEdit( this );
  imh1 = (editDBFilter->inputMethodHints() | Qt::ImhNoPredictiveText);
  editDBFilter->setInputMethodHints(imh1);
  topLayout->addWidget( editDBFilter, row, 0, 1, 3 );
  row++;

  topLayout->setRowStretch ( row, 10 );
  topLayout->setColumnStretch( 1, 10 );
  row++;

  buttonReset = new QPushButton( tr("Defaults"), this );
  buttonReset->setToolTip( tr("Reset all to defaults") );
  topLayout->addWidget( buttonReset, row, 0 );

  info = new QLabel( this );
  topLayout->addWidget( info, row, 1, 1, 2 );

  connect( buttonDownloadFN, SIGNAL(clicked()), SLOT(slotDownloadFN()) );
  connect( buttonDownloadOGN, SIGNAL(clicked()), SLOT(slotDownloadOGN()) );
  connect( buttonReset, SIGNAL(clicked()), SLOT(slotSetFactoryDefault()) );

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  ok = new QPushButton(this);
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

SettingsPageFlarmDB::~SettingsPageFlarmDB()
{
}

void SettingsPageFlarmDB::slotHelp()
{
  QString file = "cumulus-settings-flarm-database.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void SettingsPageFlarmDB::slotAccept()
{
  if( m_downloadIsRunning == true )
    {
      return;
    }

  if( fnFileStart != editFnFile->text().trimmed() ||
      fnFilterStart != editDBFilter->text().trimmed() ||
      fnUseStart != useFlarmDB->isChecked() ||
      m_downloadDone == true )
    {
      save();
      GeneralConfig::instance()->save();
      emit settingsChanged();
    }

  if( useFlarmDB->isChecked() == true &&
      ( fnUseStart != useFlarmDB->isChecked() ||
        fnFilterStart != editDBFilter->text().trimmed() ||
        m_downloadDone == true ) )
    {
      // reload database from files.
      FlarmDBThread* thread = new FlarmDBThread( 0 );
      thread->start();
    }
  else if( useFlarmDB->isChecked() == false )
    {
      FlarmDB::unloadData();
    }

  QWidget::close();
}

void SettingsPageFlarmDB::slotReject()
{
  if( m_downloadIsRunning == true )
    {
      return;
    }

  QWidget::close();
}

void SettingsPageFlarmDB::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  editFnFile->setText( conf->getFlarmNetUrl() );
  editOGNFile->setText( conf->getFlarmOGNUrl() );
  editDBFilter->setText( conf->getFlarmDBFilter() );
  useFlarmDB->setChecked( conf->useFlarmDB() );

  if( m_first == true )
    {
      m_first = false;
      fnFileStart = conf->getFlarmNetUrl().trimmed();
      fnFilterStart = conf->getFlarmDBFilter().trimmed();
      fnUseStart = conf->useFlarmDB();
    }

  if( calculator->moving() == true )
    {
      buttonDownloadFN->setEnabled( false );
      buttonDownloadOGN->setEnabled( false );
    }
  else
    {
      buttonDownloadFN->setEnabled( true );
      buttonDownloadOGN->setEnabled( true );
    }

  setLoadedRecords();
}

void SettingsPageFlarmDB::save()
{
  GeneralConfig *conf = GeneralConfig::instance();
  conf->setFlarmNetUrl( editFnFile->text().trimmed() );
  conf->setFlarmOGNUrl( editOGNFile->text().trimmed() );
  conf->setFlarmDBFilter( editDBFilter->text().trimmed() );
  conf->setUseFlarmDB( useFlarmDB->isChecked() );
}

void SettingsPageFlarmDB::slotSetFactoryDefault()
{
  GeneralConfig *conf = GeneralConfig::instance();
  conf->setFlarmNetUrl( FLARM_NET_URL );
  conf->setFlarmOGNUrl( FLARM_OGN_URL );
  conf->setFlarmDBFilter( "" );
  conf->setUseFlarmDB( false );
  load();
  info->clear();
}

void SettingsPageFlarmDB::slotDownloadFN()
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
  buttonDownloadFN->setEnabled( false );
  buttonDownloadOGN->setEnabled( false );

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
  dir.mkdir( "flarmDB");
  QString fileDb = QFileInfo( url.mid( 6 ) ).fileName();
  QString destDb = dir.absolutePath() + "/flarmDB/" + fileDb;

  info->setText( tr("Download running") );

  m_downloadManger->downloadRequest( url, destDb, true );
}

void SettingsPageFlarmDB::slotDownloadOGN()
{
  QString url = editOGNFile->text().trimmed();

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
  buttonDownloadFN->setEnabled( false );
  buttonDownloadOGN->setEnabled( false );

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
  dir.mkdir( "flarmDB");
  QString destDb = dir.absolutePath() + "/flarmDB/" + FLARM_OGN_FILE;

  info->setText( tr("Download running") );

  m_downloadManger->downloadRequest( url, destDb, true );

}

void SettingsPageFlarmDB::slotFileDownloaded( QString& file )
{
  // we inform about the database file download, if database usage is desired.
  if( useFlarmDB->isChecked() == true )
    {
      m_downloadDone = true;
    }

  info->setText( tr("Download finished") + ": " + file );
}

void SettingsPageFlarmDB::slotNetworkError()
{
  // A network error has occurred. We delete the download manager to get faster
  // a new connection.
  m_downloadManger->deleteLater();
  m_downloadManger = nullptr;

  QString msg = QString(tr("Network error. Check Internet connection!"));
  info->setText( msg );

  m_downloadIsRunning = false;

  if( buttonDownloadFN->isEnabled() == false )
    {
      buttonDownloadFN->setEnabled( true );
    }

  if( buttonDownloadOGN->isEnabled() == false )
    {
      buttonDownloadOGN->setEnabled( true );
    }
}

void SettingsPageFlarmDB::slotDownloadsFinished( int /* requests */,
                                                 int errors )
{
  if( errors != 0 )
    {
      QString msg = QString(tr("Download error, check URL!"));
      info->setText( msg );
    }

  m_downloadIsRunning = false;

  if( buttonDownloadFN->isEnabled() == false )
    {
      buttonDownloadFN->setEnabled( true );
    }

  if( buttonDownloadOGN->isEnabled() == false )
    {
      buttonDownloadOGN->setEnabled( true );
    }
}

/**
 * Set the loaded FlarmNet records.
 */
void SettingsPageFlarmDB::setLoadedRecords()
{
  int records = FlarmDB::getRecords();
  dbFilterLabel->setText( tr("FlarmDB Filter - loaded elements") +
		          QString(": %1 ").arg( records ) );
}

/**
 * Slot to count filter items.
 */
void SettingsPageFlarmDB::slotCount()
{
  ok->setEnabled( false );
  cancel->setEnabled( false );
  QString filter = editDBFilter->text().trimmed();
  int records = FlarmDB::applyFilter( filter );
  dbFilterLabel->setText( tr("FlarmDB Filter - would load %1 elements").arg( records ) );

  ok->setEnabled( true );
  cancel->setEnabled( true );
}
