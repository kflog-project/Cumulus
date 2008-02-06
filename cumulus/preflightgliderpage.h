/***********************************************************************
**
**   preflightgliderpage.h
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

#ifndef PREFLIGHTGLIDERPAGE_H
#define PREFLIGHTGLIDERPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>

#include "gliderlist.h"

/**
 * This widget represents the glider-selection page for the PreFlight dialog.
 * 
 * @author André Somers
 */
class PreFlightGliderPage : public QWidget
  {
    Q_OBJECT
  public:
    PreFlightGliderPage(QWidget *parent=0, const char *name=0);
    ~PreFlightGliderPage();
    void save();

  protected:
    void showEvent(QShowEvent *);

  private:
    GliderList * list;
    QLineEdit * edtCoPilot;
    QSpinBox * spinLoad;
    QSpinBox * spinWater;
    Glider * lastGlider;

    void getCurrent();

  private slots:
    void slot_gliderChanged();
  };

#endif
