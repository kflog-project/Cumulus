/***********************************************************************
**
**   wpeditdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WPEDIT_DIALOG_H
#define WPEDIT_DIALOG_H

#include <QWidget>
#include <QDialog>
#include <QTextEdit>
#include "waypoint.h"

/**
 * The WpEditDialog allows the creation of a new waypoint or the modification
 * of an existing waypoint.
 * @author André Somers
 */

class WpEditDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent The parent widget
     * @param name A name for this QObject
     * @param wp A waypoint to show the dialog for
     */
    WpEditDialog(QWidget *parent=0, wayPoint * wp=0);

    virtual ~WpEditDialog();

private:
    /**
     * This method is called just before showing the dialog,
     * and loads the current waypoint data.
     */
    void loadWaypointData();

    /**
     * This method checks, if all mandatory waypoint data have been defined.
     * Returns true on ok otherwise false.
     */
    bool checkWaypointData( wayPoint& wp );

    /**
     * This method checks, if the passed waypoint name is already to find
     * in the global waypoint list. If yes the user is informed with a
     * message box about this fact.
     * Returns true if yes otherwise false.
     */
    bool isWaypointNameInList( QString& wpName );

    /**
     * This method checks, if the passed waypoint name is multiple to find
     * in the global waypoint list. If yes the user is informed with a
     * message box about this fact.
     * Returns true if yes otherwise false.
     */
    bool countWaypointNameInList( QString& wpName );
  
private slots: // Private slots
    /**
     * Called if OK button is pressed
     */
    void accept();

signals: // Signals
    /**
     * Signal emitted to indicate the settings should be saved
     * to the configuration file
     */
    void save(wayPoint *);

    /**
     * Emitted to indicate that the settings should be
     * re-) loaded from the configuration file.
     */
    void load(wayPoint *);

    /**
     * This signal is emitted after a edited waypoint has been saved.
     * It is used to re-fill the waypoint list.
     */
    void wpListChanged(wayPoint &);

private:

    wayPoint * _wp;
    QString oldName; // old name of waypoint before modification
    QTextEdit * comment;
};

#endif
