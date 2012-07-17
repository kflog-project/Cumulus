/***********************************************************************
 **
 **   preflightflarmpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "preflightflarmpage.h"

PreFlightFlarmPage::PreFlightFlarmPage(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("PreFlightFlarmPage");

  QVBoxLayout *allLayout  = new QVBoxLayout(this);
  QHBoxLayout *lineLayout = new QHBoxLayout;

  lineLayout->addWidget( new QLabel(tr("HWV:")));
  hwVersion = new QLabel;
  lineLayout->addWidget( hwVersion );
  lineLayout->addSpacing( 20 );

  lineLayout->addWidget( new QLabel(tr("SWV:")));
  swVersion = new QLabel;
  lineLayout->addWidget( swVersion );
  lineLayout->addSpacing( 20 );

  lineLayout->addWidget( new QLabel(tr("DBV:")));
  obstVersion = new QLabel;
  lineLayout->addWidget( obstVersion );
  lineLayout->addStretch( 10 );

  allLayout->addLayout(lineLayout );

  //----------------------------------------------------------------------------

  lineLayout = new QHBoxLayout;

  lineLayout->addWidget( new QLabel(tr("Severity:")));
  errSeverity = new QLabel;
  lineLayout->addWidget( errSeverity );
  lineLayout->addSpacing( 20 );

  lineLayout->addWidget( new QLabel(tr("Error:")));
  errCode = new QLabel;
  lineLayout->addWidget( errCode );
  lineLayout->addStretch( 10 );

  allLayout->addLayout(lineLayout );

  //----------------------------------------------------------------------------

  lineLayout = new QHBoxLayout;

  lineLayout->addWidget( new QLabel(tr("Log Interval:")));
  logInt = new QComboBox;
  lineLayout->addWidget( logInt );
  lineLayout->addStretch( 10 );

  allLayout->addLayout(lineLayout );

  //----------------------------------------------------------------------------

  QGridLayout* gridLayout = new QGridLayout;

  gridLayout->addWidget( new QLabel(tr("Pilot:")), 0, 0);
  pilot = new QLineEdit;
  gridLayout->addWidget( pilot, 0, 1 );

  gridLayout->addWidget( new QLabel(tr("Co-Pilot:")), 0, 2);
  copil = new QLineEdit;
  gridLayout->addWidget( copil, 0, 3 );

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("Glider Id:")), 1, 0);
  gliderId = new QLineEdit;
  gridLayout->addWidget( gliderId, 1, 1 );

  gridLayout->addWidget( new QLabel(tr("Glider Type:")), 1, 2);
  gliderType = new QLineEdit;
  gridLayout->addWidget( gliderType, 1, 3 );

  //----------------------------------------------------------------------------

  gridLayout->addWidget( new QLabel(tr("Comp Id:"))), 2, 0;
  compId = new QLineEdit;
  gridLayout->addWidget( compId, 2, 1 );

  gridLayout->addWidget( new QLabel(tr("Comp Class:")), 2, 2);
  compClass = new QLineEdit;
  gridLayout->addWidget( compClass, 2, 3 );

  allLayout->addLayout(gridLayout );
  allLayout->addStretch( 10 );

  //----------------------------------------------------------------------------

  connect( Flarm::instance(), SIGNAL(flarmVersionInfo(const Flarm::FlarmVersion&)),
            this, SLOT(slotUpdateVersions(const Flarm::FlarmVersion&)) );

  connect( Flarm::instance(), SIGNAL(flarmErrorInfo( const Flarm::FlarmError&)),
            this, SLOT(slotUpdateErrors(const Flarm::FlarmError& )) );

}

PreFlightFlarmPage::~PreFlightFlarmPage()
{
}

void PreFlightFlarmPage::slotUpdateVersions( const Flarm::FlarmVersion& info )
{
  hwVersion->setText( info.hwVersion);
  swVersion->setText( info.swVersion);
  obstVersion->setText( info.obstVersion);
}

void PreFlightFlarmPage::slotUpdateErrors( const Flarm::FlarmError& info )
{
  errSeverity->setText( info.severity);
  errCode->setText( info.errorCode);
}

