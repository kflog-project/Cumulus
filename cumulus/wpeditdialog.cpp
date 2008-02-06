/***********************************************************************
**
**   wpeditdialog.cpp
**
**   This file is part of Cumulus
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

#include "wpeditdialog.h"
#include "wpeditdialogpagegeneral.h"
#include "wpeditdialogpageaero.h"
#include "mapcontents.h"
#include "mapmatrix.h"

WPEditDialog::WPEditDialog(QWidget *parent, const char *name, wayPoint * wp ):
  Q3TabDialog(parent,name, false, Qt::WStyle_StaysOnTop)
{
    if (wp==0) {
        this->setCaption(tr("New Waypoint"));
    } else {
        this->setCaption(tr("Edit Waypoint"));
    }
    _wp = wp;

    setOkButton();
    setCancelButton();

    WPEditDialogPageGeneral * pageGeneral=new WPEditDialogPageGeneral(this);
    addTab(pageGeneral, tr("&General"));

    WPEditDialogPageAero * pageAero=new WPEditDialogPageAero(this);
    addTab(pageAero, tr("&Aero"));

    comment=new QTextEdit(this);
    comment->setWordWrapMode(QTextOption::WordWrap);
    addTab(comment, tr("&Comments"));

    connect(this, SIGNAL(aboutToShow()),
            this, SLOT(slot_LoadCurrent()));
    connect(this, SIGNAL(load(wayPoint *)),
            pageGeneral, SLOT(slot_load(wayPoint *)));
    connect(this, SIGNAL(load(wayPoint *)),
            pageAero, SLOT(slot_load(wayPoint *)));
    connect(this, SIGNAL(save(wayPoint *)),
            pageGeneral, SLOT(slot_save(wayPoint *)));
    connect(this, SIGNAL(save(wayPoint *)),
            pageAero, SLOT(slot_save(wayPoint *)));

    showPage(comment);
    showPage(pageGeneral);
}


WPEditDialog::~WPEditDialog()
{}


/** This slot is called if the user presses the OK button. */
void WPEditDialog::slot_LoadCurrent()
{
    emit load(_wp);
    if ( _wp ) {
        comment->setText(_wp->comment);
    }
}


extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;

/** Called if OK button is pressed? */
void WPEditDialog::accept()
{
    if (_wp==0) {
        _wp=new wayPoint;
        _wp->comment=comment->text();
        emit save(_wp);
        _wp->projP = _globalMapMatrix->wgsToMap(_wp->origP);
        //qDebug("new waypoint %s", (const char *)_wp->name);
        _globalMapContents->getWaypointList()->append(_wp);
    } else {
        //qDebug("change waypoint %s", (const char *)_wp->name);
        _wp->projP = _globalMapMatrix->wgsToMap(_wp->origP);
        _wp->comment=comment->text();
        emit save(_wp);
    }

    emit save(_wp);
    Q3TabDialog::accept();
    emit wpListChanged(_wp);
}
