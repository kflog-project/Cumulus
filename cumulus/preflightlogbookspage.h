/***********************************************************************
**
**   preflightlogbookspage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-2022 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class PreFlightLogBooksPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for pre-flight logbooks management.
 *
 * \date 2013-2022
 *
 * \version 1.3
 *
 */

#pragma once

#include <QWidget>

class QPushButton;

class PreFlightLogBooksPage : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightLogBooksPage )

 public:

  PreFlightLogBooksPage(QWidget *parent=0);

  virtual ~PreFlightLogBooksPage();

 private slots:

  /**
   * Called, if the help button is clicked.
   */
  void slotHelp();

  /**
   * Called to open the flight logbook dialog.
   */
  void slotOpenLogbook();

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

#ifdef FLARM

  /**
   * Called to open the Flarm download flight dialog.
   */
  void slotOpenFlarmFlights();

  /**
   * Called on PowerFlarm to transfer the flights to the USB stick.
   */
  void slotTransferFlights2USB();

#endif

 signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

  /**
   * Emitted, if the widget is closed.
   */
  void closingWidget();
};
