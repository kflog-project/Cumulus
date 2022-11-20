/***********************************************************************
 **
 **   singlepoint.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2022 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \class SinglePoint
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Map element used for small point objects.
 *
 * Map element used for small point objects like obstacles and reporting points.
 *
 * \date 2000-2022
 *
 * \version 1.1
 */

#pragma once

#include "basemapelement.h"
#include "wgspoint.h"

class SinglePoint : public BaseMapElement
{
 public:

  /**
   * Default constructor.
   */
  SinglePoint();

  /**
   * Creates a new "SinglePoint".
   *
   * @param name  The name
   * @param shortName An alias-name, used for the gps-logger
   * @param typeID  The typeid
   * @param wgsPos  The original WGS-position
   * @param pos     The projected position
   * @param elevation The elevation of the point when available
   * @param country The country of the point as two letter code
   * @param comment An additional comment related to the point
   * @param secID  The map section ID
   * @param compulsory The compulsory of the single point
   */
  SinglePoint(const QString& name,
              const QString& shortName,
              const BaseMapElement::objectType typeID,
              const WGSPoint& wgsPos,
              const QPoint& pos,
              const float elevation = 0.0,
              const QString country = "",
              const QString comment = "",
              const unsigned short secID=0,
              const bool compulsory = false );
  /**
   * Destructor
   */
  virtual ~SinglePoint();

  /**
   * Draws the element into the given painter. Reimplemented from
   * \ref BaseMapElement.
   *
   * @param  targetP  The painter to draw the element into.
   * @return true, if element was drawn otherwise false.
   */
  virtual bool drawMapElement(QPainter* targetP);

  /**
   * @return the projected position of the element.
   */
  virtual QPoint getPosition() const
    {
      return position;
    };

  /**
   * Set the projected position of the element.
   */
  virtual void setPosition( const QPoint& value )
    {
      position = value;
    };

  /**
   * @return The WGS position of the element in kflog format.
   */
  virtual WGSPoint getWGSPosition() const
    {
      return wgsPosition;
    };

  /**
   * @return The WGS position of the element in kflog format.
   */
  virtual WGSPoint* getWGSPositionPtr()
    {
      return &wgsPosition;
    };

  /**
   * @return The WGS position of the element in kflog format.
   */
  virtual WGSPoint& getWGSPositionRef()
    {
      return wgsPosition;
    };

  /**
   * @param newPos The new WGS position of the element in kflog format.
   */
  virtual void setWGSPosition( const WGSPoint& value )
    {
      wgsPosition = value;
    };

  /**
   * @return the  short name of the element.
   */
  virtual QString getWPName() const
    {
      return shortName;
    };

  /**
   * @param newName The new short name of the element.
   */
  virtual void setWPName( const QString& newName )
    {
      shortName = newName;
    };

  /**
   * @return the position in the current map.
   */
  virtual QPoint getMapPosition() const
    {
      return curPos;
    };

  /**
   * @param newPos The new position in the current map.
   */
  virtual void setMapPosition( const QPoint& newPos )
    {
      curPos = newPos;
    };

  /**
   * @return the elevation of the element.
   */
  virtual float getElevation() const
    {
      return elevation;
    };

  /**
   * @param newElevation The new elevation of the element.
   */
  virtual void setElevation( const float value )
    {
      elevation = value;
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

  /**
   * @return The comment text of the element.
   */
  virtual QString getComment() const
    {
      return comment;
    };

  /**
   * Sets the comment text of the single point.
   *
   * @param newValue New country code of the element.
   */
  virtual void setComment( QString value )
    {
      comment = value;
    };

  /**
   * @return The compulsory of the single point.
   */
  virtual bool getCompulsory() const
    {
      return compulsory;
    }

  /**
   * Sets the compulsory of the single point.
   *
   * @param newValue New country code of the element.
   */
  virtual void setCompulsory( bool value )
    {
      compulsory = value;
    }

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
  float elevation;

  /**
   * A comment related to the point.
   */
  QString comment;

  /*
   * Feature of single point.
   */
  bool compulsory;
};

