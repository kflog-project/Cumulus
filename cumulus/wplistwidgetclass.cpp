/***********************************************************************
**
**   wplistwidgetclass.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008 Josua Dietze
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QVBoxLayout>

#include "wplistwidgetclass.h"
#include "generalconfig.h"

// A new widget to remove double code in airfield list view, waypoint list view and
// task editor.
// Contains standard airfield list and attached filter (filter button row on demand)
// Subclassed by airfieldlistwidget and waypointlistwidget

WPListWidgetClass::WPListWidgetClass(QWidget *parent) : QWidget(parent)
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  list = new QTreeWidget( this );

  list->setObjectName("waypointlist");
  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
//  list->setSortingEnabled(true);
  list->setColumnCount(3);
  list->setColumnWidth( 0, 160 );
  list->setColumnWidth( 1, 220 );
  list->setAllColumnsShowFocus(true);
  list->setSelectionMode(QAbstractItemView::SingleSelection);
  list->setSelectionBehavior(QAbstractItemView::SelectRows);
  list->setFocusPolicy( Qt::StrongFocus );

  QStringList sl;
  sl << tr("Name") << tr("Description") << tr("ICAO");
  list->setHeaderLabels(sl);
  list->setFocus();

  filter=new ListViewFilter(list, this);
  filter->setObjectName("listfilter");
  topLayout->addWidget(filter);

  topLayout->addWidget(list,10);

  connect( list, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slot_SelectionChanged() ) );

  rowDelegate = 0;
  listFilled = false;
}


WPListWidgetClass::~WPListWidgetClass()
{
  // qDebug("WPListWidgetClass::~WPListWidgetClass()");
}


void WPListWidgetClass::showEvent(QShowEvent *)
{
  list->setFocus();
}

void WPListWidgetClass::configRowHeight()
{
  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();

  if ( rowDelegate )
    rowDelegate->setVerticalMargin(afMargin);
  else {
    rowDelegate = new RowDelegate( list, afMargin );
    list->setItemDelegate( rowDelegate );
  }
}


/** This slot is called from parent when closing */
void WPListWidgetClass::slot_Done ()
{
  filter->off();
}


/** This slot is called from parent when "Select" button was pressed */
void WPListWidgetClass::slot_Select ()
{
  QTreeWidgetItem* li = list->currentItem();
  if ( li == 0)
    return;
  
  // Special rows selected?
  QString test = li->text(1);

  if (test == "Next Page")
    filter->showPage(true); // "true" is up
  else
    if (test == "Previous Page")
      filter->showPage(false); // "false" is down
}


/** This slot sends a signal to indicate that a selection has been made. */
void WPListWidgetClass::slot_SelectionChanged()
{
  // qDebug("WPListWidgetClass::slot_SelectionChanged");
  emit wpSelectionChanged();
}
