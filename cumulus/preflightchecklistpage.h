/***********************************************************************
**
**   preflightchecklistpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014-2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class PreFlightCheckListPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for a pre-flight checklist display and editor.
 *
 * \date 2014-2018
 *
 * \version 1.2
 *
 */

#ifndef PRE_FLIGHT_CHECK_LIST_PAGE_H
#define PRE_FLIGHT_CHECK_LIST_PAGE_H

#include <QWidget>

class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class RowDelegate;

class PreFlightCheckListPage : public QWidget
{
  Q_OBJECT

 private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( PreFlightCheckListPage )

 public:

  /**
   * Filename of the checklist.
   */
  const char* CheckListFileName;

  PreFlightCheckListPage( QWidget* parent );

  virtual ~PreFlightCheckListPage();

 protected:

  void showEvent(QShowEvent *);

 private:

  /**
   * Reads the user's checklist file checklist.txt.
   *
   * \return true in case of success otherwise false
   *
   */
  bool loadCheckList();

  /**
   * Writes the user's checklist file checklist.txt.
   *
   * \return true in case of success otherwise false
   *
   */
  bool saveCheckList();

 signals:

   /**
    * Emitted, if the widget is closed.
    */
   void closingWidget();

 private slots:

  /**
  * Called to show/hide the filename of the checklist.
  */
  void slotToogleFilenameDisplay();

  /**
  * Adds a new row with one column to the table. The new column is placed
  * after the last selected list entry resp. at the end of the list.
  */
  void slotAddRow( QString text="" );

  /** Removes all selected rows from the table. */
  void slotDeleteRows();

  /** Called, if the item selection is changed. */
  void slotItemSelectionChanged();

  /**
  * The passed cell can be modified in a editor widget.
  *
  * \param row row of table cell
  *
  * \param column column of table cell
  */
  void slotEditCell( int row, int column );

  /**
  * Called, if a cell is clicked in the table.
  *
  * \param row row of table cell
  *
  * \param column column of table cell
  */
  void slotCellClicked( int row, int column );

  /**
  * Called, if the edit button is pressed.
  */
  void slotEdit();

  /**
  * Called, if the help button is pressed.
  */
  void slotHelp();

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

 private:

  /** Display for checklist filename. */
  QLabel* m_fileDisplay;

  /** Table widget with one column for check entries. */
  QTableWidget* m_list;

  /** Adds additional space in the list. */
  RowDelegate* rowDelegate;

  /** Ok button. */
  QPushButton* m_ok;

  /** Edit button. */
  QPushButton* m_editButton;

  /** Delete button. */
  QPushButton *m_deleteButton;
};

#endif
