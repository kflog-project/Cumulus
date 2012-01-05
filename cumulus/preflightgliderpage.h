/***********************************************************************
**
**   preflightgliderpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers, 2008-2012 Axel Pauli
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
 * \brief A widget for the pre-flight glider settings.
 *
 * \date 2003-2012
 *
 * \version $Id$
 *
 */

#ifndef PREFLIGHT_GLIDER_PAGE_H
#define PREFLIGHT_GLIDER_PAGE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

#include "gliderlistwidget.h"

class HSpinBox;

class PreFlightGliderPage : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( PreFlightGliderPage )

public:

  PreFlightGliderPage( QWidget *parent=0 );

  ~PreFlightGliderPage();

  /**
   * Saves the current settings permanently.
   */
  void save();

protected:

  void showEvent(QShowEvent *event);

private:

  /**
   * Selects the current activated glider, if a selection exists.
   */
  void getCurrent();

  /**
   * Updates the wingload label, if it is called.
   */
  void updateWingLoad();

  GliderListWidget *list;
  QLineEdit        *edtCoPilot;
  QSpinBox         *spinLoad;
  QSpinBox         *spinWater;
  Glider           *lastGlider;
  QLabel           *wingLoad;

  HSpinBox         *hspinLoad;
  HSpinBox         *hspinWater;

private slots:

  void slotGliderChanged();
  void slotGliderDeselected();
};

#endif
