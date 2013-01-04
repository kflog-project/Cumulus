/***********************************************************************
**
**   settingspagemapobjects.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers,
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
************************************************************************/

#include <QtGui>

#include "settingspagemapobjects.h"
#include "generalconfig.h"

#ifdef FLICK_CHARM
#include "flickcharm.h"
#endif

#ifdef USE_NUM_PAD
#include "numberEditor.h"
#else
#include "varspinbox.h"
#endif

SettingsPageMapObjects::SettingsPageMapObjects(QWidget *parent) :
  QWidget(parent),
  m_autoSip( true )
{
  setObjectName("SettingsPageMapObjects");

  int row=0;
  QGridLayout* topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  //---------------------------------------------------------------------------
  // The three waypoint scale limits are put in a group box
  QGroupBox *wpGroup = new QGroupBox( tr("Draw Waypoints until this scale"), this );
  topLayout->addWidget( wpGroup, row++, 0 );

  QHBoxLayout *hBox = new QHBoxLayout( wpGroup );
  hBox->setSpacing( 5 );

  QLabel *label = new QLabel( tr("Low"), wpGroup );
  hBox->addWidget( label );

#ifdef USE_NUM_PAD
  m_wpLowScaleLimit = new NumberEditor( wpGroup );
  m_wpLowScaleLimit->setDecimalVisible( false );
  m_wpLowScaleLimit->setPmVisible( false );
  m_wpLowScaleLimit->setMaxLength(4);
  m_wpLowScaleLimit->setPrefix( "< ");
  m_wpLowScaleLimit->setMaximum( 1200 );
  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "(0|[1-9][0-9]*)" ), this );
  m_wpLowScaleLimit->setValidator( eValidator );
  hBox->addWidget( m_wpLowScaleLimit );
  connect( m_wpLowScaleLimit, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpLowScaleLimitChanged(int)) );
#else
  VarSpinBox* hspin;

  m_wpLowScaleLimit = new QSpinBox( wpGroup );
  m_wpLowScaleLimit->setPrefix( "< ");
  m_wpLowScaleLimit->setRange(0, 1200);
  m_wpLowScaleLimit->setSingleStep(5);

  hspin = new VarSpinBox(m_wpLowScaleLimit);
  hBox->addWidget( hspin );
  connect( m_wpLowScaleLimit, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpLowScaleLimitChanged(int)) );
#endif

  hBox->addSpacing( 5 );
  hBox->addStretch( 5 );

  label = new QLabel( tr("Normal"), wpGroup );
  hBox->addWidget( label );

#ifdef USE_NUM_PAD
  m_wpNormalScaleLimit = new NumberEditor( wpGroup );
  m_wpNormalScaleLimit->setDecimalVisible( false );
  m_wpNormalScaleLimit->setPmVisible( false );
  m_wpNormalScaleLimit->setMaxLength(4);
  m_wpNormalScaleLimit->setPrefix( "< ");
  m_wpNormalScaleLimit->setMaximum( 1200 );
  eValidator = new QRegExpValidator( QRegExp( "(0|[1-9][0-9]*)" ), this );
  m_wpNormalScaleLimit->setValidator( eValidator );
  hBox->addWidget( m_wpNormalScaleLimit );
  connect( m_wpNormalScaleLimit, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpNormalScaleLimitChanged(int)) );
#else
  m_wpNormalScaleLimit = new QSpinBox( wpGroup );
  m_wpNormalScaleLimit->setPrefix( "< ");
  m_wpNormalScaleLimit->setRange(0, 1200);
  m_wpNormalScaleLimit->setSingleStep(5);

  hspin = new VarSpinBox(m_wpNormalScaleLimit);
  hBox->addWidget( hspin );
  connect( m_wpNormalScaleLimit, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpNormalScaleLimitChanged(int)) );
#endif

  hBox->addSpacing( 5 );
  hBox->addStretch( 5 );

  label = new QLabel( tr("High"), wpGroup );
  hBox->addWidget( label );

#ifdef USE_NUM_PAD
  m_wpHighScaleLimit = new NumberEditor( wpGroup );
  m_wpHighScaleLimit->setDecimalVisible( false );
  m_wpHighScaleLimit->setPmVisible( false );
  m_wpHighScaleLimit->setMaxLength(4);
  m_wpHighScaleLimit->setPrefix( "< ");
  m_wpHighScaleLimit->setMaximum( 1200 );
  eValidator = new QRegExpValidator( QRegExp( "(0|[1-9][0-9]*)" ), this );
  m_wpHighScaleLimit->setValidator( eValidator );
  hBox->addWidget( m_wpHighScaleLimit );
  connect( m_wpHighScaleLimit, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpHighScaleLimitChanged(int)) );
#else
  m_wpHighScaleLimit = new QSpinBox( wpGroup );
  m_wpHighScaleLimit->setPrefix( "< ");
  m_wpHighScaleLimit->setRange(0, 1200);
  m_wpHighScaleLimit->setSingleStep(5);

  hspin = new VarSpinBox(m_wpHighScaleLimit);
  hBox->addWidget( hspin );
  connect( m_wpHighScaleLimit, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpHighScaleLimitChanged(int)) );
#endif

  //hBox->addStretch( 10 );

  //---------------------------------------------------------------------------
  // table with 8 rows and 2 columns
  loadOptions = new QTableWidget(8, 2, this);

#ifdef QSCROLLER
  QScroller::grabGesture(loadOptions, QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(loadOptions);
#endif

  loadOptions->setShowGrid( true );

  connect( loadOptions, SIGNAL(cellClicked ( int, int )),
           SLOT(slot_toggleCheckBox( int, int )));

  // hide vertical headers
  QHeaderView *vHeader = loadOptions->verticalHeader();
  vHeader->setVisible(false);

  QString header = tr("Load/Draw objects");
  QTableWidgetItem *item = new QTableWidgetItem( header );
  loadOptions->setHorizontalHeaderItem( 0, item );

  header = tr("Draw objects");
  item = new QTableWidgetItem( header );
  loadOptions->setHorizontalHeaderItem( 1, item );

  topLayout->addWidget( loadOptions, row++, 0 );
}

SettingsPageMapObjects::~SettingsPageMapObjects()
{}

/**
 * Called, if the value in the low scale spin box has been changed.
 */
void SettingsPageMapObjects::slot_wpLowScaleLimitChanged( int newValue )
{
  if( newValue > m_wpNormalScaleLimit->value() )
    {
      // Check new value to ensure that current value of normal priority
      // is not exceeded. In such a case a reset is done.
      m_wpLowScaleLimit->setValue( m_wpNormalScaleLimit->value() );
    }

  m_wpLowScaleLimit->setMaximum( m_wpNormalScaleLimit->value() );
}

/**
 * Called, if the value in the normal scale spin box has been changed.
 */
void SettingsPageMapObjects::slot_wpNormalScaleLimitChanged( int newValue )
{
  if( newValue > m_wpHighScaleLimit->value() )
    {
      // Check new value to ensure that current value of high priority
      // is not exceeded. In such a case a reset is done.
      m_wpNormalScaleLimit->setValue( m_wpHighScaleLimit->value() );
    }

  if( m_wpLowScaleLimit->value() > m_wpNormalScaleLimit->value() )
    {
      // Check normal value to ensure that current value of low priority
      // is not exceeded. In such a case a reset is done.
      m_wpLowScaleLimit->setValue( m_wpNormalScaleLimit->value() );
    }

  // set max ranges according to the new passed value
  m_wpNormalScaleLimit->setMaximum( m_wpHighScaleLimit->value() );
  m_wpLowScaleLimit->setMaximum( m_wpNormalScaleLimit->value() );
}

/**
 * Called, if the value in the high scale spin box has been changed.
 */
void SettingsPageMapObjects::slot_wpHighScaleLimitChanged( int newValue )
{
  if( newValue < m_wpNormalScaleLimit->value() )
    {
      // Check new value to ensure that current value of normal priority
      // is not exceeded. In such a case a reset is done.
      m_wpNormalScaleLimit->setValue( newValue );
    }

  if( m_wpLowScaleLimit->value() > m_wpNormalScaleLimit->value() )
    {
      // Check low value to ensure that current value of normal priority
      // is not exceeded. In such a case a reset is done.
      m_wpLowScaleLimit->setValue( m_wpNormalScaleLimit->value() );
    }

  // set the new maxims for low and normal scale limit spin boxes
  m_wpNormalScaleLimit->setMaximum( newValue );
  m_wpLowScaleLimit->setMaximum( newValue );
}


/** Called to initiate loading of the configuration file */
void SettingsPageMapObjects::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  fillLoadOptionList();
  liIsolines->setCheckState( conf->getMapLoadIsoLines() ? Qt::Checked : Qt::Unchecked );
  liIsolineBorders->setCheckState( conf->getMapShowIsoLineBorders() ? Qt::Checked : Qt::Unchecked );
  liWpLabels->setCheckState( conf->getMapShowWaypointLabels() ? Qt::Checked : Qt::Unchecked );
  liLabelsInfo->setCheckState( conf->getMapShowLabelsExtraInfo() ? Qt::Checked : Qt::Unchecked );
  liRoads->setCheckState( conf->getMapLoadRoads() ? Qt::Checked : Qt::Unchecked );
  liMotorways->setCheckState( conf->getMapLoadMotorways() ? Qt::Checked : Qt::Unchecked );
  liRailways->setCheckState( conf->getMapLoadRailways() ? Qt::Checked : Qt::Unchecked );
  liCities->setCheckState( conf->getMapLoadCities() ? Qt::Checked : Qt::Unchecked );
  liWaterways->setCheckState( conf->getMapLoadWaterways() ? Qt::Checked : Qt::Unchecked );
  liForests->setCheckState( conf->getMapLoadForests() ? Qt::Checked : Qt::Unchecked );
  liAfLabels->setCheckState( conf->getMapShowAirfieldLabels() ? Qt::Checked : Qt::Unchecked );
  liTpLabels->setCheckState( conf->getMapShowTaskPointLabels() ? Qt::Checked : Qt::Unchecked );
  liOlLabels->setCheckState( conf->getMapShowOutLandingLabels() ? Qt::Checked : Qt::Unchecked );
  liRelBearingInfo->setCheckState( conf->getMapShowRelBearingInfo() ? Qt::Checked : Qt::Unchecked );
  liFlightTrail->setCheckState( conf->getMapDrawTrail() ? Qt::Checked : Qt::Unchecked );
  // Load scale values for spin boxes. Note! The load order is important because a value change
  // of the spin box will generate a signal.
  m_wpHighScaleLimit->setValue( conf->getWaypointScaleBorder( Waypoint::High ));
  m_wpNormalScaleLimit->setValue( conf->getWaypointScaleBorder( Waypoint::Normal ));
  m_wpLowScaleLimit->setValue( conf->getWaypointScaleBorder( Waypoint::Low ));

  // set maximums
  m_wpLowScaleLimit->setMaximum( m_wpNormalScaleLimit->value() );
  m_wpNormalScaleLimit->setMaximum( m_wpHighScaleLimit->value() );
}

/** Called to initiate saving to the configuration file. */
void SettingsPageMapObjects::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setMapLoadIsoLines( liIsolines->checkState() == Qt::Checked ? true : false );
  conf->setMapShowIsoLineBorders(liIsolineBorders->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadRoads(liRoads->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadMotorways(liMotorways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadRailways(liRailways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadCities(liCities->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadWaterways(liWaterways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadForests(liForests->checkState() == Qt::Checked ? true : false);
  conf->setMapShowAirfieldLabels(liAfLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowTaskPointLabels(liTpLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowOutLandingLabels(liOlLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowWaypointLabels(liWpLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowLabelsExtraInfo(liLabelsInfo->checkState() == Qt::Checked ? true : false);
  conf->setMapShowRelBearingInfo(liRelBearingInfo->checkState() == Qt::Checked ? true : false);
  conf->setMapDrawTrail(liFlightTrail->checkState() == Qt::Checked ? true : false);

  conf->setWaypointScaleBorder( Waypoint::Low, m_wpLowScaleLimit->value() );
  conf->setWaypointScaleBorder( Waypoint::Normal, m_wpNormalScaleLimit->value() );
  conf->setWaypointScaleBorder( Waypoint::High, m_wpHighScaleLimit->value() );
}

/** Fills the list with load options */
void SettingsPageMapObjects::fillLoadOptionList()
{
  int row = 0;
  int col = 0;

  liCities = new QTableWidgetItem( tr("Cities") );
  liCities->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liCities );

  liForests = new QTableWidgetItem( tr("Forests") );
  liForests->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liForests );

  liIsolines = new QTableWidgetItem( tr("Isolines") );
  liIsolines->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liIsolines );

  liIsolineBorders = new QTableWidgetItem( tr("Isoline borders") );
  liIsolineBorders->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liIsolineBorders );

  liMotorways = new QTableWidgetItem( tr("Motorways") );
  liMotorways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liMotorways );

  liRailways = new QTableWidgetItem( tr("Railways") );
  liRailways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liRailways );

  liRoads = new QTableWidgetItem( tr("Roads") );
  liRoads->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liRoads );

  liWaterways = new QTableWidgetItem( tr("Rivers & Canals") );
  liWaterways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liWaterways );

  // next column is one
  row = 0;
  col = 1;

  liAfLabels = new QTableWidgetItem( tr("Airfield labels") );
  liAfLabels->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liAfLabels );

  liTpLabels = new QTableWidgetItem( tr("Taskpoint labels") );
  liTpLabels->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liTpLabels );

  liOlLabels = new QTableWidgetItem( tr("Outlanding labels") );
  liOlLabels->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liOlLabels );

  liWpLabels = new QTableWidgetItem( tr("Waypoint labels") );
  liWpLabels->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liWpLabels );

  liLabelsInfo = new QTableWidgetItem( tr("Labels - Extra info") );
  liLabelsInfo->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liLabelsInfo );

  liRelBearingInfo = new QTableWidgetItem( tr("Relative bearing info") );
  liRelBearingInfo->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liRelBearingInfo );

  liFlightTrail = new QTableWidgetItem( tr("Flight trail") );
  liFlightTrail->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liFlightTrail );

  // Set a dummy into the unused cells
  QTableWidgetItem *liDummy = new QTableWidgetItem;
  liDummy->setFlags( Qt::NoItemFlags );
  loadOptions->setItem( row++, col, liDummy );
}

void SettingsPageMapObjects::showEvent(QShowEvent *)
{
  // align all columns to contents before showing
  loadOptions->resizeColumnsToContents();
  loadOptions->setFocus();

  // Switch off automatic software input panel popup
  m_autoSip = qApp->autoSipEnabled();
  qApp->setAutoSipEnabled( false );
}

void SettingsPageMapObjects::hideEvent( QHideEvent *)
{
  qApp->setAutoSipEnabled( m_autoSip );
}

/**
 * Called to toggle the check box of the clicked table cell.
 */
void SettingsPageMapObjects::slot_toggleCheckBox( int row, int column )
{
  if( column == 1 && row > 6 )
    {
      // Dummy cell was clicked
      return;
    }

  QTableWidgetItem *item = loadOptions->item( row, column );
  item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
}

/* Called to ask is confirmation on close is needed. */
void SettingsPageMapObjects::slot_query_close(bool& warn, QStringList& warnings)
{
    /* set warn to 'true' if the data has changed. Note that we can NOT
    just set warn equal to _changed, because that way we might erase a
    warning flag set by another page! */

  bool changed=false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed |= ( conf->getMapLoadIsoLines() ? Qt::Checked : Qt::Unchecked ) != liIsolines->checkState();
  changed |= ( conf->getMapShowIsoLineBorders() ? Qt::Checked : Qt::Unchecked ) != liIsolineBorders->checkState();
  changed |= ( conf->getMapShowWaypointLabels() ? Qt::Checked : Qt::Unchecked ) != liWpLabels->checkState();
  changed |= ( conf->getMapShowLabelsExtraInfo() ? Qt::Checked : Qt::Unchecked ) != liLabelsInfo->checkState();
  changed |= ( conf->getMapLoadRoads() ? Qt::Checked : Qt::Unchecked ) != liRoads->checkState();
  changed |= ( conf->getMapLoadMotorways() ? Qt::Checked : Qt::Unchecked ) != liMotorways->checkState();
  changed |= ( conf->getMapLoadRailways() ? Qt::Checked : Qt::Unchecked ) != liRailways->checkState();
  changed |= ( conf->getMapLoadCities() ? Qt::Checked : Qt::Unchecked ) != liCities->checkState();
  changed |= ( conf->getMapLoadWaterways() ? Qt::Checked : Qt::Unchecked ) != liWaterways->checkState();
  changed |= ( conf->getMapLoadForests() ? Qt::Checked : Qt::Unchecked ) != liForests->checkState();
  changed |= ( conf->getMapShowAirfieldLabels() ? Qt::Checked : Qt::Unchecked ) != liAfLabels->checkState();
  changed |= ( conf->getMapShowTaskPointLabels() ? Qt::Checked : Qt::Unchecked ) != liTpLabels->checkState();
  changed |= ( conf->getMapShowOutLandingLabels() ? Qt::Checked : Qt::Unchecked ) != liOlLabels->checkState();
  changed |= ( conf->getMapShowRelBearingInfo() ? Qt::Checked : Qt::Unchecked ) != liRelBearingInfo->checkState();
  changed |= ( conf->getMapDrawTrail() ? Qt::Checked : Qt::Unchecked ) != liFlightTrail->checkState();

  changed |= ( conf->getWaypointScaleBorder( Waypoint::Low )    != m_wpLowScaleLimit->value() );
  changed |= ( conf->getWaypointScaleBorder( Waypoint::Normal ) != m_wpNormalScaleLimit->value() );
  changed |= ( conf->getWaypointScaleBorder( Waypoint::High )   != m_wpHighScaleLimit->value() );

  if (changed)
    {
      warn=true;
      warnings.append(tr("The Map Objects"));
    }
}
