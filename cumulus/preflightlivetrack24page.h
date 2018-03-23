/***********************************************************************
**
**   preflightlivetrack24page.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-2018 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
 * \date 2013-2018
 *
 * \version 1.2
 *
 */

#ifndef PREFLIGHT_LIVE_TRACK24_H
#define PREFLIGHT_LIVE_TRACK24_H

#include <QByteArray>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;

#include "httpclient.h"

class NumberEditor;
class SkyLinesTracker;

class PreFlightLiveTrack24Page : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightLiveTrack24Page )

 public:

  PreFlightLiveTrack24Page(QWidget *parent=0);

  virtual ~PreFlightLiveTrack24Page();

 private:

  void load();

  void save();

  /**
   * Shows the login test result in a message box.
   */
  void showLoginTestResult( QString &msg );

 private slots:

  /**
   * Called, if the help button is clicked.
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

  /**
   * Called to show the live track session data.
   */
  void showSessionData();

  /**
   * Called to test the login data of the user.
   */
  void slotLoginTest();

  /** Called, if the HTTP request is finished. */
  void slotHttpResponse( QString &url, QNetworkReply::NetworkError code );

  /** Called, if the server ulr is changed. */
  void slotCurrentIndexChanged( int index );

  /** Called, if the SkyLine server is not reachable. */
  void slotSkyLinesConnectionFailed();

  /** Called to report the ping result. */
  void slotSkyLinesPingResult( quint32 result );

 signals:

  /**
   * Emitted, if on/off state has been changed.
   */
  void onOffStateChanged( bool newSate );

  /**
   * Emitted, if the live tracking server has changed.
   */
  void liveTrackingServerChanged();

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
  QPushButton*  m_loginTestButton;
  QLabel*       m_sessionDisplay;
  QTimer*       m_updateTimer;

  // Http client for the login test. */
  HttpClient* m_httpClient;

  /** Result buffer for HTTP requests. */
  QByteArray m_httpResultBuffer;

  /** SkyLines tracker for login test. */
  SkyLinesTracker* m_slt;
};

#endif
