/***********************************************************************
**
**   taskeditor.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
**   Description: This class handles creation and modification of
**   flight tasks in a simple editor.
**
***********************************************************************/

#ifndef TaskEditor_H
#define TaskEditor_H

#include <QList>
#include <QString>
#include <QTreeWidget>
#include <QLineEdit>
#include <QDialog>
#include <QStringList>
#include <QComboBox>

#include "flighttask.h"
#include "waypoint.h"
#include "listviewfilter.h"
#include "wplistwidgetclass.h"

class TaskEditor : public QDialog
{
    Q_OBJECT

public:
    /** */
    TaskEditor( QWidget* parent, QStringList &taskNamesInUse,
              FlightTask* task=0 );

    /** */
    ~TaskEditor();

protected:
    /** */
    virtual void accept();
    virtual void reject();

signals:
    /** */
    void newTask( FlightTask* );

    /** */
    void editedTask( FlightTask* );

    /** */
    void done();

private slots:
    /** */
    void slotAddWaypoint();

    /** */
    void slotRemoveWaypoint();

    /** */
    void slotMoveWaypointUp();

    /** */
    void slotMoveWaypointDown();

    /** */
    void slotInvertWaypoints();

    /** Toggle between WP or AF list on user request */
    void slotToggleList(int);

private:

    /** describes the current edit state of object */
    enum EditState { create, edit };

    /** */
    void __showWPList();

    /** */
    void __showAFList();

    /** */
    void __showTask();

    /** creates a deep copy of the waypoint list */
    QList<wayPoint*> *copyWpList();

    /** list containing defined tasks */
    QTreeWidget* taskList;
    
    /** list with all defined task names */
    QStringList& taskNamesInUse;
    
    /** number lists, at the moment waypointlist and airfield list*/
#define NUM_LISTS 2

    /** selection lists with waypoints */
    WPListWidgetClass* waypointList[NUM_LISTS];
    
    /** name of current task */
    QLineEdit* taskName;
    
    /** name of current edited task */
    QString editedTaskName;
    
    /** */
    ListViewFilter* filter[NUM_LISTS];
    
    /** */
    QList<wayPoint*> *wpList;
    
    /** */
    QList<wayPoint*> taskWPList;
    
    /** */
    FlightTask* planTask;
    
    /** Flag for indication of edit state */
    enum  EditState editState;
    
    /** Flag for indication of last selected item */
    int lastSelectedItem;
    
    /** which list to show */
    QComboBox* listSelectCB;
    
    /** the text for the combo box */
    QString listSelectText[NUM_LISTS];
};

#endif // TaskEditor_H
