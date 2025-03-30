/***********************************************************************
**
**   KRT2Widget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2025 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class KRT2Widget
 *
 * \author Axel Pauli
 *
 * \brief KRT2 user interface to set frequencies
 *
 * This widget can set frequencies at the KRT2 device as active or standby.
 *
 * \date 2025
 *
 * \version 1.0
 */

#pragma once

#include <QWidget>
#include <QList>
#include <QMessageBox>

#include "Frequency.h"

class QPushButton;
class QLabel;
class QString;
class QTableWidget;
class RowDelegate;
class QPixmap;

#include "KRT2.h"

class KRT2Widget : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY( KRT2Widget )

public:

  /**
   * Constructor
   */
  KRT2Widget( QWidget *parent, QString& header, QList<Frequency>& fqList );

  /**
   * Destructor
   */
  virtual ~KRT2Widget();

protected:

  void showEvent( QShowEvent *event );

  void closeEvent(QCloseEvent *event );

private slots:

  /** called, when the help button was clicked to open the help page. */
  void slot_Help();

  /** Close button press is handled here. */
  void slot_Close();

  /** Called, when a cell is clicked to open an extra editor. */
  void slot_CellClicked ( int row, int column );

  /**
   * Header click is handled here. It sorts the clicked column in ascending
   * order.
   */
  void slot_HeaderClicked( int section );

  /** Called, to exchange the frequencies active/standby on the KRT2 radio. */
  void slot_exchangeFrequency();

signals:

  /** Emitted if the widget was closed. */
  void closed();

private:

  /** Loads all Flarm info and configuration items into the items list. */
  void loadTableItems();

  /** Adds a new row with four columns to the table. */
  void addRow2List( const Frequency& frequency );

  /** Shows a popup message box to the user. */
  int messageBox( QMessageBox::Icon icon,
                  QString message,
                  QString title="",
                  QMessageBox::StandardButtons buttons = QMessageBox::Ok );

  /** Toggles operation of buttons. */
  void enableButtons( const bool toggle );

  /**
   * Checks the KRt2 connection and returns the check state. True in case of
   * success otherwise false.
   */
  bool checkKRT2Connection();

  /** Creates the KRT2 status icons. */
  void createIcons();

  /** Content of headline */
  QString m_header;

  /** Head line of the widget */
  QLabel* m_headline;

  /** Table widget with two columns for alias entries. */
  QTableWidget* m_table;

  /** Adds additional space in the list. */
  RowDelegate* m_rowDelegate;

  /** Exchange the KRT2 frequency standby against active and vice versa. */
  QPushButton* m_excangeFrequency;

  /** Close button for the widget. */
  QPushButton* m_closeButton;

  /**
   * Frequency list for the widget to be displayed.
   */
  QList<Frequency> m_fqList;

  /** KRT2 LED status display. */
  QLabel* m_statusLed;

  /** Red circle for a disconnected KRT2 device.*/
  QPixmap* m_red;

  /** Green circle for a connected KRT2 device.*/
  QPixmap* m_green;
};

