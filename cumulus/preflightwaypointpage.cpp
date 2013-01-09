/***********************************************************************
**
**   preflightwaypointpage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2011-2013 by Axel Pauli
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

#ifdef FLICK_CHARM
#include "flickcharm.h"
#endif

#ifdef USE_NUM_PAD
#include "numberEditor.h"
#endif

extern MapContents* _globalMapContents;

// Minimum amount of required free memory to start import of a waypoint file.
// Do not under run this limit, OS can freeze is such a case.
#define MINIMUM_FREE_MEMORY 1024*25

PreFlightWaypointPage::PreFlightWaypointPage(QWidget *parent) :
  QWidget(parent),
  m_centerRef(Position),
  m_waypointFileFormat(GeneralConfig::Binary)
{
  setObjectName("PreFlightWaypointPage");

  m_wpTypesBox = new QComboBox;
  m_wpTypesBox->addItem( tr("All"), WaypointCatalog::All );
  m_wpTypesBox->addItem( tr("Airfields"), WaypointCatalog::Airfields );
  m_wpTypesBox->addItem( tr("Gliderfields"), WaypointCatalog::Gliderfields );
  m_wpTypesBox->addItem( tr("Outlandings"), WaypointCatalog::Outlandings );
  m_wpTypesBox->addItem( tr("Other Points"), WaypointCatalog::OtherPoints );

#ifdef USE_NUM_PAD
  m_wpRadiusBox = new NumberEditor;
  m_wpRadiusBox->setDecimalVisible( false );
  m_wpRadiusBox->setPmVisible( false );
  m_wpRadiusBox->setMaxLength(4);
  m_wpRadiusBox->setSuffix( " " + Distance::getUnitText() );
  m_wpRadiusBox->setMaximum( 9999 );
  m_wpRadiusBox->setText( "500" );

  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "([1-9][0-9]{0,3})" ), this );
  m_wpRadiusBox->setValidator( eValidator );

  int mw = QFontMetrics(font()).width("9999 Km") + 10;
  m_wpRadiusBox->setMinimumWidth( mw );
#else

  m_wpRadiusBox = new QComboBox;
  m_wpRadiusBox->setEditable( true );
  m_wpRadiusBox->setValidator( new QIntValidator(1, 5000, this) );
  QStringList itemList;
  itemList << "10" << "50" << "100" << "300" << "500" << "1000" << "2000";
  m_wpRadiusBox->addItems( itemList );
  m_wpRadiusBox->setCurrentIndex( 4 );
#ifdef MAEMO5  
  m_wpRadiusBox->setMinimumWidth( m_wpRadiusBox->sizeHint().width() );
#endif
#endif
  
  QFormLayout* selectLayout1 = new QFormLayout;
  selectLayout1->addRow( tr("Type:"), m_wpTypesBox );

  QFormLayout* selectLayout2 = new QFormLayout;

#ifdef USE_NUM_PAD
  selectLayout2->addRow( tr("Radius") + ":", m_wpRadiusBox );

#else
  selectLayout2->addRow( tr("Radius") + " (" + Distance::getUnitText() + "):",
                         m_wpRadiusBox );
#endif

  QHBoxLayout* selectLayout = new QHBoxLayout;
  selectLayout->setSpacing( 5 );
  selectLayout->addLayout( selectLayout1 );
  selectLayout->addLayout( selectLayout2 );

  m_selectGroup = new QGroupBox( tr("Select") );
  m_selectGroup->setLayout( selectLayout );

  //---------------------------------------------------------------------------
  m_positionRB = new QRadioButton(tr("Position"));
  m_homeRB     = new QRadioButton(tr("Homesite"));
  m_airfieldRB = new QRadioButton(tr("Airfield"));

  m_homeRB->setChecked( true );

  QButtonGroup* radiusButtonGroup = new QButtonGroup(this);
  radiusButtonGroup->setExclusive(true);
  radiusButtonGroup->addButton( m_positionRB, Position );
  radiusButtonGroup->addButton( m_homeRB, Home );
  radiusButtonGroup->addButton( m_airfieldRB, Airfield );

  connect( radiusButtonGroup, SIGNAL( buttonClicked(int)),
           this, SLOT(slotSelectCenterReference(int)));

#ifdef USE_NUM_PAD
  m_centerLat = new LatEditNumPad;
  m_centerLon = new LongEditNumPad;
#else
  m_centerLat = new LatEdit;
  m_centerLon = new LongEdit;
#endif

  m_centerLatLabel = new QLabel(tr("Latitude:"));
  m_centerLonLabel = new QLabel(tr("Longitude:"));

  QGridLayout* latLonGrid = new QGridLayout;
  latLonGrid->setSpacing( 10 );
  latLonGrid->addWidget( m_centerLatLabel, 0 , 0 );
  latLonGrid->addWidget( m_centerLat, 0, 1 );
  latLonGrid->addWidget( m_centerLonLabel, 1 , 0 );
  latLonGrid->addWidget( m_centerLon, 1, 1 );
  latLonGrid->setColumnStretch( 2, 5 );

  m_homeLabel   = new QLabel;
  m_airfieldBox = new QComboBox;

#ifdef QSCROLLER
  QScroller::grabGesture(m_airfieldBox->view(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(m_airfieldBox->view());
#endif

  QGridLayout* centerPointGrid = new QGridLayout;
  centerPointGrid->setSpacing( 5 );
  centerPointGrid->addWidget( m_positionRB, 0, 0 );
  centerPointGrid->addWidget( m_homeRB, 1, 0 );
  centerPointGrid->addWidget( m_airfieldRB, 2, 0 );

  QVBoxLayout* centerPointVBox = new QVBoxLayout;

  centerPointVBox->setMargin( 0 );
  centerPointVBox->addLayout( latLonGrid );
  centerPointVBox->addWidget( m_homeLabel );
  centerPointVBox->addWidget( m_airfieldBox );

  centerPointGrid->addLayout( centerPointVBox, 0, 2, 3, 1 );
  centerPointGrid->setColumnStretch( 1, 5 );

  m_centerPointGroup = new QGroupBox( tr("Center Point") );
  m_centerPointGroup->setLayout( centerPointGrid );

  //---------------------------------------------------------------------------
  QPushButton* loadButton = new QPushButton( tr("Load") );
  loadButton->setMaximumWidth( loadButton->sizeHint().width() + 10 );

  connect( loadButton, SIGNAL(clicked()), this, SLOT(slotImportFile()) );

  m_filterToggle = new QCheckBox( tr("Filter") );
  m_filterToggle->setCheckable( true );
  m_filterToggle->setChecked( true );

  connect( m_filterToggle, SIGNAL(stateChanged(int)),
           this, SLOT(slotToggleFilter(int)) );

  slotToggleFilter( Qt::Checked );

  QHBoxLayout* controlBox = new QHBoxLayout;
  controlBox->addWidget( loadButton );
  controlBox->addStretch( 5 );
  controlBox->addWidget( m_filterToggle );

  QVBoxLayout* wpiBox = new QVBoxLayout;
  wpiBox->setSpacing( 5 );
  wpiBox->addWidget( m_selectGroup );
  wpiBox->addWidget( m_centerPointGroup );
  wpiBox->addSpacing( 5 );
  wpiBox->addLayout( controlBox );

  QGroupBox* wpiGroup = new QGroupBox( tr("Waypoint Import") );
  wpiGroup->setLayout( wpiBox );

  m_wpFileFormatBox = new QComboBox;
  m_wpFileFormatBox->addItem( tr("Binary"), 0 );
  m_wpFileFormatBox->addItem( tr("XML"), 1 );

  m_wpPriorityBox = new QComboBox;
  m_wpPriorityBox->addItem( tr("Source"), 3 );
  m_wpPriorityBox->addItem( tr("Low"), 0 );
  m_wpPriorityBox->addItem( tr("Normal"), 1 );
  m_wpPriorityBox->addItem( tr("High"), 2 );

  QFormLayout* storageLayout = new QFormLayout;
  storageLayout->addRow( tr("Storage Format:"), m_wpFileFormatBox );

  QFormLayout* priorityLayout = new QFormLayout;
  priorityLayout->addRow( tr("Priority:"), m_wpPriorityBox );

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

  m_waypointFileFormat = conf->getWaypointFileFormat();

  m_wpFileFormatBox->setCurrentIndex( m_waypointFileFormat );

  m_wpPriorityBox->setCurrentIndex( conf->getWaypointPriority() );

  m_homeLabel->setText( WGSPoint::printPos(conf->getHomeLat(), true) +
                      " - " +
                      WGSPoint::printPos(conf->getHomeLon(), false) );

  slotSelectCenterReference( conf->getWaypointCenterReference() );

  loadAirfieldComboBox();

  int idx = m_airfieldBox->findText( conf->getWaypointAirfieldReference() );

  if( idx == -1 )
    {
      idx = 1;
    }

  m_airfieldBox->setCurrentIndex( idx );
}

void PreFlightWaypointPage::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setWaypointFileFormat( (GeneralConfig::WpFileFormat) m_wpFileFormatBox->itemData(m_wpFileFormatBox->currentIndex()).toInt() );
  conf->setWaypointPriority( m_wpPriorityBox->currentIndex() );
  conf->setWaypointCenterReference( m_centerRef );
  conf->setWaypointAirfieldReference( m_airfieldBox->currentText() );

  if( m_waypointFileFormat != m_wpFileFormatBox->currentIndex() &&
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
  m_centerRef = static_cast<enum PreFlightWaypointPage::CenterReference>(reference);

  switch( m_centerRef )
  {
    case PreFlightWaypointPage::Home:
      m_homeRB->setChecked( true );
      m_centerLat->setVisible(false);
      m_centerLon->setVisible(false);
      m_centerLatLabel->setVisible(false);
      m_centerLonLabel->setVisible(false);
      m_airfieldBox->setVisible(false);
      m_homeLabel->setVisible(true);
      break;
    case PreFlightWaypointPage::Airfield:
      m_airfieldRB->setChecked( true );
      m_centerLat->setVisible(false);
      m_centerLon->setVisible(false);
      m_centerLatLabel->setVisible(false);
      m_centerLonLabel->setVisible(false);
      m_airfieldBox->setVisible(true);
      m_homeLabel->setVisible(false);
      break;
    case PreFlightWaypointPage::Position:
    default:
      m_positionRB->setChecked( true );
      m_centerRef = PreFlightWaypointPage::Position;
      m_centerLat->setVisible(true);
      m_centerLon->setVisible(true);
      m_centerLatLabel->setVisible(true);
      m_centerLonLabel->setVisible(true);
      m_airfieldBox->setVisible(false);
      m_homeLabel->setVisible(false);
      break;
    }
}

/**
 * Called to toggle the filter.
 */
void PreFlightWaypointPage::slotToggleFilter( int toggle )
{
  m_selectGroup->setEnabled( (toggle == Qt::Checked ? true : false) );
  m_centerPointGroup->setEnabled( (toggle == Qt::Checked ? true : false) );
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

  if( m_filterToggle->isChecked() )
    {
      // We have to set the filter values before catalog reading.
      enum WaypointCatalog::WpType type = (enum WaypointCatalog::WpType)
          m_wpTypesBox->itemData(m_wpTypesBox->currentIndex()).toInt();

#ifdef USE_NUM_PAD
      int radius = m_wpRadiusBox->value();
#else
      int radius = m_wpRadiusBox->currentText().toInt();
#endif

      WGSPoint wgsPoint;

      if( m_positionRB->isChecked() )
        {
          wgsPoint = WGSPoint( m_centerLat->KFLogDegree(), m_centerLon->KFLogDegree() );
        }
      else if( m_homeRB->isChecked() )
        {
          GeneralConfig *conf = GeneralConfig::instance();
          wgsPoint = WGSPoint( conf->getHomeLat(), conf->getHomeLon() );
        }
      else if( m_airfieldRB->isChecked() )
        {
          QString s = m_airfieldBox->currentText();

          if( m_airfieldDict.contains(s) )
            {
              SinglePoint *sp = m_airfieldDict.value(s);
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
      int priority = m_wpPriorityBox->itemData(m_wpPriorityBox->currentIndex()).toInt();

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

  m_airfieldDict.clear();
  m_airfieldBox->clear();

  QStringList airfieldList;

  for( int l = 0; l < 2; l++ )
    {
      for( uint loop = 0; loop < _globalMapContents->getListLength(searchList[l]); loop++ )
      {
        SinglePoint *hitElement = (SinglePoint *) _globalMapContents->getElement(searchList[l], loop );
        airfieldList.append( hitElement->getName() );
        m_airfieldDict.insert( hitElement->getName(), hitElement );
      }
  }

  airfieldList.sort();
  m_airfieldBox->addItems( airfieldList );
  m_airfieldBox->setCurrentIndex( 0 );
}
