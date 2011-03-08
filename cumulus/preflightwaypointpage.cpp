/***********************************************************************
**
**   preflightwaypointpage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QtGui>

#include "preflightwaypointpage.h"
#include "generalconfig.h"
#include "mapcontents.h"
#include "waypointcatalog.h"

extern MapContents* _globalMapContents;

PreFlightWaypointPage::PreFlightWaypointPage(QWidget *parent) :
  QWidget(parent),
  centerRef(Position)
{
  setObjectName("PreFlightWaypointPage");

  wpTypesBox = new QComboBox;
  wpTypesBox->addItem( tr("All"), WaypointCatalog::All );
  wpTypesBox->addItem( tr("Airfields"), WaypointCatalog::Airfields );
  wpTypesBox->addItem( tr("Gliderfields"), WaypointCatalog::Gliderfields );
  wpTypesBox->addItem( tr("Outlandings"), WaypointCatalog::Outlandings );
  wpTypesBox->addItem( tr("Other Points"), WaypointCatalog::OtherPoints );

  wpRadiusBox = new QComboBox;
  wpRadiusBox->setEditable( true );
  wpRadiusBox->setValidator( new QIntValidator(1, 5000, this) );
  QStringList itemList;
  itemList << "10" << "50" << "100" << "300" << "500" << "1000" << "2000";
  wpRadiusBox->addItems( itemList );
  wpRadiusBox->setCurrentIndex( 4 );

  QFormLayout* selectLayout1 = new QFormLayout;
  selectLayout1->addRow( tr("Type:"), wpTypesBox );

  QFormLayout* selectLayout2 = new QFormLayout;
  selectLayout2->addRow( tr("Radius") + "(" + Distance::getUnitText() + "):",
                         wpRadiusBox );

  QHBoxLayout* selectLayout = new QHBoxLayout;
  selectLayout->setSpacing( 5 );
  selectLayout->addLayout( selectLayout1 );
  selectLayout->addLayout( selectLayout2 );

  selectGroup = new QGroupBox( tr("Select") );
  selectGroup->setLayout( selectLayout );

  //---------------------------------------------------------------------------
  positionRB = new QRadioButton(tr("Position"));
  homeRB     = new QRadioButton(tr("Homesite"));
  airfieldRB = new QRadioButton(tr("Airfield"));

  homeRB->setChecked( true );

  QButtonGroup* radiusButtonGroup = new QButtonGroup(this);
  radiusButtonGroup->setExclusive(true);
  radiusButtonGroup->addButton( positionRB, Position );
  radiusButtonGroup->addButton( homeRB, Home );
  radiusButtonGroup->addButton( airfieldRB, Airfield );

  connect( radiusButtonGroup, SIGNAL( buttonClicked(int)),
           this, SLOT(slotSelectCenterReference(int)));

  centerLat = new LatEdit;
  centerLon = new LongEdit;

  QGridLayout* latLonGrid = new QGridLayout;
  latLonGrid->setSpacing( 10 );
  latLonGrid->addWidget( new QLabel(tr("Lat:")), 0 , 0 );
  latLonGrid->addWidget( centerLat, 0, 1 );
  latLonGrid->addWidget( new QLabel(tr("Lon:")), 1 , 0 );
  latLonGrid->addWidget( centerLon, 1, 1 );
  latLonGrid->setColumnStretch( 2, 5 );

  homeLabel   = new QLabel;
  airfieldBox = new QComboBox;

  QGridLayout* centerPointGrid = new QGridLayout;
  centerPointGrid->setSpacing( 5 );
  centerPointGrid->addWidget( positionRB, 0, 0 );
  centerPointGrid->addWidget( homeRB, 1, 0 );
  centerPointGrid->addWidget( airfieldRB, 2, 0 );

  centerPointGrid->addLayout( latLonGrid, 0, 1 );
  centerPointGrid->addWidget( homeLabel, 1, 1 );
  centerPointGrid->addWidget( airfieldBox, 2, 1 );

  centerPointGrid->setColumnStretch( 2, 5 );

  centerPointGroup = new QGroupBox( tr("Center Point") );
  centerPointGroup->setLayout( centerPointGrid );

  //---------------------------------------------------------------------------
  QPushButton* loadButton = new QPushButton( tr("Load") );
  loadButton->setMaximumWidth( loadButton->sizeHint().width() + 10 );

  connect( loadButton, SIGNAL(clicked()), this, SLOT(slotImportFile()) );

  filterToggle = new QCheckBox( tr("Filter") );
  filterToggle->setCheckable( true );
  filterToggle->setChecked( true );

  connect( filterToggle, SIGNAL(stateChanged(int)),
           this, SLOT(slotToggleFilter(int)) );

  slotToggleFilter( Qt::Checked );

  QHBoxLayout* controlBox = new QHBoxLayout;
  controlBox->addWidget( loadButton );
  controlBox->addStretch( 5 );
  controlBox->addWidget( filterToggle );

  QVBoxLayout* wpiBox = new QVBoxLayout;
  wpiBox->setSpacing( 5 );
  wpiBox->addWidget( selectGroup );
  wpiBox->addWidget( centerPointGroup );
  wpiBox->addSpacing( 5 );
  wpiBox->addLayout( controlBox );

  QGroupBox* wpiGroup = new QGroupBox( tr("Waypoint Import") );
  wpiGroup->setLayout( wpiBox );

  wpFileFormatBox = new QComboBox;
  wpFileFormatBox->addItem( tr("Binary"), 0 );
  wpFileFormatBox->addItem( tr("XML"), 1 );

  QFormLayout* bottomLayout = new QFormLayout;
  bottomLayout->addRow( tr("Storage Format:"), wpFileFormatBox );

  //---------------------------------------------------------------------------
  QVBoxLayout* widgetLayout = new QVBoxLayout( this );
  widgetLayout->setSpacing( 10 );
  widgetLayout->addWidget( wpiGroup );
  widgetLayout->addLayout( bottomLayout );
}

PreFlightWaypointPage::~PreFlightWaypointPage()
{
}

void PreFlightWaypointPage::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  wpFileFormatBox->setCurrentIndex( conf->getWaypointFileFormat() );

  homeLabel->setText( WGSPoint::printPos(conf->getHomeLat(), true) +
                      " - " +
                      WGSPoint::printPos(conf->getHomeLon(), false) );

  slotSelectCenterReference( conf->getWaypointCenterReference() );

  loadAirfieldComboBox();

  int idx = airfieldBox->findText( conf->getWaypointAirfieldReference() );

  if( idx == -1 )
    {
      idx = 1;
    }

  airfieldBox->setCurrentIndex( idx );
}

void PreFlightWaypointPage::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setWaypointFileFormat( wpFileFormatBox->itemData(wpFileFormatBox->currentIndex()).toInt() );
  conf->setWaypointCenterReference( centerRef );
  conf->setWaypointAirfieldReference( airfieldBox->currentText() );
}

/** Stores the new selected radius */
void PreFlightWaypointPage::slotSelectCenterReference( int reference )
{
  centerRef = static_cast<enum PreFlightWaypointPage::CenterReference>(reference);

  switch( centerRef )
  {
    case PreFlightWaypointPage::Home:
      homeRB->setChecked( true );
      centerLat->setEnabled(false);
      centerLon->setEnabled(false);
      airfieldBox->setEnabled(false);
      break;
    case PreFlightWaypointPage::Airfield:
      airfieldRB->setChecked( true );
      centerLat->setEnabled(false);
      centerLon->setEnabled(false);
      airfieldBox->setEnabled(true);
      break;
    case PreFlightWaypointPage::Position:
    default:
      positionRB->setChecked( true );
      centerRef = PreFlightWaypointPage::Position;
      centerLat->setEnabled(true);
      centerLon->setEnabled(true);
      airfieldBox->setEnabled(false);
      break;
    }
}

/**
 * Called to toggle the filter.
 */
void PreFlightWaypointPage::slotToggleFilter( int toggle )
{
  selectGroup->setEnabled( (toggle == Qt::Checked ? true : false) );
  centerPointGroup->setEnabled( (toggle == Qt::Checked ? true : false) );
}

void PreFlightWaypointPage::slotImportFile()
{
  QString wayPointDir = GeneralConfig::instance()->getUserDataDirectory();

  QString filter;
  filter.append(tr("All formats") + " (*.cup *.CUP *.kflogwp *.KFLOGWP *.kwp *.KWP);;");
  filter.append(tr("KFLog") + " (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("Cumulus") + " (*.kwp *.KWP);;");
  filter.append(tr("SeeYou") + " (*.cup *.CUP)");

  QString fName = QFileDialog::getOpenFileName( 0,
                                                tr("Open waypoint catalog"),
                                                wayPointDir,
                                                filter );
  if( fName.isEmpty() )
    {
      return;
    }

  QString fSuffix = QFileInfo( fName ).suffix().toLower();
  QList<Waypoint> wpList;
  WaypointCatalog catalog;

  catalog.showProgress( true );

  if( filterToggle->isChecked() )
    {
      // We have to use the filter values for the catalog read.
      enum WaypointCatalog::wpType type = (enum WaypointCatalog::wpType)
          wpTypesBox->itemData(wpTypesBox->currentIndex()).toInt();

      int radius = wpRadiusBox->currentText().toInt();

      WGSPoint wgsPoint;

      if( positionRB->isChecked() )
        {
          wgsPoint = WGSPoint( centerLat->KFLogDegree(), centerLon->KFLogDegree() );
        }
      else if( homeRB->isChecked() )
        {
          GeneralConfig *conf = GeneralConfig::instance();
          wgsPoint = WGSPoint( conf->getHomeLat(), conf->getHomeLon() );
        }
      else if( airfieldRB->isChecked() )
        {
          QString s = airfieldBox->currentText();

          if( airfieldDict.contains(s) )
            {
              SinglePoint *sp = airfieldDict.value(s);
              wgsPoint = sp->getWGSPosition();
            }
        }

      catalog.setFilter( type, radius, wgsPoint );
    }

  // First make a test run to get the real items count.
  int wpCount = catalog.readCup( fName, 0 );

  if( wpCount == -1 )
    {
      // Error occurred, return only.
      return;
    }

  if( wpCount == 0 )
    {
      QMessageBox::information( this,
                                tr("No entries read"),
                                QString("<html>") +
                                tr("No waypoints read from file!") +
                                "<br>" +
                                tr("Maybe you should change the filter values?") +
                                "</html>" );
      return;
    }

  int answer =
      QMessageBox::question( this,
                             tr("Continue?"),
                             QString("<html>") +
                             QString(tr("%1 waypoints would be read.")).arg(wpCount) +
                             "<br><br>" +
                             tr("Continue loading?") +
                             "</html>",
                             QMessageBox::Ok|QMessageBox::No,
                             QMessageBox::No );

  if( answer == QMessageBox::No )
    {
      return;
    }

  wpCount = catalog.readCup( fName, &wpList );

  // We have to check, if a waypoint with the same name do exist. In this case
  // we check the coordinates. If they are the same, we ignore it.
  QHash<QString, QString> nameCoordDict;

  QList<Waypoint>& wpGlobalList = _globalMapContents->getWaypointList();

  for( int i = 0; i < wpGlobalList.size(); i++ )
    {
      nameCoordDict.insert( wpGlobalList.at(i).name,
                     WGSPoint::coordinateString( wpGlobalList.at(i).origP ) );
    }

  int added = 0;
  int ignored = 0;

  for( int i = 0; i < wpList.size(); i++ )
    {
      // Look, if name is known and fetch coordinate string
      QString dcString = nameCoordDict.value( wpList.at(i).name, "" );

      QString wpcString = WGSPoint::coordinateString( wpList.at(i).origP );

      if( dcString != "" && dcString == wpcString )
        {
          // Name and coordinates are identical, waypoint is ignored.
          ignored++;
          continue;
        }

      // Add new waypoint to dictionary
      nameCoordDict.insert( wpList.at(i).name, wpcString );

      // Add new waypoint to global list.
      wpGlobalList.append( wpList.at(i) );
      added++;
    }

  if( added )
    {
      _globalMapContents->saveWaypointList();
      // Trigger a redraw of the map.
      emit waypointsAdded();
    }

  QString result = QString("<html>") +
                   QString(tr("%1 waypoints added.")).arg(added);

  if( ignored )
    {
      result += "<br>" + QString(tr("%1 waypoints ignored.")).arg(ignored);
    }

  result += "</html>";

  QMessageBox::information( this,
                            tr("Import Results"),
                            result );
}

void PreFlightWaypointPage::loadAirfieldComboBox()
{
  int searchList[] = { MapContents::GliderfieldList, MapContents::AirfieldList };

  airfieldDict.clear();
  airfieldBox->clear();

  QStringList airfieldList;

  for( int l = 0; l < 2; l++ )
    {
      for( uint loop = 0; loop < _globalMapContents->getListLength(searchList[l]); loop++ )
      {
        SinglePoint *hitElement = (SinglePoint *) _globalMapContents->getElement(searchList[l], loop );
        airfieldList.append( hitElement->getName() );
        airfieldDict.insert( hitElement->getName(), hitElement );
      }
  }

  airfieldList.sort();
  airfieldBox->addItems( airfieldList );
  airfieldBox->setCurrentIndex( 0 );
}
