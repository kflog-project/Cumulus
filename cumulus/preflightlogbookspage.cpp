/***********************************************************************
 **
 **   preflightlogbookspage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2013-2017 by Axel Pauli
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

#include "calculator.h"
#include "layout.h"
#include "logbook.h"
#include "helpbrowser.h"
#include "preflightlogbookspage.h"

#ifdef FLARM
#include "flarmbase.h"
#include "flarmlogbook.h"
#include "preflightflarmusbpage.h"
#endif

PreFlightLogBooksPage::PreFlightLogBooksPage(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("PreFlightLogBooksPage");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("PreFlight - Logbooks") );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *contentLayout = new QHBoxLayout(this);

  // Top layout's parent is the scroll widget
  QGridLayout *topLayout = new QGridLayout;
  contentLayout->addLayout( topLayout );

  int row = 0;

  QLabel* lbl = new QLabel(tr("<b>Logbooks</b>"));
  topLayout->addWidget( lbl, row, 0, 1, 2 );
  row++;

  topLayout->setRowStretch(row, 10);
  row++;

  lbl = new QLabel(tr("My flight book:"));
  topLayout->addWidget(lbl, row, 0);
  QPushButton* button = new QPushButton( tr("Open") );
  topLayout->addWidget(button, row, 1 );
  row++;

  connect(button, SIGNAL(pressed()), SLOT(slotOpenLogbook()));

#ifdef FLARM
  topLayout->setRowMinimumHeight(row, 50);
  row++;

  lbl = new QLabel(tr("Flarm flight book:"));
  topLayout->addWidget(lbl, row, 0);

  button = new QPushButton( tr("Open") );
  topLayout->addWidget(button, row, 1 );
  row++;

  extern Calculator *calculator;

  if( calculator->moving() )
    {
      // Disable Flarm flight downloads if we are moving.
      button->setEnabled( false );
    }
  else
    {
      connect(button, SIGNAL(pressed()), SLOT(slotOpenFlarmFlights()));
    }

  if( Flarm::getFlarmData().devtype.startsWith( "PowerFLARM-") == true )
    {
      topLayout->setRowMinimumHeight(row, 50);
      row++;

      lbl = new QLabel(tr("Flights to USB stick:"));
      topLayout->addWidget(lbl, row, 0);

      button = new QPushButton( tr("Start") );
      topLayout->addWidget(button, row, 1 );
      row++;

      extern Calculator *calculator;

      if( calculator->moving() )
        {
          // Disable Flarm flight downloads if we are moving.
          button->setEnabled( false );
        }
      else
        {
          connect(button, SIGNAL(pressed()), SLOT(slotTransferFlights2USB()));
        }
    }

#endif

  topLayout->setRowStretch(row, 10);
  topLayout->setColumnStretch(2, 10);

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

  connect(help, SIGNAL(pressed()), this, SLOT(slotHelp()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(help, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);
}

PreFlightLogBooksPage::~PreFlightLogBooksPage()
{
  // qDebug("PreFlightLogBooksPage::~PreFlightLogBooksPage()");
}

void PreFlightLogBooksPage::slotHelp()
{
  QString file = "cumulus-preflight-settings-logbooks.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void PreFlightLogBooksPage::slotAccept()
{
  emit closingWidget();
  QWidget::close();
}

void PreFlightLogBooksPage::slotReject()
{
  emit closingWidget();
  QWidget::close();
}

void PreFlightLogBooksPage::slotOpenLogbook()
{
  Logbook* lbw = new Logbook( this );
  lbw->setVisible( true );
}

#ifdef FLARM

/**
 * Called to open the Flarm flight download dialog.
 */
void PreFlightLogBooksPage::slotOpenFlarmFlights()
{
  FlarmLogbook* flb = new FlarmLogbook( this );
  flb->setVisible( true );
}

void PreFlightLogBooksPage::slotTransferFlights2USB()
  {
    PreFlightFlarmUsbPage* usbp = new PreFlightFlarmUsbPage( this );
    usbp->setVisible( true );
  }

#endif
