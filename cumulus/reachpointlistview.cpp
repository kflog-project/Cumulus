/***********************************************************************
**
**   reachpointlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by Eckhard Völlm
**                   2008-2025 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.ListWidgetParent
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

#include "generalconfig.h"
#include "layout.h"
#include "MainWindow.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "reachablelist.h"
#include "reachpointlistview.h"
#include "sonne.h"
#include "wpeditdialog.h"
#include "waypointcatalog.h"
#include "waypointlistview.h"

extern Calculator* calculator;
extern MapConfig* _globalMapConfig;

ReachpointListView::ReachpointListView( QWidget* parent ) :
  QWidget(parent),
  _homeChanged( false ),
  _newList(true),
  _outlandShow(true),
  rowDelegate(0),
  m_enableScroller(0)
{
  setObjectName("ReachpointListView");

  list = new QTreeWidget;
  list->setObjectName("ReachpointView");

  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
  list->setAlternatingRowColors(true);
  list->setSortingEnabled(false);
  list->setSelectionMode(QAbstractItemView::SingleSelection);
  list->setColumnCount(8);
  list->hideColumn(7);
  list->setFocusPolicy(Qt::StrongFocus);

  list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  list->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  QStringList sl;

  sl << tr(" Name")
     << tr("Channel")
     << tr("Dist.")
     << tr("Course")
     << tr("Arrvial")
     << tr("Length")
     << tr(" SS");

  list->setHeaderLabels(sl);

  list->setColumnWidth( 0, 160 );
  list->setColumnWidth( 2, 74 );

  fillRpList();

  QBoxLayout *buttonrow = new QHBoxLayout;

  QPushButton *cmdClose = new QPushButton(tr("Close"), this);
  buttonrow->addWidget(cmdClose);

  QPushButton *cmdInfo = new QPushButton(tr("Info"), this);
  buttonrow->addWidget(cmdInfo);

  cmdHideOl = new QPushButton(tr("Hide Outland"), this);
  buttonrow->addWidget(cmdHideOl);

  cmdShowOl = new QPushButton(tr("Show Outland"), this);
  buttonrow->addWidget(cmdShowOl);

  cmdHome = new QPushButton(tr("Home"), this);
  buttonrow->addWidget(cmdHome);

  cmdSelect = new QPushButton(tr("Select"), this);
  buttonrow->addWidget(cmdSelect);

  cmdPageUp = new QPushButton( this );
  cmdPageUp->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "up.png")) );
  cmdPageUp->setIconSize( QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)) );
  cmdPageUp->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );
  cmdPageUp->setToolTip( tr("move page up") );

#if defined(QSCROLLER) || defined(QTSCROLLER)

  m_enableScroller = new QCheckBox( "][");
  m_enableScroller->setCheckState( Qt::Checked );
  m_enableScroller->setMinimumHeight( Layout::getButtonSize(12) );

  connect( m_enableScroller, SIGNAL(stateChanged(int)),
	   this, SLOT(slot_ScrollerBoxToggled(int)) );

#endif

  cmdPageDown = new QPushButton( this );
  cmdPageDown->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "down.png")) );
  cmdPageDown->setIconSize( QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)) );
  cmdPageDown->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );
  cmdPageDown->setToolTip( tr("move page down") );

  QVBoxLayout* movePageBox = new QVBoxLayout;
  movePageBox->setSpacing( 0 );
  movePageBox->addWidget( cmdPageUp, 10 );

#if defined(QSCROLLER) || defined(QTSCROLLER)

  movePageBox->addSpacing( 15 * Layout::getIntScaledDensity() );
  movePageBox->addWidget( m_enableScroller, 0, Qt::AlignCenter );
  movePageBox->addSpacing( 15 * Layout::getIntScaledDensity() );

#endif

  movePageBox->addWidget( cmdPageDown, 10 );

  QHBoxLayout *listBox = new QHBoxLayout;
  listBox->addWidget( list );
  listBox->addLayout( movePageBox );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addLayout(listBox, 10);
  topLayout->addLayout(buttonrow);

  connect(cmdSelect, SIGNAL(pressed()), this, SLOT(slot_Select()));
  connect(cmdInfo, SIGNAL(pressed()), this, SLOT(slot_Info()));
  connect(cmdClose, SIGNAL(pressed()), this, SLOT(slot_Close()));
  connect(cmdHideOl, SIGNAL(pressed()), this, SLOT(slot_HideOl()));
  connect(cmdShowOl, SIGNAL(pressed()), this, SLOT(slot_ShowOl()));
  connect(cmdHome, SIGNAL(pressed()), this, SLOT(slot_Home()));
  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(slot_Selected()));
  connect(cmdPageUp, SIGNAL(pressed()), this, SLOT(slot_PageUp()));
  connect(cmdPageDown, SIGNAL(pressed()), this, SLOT(slot_PageDown()));

  cmdShowOl->hide();
  cmdHideOl->show();

#ifndef ANDROID
  // activate keyboard shortcut Return as select
  QShortcut* scSelect = new QShortcut( this );
  scSelect->setKey( Qt::Key_Return );
  connect( scSelect, SIGNAL(activated()), this, SLOT( slot_Select() ));
#endif

}

ReachpointListView::~ReachpointListView()
{}

/** Retrieves the reachable points from the map contents, and fills the list. */
void ReachpointListView::fillRpList()
{
  // qDebug() << "ReachpointListView::fillRpList() vvalue=" << list->verticalScrollBar()->value();

  if ( calculator == static_cast<Calculator *>(0) )
    {
      return;
    }

  int safetyAlt = (int) GeneralConfig::instance()->getSafetyAltitude().getMeters();

  // Save the vertical scrollbar position. It returns the number of hidden rows.
  int vvalue = list->verticalScrollBar()->value();

  QTreeWidgetItem* selectedItem = 0;
  QString sname = "";
  QPainter pnt;

  // calculate the needed icon size
  QFontMetrics qfm( font() );
  int iconSize = qfm.height() - 8;

  list->setIconSize( QSize(iconSize, iconSize) );

  // Check, if an item selection has been made.
  QList<QTreeWidgetItem *> il = list->selectedItems();

  if ( il.size() > 0 )
    {
      sname = il.at(0)->text(0);
    }

  // set row height at each list fill - has probably changed.
  // Note: rpMargin is a manifold of 2 to ensure symmetry
  int rpMargin = GeneralConfig::instance()->getListDisplayRPMargin();

  if ( rowDelegate )
    {
      rowDelegate->setVerticalMargin(rpMargin);
    }
  else
    {
      rowDelegate = new RowDelegate( list, rpMargin );
      list->setItemDelegate( rowDelegate );
    }

  list->setUpdatesEnabled(false);
  list->clear();

  // Create a pointer to the list of nearest sites
  QList<ReachablePoint> *pl = calculator->getReachList()->getList();
  int num = 0;
  QString key;

  // Do loop over all entries in the table of nearest sites
  for (int i = 0;
       i < pl->count() && num < calculator->getReachList()->getMaxNrOfSites();
       i++, num++)
    {

      ReachablePoint& rp = (*pl)[i];

      if ( !_outlandShow && (rp.getType() == BaseMapElement::Outlanding) )
        {
          continue;
        }

      // Setup string for bearing
      QString bearing = QString("%1%2").arg( rp.getBearing() ).arg( QString(Qt::Key_degree) );

      // Calculate relative bearing too, very cool feature
      int relbearing = rp.getBearing() - calculator->getLastHeading();

      while (relbearing < 0)
        {
          relbearing += 360;
        }

      // Show arrival altitude or estimated time of arrival. It depends on
      // the glider selection.
      QString arrival = "---";

      if ( rp.getArrivalAlt().isValid() )
        {
          // there is a valid altitude defined
          arrival = rp.getArrivalAlt().getText( true, 0);
        }
      else if ( calculator->getLastSpeed().getMps() > 0.5 )
        {
          // Check, if we are moving. In this case the ETA to the target is displayed.
          // Moving is required to avoid division by zero!
          int eta = (int) rint(rp.getDistance().getMeters() / calculator->getLastSpeed().getMps());

          if ( eta < 100*3600 )
            {
              // display only eta if less than 100 hours
              arrival = QString("%1:%2").arg( eta/3600 ).arg( (eta%3600)/60, 2, 10, QChar('0') );
            }
        }

      // calculate sunset
      QString sr, ss, tz;
      QDate date = QDate::currentDate();

      Sonne::sonneAufUnter( sr, ss, date, rp.getWgsPos(), tz );

      // hidden column for default sorting
      key = QString("%1").arg(num, 3, 10, QLatin1Char('0') );

      QString rLen("");

      if( rp.getRunwayLength() > 0.0 )
        {
          rLen = QString("%1").arg( rp.getRunwayLength(), 0, 'f', 0 ) + " m";
        }

      QString frequencies;

      QList<Frequency>& fList = rp.getFrequencyList();

      if( fList.size() == 1 )
        {
          frequencies = fList.at(0).frequencyAsString();
        }
      else
        {
          // Only a main frequency should be shown in the table
          for( int i = 0; i < fList.size(); i++ )
            {
              quint8 type = fList[i].getType();
              bool primary = fList[i].isPrimary();

              if( type == Frequency::Tower|| type == Frequency::Radio ||
                  type == Frequency::Information || primary == true )
                {
                  frequencies = fList.at(i).frequencyAsString();
                  break;
                }
            }
        }

      if( fList.size() > 0 && frequencies.isEmpty() )
        {
          // Nothing assigned, take the first element.
          frequencies = fList.at(0).frequencyAsString();
        }

      if( frequencies.isEmpty() )
        {
          frequencies = "   ";
        }

      QStringList sl;
      sl << rp.getName()
         << frequencies
         << rp.getDistance().getText(false,1)
         << bearing
         << arrival
         << rLen
         <<  " " + ss + " " + tz
         << key;

      QTreeWidgetItem* li = new QTreeWidgetItem( sl );

      li->setTextAlignment( 1, Qt::AlignLeft|Qt::AlignVCenter );
      li->setTextAlignment( 2, Qt::AlignRight|Qt::AlignVCenter );
      li->setTextAlignment( 3, Qt::AlignRight|Qt::AlignVCenter );
      li->setTextAlignment( 4, Qt::AlignRight|Qt::AlignVCenter );
      li->setTextAlignment( 5, Qt::AlignRight|Qt::AlignVCenter );
      li->setTextAlignment( 6, Qt::AlignLeft|Qt::AlignVCenter );

      list->addTopLevelItem( li );

      QColor iconColor;

      if ( rp.getReachable() == ReachablePoint::yes )
        {
          iconColor = QColor(0, 255, 0);
        }
      else if ( rp.getReachable() == ReachablePoint::belowSafety )
        {
          iconColor = QColor(255, 0, 255);
        }
      else
        {
          iconColor = Qt::transparent;
        }

      // create landing site type icon
      QPixmap sitePm = _globalMapConfig->getPixmap(rp.getType(), false);
      QPixmap icon( sitePm.size() + QSize(1, 1) );
      icon.fill( iconColor );

      QPainter painter;
      painter.begin(&icon);
      painter.drawPixmap( 1, 1, sitePm );
      painter.end();

      QIcon qi;
      qi.addPixmap( icon );
      li->setIcon( 0, qi );

      QPixmap directionPm;

      QPen pen(Qt::black);
      pen.setWidth(0);

      // Draw a triangle pointing into direction of landing site
      MapConfig::createTriangle( directionPm,
                                 iconSize,
                                 Qt::black,
                                 relbearing,
                                 1.0,
                                 Qt::transparent,
                                 pen );

      li->setIcon( 3, directionPm );

      // store name of last selected to avoid jump to first element on each fill
      if ( rp.getName() == sname )
        {
          selectedItem = li;
        }

      QColor c;
      // list safely reachable sites in green
      if ( rp.getArrivalAlt().isValid() && rp.getArrivalAlt().getMeters() > 0 )
        {
          c = QColor(Qt::darkGreen);
        }
      // list narrowly reachable sites in magenta
      else if (rp.getArrivalAlt().isValid() && rp.getArrivalAlt().getMeters() > -safetyAlt)
        {
          c = QColor(Qt::darkMagenta);
        }
      // list other near sites in black
      else
        {
          c = QColor(Qt::black);
        }

      for (int i=0; i < li->columnCount(); i++)
        {
          li->setForeground(i, QBrush(c));
        }
    }

  // sort list
  list->sortItems( 7, Qt::DescendingOrder );
  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->resizeColumnToContents(2);
  list->resizeColumnToContents(3);
  list->resizeColumnToContents(4);
  list->resizeColumnToContents(5);
  list->resizeColumnToContents(6);

#if 0
  if ( selectedItem == 0 )
    {
      list->scrollToTop();
      list->setCurrentItem( list->topLevelItem(0) );
    }
  else
    {
       list->scrollToItem( selectedItem );
       list->setCurrentItem( selectedItem );
    }
#endif

  if ( selectedItem  )
    {
       list->setCurrentItem( selectedItem );

       QTreeWidgetItem* item = list->topLevelItem ( vvalue );

       if( item )
         {
           list->scrollToItem( item, QAbstractItemView::PositionAtTop );
         }
    }

  list->setUpdatesEnabled(true);
}

void ReachpointListView::showEvent( QShowEvent *event )
{
  // clear an old selection
  list->clearSelection();

  cmdSelect->setEnabled(false);
  cmdHome->setEnabled(false);

  if (_newList)
    {
      fillRpList();
      _newList = false;
    }

  if( list->topLevelItemCount() )
    {
      // set list to the top.
      list->scrollToItem( list->topLevelItem(0), QAbstractItemView::PositionAtTop );
    }

  // Show the home button only if we are not to fast in move to avoid
  // usage during flight. The redefinition of the home position will trigger
  // a reload of the airfield list.
  if( calculator->moving() )
    {
      cmdHome->setVisible(false);
    }
  else
    {
      cmdHome->setVisible(true);
    }

  // Reset home changed
  _homeChanged = false;

  QWidget::showEvent( event );
}

/** This slot is called to indicate that a selection has been made. */
void ReachpointListView::slot_ShowOl()
{
  _outlandShow = true;
  cmdShowOl->hide();
  cmdHideOl->show();
  fillRpList();
}

/** This slot is called to indicate that a selection has been made. */
void ReachpointListView::slot_HideOl()
{
  _outlandShow = false;
  cmdShowOl->show();
  cmdHideOl->hide();
  fillRpList();
}

/** This slot is called to indicate that a selection has been made. */
void ReachpointListView::slot_Select()
{
  Waypoint* wp = getCurrentEntry();

  if( wp )
    {
      emit newWaypoint(getCurrentEntry(), true);
      emit done();
    }
}

/** This slot is called if the info button has been pressed */
void ReachpointListView::slot_Info()
{
  Waypoint* airfieldInfo = getCurrentEntry();

  if ( airfieldInfo )
    {
      emit info(airfieldInfo);
    }
}

/** This slot is called when the listview should be closed without selection. */
void ReachpointListView::slot_Close ()
{
  emit done();

  // Check, if we have not a valid GPS fix. In this case we do move the map
  // to the new home position.
  if( _homeChanged == true && GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      emit gotoHomePosition();
      _homeChanged = false;
    }
}

void ReachpointListView::slot_Selected()
{
  cmdHome->setEnabled(true);

  // @AP: this slot is also called, if the list is cleared and a selection
  // does exist. In such a case the returned waypoint is a Null pointer!
  Waypoint* wp = getCurrentEntry();

  if( wp == static_cast<Waypoint *>(0) )
    {
      return;
    }

  if( GeneralConfig::instance()->getHomeCoord() == wp->wgsPoint )
    {
      // no new coordinates, ignore request
      cmdHome->setEnabled(false);
    }
  else
    {
      cmdHome->setEnabled(true);
    }

  if( ReachpointListView::getCurrentEntry()->equals(calculator->getTargetWp()) )
    {
      cmdSelect->setEnabled(false);
    }
  else
    {
      cmdSelect->setEnabled(true);
    }
}

/** Returns a pointer to the currently selected reachpoint. */
Waypoint* ReachpointListView::getCurrentEntry()
{
  int n =  calculator->getReachList()->getNumberSites();
  QTreeWidgetItem* li = list->currentItem();

  if (li)
    {
      for( int i=0; i < n; i++ )
        {
          const ReachablePoint rp = calculator->getReachList()->getSite(i);

          if (rp.getName() == li->text(0))
            {
              selectedWp = *(rp.getWaypoint());
              selectedWp.priority = Waypoint::Normal;  // set priority to normal
              return &selectedWp;
            }
        }
    }

  return static_cast<Waypoint *>(0);
}

void ReachpointListView::slot_newList()
{
  // qDebug( "ReachpointListView::slot_newList() is called" );

  if ( this->isVisible() )
    {
      fillRpList();
      _newList = false;
    }
  else
    {
      _newList = true;
    }
}

/** Called to set a new home position. The change of the home position can trigger
 *  a reload of many map data, if option projection follows home is active.
 */
void ReachpointListView::slot_Home()
{
  Waypoint* _wp = getCurrentEntry();

  if( _wp == static_cast<Waypoint *> ( 0 ) )
    {
      return;
    }

  GeneralConfig *conf = GeneralConfig::instance();

  if( conf->getHomeCoord() == _wp->wgsPoint )
    {
      // no new coordinates, ignore request
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Set home site" ),
                  tr("Use point<br><b>%1</b><br>as your home site?").arg(_wp->name) +
                  tr("<br>Change can take<br>a few seconds."),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::Yes );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::Yes )
    {
      // save new home position and elevation
      conf->setHomeCountryCode( _wp->country );
      conf->setHomeName( _wp->name );
      conf->setHomeCoord( _wp->wgsPoint );
      conf->setHomeElevation( Distance(_wp->elevation) );

      emit newHomePosition( _wp->wgsPoint );
      _homeChanged = true;
    }
}

void ReachpointListView::slot_PageUp()
{
  if( cmdPageUp->isDown() )
    {
      if( list->topLevelItemCount() == 0 )
        {
          return;
        }

      // That is the height of one row in the list
      QRect rect = list->visualItemRect(list->topLevelItem(0));

      // Calculate rows per page. Headline must be subtracted.
      int pageRows = ( list->height() / rect.height() ) - 1;

      int newPos = list->verticalScrollBar()->value() - (rect.height() * pageRows);

      int minPos = list->verticalScrollBar()->minimum();

      if( newPos > minPos )
        {
          list->verticalScrollBar()->setValue( newPos );
        }
      else
        {
          // Begin of list is reached
          list->verticalScrollBar()->setValue( minPos );
          return;
        }

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slot_PageUp()));
    }
}

void ReachpointListView::slot_PageDown()
{
  if( cmdPageDown->isDown() )
    {
      if( list->topLevelItemCount() == 0 )
        {
          return;
        }

      // That is the height of one row in the list
      QRect rect = list->visualItemRect(list->topLevelItem(0));

      // Calculate rows per page. Headline must be subtracted.
      int pageRows = ( list->height() / rect.height() ) - 1;

      int newPos = list->verticalScrollBar()->value() + (rect.height() * pageRows);

      int maxPos = list->verticalScrollBar()->maximum();

      if( newPos < maxPos )
        {
          list->verticalScrollBar()->setValue( newPos );
        }
      else
        {
          // End of list is reached
          list->verticalScrollBar()->setValue( maxPos );
          return;
        }

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slot_PageDown()));
    }
}

void ReachpointListView::slot_ScrollerBoxToggled( int state )
{
  if( m_enableScroller == 0 )
    {
      return;
    }

  if( state == Qt::Checked )
    {

#ifdef QSCROLLER
      list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
      list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

#ifdef ANDROID
       // Reset scrollbar style sheet to default.
       QScrollBar* lvsb = list->verticalScrollBar();
       lvsb->setStyleSheet( "" );
#endif

    }
  else if( state == Qt::Unchecked)
    {

#ifdef QSCROLLER
      QScroller::ungrabGesture( list->viewport() );
 #endif

#ifdef QTSCROLLER
       QtScroller::ungrabGesture( list->viewport() );
#endif

#ifdef ANDROID
       // Make the vertical scrollbar bigger for Android
       QScrollBar* lvsb = list->verticalScrollBar();
       lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

    }
}
