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
#include "hwinfo.h"
#include "mapcontents.h"
#include "waypointcatalog.h"

extern MapContents* _globalMapContents;

// Minimum amount of required free memory to start import of a waypoint file.
// Do not under run this limit, OS can freeze is such a case.
#define MINIMUM_FREE_MEMORY 1024*25

PreFlightWaypointPage::PreFlightWaypointPage(QWidget *parent) :
  QWidget(parent),
  centerRef(Position),
  _waypointFileFormat(GeneralConfig::Binary)
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
#ifdef MAEMO5  
  wpRadiusBox->setMinimumWidth( wpRadiusBox->sizeHint().width() );
#endif
  
  QFormLayout* selectLayout1 = new QFormLayout;
  selectLayout1->addRow( tr("Type:"), wpTypesBox );

  QFormLayout* selectLayout2 = new QFormLayout;
  selectLayout2->addRow( tr("Radius") + " (" + Distance::getUnitText() + "):",
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

  centerLatLabel = new QLabel(tr("Latitude:"));
  centerLonLabel = new QLabel(tr("Longitude:"));

  QGridLayout* latLonGrid = new QGridLayout;
  latLonGrid->setSpacing( 10 );
  latLonGrid->addWidget( centerLatLabel, 0 , 0 );
  latLonGrid->addWidget( centerLat, 0, 1 );
  latLonGrid->addWidget( centerLonLabel, 1 , 0 );
  latLonGrid->addWidget( centerLon, 1, 1 );
  latLonGrid->setColumnStretch( 2, 5 );

  homeLabel   = new QLabel;
  airfieldBox = new QComboBox;

  QGridLayout* centerPointGrid = new QGridLayout;
  centerPointGrid->setSpacing( 5 );
  centerPointGrid->addWidget( positionRB, 0, 0 );
  centerPointGrid->addWidget( homeRB, 1, 0 );
  centerPointGrid->addWidget( airfieldRB, 2, 0 );

  QVBoxLayout* centerPointVBox = new QVBoxLayout;

  centerPointVBox->setMargin( 0 );
  centerPointVBox->addLayout( latLonGrid );
  centerPointVBox->addWidget( homeLabel );
  centerPointVBox->addWidget( airfieldBox );

  centerPointGrid->addLayout( centerPointVBox, 0, 2, 3, 1 );
  centerPointGrid->setColumnStretch( 1, 5 );

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

  wpPriorityBox = new QComboBox;
  wpPriorityBox->addItem( tr("Source"), 3 );
  wpPriorityBox->addItem( tr("Low"), 0 );
  wpPriorityBox->addItem( tr("Normal"), 1 );
  wpPriorityBox->addItem( tr("High"), 2 );

  QFormLayout* storageLayout = new QFormLayout;
  storageLayout->addRow( tr("Storage Format:"), wpFileFormatBox );

  QFormLayout* priorityLayout = new QFormLayout;
  priorityLayout->addRow( tr("Priority:"), wpPriorityBox );

  QHBoxLayout* buttomLayout = new QHBoxLayout;
  buttomLayout->setSpacing( 5 );
  buttomLayout->addLayout( storageLayout );
  buttomLayout->addLayout( priorityLayout );

  //---------------------------------------------------------------------------
  QVBoxLayout* widgetLayout = new QVBoxLayout( this );
  widgetLayout->setSpacing( 10 );
  widgetLayout->addWidget( wpiGroup );
  widgetLayout->addSpacing( 20 );
  widgetLayout->addLayout( buttomLayout );
  widgetLayout->addStretch( 10 );
}

PreFlightWaypointPage::~PreFlightWaypointPage()
{
}

void PreFlightWaypointPage::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  _waypointFileFormat = conf->getWaypointFileFormat();

  wpFileFormatBox->setCurrentIndex( _waypointFileFormat );

  wpPriorityBox->setCurrentIndex( conf->getWaypointPriority() );

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

  conf->setWaypointFileFormat( (GeneralConfig::WpFileFormat) wpFileFormatBox->itemData(wpFileFormatBox->currentIndex()).toInt() );
  conf->setWaypointPriority( wpPriorityBox->currentIndex() );
  conf->setWaypointCenterReference( centerRef );
  conf->setWaypointAirfieldReference( airfieldBox->currentText() );

  if( _waypointFileFormat != wpFileFormatBox->currentIndex() &&
      _globalMapContents->getWaypointList().size() > 0 )
    {
      // Waypoint storage format has been changed, store all waypoints
      // in the new format, if the waypoint list is not empty and the user agrees.
      QMessageBox mb( QMessageBox::Question,
                      tr( "Continue?" ),
                      tr("The waypoint storage format was changed. "
                      "Storing data in new format can overwrite existing data!") +
                      "<br><br>" +
                      tr("Continue storing?") +
                      "</html>",
                      QMessageBox::Yes | QMessageBox::No,
                      this );

      mb.setDefaultButton( QMessageBox::No );

    #ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

    #endif

      if( mb.exec() == QMessageBox::No )
        {
          return;
        }

      _globalMapContents->saveWaypointList();
    }
}

/** Stores the new selected radius */
void PreFlightWaypointPage::slotSelectCenterReference( int reference )
{
  centerRef = static_cast<enum PreFlightWaypointPage::CenterReference>(reference);

  switch( centerRef )
  {
    case PreFlightWaypointPage::Home:
      homeRB->setChecked( true );
      centerLat->setVisible(false);
      centerLon->setVisible(false);
      centerLatLabel->setVisible(false);
      centerLonLabel->setVisible(false);
      airfieldBox->setVisible(false);
      homeLabel->setVisible(true);
      break;
    case PreFlightWaypointPage::Airfield:
      airfieldRB->setChecked( true );
      centerLat->setVisible(false);
      centerLon->setVisible(false);
      centerLatLabel->setVisible(false);
      centerLonLabel->setVisible(false);
      airfieldBox->setVisible(true);
      homeLabel->setVisible(false);
      break;
    case PreFlightWaypointPage::Position:
    default:
      positionRB->setChecked( true );
      centerRef = PreFlightWaypointPage::Position;
      centerLat->setVisible(true);
      centerLon->setVisible(true);
      centerLatLabel->setVisible(true);
      centerLonLabel->setVisible(true);
      airfieldBox->setVisible(false);
      homeLabel->setVisible(false);
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
  filter.append(tr("All") + " (*.kflogwp *.KFLOGWP *.kwp *.KWP *.cup *.CUP *.dat *.DAT);;");
  filter.append(tr("XML") + " (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("Binary") + " (*.kwp *.KWP);;");
  filter.append(tr("SeeYou") + " (*.cup *.CUP);;");
  filter.append(tr("CAI") + " (*.dat *.DAT)");

  QString fName = QFileDialog::getOpenFileName( this,
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
  QString errorMsg;

  catalog.showProgress( true );

  if( filterToggle->isChecked() )
    {
      // We have to set the filter values before catalog reading.
      enum WaypointCatalog::WpType type = (enum WaypointCatalog::WpType)
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

  // First make a test run to get the real items count which would be read.
  int wpCount = -1;

  if( fSuffix == "kwp")
    {
      wpCount = catalog.readBinary( fName, 0 );
    }
  else if( fSuffix == "kflogwp")
    {
      wpCount = catalog.readXml( fName, 0, errorMsg );
    }
  else if( fSuffix == "cup")
    {
      wpCount = catalog.readCup( fName, 0 );
    }
  else if( fSuffix == "dat")
    {
      wpCount = catalog.readDat( fName, 0 );
    }

  if( wpCount == -1 )
    {
      // Should normally not happens. Error occurred, return only.
      return;
    }

  if( wpCount == 0 )
    {
      if( errorMsg.isEmpty() )
        {
          QMessageBox mb( QMessageBox::Information,
                          tr( "No entries read" ),
                          tr("No waypoints read from file!") +
                          "<br>" +
                          tr("Maybe you should change the filter values?") +
                          "</html>",
                          QMessageBox::Ok,
                          this );

#ifdef ANDROID

          mb.show();
          QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                           height()/2 - mb.height()/2 ));
          mb.move( pos );

#endif

          mb.exec();
        }
      else
        {
          QMessageBox mb( QMessageBox::Critical,
                          tr("Error in file ") + QFileInfo( fName ).fileName(),
                          errorMsg,
                          QMessageBox::Ok,
                          this );

#ifdef ANDROID

          mb.show();
          QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                           height()/2 - mb.height()/2 ));
          mb.move( pos );

#endif
          mb.exec();
        }

      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Continue?" ),
                  QString("<html>") +
                  QString(tr("%1 waypoints would be read.")).arg(wpCount) +
                  "<br><br>" +
                  tr("Continue loading?") +
                  "</html>",
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif


  if( mb.exec() == QMessageBox::No )
    {
      return;
    }

  if( fSuffix == "kwp")
    {
      wpCount = catalog.readBinary( fName, &wpList );
    }
  else if( fSuffix == "kflogwp")
    {
      wpCount = catalog.readXml( fName, &wpList, errorMsg );
    }
  else if( fSuffix == "cup")
    {
      wpCount = catalog.readCup( fName, &wpList );
    }
  else if( fSuffix == "dat")
    {
      wpCount = catalog.readDat( fName, &wpList );
    }

  //check free memory
  int memFree = HwInfo::instance()->getFreeMemory();

  if( memFree < MINIMUM_FREE_MEMORY )
    {
      QMessageBox mb( QMessageBox::Warning,
                      tr( "Low on memory!" ),
                      QString("<html>") +
                      tr("Waypoint import failed due to low on memory!") +
                      "</html>",
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      mb.exec();
      return;
    }

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

      // Add new waypoint to the dictionary
      nameCoordDict.insert( wpList.at(i).name, wpcString );

      // Look, which waypoint priority has to be used for the import.
      int priority = wpPriorityBox->itemData(wpPriorityBox->currentIndex()).toInt();

      if( priority != 3 )
        {
          wpList[i].priority = static_cast<Waypoint::Priority>( priority );
        }

      // Add new waypoint to the global list.
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

  QMessageBox mb1( QMessageBox::Information,
                   tr("Import Results"),
                   result,
                   QMessageBox::Ok,
                   this );

#ifdef ANDROID

  mb1.show();
  QPoint pos1 = mapToGlobal(QPoint( width()/2  - mb1.width()/2,
                                    height()/2 - mb1.height()/2 ));
  mb1.move( pos1 );

#endif

  mb1.exec();
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
