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

  list->setObjectName("WaypointListWidgetClass");
  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
//  list->setSortingEnabled(true);
  list->setColumnCount(3);
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

  connect( list, SIGNAL( itemClicked(QTreeWidgetItem*,int) ),
    this, SLOT( slot_listItemClicked(QTreeWidgetItem*,int) ) );

  rowDelegate = 0;
  listFilled = false;
}


WPListWidgetClass::~WPListWidgetClass()
{
  // qDebug("WPListWidgetClass::~WPListWidgetClass()");
}


void WPListWidgetClass::showEvent(QShowEvent *)
{
  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->resizeColumnToContents(2);
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

/** This slot sends a signal to indicate that a selection has been made. */
void WPListWidgetClass::slot_listItemClicked(QTreeWidgetItem* li, int)
{
//  qDebug("WPListWidgetClass::slot_listItemClicked");
  if ( li == 0)
    return;
  
  // Special rows selected?
  QString test = li->text(1);

  if (test == "Next Page")
    filter->showPage(true); // "true" is forward
  else
    if (test == "Previous Page")
      filter->showPage(false); // "false" is backward
    else
      emit wpSelectionChanged();
}
