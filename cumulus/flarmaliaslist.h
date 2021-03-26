/***********************************************************************
**
**   flarmaliaslist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2021 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class FlarmAliasList
 *
 * \author Axel Pauli
 *
 * \brief Flarm alias list display and editor.
 *
 * This widget can create, modify or remove alias names for FLARM hexadecimal
 * identifiers. The names are displayed in a two column table. The content of
 * the table is stored in a text file in the user's data directory.
 *> >
 * \date 2010-2021
 *
 * \version 1.2
 */

#pragma once

#include <QWidget>
#include <QHash>
#include <QMutex>

class QCheckBox;
class QPushButton;
class QString;
class QTableWidget;
class RowDelegate;

class FlarmAliasList : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmAliasList )

public:

  // The alias name length is limited to 15 characters
  static const int MaxAliasLength = 15;

  /**
   * Constructor
   */
  FlarmAliasList( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~FlarmAliasList();

  /**
   * @return a aliasHash reference to the caller.
   */
  static QHash<QString, QPair<QString, bool> >& getAliasHash()
  {
    return aliasHash;
  };

  /**
   * @return a aliasShowHash reference to the caller.
   */
  static QHash<QString, QPair<QString, bool> >& getAliasShowHash()
  {
    return aliasShowHash;
  };

  /** Loads the Flarm alias data from the related file into the alias hash. */
  static bool loadAliasData();

  /** Saves the Flarm alias data from the alias hash into the related file. */
  static bool saveAliasData();

protected:

  void showEvent( QShowEvent *event );

private slots:

  /** Loads the Flarm alias show data from the alias hash. */
  static void loadAliasShowData();

  /** Adds a new row with three columns to the table. */
  void slot_AddRow( QString col0="", QString col1="", bool col2=false );

  /** Removes all selected rows from the table. */
  void slot_DeleteRows();

  /** Ok button press is handled here. */
  void slot_Ok();

  /** Close button press is handled here. */
  void slot_Close();

  /** Called if the help button was pressed. */
  void slot_Help();

  /** Cell content change is handled here. */
  void slot_CellChanged( int row, int column );

  /** Called, when a cell is clicked to open an extra editor. */
  void slot_CellClicked ( int row, int column );

  /**
   * Header click is handled here. It sorts the clicked column in ascending
   * order.
   */
  void slot_HeaderClicked( int section );

  /** Called, if the item selection is changed. */
  void slot_ItemSelectionChanged();

  /**
   * Called is the checkbox is toggled.
   */
  void slot_scrollerBoxToggled( int state );

signals:

  /**
   * Emit a new Flarm object selection.
   */
  void newObjectSelection( QString newObject );

  /** Emitted if the widget was closed. */
  void closed();

private:

  /** Table widget with two columns for alias entries. */
  QTableWidget* list;

  /** Adds additional space in the list. */
  RowDelegate* rowDelegate;

  /** Delete button. */
  QPushButton *deleteButton;

  /** A checkbox to toggle scroller against a big scrollbar. */
  QCheckBox* m_enableScroller;

  /**
   * Flarm alias hash dictionary. The key is the Flarm Id and the value the
   * assigned alias name together with a draw flag.
   */
  static QHash<QString, QPair<QString, bool> > aliasHash;

  /**
   * Flarm alias hash dictionary, where the show flag is set to true.
   */
  static QHash<QString, QPair<QString, bool> > aliasShowHash;

  /** Mutex used for alias file load and save. */
  static QMutex mutex;
};

