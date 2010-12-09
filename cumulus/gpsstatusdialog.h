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
  * \class GpsStatusDialog
  *
  * \author André Somers, Axel Pauli
  *
  * \brief Widget displaying GPS satellites and NMEA raw data stream.
  *
  * This dialog provides information about the current GPS status, including
  * satellites tracked, elevation, azimuth, signal strengths and the NMEA
  * data stream. NMEA data stream can be save into a file on user request.
  *
  * \date 2003-2010
  */

#ifndef GPS_STATUS_DIALOG_H
#define GPS_STATUS_DIALOG_H

#include <QWidget>
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

class GpsElevationAzimuthDisplay;
class GpsSnrDisplay;

class GpsStatusDialog : public QWidget
{
  Q_OBJECT

private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( GpsStatusDialog )

public:

  /**
   * Constructor
   * @param parent Pointer to parent widget
   */
  GpsStatusDialog( QWidget *parent );

  /**
   * Destructor
   */
  virtual ~GpsStatusDialog();

  /**
   * @return Returns the current number of instances.
   */
  static int getNrOfInstances()
  {
    return noOfInstances;
  };

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

private slots:

  /**
   * Called if the start/stop button is pressed to start or stop NMEA display.
   */
  void slot_ToggleStartStop();

  /**
   * Called if the save button is pressed to save NMEA display content into a file.
   */
  void slot_SaveNmeaData();

  /**
   * Called if space button is pressed to toggle the window size.
   */
  void slot_ToggleWindowSize();

protected:

    GpsElevationAzimuthDisplay *elevAziDisplay;
    GpsSnrDisplay              *snrDisplay;
    QTextEdit                  *nmeaBox;
    QPushButton                *startStop;
    QPushButton                *save;

    bool                        showNmeaData;

    /** contains the current number of class instances */
    static int noOfInstances;
};


class GpsElevationAzimuthDisplay: public QFrame
{
  Q_OBJECT

private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( GpsElevationAzimuthDisplay )

public:

  /**
   * Constructor
   * @param parent Pointer to parent widget
   */
  GpsElevationAzimuthDisplay(QWidget * parent);
  virtual ~GpsElevationAzimuthDisplay();

  void setSatInfo(QList<SIVInfo>&);

protected:

  virtual void resizeEvent ( QResizeEvent *event );
  virtual void paintEvent ( QPaintEvent *event );
  void drawSat(QPainter *, const SIVInfo &);

private:

  QPixmap *background;
  QPoint center;
  int width;
  int height;
  QList<SIVInfo> sats;
};


class GpsSnrDisplay: public QFrame
{
  Q_OBJECT

private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( GpsSnrDisplay )

public:

  /**
   * Constructor
   * @param parent Pointer to parent widget
   */
  GpsSnrDisplay(QWidget * parent);
  virtual ~GpsSnrDisplay();

  void setSatInfo(QList<SIVInfo>&);

protected:

  virtual void resizeEvent ( QResizeEvent *event );
  virtual void paintEvent ( QPaintEvent *event );
  void drawSat(QPainter *, QPainter *, int, int, const SIVInfo &);

private:

  QPixmap *background;
  QPixmap *canvas;
  QBitmap *mask;
  QPoint center;
  int width;
  int height;
  QList<SIVInfo> sats;
};

#endif
