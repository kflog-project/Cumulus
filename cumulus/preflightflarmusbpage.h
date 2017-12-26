/***********************************************************************
**
**   preflightflarmusbpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2017 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class PreflightFlarmUsbPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for Flarm flights transfer to an USB stick.
 *
 * A widget for Flarm flights transfer to an USB stick.
 *
 * \date 2017
 *
 * \version 1.0
 */

#ifndef PREFLIGHT_FLARM_USB_PAGE_H
#define PREFLIGHT_FLARM_USB_PAGE_H

#include <QWidget>

#include "flarm.h"

class QLabel;
class QProgressBar;
class QStringList;
class QTimer;

class PreFlightFlarmUsbPage : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightFlarmUsbPage )

 public:

  PreFlightFlarmUsbPage( QWidget *parent=0 );

  virtual ~PreFlightFlarmUsbPage();

 private slots:

  /** Requests the Flarm data from the device.*/
  void slotRequestIgcReadout();

  /** Called by Flarm with the IGC resonse. */
  void slotIgcResponse( QStringList& info );

  /** Called to update error info. */
  void slotUpdateErrors( const Flarm::FlarmError& info );

  /** Called to receive and report an error info. */
  void slotReportError( QStringList& info );

  /** Called to receive progress info. */
  void slotProgressInfo( QStringList& info );

  /** Called if the connection timer has expired. */
  void slotTimeout();

  /** Called if the widget is closed. */
  void slotClose();

  signals:

  /**
   * Emitted, if the widget is closed.
   */
  void closingWidget();

 private:

  QLabel*       m_info;
  QProgressBar* m_pb;
  QTimer*       m_timer;
};

#endif
