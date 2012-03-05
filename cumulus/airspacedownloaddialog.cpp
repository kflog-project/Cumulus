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

/**
 * This dialog manages the download of an airspace file via HTTP.
**/

#include <QtGui>

#include "airspacedownloaddialog.h"
#include "generalconfig.h"
#include "proxydialog.h"

AirspaceDownloadDialog::AirspaceDownloadDialog( QWidget *parent ) :
  QDialog(parent)
{
  setObjectName( "AirspaceDownloadDialog" );
  setWindowTitle( tr("Download Airspace") );
  setAttribute( Qt::WA_DeleteOnClose );
  setModal(true);
  setSizeGripEnabled( true );

  QGridLayout* gridLayout = new QGridLayout;
  int row=0;

  editProxy = new QPushButton( tr("Set Proxy"), this );
  editProxy->setToolTip(tr("Enter Proxy data if needed"));

  connect( editProxy, SIGNAL( clicked()), this, SLOT(slot_editProxy()) );

  gridLayout->addWidget(editProxy, row, 0);
  proxyDisplay = new QLabel(this);
  gridLayout->addWidget(proxyDisplay, row, 1);
  row++;

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

  QString proxyString = GeneralConfig::instance()->getProxy();

  if( ! proxyString.isEmpty() )
    {
      proxyDisplay->setText( proxyString );
    }

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

  // close and destroy dialog
  QDialog::done(QDialog::Accepted);
}

/** User has pressed Cancel button */
void AirspaceDownloadDialog::reject()
{
  // close and destroy dialog
  QDialog::done(QDialog::Rejected);
}

/**
 * Opens proxy dialog on user request.
 */
void AirspaceDownloadDialog::slot_editProxy()
{
  ProxyDialog *dialog = new ProxyDialog( this );

  if( dialog->exec() == QDialog::Accepted )
    {
      // update proxy display
      proxyDisplay->setText( GeneralConfig::instance()->getProxy() );
    }
}
