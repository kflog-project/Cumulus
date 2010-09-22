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

  /**
   * Sets the update interval to the new value. The unit is seconds.
   */
  void setUpdateInterval( int newInterval )
  {
    updateInterval = newInterval;
  };

  /**
   * Returns the hash key of the currently selected Flarm object.
   */
  static QString& getSelectedObject()
  {
    return selectedObject;
  };

protected:

  void resizeEvent( QResizeEvent *event );

  void paintEvent( QPaintEvent *event );

  void showEvent( QShowEvent *event );

  void mousePressEvent( QMouseEvent *event);

signals:

  /**
   * Emit a new Flarm object selection.
   */
  void newObjectSelection( QString newObject );

public slots:

  /** Switch to a new zoom level. */
  void slot_SwitchZoom( enum Zoom value );

  /** Update display */
  void slot_UpdateDisplay();

  /** Reset display to background. */
  void slot_ResetDisplay();

  /** Set object to be selected. It is the hash key. */
  void slot_SetSelectedObject( QString newObject )
  {
    selectedObject = newObject;
  };

private:

  /** Creates the background picture with the radar screen. */
  void createBackground();

private:

  /** Background picture according to zoom level as radar screen */
  QPixmap background;

  /** current zoom level */
  static enum Zoom zoomLevel;

  /** Hash key of the selected object */
  static QString selectedObject;

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

  /** Hash dictionary containing drawn objects and their positions at the
   *  screen.
   */
  QHash<QString, QPoint> objectHash;

  /**
   * Time interval of screen update in seconds.
   */
  int updateInterval;
};

#endif /* FLARM_DISPLAY_H */
