/***********************************************************************
**
**   runway.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2008-2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef RUNWAY_H
#define RUNWAY_H

#include <QString>
#include <QHash>
#include <QStringList>

/**
 * This class is used for defining a runway together with its surface and the
 * translation types.
 */

class Runway
{

public:

  /**
   * Used to define the surface of a runway.
   */
  enum SurfaceType {Unknown = 0, Grass = 1, Asphalt = 2, Concrete = 3, Sand = 4};

  Runway( const unsigned short len,
          const unsigned short dir,
          const unsigned short surf,
          const bool open );

  virtual ~Runway() {};

  /**
   * Get translation string for surface type.
   */
  static QString item2Text( const int surfaceType, QString defaultValue=QString("") );

  /**
   * Get surface type for translation string.
   */
  static int text2Item( const QString& text );

  /**
   * Get sorted translations
   */
  static QStringList& getSortedTranslationList();


  /**
   * The length of the runway, given in meters.
   */
  unsigned short length;

  /**
   * The direction of the runway, given in steps of 10 degree.
   */
  unsigned short direction;

  /**
   * The surface of the runway, one of SurfaceType, see above.
   */
  unsigned short surface;

  /**
   * Flag to indicate if the runway is open or closed.
   */
  bool isOpen;

  /**
   * Static pointer to surface translations
   */
  static QHash<int, QString> surfaceTranslations;
  static QStringList sortedTranslations;

  /**
   * Static method for loading of object translations
   */
  static void loadTranslations();

};

#endif
