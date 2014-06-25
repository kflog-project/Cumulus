/***********************************************************************
**
**   preflightchecklistpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreFlightCheckListPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for a pre-flight check list display and editor.
 *
 * \date 2014
 *
 * \version $Id$
 *
 */

#ifndef PRE_FLIGHT_CHECK_LIST_PAGE_H
#define PRE_FLIGHT_CHECK_LIST_PAGE_H

#include <QWidget>

class QLabel;
class QPushButton;
class QTableWidget;
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
   * \param checklist Checklist content to be returned.
   *
   * \return true in case of success otherwise false
   *
   */
  bool loadCheckList( QString& checklist );

  /**
   * Writes the user's checklist file checklist.txt.
   *
   * \param checklist Checklist content to be saved.
   *
   * \return true in case of success otherwise false
   *
   */
  bool saveCheckList( QString& checklist );

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

 /** Adds a new row with two columns to the table. */
 void slot_AddRow( QString col0="", QString col1="" );

 /** Removes all selected rows from the table. */
 void slot_DeleteRows();

 /**
  * Called, if the edit button is pressed.
  */
 void slotEdit();

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
