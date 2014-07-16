/***********************************************************************
**
**   airspacedownloaddialog.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2014 Axel Pauli
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

#include "airspacedownloaddialog.h"
#include "generalconfig.h"
#include "layout.h"
#include "mainwindow.h"
#include "proxydialog.h"

AirspaceDownloadDialog::AirspaceDownloadDialog( QWidget *parent ) :
  QWidget(parent)
{
  setObjectName( "AirspaceDownloadDialog" );
  setWindowTitle( tr("Download openAIP Airspaces") );
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute( Qt::WA_DeleteOnClose );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  QGridLayout *leftLayout = new QGridLayout;
  mainLayout->addLayout(leftLayout);

  int row = 0;
  leftLayout->addWidget( new QLabel( tr("Download openAIP Airspaces")), row++, 0, 1, 2);
  leftLayout->setRowMinimumHeight ( row++, 30 );

  leftLayout->addWidget( new QLabel( tr("Countries:")), row, 0 );

  Qt::InputMethodHints imh;
  m_editCountries = new QLineEdit(this);
  imh = (m_editCountries->inputMethodHints() | Qt::ImhNoPredictiveText);
  m_editCountries->setInputMethodHints(imh);
  m_editCountries->setText( GeneralConfig::instance()->getOpenAipAirspaceCountries() );
  leftLayout->addWidget( m_editCountries, row++, 1 );
  leftLayout->setColumnStretch( 1, 10 );
  leftLayout->setRowMinimumHeight ( row++, 30 );

  connect( m_editCountries, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  QPushButton *downloadButton = new QPushButton( tr("Download") );
  connect(downloadButton, SIGNAL(pressed()), this, SLOT(slotDownload()));

  leftLayout->addWidget( downloadButton, row++, 0 );
  leftLayout->setRowStretch( row, 10 );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(close()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  mainLayout->addLayout(buttonBox);
}

AirspaceDownloadDialog::~AirspaceDownloadDialog()
{
  // qDebug("AirspaceDownloadDialog::~AirspaceDownloadDialog()");
}

void AirspaceDownloadDialog::slotDownload()
{
  QStringList countrylist;

  if( checkCountryList( m_editCountries->text(), countrylist) == false ||
      countrylist.size() == 0 )
    {
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Download openAIP files?"),
                  tr( "Active Internet connection is needed!") +
                  QString("<p>") + tr("Start download now?"),
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

  emit downloadAirspaces( countrylist );
}

void AirspaceDownloadDialog::slotAccept()
{
  QStringList countrylist;

  if( ! checkCountryList( m_editCountries->text(), countrylist) )
    {
      return;
    }

  QWidget::close();
}

bool AirspaceDownloadDialog::checkCountryList( const QString& countries, QStringList& countrylist )
{
  GeneralConfig *conf = GeneralConfig::instance();

  QString openAipCountries = countries.trimmed().toLower();

  countrylist.clear();

  if( openAipCountries.isEmpty() )
    {
      conf->setOpenAipAirspaceCountries( openAipCountries );
      return true;
    }

  // We check, if the passed country string is correct.
  countrylist = openAipCountries.split(QRegExp("[, ]"), QString::SkipEmptyParts);

  for( int i = 0; i < countrylist.size(); i++ )
    {
      const QString& s = countrylist.at(i);

      if( s.length() == 2 && s.contains(QRegExp("[A-Za-z][A-Za-z]")) == true )
        {
          continue;
        }

      QMessageBox mb( QMessageBox::Warning,
                      tr( "Please check entries" ),
                      tr("Every openAIP country sign must consist of two letters!<br>Allowed separators are space and comma!"),
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2 - mb.width()/2, height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      mb.exec();
      countrylist.clear();
      return false;
    }

  conf->setOpenAipAirspaceCountries( openAipCountries );
  return true;
}
