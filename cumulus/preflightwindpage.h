/***********************************************************************
**
**   preflightwindpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class PreFlightWindPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for pre-flight wind settings.
 *
 * \date 2014-2021
 *
 * \version 1.2
 *
 */

#ifndef PRE_FLIGHT_WIND_PAGE_H
#define PRE_FLIGHT_WIND_PAGE_H

#include <QList>
#include <QTreeWidget>
#include <QWidget>
#include <QCheckBox>

class NumberEditor;

class PreFlightWindPage : public QWidget
{
  Q_OBJECT

 private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( PreFlightWindPage )

 public:

  PreFlightWindPage( QWidget* parent );

  virtual ~PreFlightWindPage();

 protected:

  void showEvent( QShowEvent* event );

  virtual void closeEvent( QCloseEvent* event );

 private:

 signals:

  /**
  * Emitted, if the enable state of the manual wind has been changed.
  * If the enable state is true and wind parameters have been changed,
  * the signal is emitted too.
  */
  void manualWindStateChange( bool newEnableState );

  /**
  * Emitted, if the enable state of the external wind has been changed.
  */
  void externalWindRequired( bool newState );

  /**
  * Emitted, if the widget is closed.
  */
  void closingWidget();

 private slots:

 /**
 * Called, if the help button is clicked.
 */
 void slotHelp();

 /**
  * This method fills the wind statistics list.
  */
 void slotLoadWindStatistics();

  /**
  * Called, if the state of the manually m_windCheckBox is changed.
  */
  void slotManualWindCbStateChanged( int state );

  /**
  * Called, if the state of the m_windCheckBox is changed.
  */
  void slotExternalWindCbStateChanged( int state );

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

 private:

  /** check box for manually wind activation. */
  QCheckBox* m_windCheckBox;

  /** editor box for wind direction entry*/
  NumberEditor* m_windDirection;

  /** editor box for wind speed entry */
  NumberEditor* m_windSpeed;

  /** check box for external wind usage. */
  QCheckBox* m_useExternalWind;

  /** editor box for setup start time of wind calculation in straight flight. */
  NumberEditor* m_startWindCalculation;

  /** widget with wind statistics */
  QTreeWidget* m_windListStatistics;

  /** Timer for reload wind statistics. */
  QTimer* m_reloadTimer;
};

#endif
