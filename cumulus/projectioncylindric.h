/***********************************************************************
**
**   projectioncylindric.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht, 2007 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef PROJECTIONCYLINDRIC_H
#define PROJECTIONCYLINDRIC_H

#include "projectionbase.h"

/**
 * \class ProjectionCylindric
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief Cylindric map projection class.
 *
 * This class provides a cylindrical projection.
 *
 * \date 2002-2009
 */
class ProjectionCylindric : public ProjectionBase
{

 public:

  /** */
  ProjectionCylindric(int);
  ProjectionCylindric(QDataStream &);

  /** */
  virtual ~ProjectionCylindric();

  /** */
  virtual ProjectionType projectionType() const
  {
    return Cylindric;
  };

  /**
   * returns the x-position.
   *
   * @param  latitude  This argument is unused.
   * @param  longitude The longitude of the position, given in radiant.
   */
  virtual double projectX(const double& latitude, const double& longitude)
  {
    Q_UNUSED( latitude )
    return longitude * cos_v1;
  };

  /**
   * Returns the y-position.
   *
   * @param  latitude  The latitude of the position, given in radiant.
   * @param  longitude This argument is unused.
   */
  virtual double projectY(const double& latitude, const double& longitude)
  {
    Q_UNUSED( longitude )
    return -latitude;
  };

  /**
   * Returns the latitude of a given projected position in radiant.
   *
   * @param  x  This argument is unused.
   * @param  y  The longitude of the position, given in radiant.
   */
  virtual double invertLat(const double& x, const double& y) const
  {
    Q_UNUSED( x )
    return -y;
  };

  /**
   * Returns the longitude of a given projected position in radiant.
   *
   * @param  x  The latitude of the position, given in radiant.
   * @param  y  This argument is unused.
   */
  virtual double invertLon(const double& x, const double& y) const
  {
    Q_UNUSED( y )
    return x / cos_v1;
  };

  /** */
  virtual double getRotationArc(const int, const int) const
  {
    return 0;
  };

  /** */
  virtual int getTranslationX(const int width, const int x) const
  {
    return width / 2 - x;
  };

  /** */
  virtual int getTranslationY(const int height, const int y) const
  {
    return (height / 2) - y;
  };

  /**
   * Sets the standard parallel. If it is too large,
   * the default value (45°N) is used.
   */
  bool initProjection(int v1);

  /**
   * Saves the parameters specific to this projection to a stream
   */
  virtual void saveParameters(QDataStream &);

  /**
   * Loads the parameters specific to this projection from a stream
   */
  virtual void loadParameters(QDataStream &);

  /**
   * @return The standard parallel value as integer.
   */
  int getStandardParallel() const
  {
    return i_v1;
  };


 private:
  /**
   * The standard parallel.
   */
  double v1;
  int i_v1;
  double cos_v1;
};

#endif
