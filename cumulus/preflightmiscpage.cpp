/***********************************************************************
 **
 **   preflightmiscpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004      by André Somers
 **                   2008-2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#include <QtGui>

#include "preflightmiscpage.h"
#include "igclogger.h"
#include "generalconfig.h"
#include "layout.h"

PreFlightMiscPage::PreFlightMiscPage(QWidget *parent) :
  QWidget(parent),
  lastFocusWidget(0)
{
  setObjectName("PreFlightMiscPage");

  QGridLayout *topLayout = new QGridLayout(this);
  int row = 0;

  QLabel *lbl = new QLabel(tr("Minimal arrival altitude:"));
  topLayout->addWidget(lbl, row, 0);

  // get current set altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  altUnit = Altitude::getUnit();

  // Input accept only feet and meters all other make no sense. Therefore all
  // other (FL) is treated as feet.
  edtMinimalArrival = new QSpinBox;

  if (altUnit == Altitude::meters)
    {
      edtMinimalArrival->setMaximum(1000);
      edtMinimalArrival->setSingleStep(10);
    }
  else
    {
      edtMinimalArrival->setMaximum(3000);
      edtMinimalArrival->setSingleStep(100);
    }

  edtMinimalArrival->setButtonSymbols(QSpinBox::NoButtons);
  edtMinimalArrival->setFocusPolicy(Qt::StrongFocus);
  edtMinimalArrival->setSuffix(" " + Altitude::getUnitText());
  topLayout->addWidget(edtMinimalArrival, row, 1);
  topLayout->setColumnStretch(2, 2);
  row++;

  lbl = new QLabel(tr("Arrival altitude display:"));
  topLayout->addWidget(lbl, row, 0);
  edtArrivalAltitude = new QComboBox;
  edtArrivalAltitude->addItem( tr("Landing Target"), GeneralConfig::landingTarget );
  edtArrivalAltitude->addItem( tr("Next Target"), GeneralConfig::nextTarget );
  topLayout->addWidget(edtArrivalAltitude, row, 1);
  row++;

  lbl = new QLabel(tr("QNH:"));
  topLayout->addWidget(lbl, row, 0);
  edtQNH = new QSpinBox(this);
  edtQNH->setObjectName("QNH");
  edtQNH->setMaximum(1999);
  edtQNH->setButtonSymbols(QSpinBox::NoButtons);
  edtQNH->setFocusPolicy(Qt::StrongFocus);
  edtQNH->setSuffix(" hPa");
  topLayout->addWidget(edtQNH, row, 1);
  row++;

  topLayout->setRowMinimumHeight(row, 25);
  row++;

  chkLogAutoStart = new QCheckBox(tr("Autostart IGC logger"));
  topLayout->addWidget(chkLogAutoStart, row, 0, 1, 3);
  row++;

  lbl = new QLabel(tr("B-Record Interval:"));
  topLayout->addWidget(lbl, row, 0);
  bRecordInterval = new QSpinBox(this);
  bRecordInterval->setMinimum(1);
  bRecordInterval->setMaximum(60);
  bRecordInterval->setButtonSymbols(QSpinBox::NoButtons);
  bRecordInterval->setFocusPolicy(Qt::StrongFocus);
  bRecordInterval->setSuffix(" s");
  topLayout->addWidget(bRecordInterval, row, 1);
  row++;

  lbl = new QLabel(tr("K-Record Interval:"));
  topLayout->addWidget(lbl, row, 0);
  kRecordInterval = new QSpinBox(this);
  kRecordInterval->setMinimum(0);
  kRecordInterval->setMaximum(300);
  kRecordInterval->setButtonSymbols(QSpinBox::NoButtons);
  kRecordInterval->setFocusPolicy(Qt::StrongFocus);
  kRecordInterval->setSpecialValueText(tr("Off"));
  kRecordInterval->setSuffix(" s");
  topLayout->addWidget(kRecordInterval, row, 1);
  row++;

  topLayout->setRowStretch(row, 10);
  row++;

  // button size
  int size = IconSize + 10;

  // take a bold font for the plus and minus sign
  QFont bFont = font();
  bFont.setBold(true);

  plus   = new QPushButton("+");
  minus  = new QPushButton("-");

  plus->setToolTip( tr("Increase number value") );
  minus->setToolTip( tr("Decrease number value") );

  plus->setFont(bFont);
  minus->setFont(bFont);

  plus->setMinimumSize(size, size);
  minus->setMinimumSize(size, size);

  plus->setMaximumSize(size, size);
  minus->setMaximumSize(size, size);

  // The buttons have no focus policy to avoid a focus change during click of them.
  plus->setFocusPolicy(Qt::NoFocus);
  minus->setFocusPolicy(Qt::NoFocus);

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addStretch( 10 );
  buttonLayout->addWidget( plus );
  buttonLayout->addSpacing(20);
  buttonLayout->addWidget( minus );

  topLayout->addLayout(buttonLayout, row, 0, 1, 2);

  connect( plus, SIGNAL(clicked()),  this, SLOT(slotIncrementBox()));
  connect( minus, SIGNAL(clicked()), this, SLOT(slotDecrementBox()));

  /**
   * If the plus or minus button is clicked, the focus is changed to the main
   * window. I don't know why. Therefore the previous focused widget must be
   * saved, to have an indication, if a spinbox entry should be modified.
   */
  connect( QCoreApplication::instance(), SIGNAL(focusChanged( QWidget*, QWidget*)),
           this, SLOT( slotFocusChanged( QWidget*, QWidget*)) );
}

PreFlightMiscPage::~PreFlightMiscPage()
{
  // qDebug("PreFlightMiscPage::~PreFlightMiscPage()");
}

void PreFlightMiscPage::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // @AP: Arrival Altitude is always saved as meter.
  Altitude minArrival = conf->getSafetyAltitude();

  if (altUnit == Altitude::meters) // user wants meters
    {
      edtMinimalArrival->setValue((int) rint(minArrival.getMeters()));
      edtMinimalArrival->setSingleStep(50);
    }
  else // user gets feet
    {
      edtMinimalArrival->setValue((int) rint(minArrival.getFeet()));
      edtMinimalArrival->setSingleStep(100);
    }

  edtArrivalAltitude->setCurrentIndex( conf->getArrivalAltitudeDisplay() );

  edtQNH->setValue( conf->getQNH() );
  bRecordInterval->setValue( conf->getBRecordInterval() );
  kRecordInterval->setValue( conf->getKRecordInterval() );
  chkLogAutoStart->setChecked( conf->getLoggerAutostartMode() );
}

void PreFlightMiscPage::save()
{
  GeneralConfig *conf = GeneralConfig::instance();
  IgcLogger     *log  = IgcLogger::instance();

  if( chkLogAutoStart->isChecked() )
    {
      if ( ! log->getIsLogging() )
        {
          // Change logger mode to standby only, if logger is not on.
          log->Standby();
        }
    }
  else
    {
      if (log->getIsStandby())
        {
          log->Stop();
        }
    }

  conf->setLoggerAutostartMode( chkLogAutoStart->isChecked() );

  // @AP: Store altitude always as meter.
  if (altUnit == Altitude::meters)
    {
      conf->setSafetyAltitude(Altitude(edtMinimalArrival->value()));
    }
  else
    {
      Altitude currentAlt; // Altitude will be converted to feet

      currentAlt.setFeet((double) edtMinimalArrival->value());
      conf->setSafetyAltitude(currentAlt);
    }

  conf->setArrivalAltitudeDisplay( (GeneralConfig::ArrivalAltitudeDisplay) edtArrivalAltitude->itemData(edtArrivalAltitude->currentIndex()).toInt() );
  conf->setQNH(edtQNH->value());
  conf->setBRecordInterval(bRecordInterval->value());
  conf->setKRecordInterval(kRecordInterval->value());
}

void PreFlightMiscPage::slotFocusChanged( QWidget* oldWidget, QWidget* newWidget)
{
  Q_UNUSED(newWidget)

  // We save the old widget, which has just lost the focus.
  lastFocusWidget = oldWidget;
}

void PreFlightMiscPage::slotIncrementBox()
{
  // Look which spin box has the focus. Note, focus can be changed by clicking
  // the connected button. Therefore take old focus widget under account and
  // set the focus back to the spinbox.
  QSpinBox* spinBoxList[] = {edtMinimalArrival, edtQNH, bRecordInterval, kRecordInterval};

  for( uint i = 0; i < (sizeof(spinBoxList) / sizeof(spinBoxList[0])); i++ )
    {
      if( QApplication::focusWidget() == spinBoxList[i] || lastFocusWidget == spinBoxList[i] )
        {
          spinBoxList[i]->setValue( spinBoxList[i]->value() + spinBoxList[i]->singleStep() );
          spinBoxList[i]->setFocus();
          return;
        }
    }
}

void PreFlightMiscPage::slotDecrementBox()
{
  // Look which spin box has the focus. Note, focus can be changed by clicking
  // the connected button. Therefore take old focus widget under account and
  // set the focus back to the spinbox.
  QSpinBox* spinBoxList[] = {edtMinimalArrival, edtQNH, bRecordInterval, kRecordInterval};

  for( uint i = 0; i < (sizeof(spinBoxList) / sizeof(spinBoxList[0])); i++ )
    {
      if( QApplication::focusWidget() == spinBoxList[i] || lastFocusWidget == spinBoxList[i] )
        {
          spinBoxList[i]->setValue( spinBoxList[i]->value() - spinBoxList[i]->singleStep() );
          spinBoxList[i]->setFocus();
          return;
        }
    }
}
