/***********************************************************************
**
**   gpsstatusdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef GPSSTATUSDIALOG_H
#define GPSSTATUSDIALOG_H

#include <QDialog>
#include <QFrame>
#include <QList>
#include <QPoint>
#include <QTextEdit>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QKeyEvent>

#include "gpsnmea.h"

class GPSElevationAzimuthDisplay;
class GPSSnrDisplay;

/**
  *@short dialog displaying the GPS status
  *This dialog provides info on the current GPS status, including statalites
  *tracked, elevation, azimuth and signal strengths.
  *@author André Somers
  */
class GpsStatusDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @argument parent Pointer to parentwidget
     */
    GpsStatusDialog(QWidget * parent);

    /**
     * Destructor
     */
    ~GpsStatusDialog();

    void keyPressEvent(QKeyEvent *);
public slots:

    /**
     * Called if a new sentence arrives from the GPS
     */
    void slot_Sentence(const QString& sentence);

    /**
     * Called if new info on the satelites in view is available
     */
    void slot_SIV();


protected slots:
    /**
     * Called if dialog is accepted (OK button is clicked)
     */
    virtual void accept();

    /**
     * Called if dialog is rejected (X button is clicked)
     */
    virtual void reject();
    //virtual void hide();

private slots:

protected:
    GPSElevationAzimuthDisplay * elevAziDisplay;
    GPSSnrDisplay * snrDisplay;
    QTextEdit * nmeaBox;
};


class GPSElevationAzimuthDisplay: public QFrame
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @argument parent Pointer to parentwidget
     */
    GPSElevationAzimuthDisplay(QWidget * parent);
    ~GPSElevationAzimuthDisplay();

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
     * @argument parent Pointer to parentwidget
     */
    GPSSnrDisplay(QWidget * parent);
    ~GPSSnrDisplay();

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
