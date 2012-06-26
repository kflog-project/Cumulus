/***********************************************************************
**
**    proxydialog.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/*
 * This widget asks for a proxy name and port. On accept the settings are checked
 * for correctness and stored in the GeneralConfig data.
 */

#include <QtGui>

#include "proxydialog.h"
#include "generalconfig.h"

ProxyDialog::ProxyDialog( QWidget *parent ) :
  QDialog(parent)
{
  setObjectName( "ProxyDialog" );
  setWindowTitle( tr("Proxy Configuration") );
  setAttribute( Qt::WA_DeleteOnClose );
  setSizeGripEnabled( true );

  hostEdit = new QLineEdit;
  portEdit = new QLineEdit;
  QIntValidator* iv = new QIntValidator(this);
  iv->setRange(0, 65535);
  portEdit->setValidator( iv );

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow( new QLabel(tr("Host:")), hostEdit );
  formLayout->addRow( new QLabel(tr("Port:")), portEdit );

  buttonBox = new QDialogButtonBox( QDialogButtonBox::Reset |
                                    QDialogButtonBox::Cancel |
                                    QDialogButtonBox::Ok );
  buttonBox->setCenterButtons( true );
  QPushButton *ok =  buttonBox->button( QDialogButtonBox::Ok );
  ok->setDefault( true );

  QPushButton *cancel = buttonBox->button( QDialogButtonBox::Cancel );
  cancel->setAutoDefault(false);

  QPushButton *reset = buttonBox->button( QDialogButtonBox::Reset );
  reset->setAutoDefault(false);

  QVBoxLayout *vBox = new QVBoxLayout;

  vBox->addLayout( formLayout );
  vBox->addWidget( buttonBox );

  setLayout( vBox );

  connect( buttonBox, SIGNAL(accepted()), this, SLOT(accept()) );
  connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()) );
  connect( buttonBox, SIGNAL(clicked( QAbstractButton * ) ),
           this, SLOT(clicked( QAbstractButton * )) );

  QString proxyString = GeneralConfig::instance()->getProxy();

  if( ! proxyString.isEmpty() )
    {
      QStringList list = proxyString.split( ":" );

      if( ! list.at(0).isEmpty() )
        {
          hostEdit->setText( list.at(0).trimmed() );
        }

      if( ! list.at(1).isEmpty() )
        {
          portEdit->setText( list.at(1).trimmed() );
        }
    }

  // Set minimum size of input line
  QFontMetrics fm( font() );
  int strWidth = fm.width(QString("*")) * 30;
  int strWidthTiltle = fm.width( windowTitle() );

  if( strWidth >= strWidthTiltle )
    {
      hostEdit->setMinimumWidth( strWidth );
    }
  else
    {
      hostEdit->setMinimumWidth( strWidthTiltle );
    }
}

ProxyDialog::~ProxyDialog()
{
  // qDebug("ProxyDialog::~ProxyDialog()");
}

/** User has pressed Ok button */
void ProxyDialog::accept()
{
  // Get results from input text
  QString host = hostEdit->text().trimmed();
  QString port = portEdit->text().trimmed();

  if( host.isEmpty() && port.isEmpty() )
    {
      // that means no proxy is set
      GeneralConfig::instance()->setProxy( "" );
    }
  else
    {
      bool ok = false;
      ushort portNum = port.toUShort(&ok, 10);

      if( host.isEmpty() || port.isEmpty() || ok == false || portNum == 0 )
        {
          QMessageBox::information ( this,
                                     tr("Settings invalid!"),
                                     tr("Please correct your Proxy settings!") );
          return;
        }

      QString proxyString = host + ":" + port;

      GeneralConfig::instance()->setProxy( proxyString );
    }

  // close and destroy dialog
  QDialog::done(QDialog::Accepted);
}

/** User has pressed Cancel button */
void ProxyDialog::reject()
{
  // close and destroy dialog
  QDialog::done(QDialog::Rejected);
}

/** User has clicked a button. */
void ProxyDialog::clicked( QAbstractButton *button )
{
  if( buttonBox->standardButton( button ) == QDialogButtonBox::Reset )
    {
      // Reset has pressed, clear the input fields
      hostEdit->clear();
      portEdit->clear();
    }
}
