/***********************************************************************
**
**   AirspaceHelper.cpp
**
**   Created on: 03.02.2014
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "AirspaceHelper.h"
#include "filetools.h"
#include "mapmatrix.h"
#include "projectionbase.h"
#include "resource.h"

extern MapMatrix* _globalMapMatrix;

  /**
   * Constructor
   */
  AirspaceHelper::AirspaceHelper()
  {
  }

  /**
   * Destructor
   */
  AirspaceHelper::~AirspaceHelper()
  {
  }

  bool AirspaceHelper::createCompiledFile( QString& fileName,
                                           QList<Airspace*>& airspaceList,
                                           int airspaceListStart )
  {
    if( airspaceList.size() == 0 || airspaceList.size() < airspaceListStart )
      {
        return false;
      }

    QFile file( fileName );
    QDataStream out( &file );

    out.setVersion( QDataStream::Qt_4_7 );

    if( file.open(QIODevice::WriteOnly) == false )
      {
        qWarning( "ASH: Can't open airspace file %s for writing!"
                 " Aborting ...",
                 fileName.toLatin1().data() );

        return false;
      }

    // create compiled binary version
    out << quint32( KFLOG_FILE_MAGIC );
    out << qint8( FILE_TYPE_AIRSPACE_C );
    out << quint16( FILE_VERSION_AIRSPACE_C );
    out << QDateTime::currentDateTime();
    SaveProjection( out, _globalMapMatrix->getProjection() );

    // write number of airfield records to be stored
    out << quint32( airspaceList.size() - airspaceListStart );

    // Storing starts at the given index.
    for( int i = airspaceListStart; i < airspaceList.size(); i++ )
      {
        Airspace* as = airspaceList[i];

        ShortSave( out, as->getName().toUtf8() );
        out << quint8( as->getTypeID() );
        out << quint8( as->getLowerT() );
        out << quint16( as->getLowerL() );
        out << quint8( as->getUpperT() );
        out << quint32( as->getUpperL() );
        ShortSave( out, as->getProjectedPolygon() );
      }

    file.close();
    return true;
  }

  /**
   * Read the content of a compiled file and put it into the passed
   * list.
   *
   * @param path Full name with path of OpenAir binary file
   * @param list All airspace objects have to be stored in this list
   * @returns true (success) or false (error occurred)
   */
  bool AirspaceHelper::readCompiledFile( QString &path, QList<Airspace*>& list )
  {
    QTime t;
    t.start();

    QFile inFile(path);

    if ( !inFile.open(QIODevice::ReadOnly) )
      {
        qWarning("ASH: Cannot open airspace file %s!", path.toLatin1().data());
        return false;
      }

    qDebug() << "ASH: Reading" << path;

    QDataStream in(&inFile);
    in.setVersion( QDataStream::Qt_4_7 );

    quint32 magic;
    qint8 fileType;
    quint16 fileVersion;
    QDateTime creationDateTime;

  #ifdef BOUNDING_BOX
    QRect boundingBox;
  #endif

    ProjectionBase *projectionFromFile;
    qint32 buflen;

    in >> magic;

    if ( magic != KFLOG_FILE_MAGIC )
      {
        qWarning() << "ASH: wrong magic key" << magic << "read! Aborting ...";
        inFile.close();
        return false;
      }

    in >> fileType;

    if ( fileType != FILE_TYPE_AIRSPACE_C )
      {
        qWarning() <<  "ASH: wrong file type" <<  fileType << "read! Aborting ...";
        inFile.close();
        return false;
      }

    in >> fileVersion;

    if ( fileVersion != FILE_VERSION_AIRSPACE_C )
      {
        qWarning( "ASH: wrong file version %d read! Aborting ...", fileVersion );
        inFile.close();
        return false;
      }

    in >> creationDateTime;

    projectionFromFile = LoadProjection(in);

    // projectionFromFile is allocated dynamically, we don't need it
    // here. Therefore it is immediately deleted to avoid memory leaks.
    delete projectionFromFile;
    projectionFromFile = 0;

  #ifdef BOUNDING_BOX

    in >> boundingBox;
  #endif

    // Read at first the number of airspaces contained in the compiled file.
    // We have to read it and make a preallocation in the airspace list.
    in >> buflen;

    list.reserve( list.size() + buflen );

    uint counter = 0;

    QString name;
    quint8 type;
    quint8 lowerType;
    quint16 lower;
    quint8 upperType;
    quint32 upper;
    QPolygon pa;
    QByteArray utf8_temp;

    while ( ! in.atEnd() )
      {
        counter++;
        pa.resize(0);

        ShortLoad(in, utf8_temp);
        name = QString::fromUtf8(utf8_temp);
        in >> type;
        in >> lowerType;
        in >> lower;
        in >> upperType;
        in >> upper;
        ShortLoad( in, pa );

        Airspace *a = new Airspace( name,
                                    (BaseMapElement::objectType) type,
                                    pa,
                                    upper, (BaseMapElement::elevationType) upperType,
                                    lower, (BaseMapElement::elevationType) lowerType );
        list.append(a);
      }

    inFile.close();

    QFileInfo fi( path );

    qDebug( "ASH: %d airspace objects read from file %s in %dms",
            counter, fi.fileName().toLatin1().data(), t.elapsed() );

    return true;
  }

/**
 * Get the header data of a compiled file and put it in the class
 * variables.
 *
 * \param path Full name with path of OpenAir binary file
 *
 * \param creationDateTime Date and time of file creation
 *
 * \param ProjectionBase stored projection type
 *
 * \returns true (success) or false (error occurred)
 */
bool AirspaceHelper::readHeaderData( QString &path,
                                     QDateTime& creationDateTime,
                                     ProjectionBase* projection )
{
  quint32 h_magic = 0;
  qint8 h_fileType = 0;
  quint16 h_fileVersion = 0;

  QFile inFile(path);

  if ( !inFile.open( QIODevice::ReadOnly) )
    {
      qWarning("ASH: Cannot open airspace file %s!", path.toLatin1().data());
      return false;
    }

  QDataStream in(&inFile);
  in.setVersion( QDataStream::Qt_4_7 );

  in >> h_magic;

  if ( h_magic != KFLOG_FILE_MAGIC )
    {
      qWarning( "ASH: wrong magic key %x read! Aborting ...", h_magic );
      inFile.close();
      return false;
    }

  in >> h_fileType;

  if ( h_fileType != FILE_TYPE_AIRSPACE_C )
    {
      qWarning( "ASH: wrong file type %x read! Aborting ...", h_fileType );
      inFile.close();
      return false;
    }

  in >> h_fileVersion;

  if ( h_fileVersion != FILE_VERSION_AIRSPACE_C )
    {
      qWarning( "ASH: wrong file version %x read! Aborting ...", h_fileVersion );
      inFile.close();
      return false;
    }

  in >> creationDateTime;

  projection = LoadProjection(in);

#ifdef BOUNDING_BOX
  in >> h_boundingBox;
#endif

  inFile.close();
  return true;
}
