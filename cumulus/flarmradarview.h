/***********************************************************************
**
**   flarmradarview.h
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
 * \class FlarmRadarView
 *
 * \author Axel Pauli
 *
 * \brief Flarm Radar and Operation view.
 *
 * This widget shows the Flarm radar view with the operation buttons.
 *
 * \date 2010-2011
 *
 * \version $Id$
 */

#ifndef FLARM_RADAR_VIEW_H
#define FLARM_RADAR_VIEW_H

#include <QWidget>

#include "flarmdisplay.h"

class QGroupBox;
class QPushButton;
class QString;

class FlarmDisplay;

class FlarmRadarView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmRadarView )

public:

  /**
   * Constructor
   */
  FlarmRadarView( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~FlarmRadarView();

  /**
   * @return The display widget.
   */
  FlarmDisplay* getDisplay() const
    {
      return display;
    };

public slots:

  /** Called to change the visibility of the add Flarm Id button. */
  void slotShowAddButton( QString selectedObject );

private slots:

  /** Called if close button was pressed. */
  void slotClose();

  /** Called if zoom level shall be changed. */
  void slotZoom();

  /** Called if list view button was pressed. */
  void slotOpenListView();

  /** Called if update interval button was pressed. */
  void slotUpdateInterval();

  /** Called if alias list button was pressed. */
  void slotOpenAliasList();

  /** Called to add an object to the Flarm alias list. */
  void slotAddFlarmId();

signals:

  /** Emitted if the list view shall be opened with all Flarm objects. */
  void openListView();

  /** Emitted if the alias list shall be opened with all Flarm entries. */
  void openAliasList();

  /** Emitted when the close button of the radar widget was pressed. */
  void closeRadarView();

private:

  /** Display with radar view. */
  FlarmDisplay* display;

  /** Update interval button. */
  QPushButton* updateButton;

  /** Add Flarm Id button. */
  QPushButton *addButton;
};

#endif /* FLARM_RADAR_VIEW_H */
