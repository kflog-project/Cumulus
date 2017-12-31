/***********************************************************************
 **
 **   preflightwidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2003      by André Somers
 **                   2008-2017 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "calculator.h"
#include "gpsnmea.h"
#include "igclogger.h"
#include "layout.h"
#include "mainwindow.h"
#include "mapconfig.h"
#include "map.h"
#include "preflightchecklistpage.h"
#include "preflightgliderpage.h"
#include "preflightlogbookspage.h"
#include "preflightmiscpage.h"
#include "preflighttaskpage.h"
#include "preflightwaypointpage.h"
#include "preflightwidget.h"
#include "preflightwindpage.h"

#ifdef FLARM
#include "preflightflarmpage.h"
#endif

#ifdef INTERNET
#include "preflightlivetrack24page.h"
#include "preflightweatherpage.h"
#endif

#ifdef ANDROID
#include "preflightretrievepage.h"
#endif

#include "rowdelegate.h"

// Menu labels
#define PREFLIGHT   "Preflight Menu"
#define CHECKLIST   "Checklist"
#define COMMON      "Common"
#define FLARMENTRY  "Flarm-IGC"
#define GLIDER      "Glider"
#define LIVETRACK   "LiveTrack"
#define LOGBOOKS    "Logbooks"
#define RETRIEVE    "Retrieve"
#define TASKS       "Tasks/Routes"
#define WAYPOINTS   "Waypoints"
#define WEATHER     "METAR-TAF"
#define WIND        "Wind"

PreFlightWidget::PreFlightWidget( QWidget* parent ) :
  QWidget(parent)
{
  setObjectName("PreFlightWidget");
  setWindowTitle(tr("Preflight Settings"));
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( parent )
    {
      resize( parent->size() );
    }

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
  m_setupTree->setHeaderLabel( tr( "Preflight Menu" ) );

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
  item->setText( 0, tr(GLIDER) );
  item->setData( 0, Qt::UserRole, GLIDER );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(TASKS) );
  item->setData( 0, Qt::UserRole, TASKS );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(WAYPOINTS) );
  item->setData( 0, Qt::UserRole, WAYPOINTS );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(CHECKLIST) );
  item->setData( 0, Qt::UserRole, CHECKLIST );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(COMMON) );
  item->setData( 0, Qt::UserRole, COMMON );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(LOGBOOKS) );
  item->setData( 0, Qt::UserRole, LOGBOOKS );
  m_setupTree->addTopLevelItem( item );

#ifdef FLARM
  item = new QTreeWidgetItem;
  item->setText( 0, tr(FLARMENTRY) );
  item->setData( 0, Qt::UserRole, FLARMENTRY );

  if( calculator->moving() == false )
    {
      m_setupTree->addTopLevelItem( item );
    }
#endif

#ifdef ANDROID
  item = new QTreeWidgetItem;
  item->setText( 0, tr(RETRIEVE) );
  item->setData( 0, Qt::UserRole, RETRIEVE );
  m_setupTree->addTopLevelItem( item );
#endif

#ifdef INTERNET
  item = new QTreeWidgetItem;
  item->setText( 0, tr(WEATHER) );
  item->setData( 0, Qt::UserRole, WEATHER );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(LIVETRACK) );
  item->setData( 0, Qt::UserRole, LIVETRACK );
  m_setupTree->addTopLevelItem( item );
#endif

  item = new QTreeWidgetItem;
  item->setText( 0, tr(WIND) );
  item->setData( 0, Qt::UserRole, WIND );
  m_setupTree->addTopLevelItem( item );

  m_setupTree->sortByColumn ( 0, Qt::AscendingOrder );

  contentLayout->addSpacing( 25 );
  m_menuCb = new QCheckBox( tr("close menu") );
  m_menuCb->setChecked( GeneralConfig::instance()->getClosePreFlightMenu() == true ? Qt::Checked : Qt::Unchecked );
  contentLayout->addWidget( m_menuCb, 0, Qt::AlignVCenter|Qt::AlignBottom );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png", true)));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  //titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png", true));

  titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix, 0, Qt::AlignCenter);

  contentLayout->addStretch( 10 );
  contentLayout->addLayout(buttonBox);

  m_headerLabels << ( tr ("Preflight Menu") )
                 << ( tr ("Glider") )
                 << ( tr ("Tasks/Routes") )
#ifdef ANDROID
                 << ( tr ("Retrieve") )
#endif
                 << ( tr ("Waypoints") )
                 << ( tr ("Logbooks") )
#ifdef INTERNET
                 << ( tr ("LiveTrack") )
                 << ( tr ("METAR-TAF") )
#endif

                 << ( tr ("Wind") )
                 << ( tr ("Common") )
                 << ( tr ("Checklist") );

#ifdef FLARM

  if( calculator->moving() == false )
    {
      m_headerLabels << ( tr ("FLARM" ) );
    }

  requestFlarmConfig();

#endif

  m_setupTree->setMinimumWidth( Layout::maxTextWidth( m_headerLabels, font() ) + 100 );
  setVisible( true );
}

PreFlightWidget::~PreFlightWidget()
{
}

void PreFlightWidget::keyReleaseEvent( QKeyEvent* event )
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
void PreFlightWidget::slotPageClicked( QTreeWidgetItem* item, int column )
{
  Q_UNUSED( column );

  QString itemText = item->data( 0, Qt::UserRole ).toString();

  if( itemText == COMMON )
    {
      PreFlightMiscPage* pfmp = new PreFlightMiscPage( this );

      connect( pfmp, SIGNAL( settingsChanged() ),
               IgcLogger::instance(), SLOT( slotReadConfig() ) );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfmp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfmp->show();
    }
  else if( itemText == CHECKLIST )
    {
      PreFlightCheckListPage* pfclp = new PreFlightCheckListPage( this );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfclp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfclp->show();
    }
  else if( itemText == GLIDER )
    {
      PreFlightGliderPage* pfgp = new PreFlightGliderPage( this );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfgp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfgp->show();
    }
  else if( itemText == TASKS )
    {
      PreFlightTaskPage* pftp = new PreFlightTaskPage( this );

      connect( pftp, SIGNAL( newTaskSelected() ),
               IgcLogger::instance(), SLOT( slotNewTaskSelected() ) );

      connect( pftp, SIGNAL( newTaskSelected() ),
               MainWindow::mainWindow(), SLOT( slotPreFlightDataChanged() ) );

      connect( pftp, SIGNAL( newWaypoint( Waypoint*, bool ) ),
               calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );

      connect( pftp, SIGNAL(manualWindStateChange(bool)),
               calculator, SLOT(slot_ManualWindChanged(bool)) );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pftp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pftp->show();
    }
  else if( itemText == WAYPOINTS )
    {
      PreFlightWaypointPage* pfwp = new PreFlightWaypointPage( this );

      connect( pfwp, SIGNAL(waypointsAdded()),
               Map::getInstance(), SLOT(slotRedraw()) );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfwp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfwp->show();
    }
  else if( itemText == LOGBOOKS )
    {
      PreFlightLogBooksPage* pflp = new PreFlightLogBooksPage( this );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pflp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pflp->show();
    }
  else if( itemText == WIND )
    {
      PreFlightWindPage* pfwp = new PreFlightWindPage( this );

      connect( pfwp, SIGNAL(manualWindStateChange(bool)),
               calculator, SLOT(slot_ManualWindChanged(bool)) );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfwp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfwp->show();
    }

#ifdef FLARM

  else if( itemText == FLARMENTRY )
    {
      PreFlightFlarmPage* pffp = new PreFlightFlarmPage( this );

      connect( pffp, SIGNAL( newTaskSelected() ),
               IgcLogger::instance(), SLOT( slotNewTaskSelected() ) );

      connect( pffp, SIGNAL( newTaskSelected() ),
               MainWindow::mainWindow(), SLOT( slotPreFlightDataChanged() ) );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pffp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pffp->show();
    }

#endif

#ifdef INTERNET

  else if( itemText == LIVETRACK )
    {
      PreFlightLiveTrack24Page* pflt24p = new PreFlightLiveTrack24Page( this );

      connect( pflt24p, SIGNAL( onOffStateChanged(bool) ),
               MainWindow::mainWindow()->getLiveTrack24Logger(),
               SLOT( slotNewSwitchState(bool) ) );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pflt24p, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pflt24p->show();
    }
  else if( itemText == WEATHER )
    {
      PreFlightWeatherPage* pfwp = new PreFlightWeatherPage( this );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfwp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfwp->show();
    }

#endif

#ifdef ANDROID

  else if( itemText == RETRIEVE )
    {
      PreFlightRetrievePage* pfrp = new PreFlightRetrievePage( this );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfrp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfrp->show();
    }

#endif

}

void PreFlightWidget::slotAccept()
{
  setVisible( false );
  GeneralConfig::instance()->setClosePreFlightMenu( m_menuCb->checkState() == Qt::Checked ? true : false );
  emit closeConfig();
  // Make a delay of 200 ms before the widget is closed to prevent undesired
  // selections in an underlaying list. Problem occurred on Galaxy S3.
  QTimer::singleShot(200, this, SLOT(close()));
}

void PreFlightWidget::slotReject()
{
  setVisible( false );
  GeneralConfig::instance()->setClosePreFlightMenu( m_menuCb->checkState() == Qt::Checked ? true : false );
  emit closeConfig();
  // Make a delay of 200 ms before the widget is closed to prevent undesired
  // selections in an underlaying list. Problem occurred on Galaxy S3.
  QTimer::singleShot(200, this, SLOT(close()));
}

#ifdef FLARM

/**
 * Called to request the Flarm device type from the configuration data.
 */
void PreFlightWidget::requestFlarmConfig()
{
  if( GpsNmea::gps->getConnected() != true )
    {
      // No GPS connection available
      return;
    }

  // We do request the Flarm device type.
  GpsNmea::gps->sendSentence( "$PFLAC,R,DEVTYPE" );
}

#endif

