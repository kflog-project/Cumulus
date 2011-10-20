/***********************************************************************
**
**   flarmwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2011 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class FlarmWidget
 *
 * \author Axel Pauli
 *
 * \brief Flarm widget to handle different views.
 *
 * This widget handles the two different Flarm views. It opens at first the
 * radar view. From the radar view the user can open the list view. All open
 * and close widget requests are handled by this widget.
 *
 * \date 2010-2011
 *
 * \version $Id$
 */

#ifndef FLARM_WIDGET_H
#define FLARM_WIDGET_H

#include <QWidget>

class FlarmRadarView;
class FlarmListView;
class FlarmAliasList;

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

  /** Called if alias list shall be opened with all Flarm objects. */
  void slotOpenAliasList();

  /** Called if alias list was closed with all Flarm objects. */
  void slotAliasListClosed();

signals:

  /** Emitted when the close button was pressed. */
  void closed();

private:

  FlarmRadarView* radarView;
  FlarmListView*  listView;

  /** Widget to handle Flarm alias data. */
  FlarmAliasList* aliasList;
};

#endif /* FLARM_WIDGET_H */
