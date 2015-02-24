/***********************************************************************
 **
 **   preflightmiscpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004      by André Somers
 **                   2008-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "calculator.h"
#include "doubleNumberEditor.h"
#include "generalconfig.h"
#include "igclogger.h"
#include "layout.h"
#include "numberEditor.h"
#include "preflightmiscpage.h"

PreFlightMiscPage::PreFlightMiscPage(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("PreFlightMiscPage");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("PreFlight - Common") );

  if( parent )
    {
      resize( parent->size() );
    }

  // Layout used by scroll area
  QHBoxLayout *sal = new QHBoxLayout;

  // new widget used as container for the dialog layout.
  QWidget* sw = new QWidget;

  // Scroll area
  QScrollArea* sa = new QScrollArea;
  sa->setWidgetResizable( true );
  sa->setFrameStyle( QFrame::NoFrame );
  sa->setWidget( sw );

#ifdef QSCROLLER
  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // Add scroll area to its own layout
  sal->addWidget( sa );

  QHBoxLayout *contentLayout = new QHBoxLayout(this);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal, 10 );

  // Top layout's parent is the scroll widget
  QGridLayout *topLayout = new QGridLayout(sw);

  int row = 0;

  QLabel *lbl = new QLabel(tr("Minimal arrival altitude:"));
  topLayout->addWidget(lbl, row, 0);

  // get current set altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  m_altUnit = Altitude::getUnit();

  // Input accept only feet and meters all other make no sense. Therefore all
  // other (FL) is treated as feet.
  m_edtMinimalArrival = new NumberEditor;
  m_edtMinimalArrival->setDecimalVisible( false );
  m_edtMinimalArrival->setPmVisible( false );
  m_edtMinimalArrival->setRange( 0, 9999);
  m_edtMinimalArrival->setMaxLength(4);
  m_edtMinimalArrival->setSuffix(" " + Altitude::getUnitText());

  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,4})" ), this );
  m_edtMinimalArrival->setValidator( eValidator );

  int maw = QFontMetrics(font()).width("9999 ft") + 10;
  m_edtMinimalArrival->setMinimumWidth( maw );

  topLayout->addWidget(m_edtMinimalArrival, row, 1);
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

  m_edtQNH = new NumberEditor;
  m_edtQNH->setDecimalVisible( false );
  m_edtQNH->setPmVisible( false );
  m_edtQNH->setRange( 0, 9999);
  m_edtQNH->setMaxLength(4);
  m_edtQNH->setSuffix(" hPa");

  eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,4})" ), this );
  m_edtQNH->setValidator( eValidator );

  int mqw = QFontMetrics(font()).width("9999 hPa") + 10;
  m_edtQNH->setMinimumWidth( mqw );

  topLayout->addWidget(m_edtQNH, row, 1);
  row++;

  lbl = new QLabel(tr("LD average time") + ":");
  topLayout->addWidget(lbl, row, 0);

  m_edtLDTime = new NumberEditor;
  m_edtLDTime->setDecimalVisible( false );
  m_edtLDTime->setPmVisible( false );
  m_edtLDTime->setRange( 5, 600 );
  m_edtLDTime->setMaxLength(3);
  m_edtLDTime->setSuffix(" s");
  m_edtLDTime->setTitle( tr("LD average time") );
  m_edtLDTime->setTip( "5 ... 600" );

  eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,3})" ), this );
  m_edtLDTime->setValidator( eValidator );

  topLayout->addWidget(m_edtLDTime, row, 1);
  row++;

  topLayout->setRowMinimumHeight(row, 10);
  row++;

  m_chkLogAutoStart = new QCheckBox(tr("Autostart IGC logger"));
  topLayout->addWidget(m_chkLogAutoStart, row, 0 );

  // get current used horizontal speed unit. This unit must be considered
  // during storage.
  m_speedUnit = Speed::getHorizontalUnit();

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
  row++;

  lbl = new QLabel(tr("B-Record Interval:"));
  topLayout->addWidget(lbl, row, 0);

  m_bRecordInterval = new NumberEditor;
  m_bRecordInterval->setDecimalVisible( false );
  m_bRecordInterval->setPmVisible( false );
  m_bRecordInterval->setRange( 1, 60);
  m_bRecordInterval->setTip("1...60");
  m_bRecordInterval->setMaxLength(2);
  m_bRecordInterval->setSuffix(" s");

  eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,2})" ), this );
  m_bRecordInterval->setValidator( eValidator );

  int mbrw = QFontMetrics(font()).width("99 s") + 10;
  m_bRecordInterval->setMinimumWidth( mbrw );

  topLayout->addWidget(m_bRecordInterval, row, 1);
  row++;

  lbl = new QLabel(tr("K-Record Interval:"));
  topLayout->addWidget(lbl, row, 0);

  m_kRecordInterval = new NumberEditor;
  m_kRecordInterval->setDecimalVisible( false );
  m_kRecordInterval->setPmVisible( false );
  m_kRecordInterval->setRange( 0, 300);
  m_kRecordInterval->setTip("0...300");
  m_kRecordInterval->setMaxLength(3);
  m_kRecordInterval->setSuffix(" s");
  m_kRecordInterval->setSpecialValueText(tr("Off"));

  eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,2})" ), this );
  m_kRecordInterval->setValidator( eValidator );

  int mkrw = QFontMetrics(font()).width("999 s") + 10;
  m_kRecordInterval->setMinimumWidth( mkrw );

  topLayout->addWidget(m_kRecordInterval, row, 1);
  row++;

  topLayout->setRowMinimumHeight(row, 10);
  row++;

  topLayout->setRowStretch(row, 10);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png"));

  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);

  load();
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
    }
  else // user gets feet
    {
      m_edtMinimalArrival->setValue((int) rint(minArrival.getFeet()));
    }

  m_edtArrivalAltitude->setCurrentIndex( conf->getArrivalAltitudeDisplay() );

  m_edtQNH->setValue( conf->getQNH() );
  m_edtLDTime->setValue( conf->getLDCalculationTime() );
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
  conf->setLDCalculationTime(m_edtLDTime->value());
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

void PreFlightMiscPage::slotAccept()
{
  save();
  GeneralConfig::instance()->save();
  emit settingsChanged();
  emit closingWidget();
  QWidget::close();
}

void PreFlightMiscPage::slotReject()
{
  emit closingWidget();
  QWidget::close();
}
