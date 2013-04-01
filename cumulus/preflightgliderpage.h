/***********************************************************************
**
**   preflightgliderpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers, 2008-2013 Axel Pauli
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
 * \brief A widget for the pre-flight glider selection and settings.
 *
 * \date 2003-2013
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

class NumberEditor;

#include "gliderlistwidget.h"

class PreFlightGliderPage : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightGliderPage )

 public:

  PreFlightGliderPage( QWidget *parent=0 );

  virtual ~PreFlightGliderPage();

  /**
   * Saves the current settings permanently.
   */
  void save();

 protected:

  virtual void showEvent(QShowEvent *event);

 private:

  /**
   * Selects the current activated glider, if a selection exists.
   */
  void getCurrent();

  GliderListWidget *m_gliderList;
  QLineEdit        *m_edtPilot;
  QLineEdit        *m_edtCoPilot;
  Glider           *m_lastGlider;
  QLabel           *m_wingLoad;

  NumberEditor     *m_edtLoad;
  NumberEditor     *m_edtWater;

 signals:

   /**
    * Emitted, if settings have been changed.
    */
   void settingsChanged();

   /**
    * Emitted, if the widget is closed.
    */
   void closingWidget();

 private slots:

  /**
   * Called, if the glider selection was changed.
   */
  void slotGliderChanged();

  /**
   * Called, if a glider deselection was made.
   */
  void slotGliderDeselected();

  /**
   * Updates the wingload label.
   */
  void slotUpdateWingLoad(int value);

  /**
   * Updates the wingload label.
   */
  void slotNumberEdited( const QString& number );

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();
};

#endif
