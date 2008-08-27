/***********************************************************************
**
**   settingspagemapobjects.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
*************************************************************************
**
** Contains the map objects related to load/draw settings.
**
** @author André Somers
**
************************************************************************/

#include <QGridLayout>
#include <QHeaderView>

#include "settingspagemapobjects.h"
#include "generalconfig.h"

SettingsPageMapObjects::SettingsPageMapObjects(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageMapObjects");

  int row=0;
  QGridLayout * topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  loadOptions = new QTableWidget(11, 1, this);
  loadOptions->setShowGrid( true );

  connect( loadOptions, SIGNAL(cellClicked ( int, int )),
           SLOT(slot_toggleCheckBox( int, int )));

  // hide vertical headers
  QHeaderView *vHeader = loadOptions->verticalHeader();
  vHeader->setVisible(false);
    
  QTableWidgetItem *item = new QTableWidgetItem( tr("Load/Draw map objects") );
  loadOptions->setHorizontalHeaderItem( 0, item );

  topLayout->addWidget(loadOptions, row++, 0, 1, 2);
}

SettingsPageMapObjects::~SettingsPageMapObjects()
{}

/** Called to initiate loading of the configurationfile */
void SettingsPageMapObjects::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  fillLoadOptionList();
  liIsolines->setCheckState( conf->getMapLoadIsoLines() ? Qt::Checked : Qt::Unchecked );
  liIsolines->setCheckState( conf->getMapLoadIsoLines() ? Qt::Checked : Qt::Unchecked );
  liIsolineBorders->setCheckState( conf->getMapShowIsoLineBorders() ? Qt::Checked : Qt::Unchecked );
  liWpLabels->setCheckState( conf->getMapShowWaypointLabels() ? Qt::Checked : Qt::Unchecked );
  liWpLabelsExtraInfo->setCheckState( conf->getMapShowWaypointLabelsExtraInfo() ? Qt::Checked : Qt::Unchecked );
  liRoads->setCheckState( conf->getMapLoadRoads() ? Qt::Checked : Qt::Unchecked );
  liHighways->setCheckState( conf->getMapLoadHighways() ? Qt::Checked : Qt::Unchecked );
  liRailroads->setCheckState( conf->getMapLoadRailroads() ? Qt::Checked : Qt::Unchecked );
  liCities->setCheckState( conf->getMapLoadCities() ? Qt::Checked : Qt::Unchecked );
  liWaterways->setCheckState( conf->getMapLoadWaterways() ? Qt::Checked : Qt::Unchecked );
  liForests->setCheckState( conf->getMapLoadForests() ? Qt::Checked : Qt::Unchecked );
  liTargetLine->setCheckState( conf->getMapBearLine()? Qt::Checked : Qt::Unchecked );
}


/** Called to initiate saving to the configurationfile. */
void SettingsPageMapObjects::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setMapLoadIsoLines( liIsolines->checkState() == Qt::Checked ? true : false );
  conf->setMapShowIsoLineBorders(liIsolineBorders->checkState() == Qt::Checked ? true : false);
  conf->setMapShowWaypointLabels(liWpLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowWaypointLabelsExtraInfo(liWpLabelsExtraInfo->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadRoads(liRoads->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadHighways(liHighways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadRailroads(liRailroads->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadCities(liCities->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadWaterways(liWaterways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadForests(liForests->checkState() == Qt::Checked ? true : false);
  conf->setMapBearLine(liTargetLine->checkState() == Qt::Checked ? true : false);
}


/** Fills the list with loadoptions */
void SettingsPageMapObjects::fillLoadOptionList()
{
  int row = 0;

  liIsolines = new QTableWidgetItem( tr("Isolines") );
  liIsolines->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liIsolines );

  liIsolineBorders = new QTableWidgetItem( tr("Isoline borders") );
  liIsolineBorders->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liIsolineBorders );
        
  liWpLabels = new QTableWidgetItem( tr("Waypoint labels") );
  liWpLabels->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liWpLabels );
  
  liWpLabelsExtraInfo = new QTableWidgetItem( tr("Waypoint labels - Extra info") );
  liWpLabelsExtraInfo->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liWpLabelsExtraInfo );
      
  liRoads = new QTableWidgetItem( tr("Roads") );
  liRoads->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liRoads );

  liHighways = new QTableWidgetItem( tr("Highways") );
  liHighways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liHighways );

  liRailroads = new QTableWidgetItem( tr("Railroads") );
  liRailroads->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liRailroads );

  liCities = new QTableWidgetItem( tr("Cities & Villages") );
  liCities->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liCities );

  liWaterways = new QTableWidgetItem( tr("Rivers & Canals") );
  liWaterways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liWaterways );

  liForests = new QTableWidgetItem( tr("Forests & Ice") );
  liForests->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liForests );

  liTargetLine = new QTableWidgetItem( tr("Line to selected target") );
  liTargetLine->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, 0, liTargetLine );

  loadOptions->sortItems( 0 );
//  loadOptions->adjustSize();
//  loadOptions->setColumnWidth( 0, loadOptions->maximumViewportSize().width()-20 );
  loadOptions->setColumnWidth(0,360);
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
  changed |= ( conf->getMapShowWaypointLabelsExtraInfo() ? Qt::Checked : Qt::Unchecked ) != liWpLabelsExtraInfo->checkState();
  changed |= ( conf->getMapLoadRoads() ? Qt::Checked : Qt::Unchecked ) != liRoads->checkState();
  changed |= ( conf->getMapLoadHighways() ? Qt::Checked : Qt::Unchecked ) != liHighways->checkState();
  changed |= ( conf->getMapLoadRailroads() ? Qt::Checked : Qt::Unchecked ) != liRailroads->checkState();
  changed |= ( conf->getMapLoadCities() ? Qt::Checked : Qt::Unchecked ) != liCities->checkState();
  changed |= ( conf->getMapLoadWaterways() ? Qt::Checked : Qt::Unchecked ) != liWaterways->checkState();
  changed |= ( conf->getMapLoadForests() ? Qt::Checked : Qt::Unchecked ) != liForests->checkState();
  changed |= ( conf->getMapBearLine()? Qt::Checked : Qt::Unchecked ) != liTargetLine->checkState();

  if (changed) {
    warn=true;
    warnings.append(tr("the Map Objects"));
  }
}
