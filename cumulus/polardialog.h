/***********************************************************************
**
**   polardialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Eggert Ehmke, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef POLARDIALOG_H
#define POLARDIALOG_H

#include <QDialog>
#include <QShortcut>

#include "speed.h"
#include "polar.h"

/**
 * @author Eggert Ehmke
 */
class PolarDialog : public QDialog
{
    Q_OBJECT
public:
    PolarDialog(const Polar*, QWidget*);
    virtual ~PolarDialog();

public slots:
    void slot_keyup();
    void slot_keydown();
    void slot_shiftkeyup();
    void slot_shiftkeydown();
    void slot_keyleft();
    void slot_keyright();
    void slot_keyhome();

protected:
    virtual void paintEvent (QPaintEvent*);
    virtual void mousePressEvent( QMouseEvent* );

private:
    Polar* _polar;
    Speed wind, lift, mc;
};

#endif
