/***********************************************************************
**
**   flarmlogbook.h
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
 * \class FlarmLogbook
 *
 * \author Axel Pauli
 *
 * \brief Flarm flight logbook list display.
 *
 * This widget can list all entries of a Flarm flight logbook. The user can
 * select one or more flight entries to be downloaded. The content of the
 * downloded IGC files is stored in the user's data directory.
 *
 * \date 2012
 *
 * \version $Id$
 */

#ifndef FlarmLogbook_h
#define FlarmLogbook_h

#include <QWidget>
#include <QMessageBox>

class QPushButton;
class QStringList;
class QTableWidget;
class RowDelegate;

class FlarmLogbook : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmLogbook )

public:

  FlarmLogbook( QWidget *parent=0 );

  virtual ~FlarmLogbook();

private:

  /** Sets the table header items of our own. */
  void setTableHeader();

protected:

  virtual void showEvent( QShowEvent *event );

public slots:

  /** Loads the logbook data into the table. */
  void slot_LogbookData( QString& list );

  /** Called if configuration updates from Flarm device are available. */
  void slot_UpdateConfiguration( QStringList& info );

  /**
   * This slot is called, when a new Flarm flight download info was received.
   */
  void slot_FlarmFlightDownloadInfo(const QString& info);

  /**
   * This slot is called, when a new Flarm flight download progress was received.
   */
  void slot_FlarmFlightDownloadProgress(const int idx, const int progress);

private slots:

  /** Removes all selected rows from the table. */
  void slot_ReadFlights();

  /** Removes all rows from the table. */
  void slot_DownloadFlights();

  /** Called if the connection timer has expired. */
  void slot_Timeout();

  /** Close button press is handled here. */
  void slot_Close();

signals:

  /** Emitted if the widget was closed. */
  void closed();

private:

  /** Shows a popup message box to the user. */
  void messageBox( QMessageBox::Icon icon, QString message, QString title="" );

  /** Toggles operation of buttons. */
  void enableButtons( const bool toggle );

  /** Timer for connection supervision. */
  QTimer* m_timer;

  /** Table widget with columns for the logbook entries. */
  QTableWidget* m_table;

  /** Read flights button. */
  QPushButton* m_readButton;

  /** Download flights button. */
  QPushButton* m_downloadButton;

  /** Close window button. */
  QPushButton* m_closeButton;

  /** Logbook data as string list */
  QStringList m_logbook;

  /** Adds additional space in the list. */
  RowDelegate* rowDelegate;
};

#endif /* FlarmLogbook_h */
