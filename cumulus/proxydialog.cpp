/***********************************************************************
**
**    proxydialog.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2018 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/*
 * This widget asks for a proxy name and port. On accept the settings are checked
 * for correctness and stored in the GeneralConfig data.
 */

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "proxydialog.h"
#include "generalconfig.h"

ProxyDialog::ProxyDialog( QWidget *parent ) :
  QFrame(parent)
{
  setObjectName( "ProxyDialog" );
  setWindowTitle( tr("Proxy Configuration") );
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute( Qt::WA_DeleteOnClose );

#ifdef ANDROID
  // Activate box drawing to make the widget better visible under Android.
  setFrameStyle( QFrame::Box );
  setLineWidth( 3 );
  QString style = "QWidget { background: lightgray }";
  setStyleSheet( style );
#endif

  Qt::InputMethodHints imh;

  hostEdit = new QLineEdit;
  portEdit = new QLineEdit;

  imh = (hostEdit->inputMethodHints() | Qt::ImhNoPredictiveText);
  hostEdit->setInputMethodHints(imh);
  portEdit->setInputMethodHints( Qt::ImhNoPredictiveText | Qt::ImhDigitsOnly );

  QIntValidator* iv = new QIntValidator(this);
  iv->setRange(0, 65535);
  portEdit->setValidator( iv );

  connect( hostEdit, SIGNAL(returnPressed()),
           this, SLOT(slotCloseSip()) );

  connect( portEdit, SIGNAL(returnPressed()),
           this, SLOT(slotCloseSip()) );

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow( new QLabel(tr("Host:")), hostEdit );
  formLayout->addRow( new QLabel(tr("Port:")), portEdit );

  buttonBox = new QDialogButtonBox( QDialogButtonBox::Reset |
                                    QDialogButtonBox::Cancel |
                                    QDialogButtonBox::Ok );
  buttonBox->setCenterButtons( true );
  QPushButton *ok =  buttonBox->button( QDialogButtonBox::Ok );
  ok->setDefault( false );
  ok->setAutoDefault( false );

  QPushButton *cancel = buttonBox->button( QDialogButtonBox::Cancel );
  cancel->setDefault(false);
  cancel->setAutoDefault(false);

  QPushButton *reset = buttonBox->button( QDialogButtonBox::Reset );
  reset->setDefault(false);
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
          QMessageBox mb( QMessageBox::Information,
                          tr("Settings invalid!"),
                          tr("Please correct your Proxy settings!"),
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

      QString proxyString = host + ":" + port;

      GeneralConfig::instance()->setProxy( proxyString );
    }

  emit proxyDataChanged();

  // close and destroy dialog
  QWidget::close();
}

/** User has pressed Cancel button */
void ProxyDialog::reject()
{
  // close and destroy dialog
  QWidget::close();
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

void ProxyDialog::slotCloseSip()
{
  // Get the widget which has the keyboard focus assigned.
  QWidget *widget = QApplication::focusWidget();

  if( ! widget )
    {
      // No widget has the focus.
      return;
    }

  // Request the SIP closing from the focused widget.
  QEvent event( QEvent::CloseSoftwareInputPanel );
  QApplication::sendEvent( widget, &event );
}
