/***********************************************************************
 **
 **   isohypse.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2007-2009 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef ISOHYPSE_H
#define ISOHYPSE_H

#include <QRect>
#include <QRegion>

#include "lineelement.h"

/**
 * This class is used for isohypses.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 */
class Isohypse : public LineElement
  {
  public:
    /**
     * Creates a new isohypse.
     *
     * @param  pG  The polygon containing the position points.
     * @param  elevation  The elevation
     * @param  isValles "true", if the area is a valley
     * @param  secID The tile section identifier
     * @param  typeID The type of isohypse, ground or terrain
     */
    Isohypse(QPolygon pG, uint elevation, bool isValley,
             uint secID, const ushort typeID );

    /**
     * Destructor
     */
    virtual ~Isohypse();

    /**
     * Draws the iso region into the given painter.
     *
     * @param  targetP  The painter to draw the element into.
     */
    QRegion* drawRegion( QPainter* targetP, const QRect &viewRect,
                         bool really_draw, bool isolines=false );

    /**
     * @return the elevation of the line
     */
    int getElevation() const
      {
        return _elevation;
      };

    /**
     * @return the type of isohypse, ground or terrain
     */
    ushort getTypeId() const
      {
        return _typeID;
      };

  private:
    /**
     * The elevation
     */
    uint _elevation;

    /**
     * The type of isohypse, ground or terrain.
     */
     ushort _typeID;
  };

#endif
