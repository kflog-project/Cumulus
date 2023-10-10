/***********************************************************************
**
**   SettingsPageFlarmDB.cpp
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

#include <QtWidgets>

#include "generalconfig.h"
#include "HelpBrowser.h"
#include "SettingsPageFlarmDB.h"
#include "layout.h"
#include "MainWindow.h"

SettingsPageFlarmDB::SettingsPageFlarmDB( QWidget *parent ) :
  QWidget(parent)
{
  setObjectName("SettingsPageFlarmDB");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - FLARM DB") );

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

  useFlarmDB = new QCheckBox( "use FLARM Database", this );
  useFlarmDB->setToolTip( tr("Check it for FLARM Database usage") );
  topLayout->addWidget( useFlarmDB, row, 0 );
  row++;

  topLayout->addWidget(new QLabel(tr("FLARM Database URL:"), this), row, 0);
  row++;

  Qt::InputMethodHints imh;
  editDbFile = new QLineEdit( this );
  imh = (editDbFile->inputMethodHints() | Qt::ImhNoPredictiveText);
  editDbFile->setInputMethodHints(imh);
  topLayout->addWidget( editDbFile, row, 0, 1, 2 );
  row++;

  buttonDownload = new QPushButton( tr("Download"), this );
  topLayout->addWidget( buttonDownload, row, 0 );
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

SettingsPageFlarmDB::~SettingsPageFlarmDB()
{
}

void SettingsPageFlarmDB::slotHelp()
{
  QString file = "cumulus-settings-flarm-db.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void SettingsPageFlarmDB::slotAccept()
{
  if( dbFileStart != editDbFile->text().trimmed() ||
      useFlarmDBStart != useFlarmDB->isChecked() )
    {
      save();
      GeneralConfig::instance()->save();
      emit settingsChanged();
    }

  QWidget::close();
}

void SettingsPageFlarmDB::slotReject()
{
  QWidget::close();
}

void SettingsPageFlarmDB::load()
{
  static bool first = true;

  GeneralConfig *conf = GeneralConfig::instance();

  editDbFile->setText( conf->getFlarmDBUrl() );
  useFlarmDB->setChecked( conf->useFlarmDb() );

  if( first == true )
    {
      first = false;
      dbFileStart = conf->getFlarmDBUrl().trimmed();
      useFlarmDBStart = conf->useFlarmDb();
    }
}

void SettingsPageFlarmDB::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setFlarmDBUrl( editDbFile->text().trimmed() );
  conf->setUseFlarmDb( useFlarmDB->isChecked() );
}

void SettingsPageFlarmDB::slotSetFactoryDefault()
{
  GeneralConfig *conf = GeneralConfig::instance();
  conf->setFlarmDBUrl( FLARM_DB_URL );
  conf->setUseFlarmDb( false );
  load();
}

void SettingsPageFlarmDB::slotDownload()
{

}
