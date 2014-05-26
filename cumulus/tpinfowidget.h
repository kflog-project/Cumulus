/***********************************************************************
 **
 **   tpinfowidget.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2007-2013 by Axel Pauli, kflog.cumulus@gmail.com
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ************************************************************************/

/**
 * \class TPInfoWidget
 *
 * \author Axel Pauli
 *
 * \brief This widget shows the details of a task point.
 *
 * This class provides a widget to display information like task point switch,
 * distance to next, duration to next, ETA, when a task point has been reached.
 * The widget will be closed automatically after a configurable time period,
 * if user do nothing. The user can stop the automatic close.
 *
 * \date 2007-2013
 *
 * \version $Id$
 */

#ifndef TP_INFO_WIDGET_H
#define TP_INFO_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QTextEdit>
#include <QPushButton>

#include "distance.h"
#include "speed.h"
#include "waypoint.h"

class TPInfoWidget : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( TPInfoWidget )

 public:

  TPInfoWidget( QWidget *parent=0 );

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
  void prepareSwitchText( const int currentTpIndex,
                          const double dist2Next );
  /**
   * This method fills the widget with the arrival info. The info is
   * taken from the passed waypoint.
   *
   * Waypoint: pointer to waypoint object
   *
   */
  void prepareArrivalInfoText( Waypoint *wp );

 signals:

    /**
     * Send if the widget is closed
     */
  void closed();

 public slots:

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

 private slots:

  /**
   * This slot get called on the timer timeout.
   */
  void slot_Timeout();

  /**
   * Called, if the cursor position in the text is changed to clear the text
   * selection.
   */
  void slotCursorChanged();

 protected:

    /**
     * Called, if the widget will be shown.
     */
    void showEvent(QShowEvent *);

 private:

  /**
   * This is the widget that actually contains the info we
   * want to display.
   */
  QTextEdit* m_text;

  /**  */
  QPushButton* m_cmdClose;

  /**  */
  QPushButton* m_cmdKeep;

  /**
   * Reference to a timer. When this timer fires, the widget is
   * automatically closed.
   */
  QTimer* m_timer;

  /** TimerCount */
  int m_timerCount;

  /** Contains a reference to the m_parent of the application. */
  QWidget* m_parent;
};

#endif
