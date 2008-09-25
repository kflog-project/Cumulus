/***********************************************************************
 **
 **   singlepoint.h
 **
 **   This file is part of Cumulus
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

#ifndef SINGLE_POINT_H
#define SINGLE_POINT_H

#include "basemapelement.h"
#include "wgspoint.h"

/**
 * Map element used for small objects. The object can be one of:
 * UltraLight, HangGlider, Parachute, Balloon, Village
 * or Landmark. Consists only of a name and a position.
 *
 * @see BaseMapElement#objectType
 * @see Airport
 * @see GliderSite
 * @see RadioPoint
 *
 */

class SinglePoint : public BaseMapElement
{
 public:
  /**
   * Creates a new "SinglePoint".
   *
   * @param  name  The name
   * @param  shortName An alias-name, used for the gps-logger
   * @param  typeID  The typeid
   * @param  pos  The projected position
   * @param  wgsPos  The original WGS-position
   * @param elevation The elevation of the point when avaible
   * @param secID  The mapsection ID
   */
  SinglePoint(const QString& name,
              const QString& shortName,
              const BaseMapElement::objectType typeID,
              const WGSPoint& wgsPos,
              const QPoint& pos,
              const unsigned int elevation = 0,
              const unsigned short secID=0 );

  /**
   * Destructor
   */
  virtual ~SinglePoint();

  /**
   * Draws the element into the given painter. Reimplemented from
   * BaseMapElement.
   *
   * @param  targetP  The painter to draw the element into.
   */
  virtual void drawMapElement(QPainter* targetP);

  /**
   * @return the projected position of the element.
   */
  virtual QPoint getPosition() const
    {
      return position;
    };

  /**
   * @return the WGSposition of the element. (normales Lat/Lon System)
   */
  virtual WGSPoint getWGSPosition() const
    {
      return wgsPosition;
    };

  /**
   * @return the  short name of the element.
   */
  virtual QString getWPName() const
    {
      return shortName;
    };

  /**
   * @return the position in the current map.
   */
  virtual QPoint getMapPosition() const
    {
      return curPos;
    };

  /**
   * @return the elevation of the element.
   */
  virtual unsigned int getElevation() const
    {
      return elevation;
    };

  /**
   * Reimplemented from BaseMapElement.
   *
   * Proofs, if the object is in the drawing-area of the map.
   *
   * @return "true", if the element is in the drawing-area of the map.
   */
  virtual bool isVisible() const
    {
      return glMapMatrix->isVisible(position);
    };

 protected:
  /**
   */
  WGSPoint wgsPosition;

  /**
   * The projected lat/lon-position of the element
   */
  QPoint position;

  /**
   * The abbreviation used for the GPS-logger.
   */
  QString shortName;

  /**
   * The current draw-position of the element.
   */
  QPoint curPos;

  /**
   * The elevation.
   */
  unsigned int elevation;
};

#endif
