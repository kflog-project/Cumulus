/***********************************************************************
**
**   degreespinbox.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QString>
#include "degreespinbox.h"

DegreeSpinBox::DegreeSpinBox(QWidget *parent, const char *name ) : QSpinBox(parent,name)
{
    this->setMinValue(-1);
    this->setMaxValue(36);
    this->setWrapping(true);
    this->setLineStep(1);
    this->setValue(-1); //default=Unknown
}


DegreeSpinBox::~DegreeSpinBox()
{}


QString DegreeSpinBox::mapValueToText(int value)
{
    if (value==-1)
        return QString(tr("Unknown"));

    return QString("%1").arg(value*10);
}


int DegreeSpinBox::mapTextToValue(bool */* ok*/)
{
    if (text()==tr("Unknown"))
        return -1;
    return int(text().toInt()/10);
}

