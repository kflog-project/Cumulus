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

#include <QtGui>
#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>

#include "airspacedownloaddialog.h"
#include "generalconfig.h"
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

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget( new QLabel( tr("Download openAIP Airspaces")), Qt::AlignLeft );
  mainLayout->addSpacing( 30 );

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget( new QLabel( tr("Countries:")) );

  Qt::InputMethodHints imh;
  m_editCountries = new QLineEdit(this);
  imh = (m_editCountries->inputMethodHints() | Qt::ImhNoPredictiveText);
  m_editCountries->setInputMethodHints(imh);
  m_editCountries->setText( GeneralConfig::instance()->getOpenAipAirspaceCountries() );
  hbox->addWidget( m_editCountries, 10 );
  mainLayout->addLayout( hbox );

  m_buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel |
                                      QDialogButtonBox::Ok );

  m_buttonBox->layout()->setSpacing(30);

  QPushButton *ok =  m_buttonBox->button( QDialogButtonBox::Ok );
  ok->setDefault( true );
  ok->setText( tr("Download") );

  QPushButton *cancel = m_buttonBox->button( QDialogButtonBox::Cancel );
  cancel->setAutoDefault(false);

  mainLayout->addStretch( 10 );
  mainLayout->addWidget( m_buttonBox );

  connect( m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()) );
  connect( m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()) );
}

AirspaceDownloadDialog::~AirspaceDownloadDialog()
{
  // qDebug("AirspaceDownloadDialog::~AirspaceDownloadDialog()");
}

/** User has pressed Ok button */
void AirspaceDownloadDialog::accept()
{
  GeneralConfig *conf = GeneralConfig::instance();

  QString openAipCountries = m_editCountries->text().trimmed().toLower();

  if( openAipCountries.isEmpty() )
    {
      conf->setOpenAipAirspaceCountries( openAipCountries );
      return;
    }

  // We will check, if the country entries of openAIP are valid. If not a
  // warning message is displayed and the action is aborted.
  QStringList clist = openAipCountries.split(QRegExp("[, ]"), QString::SkipEmptyParts);

  if( ! checkCountryList(clist) )
    {
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
      return;
    }

  // Save the new airspace country settings
  GeneralConfig::instance()->setOpenAipAirspaceCountries( openAipCountries );

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

  emit downloadAirspaces( clist );
  QWidget::close();
}

/** User has pressed Cancel button */
void AirspaceDownloadDialog::reject()
{
  QWidget::close();
}

bool AirspaceDownloadDialog::checkCountryList( QStringList& clist )
{
  if( clist.size() == 0 )
    {
      return false;
    }

  for( int i = 0; i < clist.size(); i++ )
    {
      const QString& s = clist.at(i);

      if( s.length() == 2 && s.contains(QRegExp("[A-Za-z][A-Za-z]")) == true )
        {
          return true;
        }
      else
        {
          return false;
        }
    }

  return false;
}
