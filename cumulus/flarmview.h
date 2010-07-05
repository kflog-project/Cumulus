/***********************************************************************
**
**   flarmview.h
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

#ifndef FLARM_VIEW_H_
#define FLARM_VIEW_H_

#include <QWidget>
#include <QPixmap>

class FlarmView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmView )

public:

  /** The three supported Zoom levels (5km, 1km, 0,5km). */
  enum Zoom { Low=0, Middle=1, High=2 };

  /**
   * Constructor
   */
  FlarmView( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~FlarmView();

private:

  /** Creates the base picture with the radar screen. */
  void createBasePicture();

private:

  /** Flarm radar screen */
  QWidget* screen;

  /** Base picture according to zoom level as radar screen */
  QPixmap basePicture;

  /** current zoom level */
  enum Zoom zoomLevel;

};

#endif /* FLARM_VIEW_H_ */
