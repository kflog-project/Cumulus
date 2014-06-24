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

#include <QLabel>
#include <QTextEdit>
#include <QWidget>
#include <QPushButton>

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

  /** Editor widget to display and edit the check list. */
  QTextEdit* m_editor;

  /** Ok button. */
  QPushButton* m_ok;

  /** Button to activate the editor function. */
  QPushButton* m_editButton;
};

#endif
