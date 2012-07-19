/***********************************************************************
**
**   gliderlistwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class GliderListWidget
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Glider management widget.
 *
 * This widget manages a list of gliders and provides some additional editor
 * functions.
 *
 * \date 2002-2012
 *
 */

#ifndef GLIDER_LIST_WIDGET_H
#define GLIDER_LIST_WIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QString>
#include <QList>

#include "glider.h"

class GliderListWidget : public QTreeWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( GliderListWidget )

public:

  GliderListWidget( QWidget *parent=0 );

  ~GliderListWidget();

  /**
   * @returns The identifier of the currently highlighted glider.
   */
  Glider *getSelectedGlider(bool take=false);

  /**
   * Retrieves the gliders from the configuration file and fills the list.
   */
  void fillList();

  /**
   * Saves all gliders from the list into the configuration file.
   */
  void save();

  /**
   * \return A flag indicating the widget's change state.
   */
  bool hasChanged()
  {
    return _changed;
  };

  /**
   * Aligns the columns to their contents.
   */
  void resizeListColumns()
    {
      resizeColumnToContents(0);
      resizeColumnToContents(1);
      resizeColumnToContents(2);
    };

  /**
   * Retrieves the glider object for the glider that was last stored
   * as the selected glider.
   *
   * @returns a @ref Glider object representing the stored glider,
   * or 0 if an error occurred or there was no stored selection.
   */
  static Glider *getStoredSelection();

  /**
   * Stores a reference in the configuration file that this glider was
   * the last selected glider. This is used to restore the selection
   * after a restart of Cumulus.
   */
  static void setStoredSelection( const Glider* glider );

  /**
   * Sets the selection to the item with this registration string.
   */
  void selectItemFromReg(const QString& registration);

protected:

  void showEvent( QShowEvent* event );

public slots:

  /**
   * Called if a glider has been edited.
   */
  void slot_Edited(Glider *glider);

  /**
   * Called if a glider has been added.
   */
  void slot_Added(Glider *glider);

  /**
   * Called if a glider has been deleted.
   */
  void slot_Deleted(Glider *glider);

private:

  int  _added;
  bool _changed;

  /** List of gliders. */
  QList<Glider*> Gliders;
};

#endif
