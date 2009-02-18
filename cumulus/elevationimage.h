/***********************************************************************
**
**   elevationimage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef ElevationImage_h
#define ElevationImage_h

#include <QWidget>
#include <QColor>
#include <QSize>

/**
 * This class shows the used elevation colors of the map in a vertical bar.
 * The right side of the bar is labeled with elevation numbers according
 * to the current altitude unit (meters or feed).
 */

class ElevationImage : public QWidget
{
  Q_OBJECT

 public:

  /** A reference to the terrain color array has to be passed. The colors
   *  from the array are taken for the elevation color bars. Update first
   *  colors in the array before a new paintEvent is fired.
   */
  ElevationImage( QColor colors[], QWidget *parent = 0);
  ~ElevationImage();

  QSize minimumSizeHint() const;
  QSize sizeHint() const;

 protected:

  /** Handles the paint events of the widget */
  void paintEvent(QPaintEvent *event);

 private:

  /** Reference to terrainColors to be used for drawing. The array contains
   * 51 colors, starting with the lowest level at index 0. Index 50 contains
   * the highest elevation level. */
  QColor *terrainColors;
};

#endif
