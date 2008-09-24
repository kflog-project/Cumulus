/***********************************************************************
**
**   lineelement.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef LINEELEMENT_H
#define LINEELEMENT_H

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
    LineElement(QString name, BaseMapElement::objectType t, QPolygon pP,
                bool isVal = false, unsigned int secID=0);

    /**
     * Destructor.
     */
    virtual ~LineElement();

    /**
     * Draws the element into the given painter. Reimplemented from
     * BaseMapElement.
     *
     * @param  targetP  The painter to draw the element into.
     */
    virtual void drawMapElement(QPainter* targetP);

    /**
     * Prints the element. Reimplemented from BaseMapElement.
     *
     * @param  printP  The painter to draw the element into.
     *
     * @param  isText  Shows, if the text of some mapelements should
     *                 be printed.
     */
    //virtual void printMapElement(QPainter* printPainter, bool isText) const;

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
     * "true", if the element is a closed polygone (like cities).
     */
    bool closed;
};

#endif
