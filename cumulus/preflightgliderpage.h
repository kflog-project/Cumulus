/***********************************************************************
**
**   preflightgliderpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers, 2008-2011 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreFlightGliderPage
 *
 * \author André Somers, Axel Pauli
 *
 * \brief A widget for pre-flight glider settings.
 *
 * \date 2003-2011
 *
 * \version $Id$
 *
 */

#ifndef PREFLIGHT_GLIDER_PAGE_H
#define PREFLIGHT_GLIDER_PAGE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include "gliderlistwidget.h"

class PreFlightGliderPage : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( PreFlightGliderPage )

public:

  PreFlightGliderPage(QWidget *parent=0);
  ~PreFlightGliderPage();
  void save();

protected:

  void showEvent(QShowEvent *);

private:

  GliderListWidget *list;
  QLineEdit        *edtCoPilot;
  QSpinBox         *spinLoad;
  QSpinBox         *spinWater;
  Glider           *lastGlider;
  QLabel           *wingLoad;

  void getCurrent();

private slots:

  void slot_gliderChanged();
  void slot_gliderDeselected();
  void slot_updateWingLoad( int value );
};

#endif
