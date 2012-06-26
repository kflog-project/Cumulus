/***********************************************************************
**
**   airspacedownloaddialog.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "airspacedownloaddialog.h"
#include "generalconfig.h"
#include "proxydialog.h"

AirspaceDownloadDialog::AirspaceDownloadDialog( QWidget *parent ) :
  QDialog(parent),
  m_autoSip(true)
{
  setObjectName( "AirspaceDownloadDialog" );
  setWindowTitle( tr("Download Airspace") );
  setAttribute( Qt::WA_DeleteOnClose );
  setModal(true);
  setSizeGripEnabled( true );

  // Save the current state of the software input panel
  m_autoSip = qApp->autoSipEnabled();

  // Set the SIP to true for this dialog. We need the SIP for the input of the URL.
  qApp->setAutoSipEnabled( true );

  QGridLayout* gridLayout = new QGridLayout;
  int row=0;

  QLabel *lbl = new QLabel( tr("Airspace Url:"), this );
  gridLayout->addWidget(lbl, row, 0 );

  editAirspaceUrl = new QLineEdit(this);
  editAirspaceUrl->setToolTip(tr("Enter URL of airspace filename to be installed"));
  editAirspaceUrl->setText( GeneralConfig::instance()->getLastAirspaceUrl() );
  gridLayout->addWidget(editAirspaceUrl, row, 1);

  buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel |
                                    QDialogButtonBox::Ok );

  QPushButton *ok =  buttonBox->button( QDialogButtonBox::Ok );
  ok->setDefault( true );
  ok->setText( tr("Install") );

  QPushButton *cancel = buttonBox->button( QDialogButtonBox::Cancel );
  cancel->setAutoDefault(false);

  QVBoxLayout *vBox = new QVBoxLayout;

  vBox->addLayout( gridLayout );
  vBox->addWidget( buttonBox );

  setLayout( vBox );

  connect( buttonBox, SIGNAL(accepted()), this, SLOT(accept()) );
  connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()) );

  // Set minimum size of input line
  QFontMetrics fm( font() );
  int strWidth = fm.width(QString("*")) * 50;
  int strWidthTiltle = fm.width( windowTitle() );

  if( strWidth >= strWidthTiltle )
    {
      editAirspaceUrl->setMinimumWidth( strWidth );
    }
  else
    {
      editAirspaceUrl->setMinimumWidth( strWidthTiltle );
    }
}

AirspaceDownloadDialog::~AirspaceDownloadDialog()
{
  // qDebug("AirspaceDownloadDialog::~AirspaceDownloadDialog()");
}

/** User has pressed Ok button */
void AirspaceDownloadDialog::accept()
{
  QString urlString = editAirspaceUrl->text().trimmed();
  QUrl url( urlString );

  if( url.isEmpty() || ! url.isValid() || url.scheme().isEmpty() ||
      url.host().isEmpty() || url.path().isEmpty() )
    {
      // No valid entry, ignore button press
      QMessageBox mb( QMessageBox::Information,
                      tr("Settings invalid!"),
                      tr("Please correct your URL settings!"),
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

  // Save last used url
  GeneralConfig::instance()->setLastAirspaceUrl( urlString );
  emit downloadAirspace( urlString );

  // restore sip state
  qApp->setAutoSipEnabled( m_autoSip );

  // close and destroy dialog
  QDialog::done(QDialog::Accepted);
}

/** User has pressed Cancel button */
void AirspaceDownloadDialog::reject()
{
  // restore sip state
  qApp->setAutoSipEnabled( m_autoSip );

  // close and destroy dialog
  QDialog::done(QDialog::Rejected);
}
