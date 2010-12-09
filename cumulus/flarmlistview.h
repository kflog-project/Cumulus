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
 * \class FlarmListView
 *
 * \author Axel Pauli
 *
 * \brief Flarm list view display.
 *
 * This widget shows the Flarm object data in a list.
 *
 * \date 2010
 */

#ifndef FLARM_LIST_VIEW_H
#define FLARM_LIST_VIEW_H

#include <QWidget>

#include "rowdelegate.h"

class QTreeWidget;
class QString;
class QTreeWidget;
class QTreeWidgetItem;

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
  void fillItemList( QString& object2Select );

  /**
   * aligns the columns to their contents
   */
  void resizeListColumns();

protected:

  void showEvent( QShowEvent *event );

public slots:

  /**
   * Called if new Flarm data are available.
   */
  void slot_Update();

  /** Set object to be selected. It is the object's hash key. */
  void slot_SetSelectedObject( QString newObject )
  {
    selectedFlarmObject = newObject;
  };

private slots:

  /**
   * This slot is called if the user clicks in a new row of the list. The new
   * list selection must be saved otherwise it will get lost during the next
   * update cycle.
   */
  void slot_ListItemClicked( QTreeWidgetItem* item, int column );

  /**
   * This slot is called to indicate that a selection has been made.
   */
  void slot_Select();

  /**
   * This slot is called to make a deselection of the selected row.
   */
  void slot_Unselect();

  /**
   * This slot is called when the list view should be closed without selection.
   */
  void slot_Close ();

signals:

  /**
   * Requests FlarmWidget to close this widget.
   */
  void closeListView();

  /**
   * Emit a new Flarm object selection.
   */
  void newObjectSelection( QString newObject );

private:

  QTreeWidget* list;
  RowDelegate* rowDelegate;

  /** Hash key of the selected Flarm object in the FlarmRadarView. */
  static QString selectedFlarmObject;

  /** Hash key of the current selected object in the list view. */
  static QString selectedListObject;

};

#endif /* FLARM_LIST_VIEW_H */
