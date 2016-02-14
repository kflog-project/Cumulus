/***********************************************************************
 **
 **   preflightlogbookspage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2013-2016 by Axel Pauli
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
#include "preflightlogbookspage.h"

#ifdef FLARM
#include "flarmlogbook.h"
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

#endif

  topLayout->setRowStretch(row, 10);
  topLayout->setColumnStretch(2, 10);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
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

#endif
