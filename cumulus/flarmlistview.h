/***********************************************************************
**
**   flarmlistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief Flarm list view display.
 *
 * This widget shows the Flarm object data in a list.
 *
 */

#ifndef FLARM_LIST_VIEW_H
#define FLARM_LIST_VIEW_H

#include <QWidget>

#include "rowdelegate.h"

class QTreeWidget;

class FlarmListView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmListView )

public:

  /**
   * Constructor
   */
  FlarmListView( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~FlarmListView();

  /**
   * sets the list row height from configuration
   */
  void configRowHeight();

  /**
   * Fills the item list with their data.
   */
  void fillItemList();

  /**
   * aligns the columns to their contents
   */
  void resizeListColumns();

protected:

  void showEvent( QShowEvent *event );

private slots:

  /**
   * This slot is called to indicate that a selection has been made.
   */
  void slot_Select();

  /**
   * This slot is called when the list view should be closed without selection.
   */
  void slot_Close ();

signals:

  /*
   * Requests FlarmWidget to close this widget.
   */
  void closeListView();

private:

  QTreeWidget* list;
  RowDelegate* rowDelegate;

};

#endif /* FLARM_LIST_VIEW_H */
