/***********************************************************************
 **
 **   settingspagemap.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QGridLayout>
#include <QHeaderView>

#include "settingspagemap.h"
#include "generalconfig.h"

SettingsPageMap::SettingsPageMap(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageMap");

  int row=0;
  QGridLayout * topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  lvLoadOptions = new QTableWidget(11, 1, this);

  // hide vertical headers
  QHeaderView *vHeader = lvLoadOptions->verticalHeader();
  vHeader->setVisible(false);
    
  QTableWidgetItem *item = new QTableWidgetItem( tr("Load/Draw map objects") );
  lvLoadOptions->setHorizontalHeaderItem( 0, item );

  topLayout->addWidget(lvLoadOptions, row++, 0, 1, 2);
}

SettingsPageMap::~SettingsPageMap()
{}

/** Called to initiate loading of the configurationfile */
void SettingsPageMap::slot_load()
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
void SettingsPageMap::slot_save()
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
void SettingsPageMap::fillLoadOptionList()
{
  int row = 0;

  liIsolines = new QTableWidgetItem( tr("Isolines") );
  liIsolines->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liIsolines );

  liIsolineBorders = new QTableWidgetItem( tr("Isoline borders") );
  liIsolineBorders->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liIsolineBorders );
        
  liWpLabels = new QTableWidgetItem( tr("Waypoint labels") );
  liWpLabels->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liWpLabels );
  
  liWpLabelsExtraInfo = new QTableWidgetItem( tr("Waypoint labels - Extra info") );
  liWpLabelsExtraInfo->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liWpLabelsExtraInfo );
      
  liRoads = new QTableWidgetItem( tr("Roads") );
  liRoads->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liRoads );

  liHighways = new QTableWidgetItem( tr("Highways") );
  liHighways->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liHighways );

  liRailroads = new QTableWidgetItem( tr("Railroads") );
  liRailroads->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liRailroads );

  liCities = new QTableWidgetItem( tr("Cities & Villages") );
  liCities->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liCities );

  liWaterways = new QTableWidgetItem( tr("Rivers & Canals") );
  liWaterways->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liWaterways );

  liForests = new QTableWidgetItem( tr("Forests & Ice") );
  liForests->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liForests );

  liTargetLine = new QTableWidgetItem( tr("Line to selected target") );
  liTargetLine->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, liTargetLine );

  lvLoadOptions->sortItems( 0 );
  lvLoadOptions->adjustSize();
  lvLoadOptions->setColumnWidth( 0, lvLoadOptions->maximumViewportSize().width()-20 );
}

/* Called to ask is confirmation on the close is needed. Not yet
   supported atm */
void SettingsPageMap::slot_query_close(bool& warn, QStringList& warnings)
{
  return;
}
