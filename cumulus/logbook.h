/***********************************************************************
**
**   logbook.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class Logbook
 *
 * \author Axel Pauli
 *
 * \brief Flight logbook list display and editor.
 *
 * This widget can list all entries of a flight logbook. The user can remove
 * selected or all log entries if he wants. The content of the logbook is stored
 * in a text file in the user's data directory.
 *
 * \date 2012
 *
 * \version $Id$
 */

#ifndef Logbook_h
#define Logbook_h

#include <QWidget>

class QPushButton;
class QStringList;
class QTableWidget;
class RowDelegate;

class Logbook : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( Logbook )

public:

  Logbook( QWidget *parent=0 );

  virtual ~Logbook();

private:

  /** Sets the table header items of our own. */
  void setTableHeader();

  /** Loads the logbook data into the table. */
  void loadLogbookData();

protected:

  virtual void showEvent( QShowEvent *event );

private slots:

  /** Removes all selected rows from the table. */
  void slot_DeleteRows();

  /** Removes all rows from the table. */
  void slot_DeleteAllRows();

  /** Ok button press is handled here. */
  void slot_Ok();

  /** Close button press is handled here. */
  void slot_Close();

  /**
   * Header click is handled here. It sorts the clicked column in ascending
   * order.
   */
  void slot_HeaderClicked( int section );

signals:

  /** Emitted if the widget was closed. */
  void closed();

private:

  /** Flag to store modifications done on the logbook. */
  bool m_tableModified;

  /** Table widget with columns for the logbook entries. */
  QTableWidget* m_table;

  /** Delete button. */
  QPushButton* m_deleteButton;

  /** Delete all button. */
  QPushButton* m_deleteAllButton;

  /** ok button. */
  QPushButton* m_okButton;

  /** Logbook data as string list */
  QStringList m_logbook;

  /** Adds additional space in the list. */
  RowDelegate* rowDelegate;
};

#endif /* Logbook_h */
