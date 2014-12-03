/***********************************************************************
**
**   openairparser.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2005      by André Somers
**                   2008-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class OpenAirParser
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Parser for OpenAir SUA files
 *
 * This class implements a parser for OpenAir SUA files, containing
 * descriptions of airspace structures.
 *
 * A description of the OpenAir format is to find here:
 *
 * http://www.winpilot.com/UsersGuide/UserAirspace.asp
 *
 * Since the build in airspace types do not exactly match the list of
 * airspaces found in "the world", it is possible to use a special
 * mapping file that modifies and/or extends the default mapping.
 * For a file named airspace.txt, the matching mapping file would be
 * named airspace_mappings.conf and must be placed in the same directory.
 *
 * \date 2005-2014
 *
 * \version 1.0
 */

#ifndef _openair_parser_h
#define _openair_parser_h

#include <QString>
#include <QList>
#include <QMap>
#include <QRect>
#include <QDateTime>
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QPolygon>
#include <QPoint>

#include "basemapelement.h"

class Airspace;

class OpenAirParser
{
 public:

  /**
   * Constructor
   */
  OpenAirParser();

  /**
   * Destructor
   */
  virtual ~OpenAirParser();

  /**
   * Parses the file indicated and adds them to the indicated
   * airspace list.
   *
   * @return true on success, and false on failure
   * @param path the path for the OpenAir file
   * @param list the list of Airspace objects, where the objects in this
   *        file should be added to.
   * @param doComilie create a compiled version if argument is true
   */
  bool parse(const QString& path, QList<Airspace*>& list, bool doCompile=true );

 private:

  void resetState();
  void parseLine(QString&);
  void newAirspace();
  void newPA();
  void finishAirspace();
  void parseType(QString&);
  void parseAltitude(QString&, BaseMapElement::elevationType&, uint&);
  bool parseCoordinate(QString&, int& lat, int& lon);
  bool parseCoordinate(QString&, QPoint&);
  bool parseCoordinatePart(QString&, int& lat, int& lon);
  bool parseVariable(QString);
  bool makeAngleArc(QString);
  bool makeCoordinateArc(QString);
  double bearing( QPoint& p1, QPoint& p2 );
  void addCircle(const double& rLat, const double& rLon);
  void addCircle(const double& radius);
  void addArc(const double& rLat, const double& rLon,
              double angle1, double angle2);

private:

  QList<Airspace*> _airlist;
  uint _lineNumber;
  uint _objCounter; // counter for allocated objects
  bool _isCurrentAirspace;

  // Set, if a parse error was detected
  bool _parseError;

  // Read condition flags
  bool _acRead;
  bool _anRead;

  QString asName;
  BaseMapElement::objectType asType;
  QPolygon asPA;
  unsigned int asUpper;
  BaseMapElement::elevationType asUpperType;
  unsigned int asLower;
  BaseMapElement::elevationType asLowerType;

  QPoint _center;
  double _awy_width;
  int _direction; //1 for clockwise, -1 for anti clockwise

  /**
   * Mapper openair airspace type to Cumulus airspace type.
   */
  QMap<QString, BaseMapElement::objectType> m_airspaceTypeMapper;

  // bounding box
  QRect *_boundingBox;
};

#endif
