/***********************************************************************
**
**   wplistwidgetparent.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008      by Josua Dietze
**                   2009-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
************************************************************************/

#include <QtGui>

#include "wplistwidgetparent.h"
#include "generalconfig.h"

WpListWidgetParent::WpListWidgetParent(QWidget *parent) : QWidget(parent)
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setContentsMargins( 0, 0, 0, 0  );

  list = new QTreeWidget( this );
  list->setObjectName("WpListWidgetParent");
  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
  list->setAlternatingRowColors(true);
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

  filter = new ListViewFilter( list, this );
  filter->setObjectName( "ListViewFilter" );

  topLayout->addWidget(filter);
  topLayout->addWidget(list, 10);

  connect( list, SIGNAL( itemClicked(QTreeWidgetItem*,int) ),
           this, SLOT( slot_listItemClicked(QTreeWidgetItem*,int) ) );

  rowDelegate   = 0;
  firstLoadDone = false;
}

WpListWidgetParent::~WpListWidgetParent()
{
  // qDebug("WpListWidgetParent::~WpListWidgetParent()");
  delete filter;
}

void WpListWidgetParent::showEvent( QShowEvent *event )
{
  Q_UNUSED(event)

  // align colums to contents before showing
  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->resizeColumnToContents(2);
  list->setFocus();
}

void WpListWidgetParent::configRowHeight()
{
  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();

  if ( rowDelegate )
    {
      rowDelegate->setVerticalMargin(afMargin);
    }
  else
    {
      rowDelegate = new RowDelegate( list, afMargin );
      list->setItemDelegate( rowDelegate );
    }
}

/** This slot is called from parent when closing */
void WpListWidgetParent::slot_Done()
{
  // Remove all list items and the filter items.
  filter->clear();
  list->clear();
  firstLoadDone = false;
}

/** This slot sends a signal to indicate that a selection has been made. */
void WpListWidgetParent::slot_listItemClicked(QTreeWidgetItem* li, int)
{
//  qDebug("WpListWidgetParent::slot_listItemClicked");
  if ( li == 0)
    {
      return;
    }

  // Special rows selected?
  QString test = li->text(1);

  if (test == ListViewFilter::NextPage)
    {
      list->setUpdatesEnabled(false);
      filter->showPage(true); // "true" is forward
      list->setUpdatesEnabled(true);
    }
  else if (test == ListViewFilter::PreviousPage)
    {
      list->setUpdatesEnabled(false);
      filter->showPage(false); // "false" is backward
      list->setUpdatesEnabled(true);
    }
  else
    {
      emit wpSelectionChanged();
    }
}
