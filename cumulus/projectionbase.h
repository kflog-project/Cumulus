/***********************************************************************
 **
 **   projectionbase.h
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

#ifndef PROJECTION_BASE_H
#define PROJECTION_BASE_H

#include <QDataStream>

/**
 * \class ProjectionBase
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief This class is used as a base class for the map projection.
 *
 * This class is used as a base class for the map projection. It will
 * be inherited by all classes, which implement one type of map projection.
 *
 * \date 2002-2009
 */
class ProjectionBase
{
 public:
  /**
   * List of possible projection types.
   */
  enum ProjectionType {Unknown = 0, Lambert=1, Cylindric=2};

 public:
  /** */
  ProjectionBase();

  /** */
  virtual ~ProjectionBase();

  /** */
  virtual ProjectionType projectionType() const = 0;

  /** */
  virtual double projectX(const double& latitude, const double& longitude)  = 0;

  /** */
  virtual double projectY(const double& latitude, const double& longitude)  = 0;

  /** */
  virtual double invertLat(const double& x, const double& y) const = 0;

  /** */
  virtual double invertLon(const double& x, const double& y) const = 0;

  /** */
  virtual double getRotationArc(const int x, const int y) const = 0;

  /** */
  virtual int getTranslationX(const int width, const int x) const = 0;

  /** */
  virtual int getTranslationY(const int height, const int y) const = 0;

  /**
   * Saves the parameters specific to this projection to a stream
   */
  virtual void saveParameters(QDataStream &) = 0;

  /**
   * Loads the parameters specific to this projection from a stream
   */
  virtual void loadParameters(QDataStream &) = 0;
};

/**
 * Saves a ProjectionBase derived projection to a stream
 */
void SaveProjection(QDataStream &, ProjectionBase *);

/**
 * Loads a ProjectionBase derived projection from a stream
 */
ProjectionBase * LoadProjection(QDataStream &);

#endif
