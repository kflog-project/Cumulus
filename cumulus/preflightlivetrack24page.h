/***********************************************************************
**
**   preflightlivetrack24page.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreFlightLiveTrack24Page
 *
 * \author Axel Pauli
 *
 * \brief A widget for pre-flight live tracking settings.
 *
 * A widget for pre-flight live tracking settings. Live tracking data are
 * submitted to the LiveTracking24 server during flight via an active Internet
 * connection, if the user has enabled that.
 *
 * \date 2013
 *
 * \version $Id$
 *
 */

#ifndef PREFLIGHT_LIVE_TRACK24_H
#define PREFLIGHT_LIVE_TRACK24_H

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;

class NumberEditor;

class PreFlightLiveTrack24Page : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightLiveTrack24Page )

 public:

  PreFlightLiveTrack24Page(QWidget *parent=0);

  virtual ~PreFlightLiveTrack24Page();

  void load();

  void save();

 private slots:

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

 signals:

  /**
   * Emitted, if on/off state has been changed.
   */
  void onOffStateChanged( bool newSate );

  /**
   * Emitted, if the widget is closed.
   */
  void closingWidget();

 private:

  QCheckBox*    m_liveTrackEnabled;
  NumberEditor* m_trackingIntervalMin;
  NumberEditor* m_trackingIntervalSec;
  QComboBox*    m_airplaneType;
  QComboBox*    m_server;
  QLineEdit*    m_username;
  QLineEdit*    m_password;
};

#endif
