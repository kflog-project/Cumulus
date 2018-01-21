/***********************************************************************
**
**   SettingsPageFlarm.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2018 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SettingsPageFlarm
 *
 * \author Axel Pauli
 *
 * \brief Flarm setup page.
 *
 * This widget can get and set Flarm configuration items.
 *
 * \date 2018
 *
 * \version 1.0
 */

#ifndef SettingsPageFlarm_H
#define SettingsPageFlarm_H

#include <QWidget>
#include <QMap>

class QCheckBox;
class QMutex;
class QPushButton;
class QString;
class QTableWidget;
class RowDelegate;

class SettingsPageFlarm : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY( SettingsPageFlarm )

public:

  /**
   * Constructor
   */
  SettingsPageFlarm( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~SettingsPageFlarm();

  /**
   * @return the aliasHash to the caller.
   */
  static QHash<QString, QString>& getConfigMap()
  {
    return aliasHash;
  };

  /** Loads the Flarm alias data from the related file into the alias hash. */
  static bool loadAliasData();

  /** Saves the Flarm alias data from the alias hash into the related file. */
  static bool saveAliasData();

protected:

  void showEvent( QShowEvent *event );

private slots:

  /** Adds a new row with two columns to the table. */
  void slot_AddRow( QString col0="", QString col1="" );

  /** Removes all selected rows from the table. */
  void slot_DeleteRows();

  /** Ok button press is handled here. */
  void slot_Ok();

  /** Close button press is handled here. */
  void slot_Close();

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
   * Flarm configuration map. The key is the Flarm configuration item and the
   * value the assigned property (rw, ro).
   */
  QHash<QString, QString> m_configMap;

};

#endif /* SettingsPageFlarm_H */
