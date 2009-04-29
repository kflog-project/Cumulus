/***********************************************************************
**
**   lineelement.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef LINE_ELEMENT_H
#define LINE_ELEMENT_H

#include "basemapelement.h"

/**
 * Class used for all elements, which consist of a point array.
 *
 * @see BaseMapElement#objectType
 *
 */

class LineElement : public BaseMapElement
{
public:
    /**
     * Creates a new map element.
     *
     * @param  name  The name
     * @param  pA  The point array containing the positions
     * @param  isVal  "true", if the element is a "valley".
     */
    LineElement( const QString& name,
                 const BaseMapElement::objectType t,
                 const QPolygon& pP,
                 const bool isVal = false,
                 const unsigned short secID=0 );

    /**
     * Destructor.
     */
    virtual ~LineElement();

    /**
     * Draws the element into the given painter. Reimplemented from
     * BaseMapElement.
     *
     * @param  targetP  The painter to draw the element into.
     * @return true, if element was drawn otherwise false.
     */
    virtual bool drawMapElement(QPainter* targetP);

    /**
     * @return "true", if the element is a valley.
     *
     * @see #valley
     */
    virtual bool isValley() const
      {
        return valley;
      };

    /**
     * Proofs, if the object is in the drawing-area of the map.
     *
     * @return "true", if the bounding-box of the element intersects
     *         with the drawing-area of the map.
     */
    virtual bool isVisible() const
      {
        return glMapMatrix->isVisible(bBox, getTypeID());
      };

protected:
    /**
     * Contains the projected positions of the item.
     */
    QPolygon projPolygon;

    /**
     * The bounding-box of the element.
     */
    QRect bBox;

    /**
     * "true", if the element is a valley. Valleys are used for Isohypsen
     * and f.e. unurban-areas within urban areas or island in a lake.
     */
    bool valley;

    /**
     * "true", if the element is a closed polygon (like cities).
     */
    bool closed;
};

#endif
