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

#include "calculator.h"
#include "generalconfig.h"
#include "layout.h"
#include "logbook.h"
#include "igclogger.h"
#include "varspinbox.h"

#ifdef FLARM
#include "flarm.h"
#include "flarmlogbook.h"
#include "gpsnmea.h"
#endif

#ifdef USE_NUM_PAD
#include "doubleNumberEditor.h"
#include "numberEditor.h"
#endif

PreFlightMiscPage::PreFlightMiscPage(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("PreFlightMiscPage");

  QGridLayout *topLayout = new QGridLayout(this);
  int row = 0;

  QLabel *lbl = new QLabel(tr("Minimal arrival altitude:"));
  topLayout->addWidget(lbl, row, 0);

  // get current set altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  m_altUnit = Altitude::getUnit();

  VarSpinBox* hspin;

  // Input accept only feet and meters all other make no sense. Therefore all
  // other (FL) is treated as feet.
#ifdef USE_NUM_PAD
  m_edtMinimalArrival = new NumberEditor;
  m_edtMinimalArrival->setDecimalVisible( false );
  m_edtMinimalArrival->setPmVisible( false );
  m_edtMinimalArrival->setRange( 0, 9999);
  m_edtMinimalArrival->setMaxLength(4);
  m_edtMinimalArrival->setSuffix(" " + Altitude::getUnitText());

  int maw = QFontMetrics(font()).width("9999 ft") + 10;
  m_edtMinimalArrival->setMinimumWidth( maw );

  topLayout->addWidget(m_edtMinimalArrival, row, 1);
#else
  m_edtMinimalArrival = new QSpinBox;

  if (m_altUnit == Altitude::meters)
    {
      m_edtMinimalArrival->setMaximum(1000);
      m_edtMinimalArrival->setSingleStep(10);
    }
  else
    {
      m_edtMinimalArrival->setMaximum(3000);
      m_edtMinimalArrival->setSingleStep(100);
    }

  m_edtMinimalArrival->setSuffix(" " + Altitude::getUnitText());
  hspin = new VarSpinBox(m_edtMinimalArrival);
  topLayout->addWidget(hspin, row, 1);
#endif

  topLayout->setColumnStretch(2, 2);
  row++;

  lbl = new QLabel(tr("Arrival altitude display:"));
  topLayout->addWidget(lbl, row, 0);
  m_edtArrivalAltitude = new QComboBox;
  m_edtArrivalAltitude->addItem( tr("Landing Target"), GeneralConfig::landingTarget );
  m_edtArrivalAltitude->addItem( tr("Next Target"), GeneralConfig::nextTarget );
  topLayout->addWidget(m_edtArrivalAltitude, row, 1);
  row++;

  lbl = new QLabel(tr("QNH:"));
  topLayout->addWidget(lbl, row, 0);

#ifdef USE_NUM_PAD
  m_edtQNH = new NumberEditor;
  m_edtQNH->setDecimalVisible( false );
  m_edtQNH->setPmVisible( false );
  m_edtQNH->setRange( 0, 9999);
  m_edtQNH->setMaxLength(4);
  m_edtQNH->setSuffix(" hPa");

  int mqw = QFontMetrics(font()).width("9999 hPa") + 10;
  m_edtQNH->setMinimumWidth( mqw );

  topLayout->addWidget(m_edtQNH, row, 1);
#else
  m_edtQNH = new QSpinBox(this);
  m_edtQNH->setMaximum(1999);
  m_edtQNH->setSuffix(" hPa");
  hspin = new VarSpinBox(m_edtQNH);
  topLayout->addWidget(hspin, row, 1);
#endif

  row++;

  topLayout->setRowMinimumHeight(row, 10);
  row++;

  m_chkLogAutoStart = new QCheckBox(tr("Autostart IGC logger"));
  topLayout->addWidget(m_chkLogAutoStart, row, 0 );

  // get current used horizontal speed unit. This unit must be considered
  // during storage.
  m_speedUnit = Speed::getHorizontalUnit();

#ifdef USE_NUM_PAD
  m_logAutoStartSpeed = new DoubleNumberEditor( this );
  m_logAutoStartSpeed->setDecimalVisible( true );
  m_logAutoStartSpeed->setPmVisible( false );
  m_logAutoStartSpeed->setMaxLength(4);
  m_logAutoStartSpeed->setRange( 1.0, 99.9);
  m_logAutoStartSpeed->setPrefix( "> " );
  m_logAutoStartSpeed->setSuffix( QString(" ") + Speed::getHorizontalUnitText() );
  m_logAutoStartSpeed->setDecimals( 1 );

  int mlw = QFontMetrics(font()).width("99.9" + Speed::getHorizontalUnitText()) + 10;
  m_logAutoStartSpeed->setMinimumWidth( mlw );

  topLayout->addWidget( m_logAutoStartSpeed, row, 1 );
#else
  m_logAutoStartSpeed = new QDoubleSpinBox( this );
  m_logAutoStartSpeed->setButtonSymbols(QSpinBox::PlusMinus);
  m_logAutoStartSpeed->setRange( 1.0, 99.0);
  m_logAutoStartSpeed->setSingleStep( 1 );
  m_logAutoStartSpeed->setPrefix( "> " );
  m_logAutoStartSpeed->setDecimals( 1 );
  m_logAutoStartSpeed->setSuffix( QString(" ") + Speed::getHorizontalUnitText() );
  hspin = new VarSpinBox( m_logAutoStartSpeed );
  topLayout->addWidget( hspin, row, 1 );
#endif

  row++;

  lbl = new QLabel(tr("B-Record Interval:"));
  topLayout->addWidget(lbl, row, 0);
  m_bRecordInterval = new QSpinBox(this);
  m_bRecordInterval->setMinimum(1);
  m_bRecordInterval->setMaximum(60);
  m_bRecordInterval->setSuffix(" s");
  hspin = new VarSpinBox(m_bRecordInterval);
  topLayout->addWidget(hspin, row, 1);
  row++;

  lbl = new QLabel(tr("K-Record Interval:"));
  topLayout->addWidget(lbl, row, 0);
  m_kRecordInterval = new QSpinBox(this);
  m_kRecordInterval->setMinimum(0);
  m_kRecordInterval->setMaximum(300);
  m_kRecordInterval->setSpecialValueText(tr("Off"));
  m_kRecordInterval->setSuffix(" s");
  hspin = new VarSpinBox(m_kRecordInterval);
  topLayout->addWidget(hspin, row, 1);
  row++;

  topLayout->setRowMinimumHeight(row, 10);
  row++;

  lbl = new QLabel(tr("My flight book:"));
  topLayout->addWidget(lbl, row, 0);
  QPushButton* button = new QPushButton( tr("Open") );
  topLayout->addWidget(button, row, 1 );
  row++;

  connect(button, SIGNAL(pressed()), SLOT(slotOpenLogbook()));

#ifdef FLARM
  topLayout->setRowMinimumHeight(row, 10);
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

  if (m_altUnit == Altitude::meters) // user wants meters
    {
      m_edtMinimalArrival->setValue((int) rint(minArrival.getMeters()));

#ifndef USE_NUM_PAD
      m_edtMinimalArrival->setSingleStep(50);
#endif
    }
  else // user gets feet
    {
      m_edtMinimalArrival->setValue((int) rint(minArrival.getFeet()));

#ifndef USE_NUM_PAD
      m_edtMinimalArrival->setSingleStep(100);
#endif
    }

  m_edtArrivalAltitude->setCurrentIndex( conf->getArrivalAltitudeDisplay() );

  m_edtQNH->setValue( conf->getQNH() );
  m_bRecordInterval->setValue( conf->getBRecordInterval() );
  m_kRecordInterval->setValue( conf->getKRecordInterval() );
  m_chkLogAutoStart->setChecked( conf->getLoggerAutostartMode() );

  Speed speed;
  // speed is stored in Km/h
  speed.setKph( GeneralConfig::instance()->getAutoLoggerStartSpeed() );
  m_logAutoStartSpeed->setValue( speed.getValueInUnit( m_speedUnit ) );

  // save loaded value for change control
  m_loadedSpeed = m_logAutoStartSpeed->value();
}

void PreFlightMiscPage::save()
{
  GeneralConfig *conf = GeneralConfig::instance();
  IgcLogger     *log  = IgcLogger::instance();

  if( m_chkLogAutoStart->isChecked() )
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

  conf->setLoggerAutostartMode( m_chkLogAutoStart->isChecked() );

  // @AP: Store altitude always as meter.
  if (m_altUnit == Altitude::meters)
    {
      conf->setSafetyAltitude(Altitude(m_edtMinimalArrival->value()));
    }
  else
    {
      Altitude currentAlt; // Altitude will be converted to feet

      currentAlt.setFeet((double) m_edtMinimalArrival->value());
      conf->setSafetyAltitude(currentAlt);
    }

  conf->setArrivalAltitudeDisplay( (GeneralConfig::ArrivalAltitudeDisplay) m_edtArrivalAltitude->itemData(m_edtArrivalAltitude->currentIndex()).toInt() );
  conf->setQNH(m_edtQNH->value());
  conf->setBRecordInterval(m_bRecordInterval->value());
  conf->setKRecordInterval(m_kRecordInterval->value());

  if( m_loadedSpeed != m_logAutoStartSpeed->value() )
    {
      Speed speed;
      speed.setValueInUnit( m_logAutoStartSpeed->value(), m_speedUnit );

      // store speed in Km/h
      GeneralConfig::instance()->setAutoLoggerStartSpeed( speed.getKph() );
    }
}

void PreFlightMiscPage::slotOpenLogbook()
{
  Logbook* lbw = new Logbook( this );
  lbw->setVisible( true );
}

#ifdef FLARM

/**
 * Called to open the Flarm flight download dialog.
 */
void PreFlightMiscPage::slotOpenFlarmFlights()
{
  FlarmLogbook* flb = new FlarmLogbook( this );
  flb->setVisible( true );
}

#endif
