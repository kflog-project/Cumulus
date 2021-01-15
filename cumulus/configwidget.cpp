/***********************************************************************
 **
 **   configwidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by André Somers
 **                   2007-2021 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * This is the configuration widget of Cumulus. All general settings are
 * handled here. These are in general not related to the pre-flight
 * preparation. For that exists a separate configuration widget.
 */

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "calculator.h"
#include "configwidget.h"
#include "generalconfig.h"
#include "gpsnmea.h"
#include "helpbrowser.h"
#include "layout.h"
#include "mainwindow.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "rowdelegate.h"

#include "settingspageairspace.h"

#ifdef FLARM
#include "SettingsPageFlarm.h"
#endif

#include "settingspageglider.h"
#include "settingspageinformation.h"
#include "settingspagelines.h"
#include "settingspagelooknfeel.h"
#include "settingspagemapobjects.h"
#include "settingspagemapsettings.h"
#include "settingspagepersonal.h"
#include "SettingsPagePointData.h"
#include "settingspageunits.h"
#include "settingspagetask.h"
#include "settingspageterraincolors.h"

#ifndef ANDROID
#include "settingspagegps.h"
#else
#include "settingspagegps4a.h"
#endif

// Menu labels
#define POINT_DATA      "Point Data"
#define AIRSPACES       "Airspaces"
#ifdef FLARM
#define FLARML          "FLARM"
#endif
#define GLIDERS         "Gliders"
#define GPS             "GPS"
#define INFORMATION     "Information"
#define LINES           "Lines"
#define LOOK_FEEL       "Look&Feel"
#define MAP_OBJECTS     "Map Objects"
#define MAP_SETTINGS    "Map Settings"
#define PERSONAL        "Personal"
#define TASK            "Task"
#define TERRAIN_COLORS  "Terrain Colors"
#define UNITS           "Units"

extern Calculator* calculator;

ConfigWidget::ConfigWidget( QWidget* parent ) :
  QWidget(parent)
{
  setObjectName("ConfigWidget");
  setWindowTitle( tr("General Settings") );
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( parent )
    {
      resize( parent->size() );
    }

  m_headerLabels  << tr("Point Data")
                  << tr("Airspaces")
#ifdef FLARM
                  << tr("FLARM")
#endif
                  << tr("Gliders")
                  << tr("GPS")
                  << tr("Information")
                  << tr("Lines")
                  << tr("Look&Feel")
                  << tr("Map Objects")
                  << tr("Map Settings")
                  << tr("Personal")
                  << tr("Task")
                  << tr("Terrain Colors")
                  << tr("Units");

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  m_setupTree = new QTreeWidget( this );
  m_setupTree->setRootIsDecorated( false );
  m_setupTree->setItemsExpandable( false );
  m_setupTree->setSortingEnabled( true );
  m_setupTree->setSelectionMode( QAbstractItemView::SingleSelection );
  m_setupTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_setupTree->setAlternatingRowColors(true);
  m_setupTree->setColumnCount( 1 );
  m_setupTree->setFocusPolicy( Qt::StrongFocus );
  m_setupTree->setUniformRowHeights(true);
  m_setupTree->setHeaderLabel( tr( "Settings Menu" ) );

  m_headerLabels << ( tr( "  Settings Menu  " ) );

  // Set additional space per row
  RowDelegate* rowDelegate = new RowDelegate( m_setupTree, 10 );
  m_setupTree->setItemDelegate( rowDelegate );

  QTreeWidgetItem* headerItem = m_setupTree->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );

  m_setupTree->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_setupTree->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef ANDROID
  QScrollBar* lvsb = m_setupTree->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture(m_setupTree->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_setupTree->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  connect( m_setupTree, SIGNAL(itemClicked( QTreeWidgetItem*, int )),
           this, SLOT( slotPageClicked( QTreeWidgetItem*, int )) );

  contentLayout->addWidget( m_setupTree, 5 );

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr(PERSONAL) );
  item->setData( 0, Qt::UserRole, PERSONAL );
  // item->setIcon( 0, _mainWindow->getPixmap("kde_identity_32.png") );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(GPS) );
  item->setData( 0, Qt::UserRole, GPS );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(GLIDERS) );
  item->setData( 0, Qt::UserRole, GLIDERS );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(MAP_SETTINGS) );
  item->setData( 0, Qt::UserRole, MAP_SETTINGS );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(MAP_OBJECTS) );
  item->setData( 0, Qt::UserRole, MAP_OBJECTS );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(TASK) );
  item->setData( 0, Qt::UserRole, TASK );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(LINES) );
  item->setData( 0, Qt::UserRole, LINES );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(POINT_DATA) );
  item->setData( 0, Qt::UserRole, POINT_DATA );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(AIRSPACES) );
  item->setData( 0, Qt::UserRole, AIRSPACES );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(TERRAIN_COLORS) );
  item->setData( 0, Qt::UserRole, TERRAIN_COLORS );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(UNITS) );
  item->setData( 0, Qt::UserRole, UNITS );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(INFORMATION) );
  item->setData( 0, Qt::UserRole, INFORMATION );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(LOOK_FEEL) );
  item->setData( 0, Qt::UserRole, LOOK_FEEL );
  m_setupTree->addTopLevelItem( item );

#ifdef FLARM
  if( calculator->moving() == false )
    {
      item = new QTreeWidgetItem;
      item->setText( 0, tr(FLARML) );
      item->setData( 0, Qt::UserRole, FLARML );
      m_setupTree->addTopLevelItem( item );
    }
#endif

  m_setupTree->sortByColumn ( 0, Qt::AscendingOrder );

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png"));

  connect(help, SIGNAL(pressed()), this, SLOT(slotHelp()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(help, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix, 0, Qt::AlignCenter);
  contentLayout->addStretch( 10 );
  contentLayout->addLayout(buttonBox);

  m_setupTree->setMinimumWidth( Layout::maxTextWidth( m_headerLabels, font() ) + 100 );
  setVisible( true );
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::keyReleaseEvent( QKeyEvent* event )
{
  // close the dialog on key press
  switch(event->key())
    {
      case Qt::Key_Close:
      case Qt::Key_Escape:
        slotReject();
        break;

      default:
        QWidget::keyReleaseEvent( event );
        break;
    }
}

/**
 * Called, if an item is pressed in the tree view.
 */
void ConfigWidget::slotPageClicked( QTreeWidgetItem* item, int column )
{
  Q_UNUSED( column );

  QString itemText = item->data( 0, Qt::UserRole ).toString();

  if( itemText == POINT_DATA )
    {
      extern MapContents* _globalMapContents;

      SettingsPagePointData* page = new SettingsPagePointData( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      connect( page,  SIGNAL( reloadOpenAip() ),
               _globalMapContents, SLOT( slotReloadOpenAipPoi() ) );

      connect( page, SIGNAL(downloadOpenAipPois( const QStringList& )),
               _globalMapContents, SLOT(slotDownloadOpenAipPois( const QStringList& )) );

      page->show();
      return;
    }

  if( itemText == AIRSPACES )
    {
      extern MapConfig*   _globalMapConfig;
      extern MapContents* _globalMapContents;

      SettingsPageAirspace* page = new SettingsPageAirspace( this );

      connect( page, SIGNAL(airspaceColorsUpdated()),
               _globalMapConfig, SLOT(slotReloadAirspaceColors()));

      connect( page, SIGNAL(downloadAirspaces(const QStringList& )),
              _globalMapContents, SLOT(slotDownloadAirspaces(const QStringList& )));

      page->show();
      return;
    }

  if( itemText == GLIDERS )
    {
      SettingsPageGlider* page = new SettingsPageGlider( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      page->show();
      return;
    }

  if( itemText == GPS )
    {

#ifndef ANDROID
      SettingsPageGPS* page = new SettingsPageGPS( this );
#else
      SettingsPageGPS4A* page = new SettingsPageGPS4A( this );
#endif

      connect( page, SIGNAL(settingsChanged()),
               GpsNmea::gps, SLOT(slot_reset()) );

      connect( page, SIGNAL(startNmeaLog()),
               GpsNmea::gps, SLOT(slot_openNmeaLogFile()) );

      connect( page , SIGNAL(endNmeaLog()),
               GpsNmea::gps, SLOT(slot_closeNmeaLogFile()) );

      connect( page , SIGNAL(newPressureDevice( const QString& )),
               GpsNmea::gps, SLOT(slot_pressureDevice( const QString&)) );

      page->show();
      return;
    }

  if( itemText == INFORMATION )
    {
      SettingsPageInformation* page = new SettingsPageInformation( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      page->show();
      return;
    }

  if( itemText == LINES )
    {
      SettingsPageLines* page = new SettingsPageLines( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      page->show();
      return;
    }

  if( itemText == LOOK_FEEL )
    {
      SettingsPageLookNFeel* page = new SettingsPageLookNFeel( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      page->show();
      return;
    }

  if( itemText == MAP_OBJECTS )
    {
      SettingsPageMapObjects* page = new SettingsPageMapObjects( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      page->show();
      return;
    }

  if( itemText == MAP_SETTINGS )
    {
      extern MapContents* _globalMapContents;

      SettingsPageMapSettings* page = new SettingsPageMapSettings( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      connect( page, SIGNAL(downloadMapArea( const QPoint&, const Distance& )),
              _globalMapContents, SLOT(slotDownloadMapArea( const QPoint&, const Distance&)));


      page->show();
      return;
    }

  if( itemText == PERSONAL )
    {
      SettingsPagePersonal* page = new SettingsPagePersonal( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      connect( page, SIGNAL( homePositionChanged() ),
               this, SLOT( slotNewHomePosition() ) );

      page->show();
      return;
    }

  if( itemText == TASK )
    {
      SettingsPageTask* page = new SettingsPageTask( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      page->show();
      return;
    }

  if( itemText == TERRAIN_COLORS )
    {
      SettingsPageTerrainColors* page = new SettingsPageTerrainColors( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      page->show();
      return;
    }

  if( itemText == UNITS )
    {
      SettingsPageUnits* page = new SettingsPageUnits( this );

      connect( page, SIGNAL( settingsChanged() ),
               MainWindow::mainWindow(), SLOT( slotReadconfig() ) );

      page->show();
      return;
    }

#ifdef FLARM
  if( itemText == FLARML )
    {
      SettingsPageFlarm* page = new SettingsPageFlarm( this );

      page->show();
      return;
    }
#endif

}

void ConfigWidget::slotNewHomePosition()
{
  // Check, if we have not a valid GPS fix. In this case we do move the map
  // to the new home position.
  if( GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      emit gotoHomePosition();
    }
}

void ConfigWidget::slotHelp()
{
  QString file = "cumulus-settings.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void ConfigWidget::slotAccept()
{
  emit closeConfig();
  // Make a delay of 200 ms before the widget is closed to prevent undesired
  // selections in an underlaying list. Problem occurred on Galaxy S3.
  QTimer::singleShot(200, this, SLOT(close()));
}

void ConfigWidget::slotReject()
{
  emit closeConfig();
  // Make a delay of 200 ms before the widget is closed to prevent undesired
  // selections in an underlaying list. Problem occurred on Galaxy S3.
  QTimer::singleShot(200, this, SLOT(close()));
}

void ConfigWidget::closeEvent( QCloseEvent *event )
{
  emit closeConfig();
  event->accept();
}
