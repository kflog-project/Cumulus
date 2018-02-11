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
 * \version 1.1
 */

#ifndef SettingsPageFlarm_H
#define SettingsPageFlarm_H

#include <QWidget>
#include <QHash>
#include <QList>
#include <QMessageBox>
#include <QQueue>

class QPushButton;
class QString;
class QTableWidget;
class RowDelegate;
class QTimer;

#include "flarm.h"

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

protected:

  void showEvent( QShowEvent *event );

private slots:

  /** called, when the help button was clicked to open the help page. */
  void slot_Help();

  /** Loads all Flarm data into the table. */
  void slot_GetAllFlarmData();

  /** Close button press is handled here. */
  void slot_Close();

  /** Called, when a cell is clicked to open an extra editor. */
  void slot_CellClicked ( int row, int column );

  /** Called, when a cell is double clicked to open a help tool tip. */
  void slot_CellDoubleClicked(int row, int column);

  /**
   * Header click is handled here. It sorts the clicked column in ascending
   * order.
   */
  void slot_HeaderClicked( int section );

  /**
   * Called, if the supervision request timer has expired.
   */
  void slot_Timeout();

  /**
   * This slot is called, if a new PFLAC sentence is available.
   */
  void slot_PflacSentence( QStringList& sentence );

signals:

  /**
   * Emit a new Flarm object selection.
   */
  void newObjectSelection( QString newObject );

  /** Emitted if the widget was closed. */
  void closed();

private:

  /** Loads all Flarm info and configuration items into the items list. */
  void loadTableItems();

  /** Adds a new row with four columns to the table. */
  void addRow2List( const QString& rowData );

  /** Sends the command to the connected Flarm device. */
  void requestFlarmData( QString &command, bool overwriteCursor );

  /** Sends the next command to Flarm from the command queue. */
  void nextFlarmCommand();

  /** Shows a popup message box to the user. */
  int messageBox( QMessageBox::Icon icon,
                  QString message,
                  QString title="",
                  QMessageBox::StandardButtons buttons = QMessageBox::Ok );

  /** Toggles operation of buttons. */
  void enableButtons( const bool toggle );

  /**
   * Checks the Flarm connection and returns the check state. True in case of
   * success otherwise false.
   */
  bool checkFlarmConnection();

  /**
   * Loads the FLARM item help.
   */
  void loadFlarmItemHelp();

  /** Table widget with two columns for alias entries. */
  QTableWidget* m_table;

  QPushButton* m_loadButton;
  QPushButton* m_closeButton;

  /** Adds additional space in the list. */
  RowDelegate* m_rowDelegate;

  /**
   * List with all Flarm info and configuration items.
   */
  QList<QString> m_items;

  /**
   * Queue with the Flarm commands.
   */
  QQueue<QString> m_commands;

  /** Supervision timer for the requests sent to the Flarm device. */
  QTimer* m_timer;

  /** Hash with FLARM item help data. */
  QHash<QString, QString> m_itemHelp;
};

#endif /* SettingsPageFlarm_H */
