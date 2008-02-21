/***********************************************************************
**
**   degreespinbox.h
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

#ifndef DEGREESPINBOX_H
#define DEGREESPINBOX_H

#include <QWidget>
#include <QSpinBox>

/**
 * A spinbox that can be used to enter a runway heading
 * @author André Somers
 */
class DegreeSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    /**
     * Constructor
     */
    DegreeSpinBox(QWidget *parent=0, const char *name=0);

    /**
     * Destructor
     */
    ~DegreeSpinBox();

protected:
    /**
     * @returns the string representing @arg value
     */
    QString mapValueToText(int value);

    /**
     * @returns the value representing the text.
     * @param ok. If set, the function sets the value of ok
     *            to true if the conversion succeeded, and to
     *            false if not.
     */
    int mapTextToValue(bool * ok);

};

#endif
