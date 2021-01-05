/***********************************************************************
**
**   runway.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2008-2021 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class Runway
 *
 * \author Axel Pauli
 *
 * \brief Runway data class.
 *
 * This class is used for defining a runway together with its surface and the
 * translation types.
 *
 * \date 2008-2021
 *
 */

#ifndef RUNWAY_H
#define RUNWAY_H

#include <QHash>
#include <QString>
#include <QStringList>

class Runway
{

public:

  /**
   * Used to define the surface of a runway.
   */
  enum SurfaceType { Unknown = 0,
                     Grass = 1,
                     Asphalt = 2,
                     Concrete = 3,
                     Sand = 4,
                     Water = 5,
                     Gravel = 6,
                     Ice = 7,
                     Snow = 8,
                     Soil = 9 };
  Runway() :
    m_name(),
    m_length(0),
    m_heading(0),
    m_surface(Unknown),
    m_isOpen(false),
    m_isBidirectional(true),
    m_width(0)
    {
    };

  Runway( const QString& name,
          const float rwLength,
          const unsigned short head,
          const unsigned short surf,
          const bool open=true,
          const bool bidirectional=true,
          const float width=0.0 );

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
   * Returns the name of the runway.
   */
  QString getName()
  {
    return m_name;
  }

  /**
   * Prints out all runway data.
   */
  void printData();

  /**
   * The name of the runway.
   */
  QString m_name;

  /**
   * The length of the runway, given in meters.
   */
  float m_length;

  /**
   * The heading of the runway, given in steps of 1/10 degree (0-36).
   * Two headings are stored in every byte. (dir1*256 + dir2).
   */
  unsigned short m_heading;

  /**
   * The surface of the runway, one of SurfaceType, see above.
   */
  unsigned short m_surface;

  /**
   * Flag to indicate if the runway is open or closed.
   */
  bool m_isOpen;

  /**
   * Flag to indicate if the runway is bidirectional or not.
   */
  bool m_isBidirectional;

  /**
   * The width of the runway, given in meters.
   */
  float m_width;

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
