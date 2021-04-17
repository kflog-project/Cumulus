/***********************************************************************
 **
 **   SettingsPageWind.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2021 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
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

#include "doubleNumberEditor.h"
#include "generalconfig.h"
#include "helpbrowser.h"
#include "layout.h"
#include "numberEditor.h"
#include "SettingsPageWind.h"

SettingsPageWind::SettingsPageWind( QWidget *parent ) :
  QWidget(parent)
{
  setObjectName("SettingsPageWind");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Wind") );

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

  QLabel *lbl = new QLabel( tr( "Wind settings for straight flight" ) );
  topLayout->addWidget( lbl, row, 0, 1, 2 );
  row++;

  topLayout->setRowMinimumHeight( row, 50 * Layout::getScaledDensity() );

  m_enableWindInSF = new QCheckBox(tr("Enable wind calculation in straight flight"));
  topLayout->addWidget( m_enableWindInSF, row, 0, 1, 2 );
  row++;

  // get current used horizontal speed unit. This unit must be considered
  // during storage.
  m_speedUnit = Speed::getHorizontalUnit();

  lbl = new QLabel( tr( "Minimum airspeed:" ) );
  topLayout->addWidget( lbl, row, 0 );

  m_minimumAirSpeed = new DoubleNumberEditor( this );
  m_minimumAirSpeed->setDecimalVisible( true );
  m_minimumAirSpeed->setPmVisible( false );
  m_minimumAirSpeed->setMaxLength(4);
  m_minimumAirSpeed->setRange( 1.0, 99.9);
  m_minimumAirSpeed->setPrefix( "> " );
  m_minimumAirSpeed->setSuffix( QString(" ") + Speed::getHorizontalUnitText() );
  m_minimumAirSpeed->setDecimals( 1 );

  int mlw = QFontMetrics(font()).width("99.9" + Speed::getHorizontalUnitText()) + 10;
  m_minimumAirSpeed->setMinimumWidth( mlw );

  topLayout->addWidget( m_minimumAirSpeed, row, 1 );
  row++;

  lbl = new QLabel( tr( "Speed tolerance:" ) );
  topLayout->addWidget( lbl, row, 0 );

  m_speedTolerance = new DoubleNumberEditor( this );
  m_speedTolerance->setDecimalVisible( true );
  m_speedTolerance->setPmVisible( false );
  m_speedTolerance->setMaxLength(4);
  m_speedTolerance->setRange( 1.0, 99.9);
  m_speedTolerance->setSuffix( QString(" ") + Speed::getHorizontalUnitText() );
  m_speedTolerance->setDecimals( 1 );

  mlw = QFontMetrics(font()).width("99.9" + Speed::getHorizontalUnitText()) + 10;
  m_speedTolerance->setMinimumWidth( mlw );

  topLayout->addWidget( m_speedTolerance, row, 1 );
  row++;

  lbl = new QLabel( tr( "Heading tolerance:" ) );
  topLayout->addWidget( lbl, row, 0 );

  m_headingTolerance = new NumberEditor;
  m_headingTolerance->setDecimalVisible( false );
  m_headingTolerance->setPmVisible( false );
  m_headingTolerance->setRange( 0, 359);
  m_headingTolerance->setMaxLength( 3 );
  m_headingTolerance->setSuffix( " °" );

  QIntValidator* iValidator = new QIntValidator( 0, 359, this );
  m_headingTolerance->setValidator( iValidator );

  int maw = QFontMetrics(font()).width("999 °") + 10;
  m_headingTolerance->setMinimumWidth( maw );

  topLayout->addWidget(m_headingTolerance, row, 1);
  topLayout->setColumnStretch(2, 2);
  row++;

  lbl = new QLabel( tr( "Wind after:" ) );
  topLayout->addWidget( lbl, row, 0 );

  m_windAfter = new NumberEditor;
  m_windAfter->setDecimalVisible( false );
  m_windAfter->setPmVisible( false );
  m_windAfter->setRange( 3, 60);
  m_windAfter->setMaxLength( 2 );
  m_windAfter->setSuffix( " s" );

  iValidator = new QIntValidator( 3, 60, this );
  m_windAfter->setValidator( iValidator );

  maw = QFontMetrics(font()).width("999 °") + 10;
  m_windAfter->setMinimumWidth( maw );

  topLayout->addWidget(m_windAfter, row, 1);
  topLayout->setColumnStretch(2, 2);
  row++;

  // Acquire all the rest space
  topLayout->setRowStretch(row, 10);

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

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
  titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

  connect(help, SIGNAL(pressed()), this, SLOT(slotHelp()));
  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(help, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);

  load();
}

SettingsPageWind::~SettingsPageWind()
{
  // qDebug("SettingsPageWind::~SettingsPageWind()");
}

void SettingsPageWind::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  m_enableWindInSF->setChecked( conf->isSfWCEnabled() );

  Speed mas = conf->getMinimumAirSpeed4WC();
  m_minimumAirSpeed->setValue( mas.getValueInUnit( m_speedUnit ) );

  Speed st = conf->getSpeedTolerance4WC();
  m_speedTolerance->setValue( st.getValueInUnit( m_speedUnit ) );

  m_headingTolerance->setValue( conf->getHeadingTolerance4WC() );
  m_windAfter->setValue( conf->getStartWindCalcInStraightFlight() );
}

void SettingsPageWind::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setSfWCEnabled( m_enableWindInSF->isChecked() );

  Speed mas;
  mas.setValueInUnit( m_minimumAirSpeed->value(), m_speedUnit );
  conf->setMinimumAirSpeed4WC( mas );

  Speed st;
  st.setValueInUnit( m_speedTolerance->value(), m_speedUnit );
  conf->setSpeedTolerance4WC( st );

  conf->setHeadingTolerance4WC( m_headingTolerance->value() );
  conf->setStartWindCalcInStraightFlight( m_windAfter->value() );
}

void SettingsPageWind::slotHelp()
{
  QString file = "cumulus-settings-wind.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void SettingsPageWind::slotAccept()
{
  save();
  GeneralConfig::instance()->save();
  emit settingsChanged();
  emit closingWidget();
  QWidget::close();
}

void SettingsPageWind::slotReject()
{
  emit closingWidget();
  QWidget::close();
}
