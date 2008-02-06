/***********************************************************************
 **
 **   tpinfowidget.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2007, 2008 Axel Pauli, axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ************************************************************************
 **
 **   This class is part of Cumulus. It provides a widget to display
 **   information like task point switch, distance to next, duration to
 **   next, ETA, when a task point has been reached. Widget will be
 **   closed automatically after a configurable time period, if user do
 **   nothing. The user can stop the automatic close.
 **   
 ************************************************************************/

#ifndef TPINFOWIDGET_H
#define TPINFOWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <Q3Accel>

#include "distance.h"
#include "speed.h"
#include "waypoint.h"

class TPInfoWidget : public QWidget
{
  Q_OBJECT

public:

  TPInfoWidget( QWidget *parent=0, const char *name=0 );

  virtual ~TPInfoWidget();

  /**
   * Shows task point info to the user. Close timer is activated per
   * default. Use false to deactivate it.
   */
  void showTP( bool automaticClose=true );

  /**
   * This method fills the widget with the taskpoint switch
   * info. The info is taken from the current active fligh task.
   *
   * currentTpIndex: index of current taskpoint in flight task
   * dist2Next: distance to next taskpoint in km
   *
   */
  void prepareSwitchText( const uint currentTpIndex,
                          const double dist2Next );
  /**
   * This method fills the widget with the arrival info. The info is
   * taken from the passed waypoint.
   *
   * wayPoint: pointer to waypoint object
   *
   */
  void prepareArrivalInfoText( wayPoint *wp );

signals: // Signals

    /**
     * Send if the widget is closed
     */
  void close();

public slots: // Public slots

  /**
   * This slot is called by the KeepOpen button to keep the dialog
   * open. :-) If the timer expires, the widget will be closed.
   */
  void slot_KeepOpen();

  /**
   * This slot is called, if the user presses the close button.
   * Widget will be closed and destroyed
   */
  void slot_Close();

private slots: // Private slots

  /**
   * This slot get called on the timer timeout.
   */
  void slot_Timeout();

protected:

    /**
     * Called, if the widget will be shown.
     */
    void showEvent(QShowEvent *);

private: // Private objects

  /**
   * This is the object that actually contains the info we
   * want to display.
   */
  QTextEdit *text;

  /** Accelerator for closing widget */
  Q3Accel *accClose;

  /**  */
  QPushButton *cmdClose;

  /**  */
  QPushButton *cmdKeep;

  /**
   * Reference to a timer. When this timer fires, the widget is
   * automatically closed.
   */
  QTimer *timer;

  /** TimerCount */
  int _timerCount;


  /** Contains a reference to the parent of the application. */
  QWidget     *parent;
  QHBoxLayout *buttonrow;

};

#endif
