/***********************************************************************
**
**   flarmwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief Flarm widget to handle different views.
 *
 * This widget handles the different Flram views.
 *
 */

#ifndef FLARM_WIDGET_H
#define FLARM_WIDGET_H

#include <QWidget>

class FlarmRadarView;
class FlarmListView;

class FlarmWidget : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmWidget )

public:

  /**
   * Constructor
   */
  FlarmWidget( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~FlarmWidget();

protected:

  /** Called by parent before widget is shown. */
  void showEvent( QShowEvent *event );

public slots:

  /** Called if list view shall be opened with all Flarm objects. */
  void slotOpenListView();

  /** Called if list view shall be closed with all Flarm objects. */
  void slotCloseListView();

  /** Called if radar view shall be closed. */
  void slotCloseRadarView();

signals:

  /** Emitted when the close button was pressed. */
  void closed();

private:

  FlarmRadarView* radarView;
  FlarmListView*  listView;
};

#endif /* FLARM_WIDGET_H */
