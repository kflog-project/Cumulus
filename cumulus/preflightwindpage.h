/***********************************************************************
**
**   preflightwindpage.h
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
 * \class PreFlightWindPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for pre-flight wind settings.
 *
 * \date 2014
 *
 * \version $Id$
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

  void showEvent(QShowEvent *);

 private:

 signals:

   /**
    * Emitted, if the enable state of the manual wind has been changed.
    * If the enable state is true and wind parameters have been changed,
    * the signal is emitted too.
    */
   void manualWindStateChange( bool newEnableState );

   /**
    * Emitted, if the widget is closed.
    */
   void closingWidget();

 private slots:

  /**
  * Called, if the state of the m_windCheckBox is changed.
  */
  void slotWindCbStateChanged( int state );

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

  /** widget with wind statistics */
  QTreeWidget* m_windListStatistics;
};

#endif
