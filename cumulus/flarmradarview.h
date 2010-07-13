/***********************************************************************
**
**   flarmradarview.h
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
 * \brief Flarm Radar and Operation view.
 *
 * This widget shows the Flarm radar view with the operation buttons.
 *
 */

#ifndef FLARM_RADAR_VIEW_H
#define FLARM_RADAR_VIEW_H

#include <QWidget>

class QGroupBox;
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

private slots:

  /** Called if close button was pressed. */
  void slotClose();

  /** Called if zoom level shall be changed. */
  void slotZoom();

  /** Called if list view button was pressed. */
  void slotOpenListView();

signals:

  /** Emitted if the list view shall be opened with all Flarm objects. */
  void openListView();

  /** Emitted when the close button was pressed. */
  void closeRadarView();

private:

  /** Display with radar view. */
  FlarmDisplay* display;

  /** Button box. */
  QGroupBox* buttonBox;
};

#endif /* FLARM_RADAR_VIEW_H */
