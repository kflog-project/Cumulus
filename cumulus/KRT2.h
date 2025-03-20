/***********************************************************************
 **
 **   KRT2.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2025 by Axel Pauli (kflog.cumulus@gmail.com)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 ***********************************************************************/

#pragma once

#include <QtCore>
#include <QTcpSocket>

class KRT2Thread;

class KRT2 : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( KRT2 )

 public:

  static constexpr uint16_t CMD_TIMEOUT = 250; //!< Command timeout 250ms
  static constexpr unsigned NR_RETRIES = 3; //!< Number of tries to send a command.

  KRT2( QObject *parent, QString ip, QString port );

  virtual ~KRT2();

  /**
   * Sets the active frequency on the radio.
   */
  bool setActiveFrequency( const float frequency,
                           const QString& name );
  /**
   * Sets the standby frequency on the radio.
   */
  bool setStandbyFrequency( const float frequency,
                            const QString& name );

  /**
   * Sends the command to the KRT2 device.
   */
  bool sendFrequency( const uint8_t cmd,
                      const float frequency,
                      const QString& name );
  /**
   * Splits the frequency in the desired KRT2 order.
   *
   * @param fin Frequency in MHz
   * @param mhz Megahertz part of the frequency
   * @param channel KRT Channel part of the frequency
   * @return
   */
  bool splitFreqency( const float fin, uint8_t& mhz, uint8_t& channel );

 private slots:

  /**
   * Handle KRT message.
   */
  void handleKRTMessage( QByteArray& data );

  private:

  // WiFi3 data
  QString m_ip;
  QString m_port;
  bool m_active;

  // RX handler thread of KRT2
  KRT2Thread* krt2;
};

class KRT2Thread : public QThread
{
  Q_OBJECT

 public:

  KRT2Thread( QObject *parent, QString ip, QString port );

  virtual ~KRT2Thread();

  /**
   * Send the passed data to the KRT2 device.
   *
   * @param data
   * @return true in case of success otherwise false.
   */
  bool send( QByteArray& data );

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();

 private:

  /**
   * Handle data coming in from the KRT2 device.
   *
   * returns true, if all necessary data are received, otherwise false.
   */
  bool handleSTX();

 public slots:

  /**
   * Wrapper to connect by slot.
   */
  void slotConnect()
  {
    connect();
  }

  void slotSend( QByteArray data )
  {
    send( data );
  }

 private slots:

  /**
   * Called by the socket notifier, when data are received from the KRT2 device.
   */
  void handleRxData( int type );

  /**
   * Called by the socket notifier, when an exception is received.
   */
  void handleException( int type );

  /**
   * Try to establish the connection to the KRT2 device.
   */
  bool connect();

  /**
   * Retry connection after timeout.
   */
  void slotRetry()
  {
    connect();
  }

  /**
   * Called when the thread has got the signal finished.
   */
  void slotFinished();

 signals:

  void forwardDeviceError( QString error );

 private:

  QString m_ip;
  QString m_port;
  bool m_connected;

  QQueue<QByteArray> m_txQueue;

  QTcpSocket *m_socket;
  QSocketNotifier* m_snRX;
  QSocketNotifier* m_snExcept;
  QByteArray rxBuffer;
  QMutex mutex;
};

