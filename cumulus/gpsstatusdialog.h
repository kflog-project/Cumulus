/***********************************************************************
**
**   gpsstatusdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2003 by André Somers
**                  2008-2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
  * \author André Somers, Axel Pauli
  *
  * \brief dialog displaying the GPS status
  *
  * This dialog provides info on the current GPS status, including satellites
  * tracked, elevation, azimuth and signal strengths.
  *
  */

#ifndef GPS_STATUS_DIALOG_H
#define GPS_STATUS_DIALOG_H

#include <QDialog>
#include <QFrame>
#include <QList>
#include <QPoint>
#include <QTextEdit>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QKeyEvent>
#include <QPushButton>

#include "gpsnmea.h"

class GPSElevationAzimuthDisplay;
class GPSSnrDisplay;

class GpsStatusDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * Constructor
   * @argument parent Pointer to parent widget
   */
  GpsStatusDialog( QWidget *parent );

  /**
   * Destructor
   */
  virtual ~GpsStatusDialog();

  void keyPressEvent( QKeyEvent *event );

public slots:

  /**
   * Called if a new sentence arrives from the GPS
   */
  void slot_Sentence(const QString& sentence);

  /**
   * Called if new info on the satellites in view is available
   */
  void slot_SIV( QList<SIVInfo>& siv );

protected slots:
  /**
   * Called if dialog is accepted (OK button is clicked)
   */
  virtual void accept();

  /**
   * Called if dialog is rejected (X button is clicked)
   */
  virtual void reject();

private slots:

  /**
   * Called if the start/stop button is pressed to start or stop NMEA display.
   */
  void slot_toggleStartStop();

  /**
   * Called if the save button is pressed to save NMEA display content into a file.
   */
  void slot_SaveNmeaData();

protected:

    GPSElevationAzimuthDisplay *elevAziDisplay;
    GPSSnrDisplay              *snrDisplay;
    QTextEdit                  *nmeaBox;
    QPushButton                *startStop;
    QPushButton                *save;

    bool                        showNmeaData;
};


class GPSElevationAzimuthDisplay: public QFrame
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @argument parent Pointer to parent widget
     */
    GPSElevationAzimuthDisplay(QWidget * parent);
    virtual ~GPSElevationAzimuthDisplay();

    void setSatInfo(QList<SIVInfo>&);

protected:
    virtual void resizeEvent ( QResizeEvent * );
    virtual void paintEvent ( QPaintEvent * );
    void drawSat(QPainter *, const SIVInfo &);

private:
    QPixmap * background;
    QPoint center;
    int width;
    int height;
    QList<SIVInfo> sats;
};


class GPSSnrDisplay: public QFrame
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @argument parent Pointer to parent widget
     */
    GPSSnrDisplay(QWidget * parent);
    virtual ~GPSSnrDisplay();

    void setSatInfo(QList<SIVInfo>&);

protected:
    virtual void resizeEvent ( QResizeEvent * );
    virtual void paintEvent ( QPaintEvent * );
    void drawSat(QPainter *, QPainter *, int, int, const SIVInfo &);

private:
    QPixmap * background;
    QPixmap * canvas;
    QBitmap * mask;
    QPoint center;
    int width;
    int height;
    QList<SIVInfo> sats;
};


#endif
