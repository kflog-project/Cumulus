/***********************************************************************
**
**   flarmdislay.h
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
 * \brief Flarm display view.
 *
 * This widget shows the Flarm display view.
 *
 */

#ifndef FLARM_DISPLAY_H
#define FLARM_DISPLAY_H

#include <QWidget>
#include <QPixmap>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QShowEvent>
#include <QMouseEvent>
#include <QHash>
#include <QPoint>

class FlarmDisplay : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmDisplay )

public:

  /** The three supported Zoom levels (5km, 1km, 0,5km). */
  enum Zoom { Low=0, Middle=1, High=2 };

  /**
   * Constructor
   */
  FlarmDisplay( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~FlarmDisplay();

  /**
   * Returns the current active zoom level.
   */
  enum Zoom getZoomLevel() const
  {
    return zoomLevel;
  };

protected:

  void resizeEvent( QResizeEvent *event );

  void paintEvent( QPaintEvent *event );

  void showEvent( QShowEvent *event );

  void mousePressEvent( QMouseEvent *event);

public slots:

  /** Switch to a new zoom level. */
  void slotSwitchZoom( enum Zoom value );

  /** Update display */
  void slotUpdateDisplay();

private:

  /** Creates the background picture with the radar screen. */
  void createBackground();

private:

  /** Background picture according to zoom level as radar screen */
  QPixmap background;

  /** current zoom level */
  enum Zoom zoomLevel;

  /** Current used center point */
  int centerX;
  int centerY;

  /** Current used width */
  int width;

  /** Current used height */
  int height;

  /** Current used scale radius as pixels per distance */
  double scale;

  /** Current used outer circle radius. */
  int radius;

  /** Hash key of the selected object */
  QString selectedObject;

  /** Hash dictionary containing drawn objects and their positions at the
   *  screen.
   */
  QHash<QString, QPoint> objectHash;

};

#endif /* FLARM_DISPLAY_H */
