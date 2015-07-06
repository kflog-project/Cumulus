/***********************************************************************
**
**   gliderlistwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
 * \date 2002-2015
 *
 */

#ifndef GLIDER_LIST_WIDGET_Hm_glider
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

  /**
   * Class constructor
   *
   * \param[in] parent The parent widget
   *
   * \param[in] considerSelectionChanges Set it to true, if selection changes
   *            in the list should be considered.
   *
   */
  GliderListWidget( QWidget *parent, bool considerSelectionChanges=false );

  /**
   * Default constructor used for glider list migration.
   */
  GliderListWidget();

  ~GliderListWidget();

  /**
   * @returns The selected glider object from the glider list.
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
    return m_changed;
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
  static Glider *getUserSelectedGlider();

  /**
   * Sets the selection to the item with this registration string.
   */
  void selectItemFromReg(const QString& registration);

  /**
   * Selects the item in the QTreeWidget, which is marked in the glider list.
   */
  void selectItemFromList();

  /**
   * Clears the user selection flag of all gliders in the glider list.
   */
  void clearSelectionInGliderList();

 private:

  /**
   * Called to migrate a glider list to the new format.
   */
  void migrateGliderList();

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

  /**
   * This slot is called when the current item changes. The current item is
   * specified by current, and this replaces the previous current item.
   */
  void currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

 private:

  int  m_added;
  bool m_changed;

  /** List of gliders. */
  QList<Glider*> m_gliders;

  /**
   * Extension of class QTreeWidgetItem with a glider object.
   */
  class GliderItem : public QTreeWidgetItem
    {
      public:

      GliderItem( Glider* glider, const QStringList& strings, int type=0 ) :
	QTreeWidgetItem(strings, type),
	m_glider(glider)
	{
	};

      Glider* getGlider() const
	{
	  return m_glider;
	}

      private:

      Glider* m_glider;
    };
};

#endif
