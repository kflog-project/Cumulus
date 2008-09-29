/***********************************************************************
**
**   reachpointlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by Eckhard Voellm, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QPushButton>
#include <QFont>
#include <QTreeWidgetItem>
#include <QShortcut>

#include "reachpointlistview.h"
#include "cumulusapp.h"
#include "waypointlistview.h"
#include "generalconfig.h"
#include "mapcontents.h"
#include "mapconfig.h"
#include "wpeditdialog.h"
#include "waypointcatalog.h"
#include "reachablelist.h"
#include "sonne.h"

extern CuCalc* calculator;

ReachpointListView::ReachpointListView(CumulusApp *parent ) : QWidget(parent)
{
  setObjectName("ReachpointListView");
  // load pixmap of arrows for relative bearing
  _arrows = GeneralConfig::instance()->loadPixmap("arrows-15.png");
  _newList=true; //make sure we fill our list the first time it is shown

  par=parent;
  _outlandShow = true;  // Show outlandigs by default;
  QBoxLayout *topLayout = new QVBoxLayout( this );

  list = new QTreeWidget( this );
  list->setObjectName("reachpointview");

  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
  list->setAlternatingRowColors(true);
  list->setSortingEnabled(false);
  list->setSelectionMode(QAbstractItemView::SingleSelection);
  list->setColumnCount(7);
  list->hideColumn(6);
  list->setFocusPolicy(Qt::StrongFocus);

  QStringList sl;

  sl << tr(" Name")
     << tr("Dist.")
     << tr("Course")
     << tr("R")
     << tr("Arrvial")
     << tr(" SS");

  list->setHeaderLabels(sl);

  list->setColumnWidth( 0, 160 );
  list->setColumnWidth( 1, 74 );
  list->setColumnWidth( 3, 24 ); // the bearing icon

  rowDelegate = 0;
  fillRpList();

  topLayout->addWidget(list,10);
  QBoxLayout *buttonrow=new QHBoxLayout;
  topLayout->addLayout(buttonrow);

  /** @ee add a close button */
  QPushButton *cmdClose = new QPushButton(tr("Close"), this);
  buttonrow->addWidget(cmdClose);

  QPushButton *cmdInfo = new QPushButton(tr("Info"), this);
  buttonrow->addWidget(cmdInfo);

  cmdHideOl = new QPushButton(tr("Hide Outland"), this);
  buttonrow->addWidget(cmdHideOl);

  cmdShowOl = new QPushButton(tr("Show Outland"), this);
  buttonrow->addWidget(cmdShowOl);

  cmdSelect = new QPushButton(tr("Select"), this);
  buttonrow->addWidget(cmdSelect);

  connect(cmdSelect, SIGNAL(clicked()), this, SLOT(slot_Select()));
  connect(cmdInfo, SIGNAL(clicked()), this, SLOT(slot_Info()));
  // @ee add a close button
  connect(cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()));
  connect(cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()));
  connect(cmdHideOl, SIGNAL(clicked()), this, SLOT(slot_HideOl()));
  connect(cmdShowOl, SIGNAL(clicked()), this, SLOT(slot_ShowOl()));
  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(slot_Selected()));
  cmdShowOl->hide();
  cmdHideOl->show();

  // activate keyboard shortcut Return as select
  QShortcut* scSelect = new QShortcut( this );
  scSelect->setKey( Qt::Key_Return );
  connect( scSelect, SIGNAL(activated()), this, SLOT( slot_Select() ));
}


ReachpointListView::~ReachpointListView()
{}


/** Retrieves the reachable points from the map contents, and fills the list. */
void ReachpointListView::fillRpList()
{
  int safetyAlt = (int)GeneralConfig::instance()->getSafetyAltitude().getMeters();

  if ( calculator == 0 )
    {
      return;
    }

  // int n = calculator->getReachList()->getNumberSites();
  extern MapConfig * _globalMapConfig;
  QTreeWidgetItem* si = list->currentItem();
  QTreeWidgetItem* selectedItem = 0;
  QString sname = "";
  QPixmap icon;
  QPixmap iconImage;
  QBitmap iconMask;
  QPainter pnt;
  icon = QPixmap(18,18);

  if ( si )
    {
      sname = si->text(0);
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
  QList<ReachablePoint*> *pl = calculator->getReachList()->getList();
  int num = 0;
  QString key;

  // Do loop over all entries in the table of nearest sites
  for (int i = 0;
       i < pl->count() && num < calculator->getReachList()->getMaxNrOfSites();
       i++, num++)
    {

      ReachablePoint *rp = pl->at(i);

      if ( !_outlandShow && (rp->getType() == BaseMapElement::Outlanding) )
        {
          continue;
        }

      // Setup string for bearing
      QString bearing=QString("%1%2").arg( rp->getBearing() ).arg( QString(Qt::Key_degree) );

      // Calculate relative bearing too, very cool feature
      int relbearing = rp->getBearing() - calculator->getlastHeading();

      while (relbearing < 0)
        {
          relbearing += 360;
        }

      // This string is only used for sorting and is eclipsed by next field
      QString RB = QString("  %1").arg(relbearing);

      // Show arrival altitude or estimated time of arrival. It depends on
      // the glider selection.
      QString arrival = "---";

      if ( rp->getArrivalAlt().isValid() )
        {
          // there is a valid altitude defined
          arrival = rp->getArrivalAlt().getText( true, 0);
        }
      else if ( calculator->getLastSpeed().getMps() > 0.5 )
        {
          // Check, if we are moving. In this case the ETA to the target is displayed.
          // Moving is required to avoid division by zero!
          int eta = (int) rint(rp->getDistance().getMeters() / calculator->getLastSpeed().getMps());

          if ( eta < 100*3600 )
            {
              // display only eta if less than 100 hours
              arrival = QString("%1:%2").arg( eta/3600 ).arg( (eta%3600)/60, 2, 10, QChar('0') );
            }
        }

      // calculate sunset
      QString sr, ss;
      QDate date = QDate::currentDate();

      Sonne::sonneAufUnter( sr, ss, date, rp->getWgsPos(), 0 );

      // hidden column for default sorting
      key = QString("%1").arg(num, 3, 10, QLatin1Char('0') );

      QStringList sl;
      sl << rp->getName()
         << rp->getDistance().getText(false,1)
         << bearing
         << RB
         << arrival
         <<  " " + ss
         << key;

      QTreeWidgetItem* li = new QTreeWidgetItem( sl );
      li->setTextAlignment( 1, Qt::AlignRight|Qt::AlignVCenter );
      li->setTextAlignment( 2, Qt::AlignRight|Qt::AlignVCenter );
      li->setTextAlignment( 4, Qt::AlignRight|Qt::AlignVCenter );

      list->addTopLevelItem( li );

      // create landing site type icon
      pnt.begin(&icon);

      if ( rp->getReachable() == yes )
        {
          icon.fill( QColor(0,255,0) );
        }
      else if ( rp->getReachable() == belowSafety )
        {
          icon.fill( QColor(255,0,255) );
        }
      else
        {
          icon.fill( Qt::white );
        }

      pnt.drawPixmap(1, 1, _globalMapConfig->getPixmap(rp->getType(),false,true) );
      pnt.end();
      QIcon qi;
      qi.addPixmap( icon );
      qi.addPixmap( icon, QIcon::Selected );
      li->setIcon( 0, qi );

      // set Pixmap for rel. Bearing
      int rot=((relbearing+7)/15) % 24;
      QPixmap arrow;
      arrow = _arrows.copy( rot*16, 0, 16, 16);
      qi.addPixmap( arrow );
      qi.addPixmap( arrow, QIcon::Selected );
      li->setIcon( 3, qi );

      // store name of last selected to avoid jump to first element on each fill
      if ( rp->getName() == sname )
        {
          selectedItem = li;
        }

      QColor c;
      // list safely reachable sites in green
      if ( rp->getArrivalAlt().isValid() && rp->getArrivalAlt().getMeters() > 0 )
        {
          c = QColor(Qt::darkGreen);
        }
      // list narrowly reachable sites in magenta
      else if (rp->getArrivalAlt().isValid() && rp->getArrivalAlt().getMeters() > -safetyAlt)
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
  list->sortItems( 6, Qt::DescendingOrder );
  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->resizeColumnToContents(2);
  list->resizeColumnToContents(4);

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

  list->setFocus();
  list->setUpdatesEnabled(true);
}


void ReachpointListView::showEvent(QShowEvent *)
{
  if (_newList)
    {
      fillRpList();
      _newList=false;
    }
}


/** This signal is called to indicate that a selection has been made. */
void ReachpointListView::slot_ShowOl()
{
  _outlandShow = true;
  cmdShowOl->hide();
  cmdHideOl->show();
  fillRpList();
}


/** This signal is called to indicate that a selection has been made. */
void ReachpointListView::slot_HideOl()
{
  _outlandShow = false;
  cmdShowOl->show();
  cmdHideOl->hide();
  fillRpList();
}


/** This signal is called to indicate that a selection has been made. */
void ReachpointListView::slot_Select()
{
  emit newWaypoint(getSelectedWaypoint(), true);
  emit done();
}


/** This slot is called if the info button has been clicked */
void ReachpointListView::slot_Info()
{
  wayPoint *airfieldInfo = getSelectedWaypoint();

  if ( airfieldInfo )
    {
      emit info(airfieldInfo);
    }
}


/** @ee This slot is called if the listview is closed without selecting */
void ReachpointListView::slot_Close ()
{
  emit done();
}

void ReachpointListView::slot_Selected()
{
  if (ReachpointListView::getSelectedWaypoint()->equals(calculator->getselectedWp()))
    {
      cmdSelect->setEnabled(false);
    }
  else
    {
      cmdSelect->setEnabled(true);
    }
}


/** Returns a pointer to the currently high lighted reachpoint. */
wayPoint * ReachpointListView::getSelectedWaypoint()
{
  ReachablePoint * rp;
  int i,n;
  n =  calculator->getReachList()->getNumberSites();
  QTreeWidgetItem* li = list->currentItem();

  // @ee may be null
  if (li)
    {
      for (i=0; i < n; i++)
        {
          rp=calculator->getReachList()->getSite(i);

          if (rp->getName()==li->text(0))
            {
              selectedWp = *(rp->getWaypoint());
              selectedWp.importance = wayPoint::Normal;  // set importance to normal

              return &selectedWp;
              break;
            }
        }
    }
  return 0;
}


void ReachpointListView::slot_newList()
{
  if ( this->isVisible())
    {
      fillRpList();
      _newList=false;
    }
  else
    {
      _newList=true;
    }
}
