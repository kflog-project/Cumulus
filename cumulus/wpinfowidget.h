/***********************************************************************
**
**   wpinfowidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Andr� Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WPINFOWIDGET_H
#define WPINFOWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QTextEdit>
#include <QPushButton>
#include <QMainWindow>
#include <QShortcut>

#include "waypoint.h"
#include "tpinfowidget.h"

/**
 * This widget provides a view to the details of a waypoint.
 * @author Andrè Somers
 */

class CumulusApp;

class WPInfoWidget : public QWidget
  {
    Q_OBJECT

  public:

    WPInfoWidget( CumulusApp *parent=0 );

    virtual ~WPInfoWidget();

    /**
     * This method is called by CumulusApp to set the view to which
     * there must be returned and the waypoint to view.
     */
    bool showWP(int lastView, const wayPoint * wp=0);

  public slots: // Public slots

    /**
     * No descriptions
     */
    void slot_SwitchBack();

    /**
     * This slot is called by the KeepOpen button to...
     * yes... keep the dialog open. :-)
     * Any timer that would close the dialog will be stopped.
     */
    void slot_KeepOpen();


  signals: // Signals

    /**
     * Emitted if a waypoint has been added to the list.
     */
    void waypointAdded(wayPoint * wp);

    /**
     * Emitted if a waypoint has been selected.
     */
    void waypointSelected(wayPoint*, bool);

    /**
     * Emitted if a new home position is selected
     */
    void newHomePosition(const QPoint*);

  protected:

    /**
     * Called, if the widget will be shown.
     */
    void showEvent(QShowEvent *);

  private: // Private methods

    /**
     * Set the visibility of the widget buttons
     */
    void setButtonsVisibility();

    /**
     * get back the current state of cumlus. In flight
     * true, otherwise false
     */
    bool inFlight();

    /**
     * This method actually fills the widget with the info.
     */
    void writeText();


  private slots: // Private slots

    /**
     * This slot get called on the timer timeout.
     */
    void slot_timeout();

    /**
     * This slot is called if the Add as Waypoint button is clicked.
     */
    void slot_addAsWaypoint();

    /**
     * This slot is called if the Home button is clicked.
     */
    void slot_setAsHome();

    /**
     * This slot is called if the Select Waypoint button is clicked.
     */
    void slot_selectWaypoint();

    /**
     * This slot is called if the Unselect Waypoint button is clicked.
     */
    void slot_unselectWaypoint();

    /**
     * This slot is called if the arrival button is clicked.
     */
    void slot_arrival();

    /**
     * This slot is called if the arrival widget is closed
     */
    void slot_arrivalClose();

  private: // Private attributes

    /**
     * These are the widget object that actually contains the info we
     * want to represent.
     */
    QTextEdit * text;

    QHBoxLayout * buttonrow1;
    QHBoxLayout * buttonrow2;

    QPushButton * cmdClose;
    QPushButton * cmdKeep;
    QPushButton * cmdAddWaypoint;
    QPushButton * cmdSetHome;
    QPushButton * cmdSelectWaypoint;
    QPushButton * cmdUnselectWaypoint;
    QPushButton * cmdArrival;

    QShortcut* scClose;
    QShortcut* scSelect;
    
    /** arrival info widget */
    TPInfoWidget * arrivalInfo;

    /**
     * Reference to a timer. When this timer fires, the view
     * is returned to the previous one. This allow for a brief
     * display of data (i.e.: what is the frequency of my target
     * field?) after wich the view is automatically returned to
     * the last one.
     */
    QTimer * timer;

    /** TimerCount */
    int _timerCount;

    /** Reference to the waypoint who's details to be displayed. */
    wayPoint * _wp;

    /** last selected waypoint  */
    wayPoint    myWp;

    /** contains the ID of the last view (the view that called this one) */
    int _lastView;

    /** contains a ref to the parent, the application */
    CumulusApp *cuApp;
  };

#endif
