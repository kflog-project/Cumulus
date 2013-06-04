/***********************************************************************
**
**   preflightreturnerpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreFlightReturnerPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for pre-flight returner settings.
 *
 * \date 2013
 *
 * \version $Id$
 *
 */

#ifndef PREFLIGHT_RETURNER_PAGE_H
#define PREFLIGHT_RETURNER_PAGE_H

#include <QWidget>

class QComboBox;
class NumberEditor;

class PreFlightReturnerPage : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightReturnerPage )

 public:

  PreFlightReturnerPage(QWidget *parent=0);

  virtual ~PreFlightReturnerPage();

  void load();

  void save();

 private slots:

  /**
   * Called when the returner button is pressed..
   */
  void slotCallReturner();

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

 signals:

  /**
   * Emitted, if the widget is closed.
   */
  void closingWidget();

 private:

  /**
   * Editor for mobile number of returner.
   */
  NumberEditor* m_mobileNumber;

  /**
   * Coordinate position format selection box.
   */
  QComboBox* m_positionFormat;
};

#endif
