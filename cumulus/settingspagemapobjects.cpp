/***********************************************************************
**
**   settingspagemapobjects.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers,
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
*************************************************************************
**
** The widget provides all options related to load and draw map items.
**
** @author André Somers, Axel Pauli
**
************************************************************************/

#include <QGridLayout>
#include <QHeaderView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

#include "settingspagemapobjects.h"
#include "generalconfig.h"

SettingsPageMapObjects::SettingsPageMapObjects(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageMapObjects");

  int row=0;
  QGridLayout * topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  //---------------------------------------------------------------------------
  // The three waypoint scale limits are put in a group box
  QGroupBox *wpGroup = new QGroupBox( tr("Draw Waypoints until this scale"), this );
  topLayout->addWidget( wpGroup, row++, 0 );

  QHBoxLayout *hBox = new QHBoxLayout( wpGroup );
  hBox->setSpacing( 5 );

  QLabel *label = new QLabel( tr("Low:"), wpGroup );
  hBox->addWidget( label );
  wpLowScaleLimitSpinBox = new QSpinBox( wpGroup );
  wpLowScaleLimitSpinBox->setPrefix( "< ");
  wpLowScaleLimitSpinBox->setRange(0, 1200);
  wpLowScaleLimitSpinBox->setSingleStep(10);
  wpLowScaleLimitSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
  hBox->addWidget( wpLowScaleLimitSpinBox );
  connect( wpLowScaleLimitSpinBox, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpLowScaleLimitChanged(int)) );

  hBox->addSpacing( 5 );

  label = new QLabel( tr("Normal:"), wpGroup );
  hBox->addWidget( label );
  wpNormalScaleLimitSpinBox = new QSpinBox( wpGroup );
  wpNormalScaleLimitSpinBox->setPrefix( "< ");
  wpNormalScaleLimitSpinBox->setRange(0, 1200);
  wpNormalScaleLimitSpinBox->setSingleStep(10);
  wpNormalScaleLimitSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
  hBox->addWidget( wpNormalScaleLimitSpinBox );
  connect( wpNormalScaleLimitSpinBox, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpNormalScaleLimitChanged(int)) );

  hBox->addSpacing( 5 );

  label = new QLabel( tr("High:"), wpGroup );
  hBox->addWidget( label );
  wpHighScaleLimitSpinBox = new QSpinBox( wpGroup );
  wpHighScaleLimitSpinBox->setPrefix( "< ");
  wpHighScaleLimitSpinBox->setRange(0, 1200);
  wpHighScaleLimitSpinBox->setSingleStep(10);
  wpHighScaleLimitSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
  hBox->addWidget( wpHighScaleLimitSpinBox );
  connect( wpHighScaleLimitSpinBox, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpHighScaleLimitChanged(int)) );

  hBox->addStretch( 10 );

  //---------------------------------------------------------------------------
  // table with 7 rows and 2 columns
  loadOptions = new QTableWidget(7, 2, this);
  loadOptions->setShowGrid( true );

  connect( loadOptions, SIGNAL(cellClicked ( int, int )),
           SLOT(slot_toggleCheckBox( int, int )));

  // hide vertical headers
  QHeaderView *vHeader = loadOptions->verticalHeader();
  vHeader->setVisible(false);

  QString header = tr("Load/Draw map objects");
  QTableWidgetItem *item = new QTableWidgetItem( header );
  loadOptions->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( header );
  loadOptions->setHorizontalHeaderItem( 1, item );

  topLayout->addWidget( loadOptions, row++, 0 );
}

SettingsPageMapObjects::~SettingsPageMapObjects()
{}

/**
 * Called, if the value in the normal scale spin box has been changed.
 */
void SettingsPageMapObjects::slot_wpLowScaleLimitChanged( int newValue )
{
  if( newValue > wpNormalScaleLimitSpinBox->value() )
    {
      // Check new value to ensure that current value of normal importance
      // is not exceeded. In such a case a reset is done.
      wpLowScaleLimitSpinBox->setValue( wpNormalScaleLimitSpinBox->value() );
    }

  wpLowScaleLimitSpinBox->setMaximum( wpNormalScaleLimitSpinBox->value() );
}

/**
 * Called, if the value in the normal scale spin box has been changed.
 */
void SettingsPageMapObjects::slot_wpNormalScaleLimitChanged( int newValue )
{
  if( newValue > wpHighScaleLimitSpinBox->value() )
    {
      // Check new value to ensure that current value of high importance
      // is not exceeded. In such a case a reset is done.
      wpNormalScaleLimitSpinBox->setValue( wpHighScaleLimitSpinBox->value() );
    }

  if( wpLowScaleLimitSpinBox->value() > wpNormalScaleLimitSpinBox->value() )
    {
      // Check normal value to ensure that current value of low importance
      // is not exceeded. In such a case a reset is done.
      wpLowScaleLimitSpinBox->setValue( wpNormalScaleLimitSpinBox->value() );
    }

  // set max ranges according to the new passed value
  wpNormalScaleLimitSpinBox->setMaximum( wpHighScaleLimitSpinBox->value() );
  wpLowScaleLimitSpinBox->setMaximum( wpNormalScaleLimitSpinBox->value() );
}

/**
 * Called, if the value in the high scale spin box has been changed.
 */
void SettingsPageMapObjects::slot_wpHighScaleLimitChanged( int newValue )
{
  if( newValue < wpNormalScaleLimitSpinBox->value() )
    {
      // Check new value to ensure that current value of normal importance
      // is not exceeded. In such a case a reset is done.
      wpNormalScaleLimitSpinBox->setValue( newValue );
    }

  if( wpLowScaleLimitSpinBox->value() > wpNormalScaleLimitSpinBox->value() )
    {
      // Check low value to ensure that current value of normal importance
      // is not exceeded. In such a case a reset is done.
      wpLowScaleLimitSpinBox->setValue( wpNormalScaleLimitSpinBox->value() );
    }

  // set the new maxims for low and normal scale limit spin boxes
  wpNormalScaleLimitSpinBox->setMaximum( newValue );
  wpLowScaleLimitSpinBox->setMaximum( newValue );
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
  liHighways->setCheckState( conf->getMapLoadHighways() ? Qt::Checked : Qt::Unchecked );
  liRailroads->setCheckState( conf->getMapLoadRailroads() ? Qt::Checked : Qt::Unchecked );
  liCities->setCheckState( conf->getMapLoadCities() ? Qt::Checked : Qt::Unchecked );
  liWaterways->setCheckState( conf->getMapLoadWaterways() ? Qt::Checked : Qt::Unchecked );
  liForests->setCheckState( conf->getMapLoadForests() ? Qt::Checked : Qt::Unchecked );
  liTargetLine->setCheckState( conf->getMapBearLine()? Qt::Checked : Qt::Unchecked );
  liAfLabels->setCheckState( conf->getMapShowAirfieldLabels() ? Qt::Checked : Qt::Unchecked );
  liTpLabels->setCheckState( conf->getMapShowTaskPointLabels() ? Qt::Checked : Qt::Unchecked );
  liOlLabels->setCheckState( conf->getMapShowOutLandingLabels() ? Qt::Checked : Qt::Unchecked );

  // Load scale values for spin boxes. Note! The load order is important because a value change
  // of the spin box will generate a signal.
  wpHighScaleLimitSpinBox->setValue( conf->getWaypointScaleBorder( wayPoint::High ));
  wpNormalScaleLimitSpinBox->setValue( conf->getWaypointScaleBorder( wayPoint::Normal ));
  wpLowScaleLimitSpinBox->setValue( conf->getWaypointScaleBorder( wayPoint::Low ));

  // set maximums
  wpLowScaleLimitSpinBox->setMaximum( wpNormalScaleLimitSpinBox->value() );
  wpNormalScaleLimitSpinBox->setMaximum( wpHighScaleLimitSpinBox->value() );
}

/** Called to initiate saving to the configuration file. */
void SettingsPageMapObjects::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setMapLoadIsoLines( liIsolines->checkState() == Qt::Checked ? true : false );
  conf->setMapShowIsoLineBorders(liIsolineBorders->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadRoads(liRoads->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadHighways(liHighways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadRailroads(liRailroads->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadCities(liCities->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadWaterways(liWaterways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadForests(liForests->checkState() == Qt::Checked ? true : false);
  conf->setMapBearLine(liTargetLine->checkState() == Qt::Checked ? true : false);
  conf->setMapShowAirfieldLabels(liAfLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowTaskPointLabels(liTpLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowOutLandingLabels(liOlLabels->checkState() == Qt::Checked ? true : false);

  conf->setMapShowWaypointLabels(liWpLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowLabelsExtraInfo(liLabelsInfo->checkState() == Qt::Checked ? true : false);
  conf->setWaypointScaleBorder( wayPoint::Low, wpLowScaleLimitSpinBox->value() );
  conf->setWaypointScaleBorder( wayPoint::Normal, wpNormalScaleLimitSpinBox->value() );
  conf->setWaypointScaleBorder( wayPoint::High, wpHighScaleLimitSpinBox->value() );
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

  liHighways = new QTableWidgetItem( tr("Highways") );
  liHighways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liHighways );

  liIsolines = new QTableWidgetItem( tr("Isolines") );
  liIsolines->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liIsolines );

  liIsolineBorders = new QTableWidgetItem( tr("Isoline borders") );
  liIsolineBorders->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liIsolineBorders );

  liTargetLine = new QTableWidgetItem( tr("Line to selected target") );
  liTargetLine->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liTargetLine );

  liRailroads = new QTableWidgetItem( tr("Railroads") );
  liRailroads->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liRailroads );

  // next column is one
  row = 0;
  col = 1;

  liRoads = new QTableWidgetItem( tr("Roads") );
  liRoads->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liRoads );

  liWaterways = new QTableWidgetItem( tr("Rivers & Canals") );
  liWaterways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liWaterways );

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
}

void SettingsPageMapObjects::showEvent(QShowEvent *)
{
  // align all columns to contents before showing
  loadOptions->resizeColumnsToContents();
  loadOptions->setFocus();
}

/**
 * Called to toggle the check box of the clicked table cell.
 */
void SettingsPageMapObjects::slot_toggleCheckBox( int row, int column )
{
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
  changed |= ( conf->getMapLoadHighways() ? Qt::Checked : Qt::Unchecked ) != liHighways->checkState();
  changed |= ( conf->getMapLoadRailroads() ? Qt::Checked : Qt::Unchecked ) != liRailroads->checkState();
  changed |= ( conf->getMapLoadCities() ? Qt::Checked : Qt::Unchecked ) != liCities->checkState();
  changed |= ( conf->getMapLoadWaterways() ? Qt::Checked : Qt::Unchecked ) != liWaterways->checkState();
  changed |= ( conf->getMapLoadForests() ? Qt::Checked : Qt::Unchecked ) != liForests->checkState();
  changed |= ( conf->getMapBearLine()? Qt::Checked : Qt::Unchecked ) != liTargetLine->checkState();
  changed |= ( conf->getMapShowAirfieldLabels()? Qt::Checked : Qt::Unchecked ) != liAfLabels->checkState();
  changed |= ( conf->getMapShowTaskPointLabels()? Qt::Checked : Qt::Unchecked ) != liTpLabels->checkState();
  changed |= ( conf->getMapShowOutLandingLabels()? Qt::Checked : Qt::Unchecked ) != liOlLabels->checkState();

  changed |= ( conf->getWaypointScaleBorder( wayPoint::Low )    != wpLowScaleLimitSpinBox->value() );
  changed |= ( conf->getWaypointScaleBorder( wayPoint::Normal ) != wpNormalScaleLimitSpinBox->value() );
  changed |= ( conf->getWaypointScaleBorder( wayPoint::High )   != wpHighScaleLimitSpinBox->value() );

  if (changed)
    {
      warn=true;
      warnings.append(tr("The Map Objects"));
    }
}
