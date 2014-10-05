/***********************************************************************
 **
 **   welt2000.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2006-2014 by Axel Pauli, kflog.cumulus@gmail.com
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <cmath>
#include <unistd.h>
#include <libgen.h>

#include <QtCore>

#include "airfield.h"
#include "basemapelement.h"
#include "filetools.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "projectionbase.h"
#include "resource.h"
#include "runway.h"
#include "wgspoint.h"
#include "generalconfig.h"
#include "distance.h"

#include "welt2000.h"

// All is prepared for additional calculation, storage and
// reconstruction of a bounding box. Be free to switch on/off it via
// conditional define.

#undef BOUNDING_BOX
// #define BOUNDING_BOX 1

#ifdef BOUNDING_BOX
extern MapContents*  _globalMapContents;
#endif

extern MapMatrix*    _globalMapMatrix;

// set static member variable
QMutex Welt2000::mutex;

Welt2000::Welt2000() :
  c_homeRadius(0.0),
  c_runwayLengthFilter(0.0),
  h_magic(0),
  h_fileType(0),
  h_fileVersion(0),
  h_homeRadius(0.0),
  h_runwayLengthFilter(0.0),
  h_outlandings(false),
  h_projection(static_cast<ProjectionBase *> (0)),
  h_headerIsValid(false)
{
  // prepare base mappings of Cumulus
  c_baseTypeMap.insert( "IntAirport", BaseMapElement::IntAirport );
  c_baseTypeMap.insert( "Airport", BaseMapElement::Airport );
  c_baseTypeMap.insert( "MilAirport", BaseMapElement::MilAirport );
  c_baseTypeMap.insert( "CivMilAirport", BaseMapElement::CivMilAirport );
  c_baseTypeMap.insert( "Airfield", BaseMapElement::Airfield );
  c_baseTypeMap.insert( "ClosedAirfield", BaseMapElement::ClosedAirfield );
  c_baseTypeMap.insert( "CivHeliport", BaseMapElement::CivHeliport );
  c_baseTypeMap.insert( "MilHeliport", BaseMapElement::MilHeliport );
  c_baseTypeMap.insert( "AmbHeliport", BaseMapElement::AmbHeliport );
  c_baseTypeMap.insert( "Gliderfield", BaseMapElement::Gliderfield );
  c_baseTypeMap.insert( "UltraLight", BaseMapElement::UltraLight );
  c_baseTypeMap.insert( "HangGlider", BaseMapElement::HangGlider );
}

Welt2000::~Welt2000()
{
  if( h_projection )
    {
      delete h_projection;
    }
}

/**
 * search on default places a welt2000 file and load it. A source can
 * be the original ASCII file or a compiled version of it. The results
 * are put in the passed lists
 */
bool Welt2000::load( QList<Airfield>& airfieldList,
                     QList<Airfield>& gliderfieldList,
                     QList<Airfield>& outlandingList )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &mutex );

  qDebug() << "Welt2000::load";

  // Rename WELT2000.TXT -> welt2000.txt.
  QString wl = "welt2000.txt";
  QString wu = "WELT2000.TXT";
  QString sd = "/points/";

  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      QString pLower = mapDirs.at(i) + sd + wl;
      QString pUpper = mapDirs.at(i) + sd + wu;

      if( QFileInfo(pUpper).exists() )
        {
          // On a FAT32 SDCard upper and lower case filenames are not
	  // divided. If one of that filenames exist, QFileInfo returns always
	  // true. The linux system call rename seems to handle that but
	  // QFile::rename requires a QFile::remove of the new destination file
	  // name before and that removes the wrong one.
          rename( pUpper.toLatin1().data(), pLower.toLatin1().data() );
        }
    }

  QString w2PathTxt;
  QString w2PathTxc;

  // Search for Welt2000 source resp. compiled files.
  bool resTxt = MapContents::locateFile( "points/welt2000.txt", w2PathTxt );
  bool resTxc = MapContents::locateFile( "points/welt2000.txc", w2PathTxc );

  if( ! resTxt && ! resTxc )
    {
      qWarning( "W2000: No Welt2000 files could be found in the map directories" );
      return false;
    }

  // If a compiled file exists, we will read the header data of it
  if( resTxc )
    {
      resTxc = setHeaderData( w2PathTxc );

      if( ! resTxc )
        {
          qDebug( "W2000: Format of welt2000.txc has been changed --> reparse welt2000.txt" );
          // Compiled file format is not the expected one, remove
          // wrong file and start a reparsing of source file.
          QFile::remove( w2PathTxc );
          return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
        }

      // Do a date-time check, if a compiled file was localized and a
      // source file too. If the source file is younger in its
      // modification time as the compiled file, a new compilation must
      // be forced.
      if( resTxt )
        {
          QFileInfo fi(w2PathTxt);
          QDateTime lastModTxt = fi.lastModified();

          if( h_creationDateTime < lastModTxt )
            {
              qDebug( "W2000: Time of welt2000.txt has been changed --> reparse welt2000.txt" );
              // Modification date-time of source is younger as from
              // compiled file. Therefore we do start a reparsing of source
              // file.
              QFile::remove( w2PathTxc );
              return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
            }
        }

      // Check against the config file
      QFileInfo fi(w2PathTxt);
      QString path = fi.path() + "/welt2000.conf";
      QFileInfo fiConf(path);

      if( fiConf.exists() && fi.isReadable() &&
          h_creationDateTime < fiConf.lastModified() )
        {
          qDebug( "W2000: welt2000.conf has been changed --> reparse welt2000.txt" );
          // Conf file was modified, make a new compilation. It is not
          // deeper checked, what was modified due to the effort and
          // in the assumption that a configuration file will not be changed
          // every minute.
          QFile::remove( w2PathTxc );
          return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
        }

      // We do check, if the user has countries defined. If yes, we
      // must check the country list of the compiled file against the
      // user list.
      QString uFilter = GeneralConfig::instance()->getWelt2000CountryFilter();
      QStringList ulist = uFilter.split( QRegExp("[, ]"), QString::SkipEmptyParts );

      if( h_countryList.count() != ulist.count() )
        {
          qDebug( "W2000: Country list has been changed --> reparse welt2000.txt" );
          // There is a difference in the country lists. Therefore we
          // start a reparsing of the source file.
          QFile::remove( w2PathTxc );
          return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
        }

      if( h_countryList.count() > 0 )
        {
          // There are country definitions contained in the compiled
          // file. Check, if the current defined filter list is equal
          // to the file list.
          bool result = true;

          for( int i = 0; i < ulist.count(); i++ )
            {
              QString e = ulist[i].trimmed().toUpper();

              if( ! h_countryList.contains(e) )
                {
                  result = false;
                  break;
                }
            }

          if( result == false )
            {
              qDebug( "W2000: Country list has been changed --> reparse welt2000.txt" );
              // The configured country list is not identical to the
              // compiled country list. Therefore we start a reparsing.
              QFile::remove( w2PathTxc );
              return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
            }
        }

      // Check, if home position has been changed in the meantime. In this
      // case a new parsing of the source should be started.
      QPoint curHome = _globalMapMatrix->getHomeCoord();

      if( curHome != h_homeCoord )
        {
          qDebug( "W2000: Home coordinates have been changed --> reparse welt2000.txt" );
          // Home coordinates have been changed, make a reparsing of
          // source file
          QFile::remove( w2PathTxc );
          return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
        }

      // Check, if the home radius has been changed in the mean time.
      // In this case a new parsing of the source must be started.
      // Get home radius from the configuration data in kilometers.
      // If set to zero, no radius is defined
      double dRadius = GeneralConfig::instance()->getAirfieldHomeRadius() / 1000.;

      if( h_countryList.isEmpty() && dRadius == 0.0 )
        {
          // Define a default radius of 500Km, if no country filter is defined.
          dRadius = 500.0;
        }

      if( fabs( dRadius - h_homeRadius ) > 0.5 )
        {
          qDebug( "W2000: Home radius has been changed --> reparse welt2000.txt" );
          // Home radius has been changed, make a reparsing of the source file
          QFile::remove( w2PathTxc );
          return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
        }

      // Check, if the runway length filter has been changed.
      if( GeneralConfig::instance()->getAirfieldRunwayLengthFilter() != h_runwayLengthFilter )
        {
          qDebug( "W2000: Runway length filter has been changed --> reparse welt2000.txt" );
          // Home radius has been changed, make a reparsing of the source file
          QFile::remove( w2PathTxc );
          return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
        }

      // Check, if the projection has been changed in the meantime
      ProjectionBase *currentProjection = _globalMapMatrix->getProjection();

      if( ! MapContents::compareProjections( h_projection, currentProjection ) )
        {
          qDebug( "W2000: Projection has been changed --> reparse welt2000.txt" );
          // Projection has changed in the meantime
          if(  h_projection )
            {
              delete h_projection;
              h_projection = 0;
            }

          QFile::remove( w2PathTxc );
          return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
        }

      // Nothing has been changed, read in compiled file
      if( ! readCompiledFile( w2PathTxc, airfieldList, gliderfieldList, outlandingList ) )
        {
          // reading of compiled file failed, let's parse the source
          QFile::remove( w2PathTxc );
          return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
        }

      return true;
    } // End of if( resTxc )

  // parse source file
  return parse( w2PathTxt, airfieldList, gliderfieldList, outlandingList, true );
}

bool Welt2000::check4File()
{
  QString path2File;

  // Search for Welt2000 source file.
  bool exits = MapContents::locateFile( "points/welt2000.txt", path2File );

  if( ! exits )
    {
      // No welt2000 exists and we return false in this case because we cannot
      // check the update state.
      return false;
    }

  return true;
}

bool Welt2000::check4update()
{
  // Update check string for Welt2000, must be adapted after every Welt2000 update!
  const char* w2000CheckString = "$ UPDATED AT: 24.FEB.2014";

  // Line number in welt2000 file, on which the w2000CheckString is expected.
  // Note! Line counting starts with 1.
  const int ckeckLineNo = 17;

  QString path2File;

  // Search for Welt2000 source file.
  bool exits = MapContents::locateFile( "points/welt2000.txt", path2File );

  if( ! exits )
    {
      // No welt2000 exists and we return false in this case because we cannot
      // check the update state.
      return false;
    }

  QFile in(path2File);

  if( ! in.open(QIODevice::ReadOnly) )
    {
      return false;
    }

  QTextStream ins(&in);
  ins.setCodec( "ISO 8859-15" );

  bool ret = false;
  int lineNo = 0;

  while( ! ins.atEnd() )
    {
      QString line;
      line = ins.readLine(128);
      lineNo++;

      // The constant ckeckLineNo addresses a line which contains the
      // expected string to be compared.
      if( lineNo < ckeckLineNo )
        {
          continue;
        }

      if( line.startsWith(w2000CheckString) == false )
        {
          // The expected data is not to find in the line of the read file.
          // We assume, that this is an older Welt2000 file. This assumption
          // is not right, if the user has installed a newer file as we expect.
          ret = true;
        }

      break;
    }

  in.close();

  if( lineNo < ckeckLineNo )
    {
      // The file seems to be empty or has too less lines. We trigger a reload
      // in this case.
      ret = true;
    }

  return ret;
}

/**
 * The passed file has to be a welt2000 file. All not relevant
 * entries, like turn points, will be filtered out. A new file is
 * written with the same name and an own header.
 */
bool Welt2000::filter( QString& path )
{
  QString header1 = "# Welt2000 file was filtered by cumulus at ";
  QString header2 = "# Please do not modify or remove this header";

  QFile in(path);
  QString fout = path + ".filtered";
  QFile out(fout);

  if( !in.open(QIODevice::ReadOnly) )
    {
      qWarning("W2000: Cannot open airfield file %s!", path.toLatin1().data());
      return false;
    }

  QTextStream ins(&in);
  QTextStream outs;

  ins.setCodec( "ISO 8859-15" );
  outs.setCodec( "ISO 8859-15" );

  uint outLines = 0; // counter for written lines

  while( ! ins.atEnd() )
    {
      QString line = ins.readLine();

      if( outLines == 0 )
        {
          if( line.startsWith( header1 ) )
            {
              // File was already filtered, stop all doing
              in.close();
              return true;
            }

          // open output file for filtering
          if( !out.open(QIODevice::WriteOnly) )
            {
              in.close();
              qWarning("W2000: Cannot open temporary file %s!", fout.toLatin1().data());
              return false;
            }

          outs.setDevice( &out );
          outs << header1.toLatin1().data()
               << QDateTime::currentDateTime().toString().toLatin1().data() << '\n'
               << header2.toLatin1().data() << '\n' << '\n';
          outLines += 2;
        }

      // remove white spaces and line end characters
      line = line.trimmed();

      // remove temporary commented out entries
      if( line.startsWith("$-") || line.startsWith("$+") ||
          line.startsWith("$*") || line.startsWith("$$") )
        {
          continue;
        }

      // real comments are not filtered out
      if( line.startsWith("#") || line.startsWith("$") )
        {
          outs << line.toLatin1().data() << '\n';
          outLines++;
          continue;
        }

      if( line.length() < 62 )
        {
          // country sign not included, skip over it
          continue;
        }

      // look, what kind of line was read.
      // COL5 = 1 Airfield
      // COL5 = 2 Outlanding, can contain UL fields
      QString kind = line.mid( 5, 1 );

      if( kind != "1" && kind != "2" )
        {
          continue; // skip it, not of interest for us
        }

      outs << line.toLatin1().data() << '\n';
      outLines++;
    }

  in.close();
  out.close();

  if( outLines > 2 )
    {
      // overwrite old file with new extracted file
      QFile::remove( path );
      QFile::rename( fout, path );
    }
  else
    {
      // remove unneeded file, if nothing could be extracted
      QFile::remove( fout );
    }

  return true;
}

/**
 * The passed file can contain country information, to be used during
 * parsing of welt2000.txt file. The entries country and home radius
 * can be overwritten by user values defined in GeneralConfig class.
 *
 * File syntax: [#$] These 2 signs starts a comment line, it ends with the newline
 *              FILTER countries=<country_1>,<country_2>,...<country_n>'\nl'
 *              FILTER countries=....
 *              MAP_ICAO <name>=[IntAirport|Airport|MilAirport|CivMilAirport|Airfield|ClosedAirfield|CivHeliport|MilHeliport|AmbHeliport|Gliderfield|UltraLight|HangGlider]
 *              MAP_SHORT_NAME <name>=[IntAirport|Airport|MilAirport|CivMilAirport|Airfield|ClosedAirfield|CivHeliport|MilHeliport|AmbHeliport|Gliderfield|UltraLight|HangGlider]
 *
 * You can define several filter lines, all will be processed.
 *
 */
bool Welt2000::readConfigEntries( QString &path )
{
  c_countryList.clear();
  c_homeRadius = 0.0;
  c_icaoMap.clear();
  c_shortMap.clear();

  QFile in(path);

  if( !in.open(QIODevice::ReadOnly) )
    {
      return false;
    }

  qDebug() << "W2000: Reading configuration file" << path;

  QTextStream ins(&in);

  while( ! ins.atEnd() )
    {
      QString line = ins.readLine();

      if( line.isEmpty() )
        {
          continue; // skip empty lines
        }

      // remove white spaces and line end characters
      line = line.trimmed();

      // step over comment lines
      if( line.startsWith("#") || line.startsWith("$") )
        {
          continue;
        }

      if( line.startsWith("FILTER") || line.startsWith("filter") )
        {
          QStringList list = line.split(QRegExp("[=,]"), QString::SkipEmptyParts);

          if( list.count() < 2 || list[0].contains("countries", Qt::CaseInsensitive) == false )
            {
              // No country elements to find in list
              continue;
            }

          // remove first entry, it is the filter-country key
          list.removeAt( 0 );

          for( int i = 0; i < list.count(); i++ )
            {
              QString e = list[i].trimmed().toUpper();

              if( c_countryList.contains(e) )
                continue;

              c_countryList += e;
            }

          c_countryList.sort();
          continue;
        }

      if( line.startsWith("MAP_") || line.startsWith("map_") )
        {
          QStringList list = line.split(QRegExp("[=]"), QString::SkipEmptyParts);

          if( list.count() < 2 )
            {
              // No map elements to find in list
              continue;
            }

          if( list[0].contains("MAP_ICAO",Qt::CaseInsensitive) )
            {
              list[0].remove( 0, 8 );
              list[0] = list[0].trimmed().toUpper(); // icao name of airfield
              list[1] = list[1].trimmed(); // new map type for airfield
              c_icaoMap.insert( list[0], list[1] );
              // qDebug("W2000: c_icaoMap.insert(%s, %s)", list[0].toLatin1().data(), list[1].toLatin1().data());
            }
          else if( list[0].contains("MAP_SHORT_NAME",Qt::CaseInsensitive) )
            {
              list[0].remove( 0, 14 );
              list[0] = list[0].trimmed(); // short name of airfield
              list[1] = list[1].trimmed(); // new map type for airfield
              c_shortMap.insert( list[0], list[1] );
              // qDebug("W2000: c_shortMap.insert(%s, %s)", list[0].toLatin1().data(), list[1].toLatin1().data());
            }
        }

    } // End of while( ! in.atEnd() )

  in.close();

  return true;
}

/**
 * Parses the passed file in Welt2000 format and put the appropriate
 * entries in the related lists.
 *
 * arg1 path: Full name with path of welt2000 file
 * arg2 airfieldList: All airfields have to be stored in this list
 * arg3 glidertList: All gilder fields have to be stored in this list
 * arg4 glidertList: All outlanding fields have to be stored in this list, when
 *                   the outlanding option is set in the user configuration
 * arg5 doCompile: create a binary file of the parser results,
 *                 if flag is set to true. Default is false.
 * returns true (success) or false (error occurred)
 */
bool Welt2000::parse( QString& path,
                      QList<Airfield>& airfieldList,
                      QList<Airfield>& gliderfieldList,
                      QList<Airfield>& outlandingList,
                      bool doCompile )
{
  QTime t;
  t.start();

#if 0
  // Filter out the needed extract for MAEMO from the Welt2000 file. That will
  // reduce the file size over the half and shorten later reads.
  if( filter( path ) == false )
    {
      // It seems, that no Welt2000 file has been passed
      return false;
    }
#endif

  QFile in(path);

  if( !in.open(QIODevice::ReadOnly) )
    {
      qWarning("W2000: Cannot open file %s!", path.toLatin1().data());
      return false;
    }

  QTextStream ins(&in);
  ins.setCodec( "ISO 8859-15" );

  // look, if a configuration file is accessible. If yes read out its data.
  QFileInfo fi( path );
  QString confFile = fi.path() + "/welt2000.conf";

  // It is expected that the filter file is located in the same
  // directory as the welt2000.txt file and carries the name
  // welt2000.conf
  readConfigEntries( confFile );

  // Check, if in GeneralConfig other definitions exist. These will
  // overwrite the definitions in the configuration file.
  GeneralConfig *conf  = GeneralConfig::instance();
  QString cFilter      = conf->getWelt2000CountryFilter();

  if( cFilter.length() > 0 )
    {
      // load new country filter definitions
      c_countryList.clear();

      QStringList clist = cFilter.split( QRegExp("[, ]"), QString::SkipEmptyParts );

      for( int i = 0; i < clist.count(); i++ )
        {
          QString e = clist[i].trimmed().toUpper();

          if( c_countryList.contains(e) )
            {
              continue;
            }

          c_countryList += e;
        }

      c_countryList.sort();
    }

  // get outlanding load flag from configuration data
  bool outlandings = conf->getWelt2000LoadOutlandings();

  // Get home radius from configuration data in kilometers
  c_homeRadius = conf->getAirfieldHomeRadius() / 1000.;

  if( cFilter.isEmpty() && c_homeRadius == 0.0 )
    {
      // If the country filter is empty and no home radius is defined,
      // we set a default home radius of 500Km.
      c_homeRadius = 500.0;
    }

  float c_runwayLengthFilter = GeneralConfig::instance()->getAirfieldRunwayLengthFilter();

  qDebug() << "W2000: Country Filter:" << c_countryList;
  qDebug() << "W2000: Load Outlandings?" << outlandings;
  qDebug( "W2000: Home Radius: %.1f Km", c_homeRadius );
  qDebug( "W2000: Runway length filter: %.0f m", c_runwayLengthFilter );

  // put all entries of country list into a dictionary for faster access
  QHash<QString, QString> countryDict;

  for( int i = 0; i < c_countryList.count(); i++ )
    {
      // populate country dictionary
      countryDict.insert( c_countryList[i], c_countryList[i] );
    }

  // Prepare all for a binary storage of the parser results.
  QString compileFile;
  QFile   compFile;
  QDataStream out;
  QByteArray bufdata;
  QBuffer buffer(&bufdata);
  QDataStream outbuf;

  if( doCompile )
    {
      compileFile = fi.path() + "/welt2000.txc";
      compFile.setFileName( compileFile );
      out.setDevice( &compFile );
      out.setVersion( QDataStream::Qt_4_7 );

      if( !compFile.open(QIODevice::WriteOnly) )
        {
          // Can't open output file, reset compile flag and parse the
          // original file as alternative.
          qWarning("W2000: Cannot open file %s!", compileFile.toLatin1().data());
          doCompile = false;
        }
      else
        {
          // create and prepare out buffer and the stream to it
          buffer.open(QIODevice::ReadWrite);
          outbuf.setDevice(&buffer);
        }
    }

#ifdef BOUNDING_BOX
  QRect boundingBox;
#endif

  uint lineNo = 0;
  QSet<QString> shortNameSet; // contains all short names already in use

  // Contains the coordinates of the objects put in the lists. Used as filter
  // to avoid multiple entries at the same point.
  QSet<QString> pointFilter;
  uint counter = 0;

  // statistics counter
  uint ul, gl, af, ol;
  ul = gl = af = ol = 0;

  // Input file was taken from Michael Meiers Welt2000 data base.
  //
  // 0         1         2         3         4         5         6
  // 0123456789012345678901234567890123456789012345678901234567890123
  // 1234567890123456789012345678901234567890123456789012345678901230
  // AACHE1 AACHEN  MERZBRUC#EDKAA 53082612287 189N504923E0061111DEO5
  // AICHA1 AICHACH         # S !G 43022012230 440N482824E0110807DEX
  // ARGEN2 ARGENBUEHL EISE?*ULM G 40082612342 686N474128E0095744DEN
  // BASAL2 BAD SALZUNGEN UL*ULM G 65092712342 233N504900E0101305DEN
  // FUERS1 FUERSTENWALDE   #EDALG 80112912650  55N522323E0140539DEO3
  // German international airport, ICAO starts with EDD
  // BERLT1 BERLIN  TEGEL   #EDDTA303082611870  37N523335E0131716DEO
  // BERSC1 BERLIN SCHOENFEL#EDDBC300072512002  49N522243E0133114DEO
  // German military airport, ICAO starts with ET
  // HOLZD1 HOLZDORF MIL    #ETSHA242092712210  82N514605E0131003DEQ0
  // UL Fields new coding variant
  // SIEWI1 SIEWISCH UL    !# ULMG 51082612342  89N514115E0141231DEO0
  // OUTLANDING EXAMPLES
  // ESPIN2 ESPINASSES     !*FL10S 3509271     648N442738E0061305FRQ0
  // BAERE2 BAERENTAL       *FELDS 2505231     906N475242E0080643DEO3
  // BAIBR2 BAIERSBRON CLS  *WIESG 2317351     507N483217E0082354DEM5
  // BAIYY2 BAIERSBRONN     *FL03S 2205231     574N483056E0082224DEO0
  // DAMGA2 DAMGARTEN CLS   *   !C200072512150   5N541551E0122640DEE0
  // PIEVE2 PIEVERSTORF 25M *AGR!A 3208261      27N534906E0110841DEX0

  while( ! in.atEnd() )
    {
      bool ok, ok1;
      QString line, buf;
      line = in.readLine(128);
      lineNo++;

      if( line.isEmpty() )
        {
          continue;
        }

      // step over comment or invalid lines
      if( line.startsWith("#") || line.startsWith("$") ||
          line.startsWith("\t") || line.startsWith(" ") )
        {
          continue;
        }

      // remove white spaces and line end characters
      line = line.trimmed();

      // replace markers against space
      line.replace( QRegExp("[!?]+"), " " );

      if( line.length() < 62 )
        {
          // country sign not included
          continue;
        }

      // get short name for user mapping before changing line
      QString shortName = line.mid( 0, 6 );

      // convert all to toUpper case
      line = line.toUpper();

      // Extract country sign. It is coded according to ISO 3166.
      QString country = line.mid( 60, 2 );

      if( ! countryDict.isEmpty() )
        {
          if( ! countryDict.contains(country) )
            {
              continue;
            }
        }

      // look, what kind of line was read.
      // COL5 = 1 Airfield or also UL site
      // COL5 = 2 Outlanding, contains also UL places
      QString kind = line.mid( 5, 1 );

      if( kind != "1" && kind != "2" )
        {
          continue; // not of interest for us
        }

      bool ulField = false;
      bool glField = false;
      bool afField = false;
      bool olField = false;

      QString commentShort;
      QString commentLong;
      QString icao;

      if( kind == "2" ) // can be an UL field
        {
          if( line.mid( 23, 4 ) == "*ULM" )
            {
              ulField = true;
            }
          else
            {
              // outlanding found
              if( outlandings == false )
                {
                  // ignore outlandings
                  continue;
                }

              olField = true;
              commentShort = line.mid( 24, 4 );
              commentShort.replace(QRegExp("[!?]+"), " " );
              commentShort = commentShort.toUpper().trimmed();

              if( commentShort.startsWith( "FL" ) )
                {
                  commentLong = QString( QObject::tr("Emergency Field No: ")) +
                                commentShort.mid( 2, 2 );
                }
            }
        }
      else if( line.mid( 23, 4 ) == "#GLD" )
        {
          // Glider field
          glField = true;
        }
      else if( line.mid( 23, 5 ) == "# ULM" )
        {
          // newer coding for UL field
          ulField = true;
        }
      else
        {
          afField = true;
          icao = line.mid( 24, 4 ).trimmed().toUpper();

          if( line.mid( 20, 4 ) == "GLD#" )
            {
              // other possibility for a glider field with ICAO code
              glField = true;
            }
        }

      // Airfield name
      QString afName = line.mid( 7, 16 );

      // remove special mark signs
      afName.replace( QRegExp("[!?]+"), "" );

      // remove resp. replace white spaces against one space
      afName = afName.simplified();

      if( afName.length() == 0 )
        {
          qWarning( "W2000, Line %d: Airfield name is undefined, ignoring entry!",
                    lineNo );
          continue;
        }

      // airfield type
      BaseMapElement::objectType afType = BaseMapElement::NotSelected;

      // determine airfield type so good as possible
      if( ulField == true )
        {
          afType = BaseMapElement::UltraLight;
        }
      else if( glField == true )
        {
          afType = BaseMapElement::Gliderfield;
        }
      else if( olField == true )
        {
          afType = BaseMapElement::Outlanding;
        }
      else if( afField == true )
        {
          if( icao.startsWith("ET") )
            {
              // German military airport
              afType = BaseMapElement::MilAirport;
            }
          else if( afName.contains(QRegExp(" MIL$")) )
            {
              // should be an military airport but not 100% sure
              afType = BaseMapElement::MilAirport;
            }
          else if( icao.startsWith("EDD") )
            {
              // German international airport
              afType = BaseMapElement::IntAirport;
            }
          else
            {
              afType = BaseMapElement::Airfield;
            }
        }

      // make the user's desired mapping for short name
      if( c_shortMap.contains(shortName) )
        {
          QString val = c_shortMap[shortName];

          if( c_baseTypeMap.contains(val) )
            {
              afType = c_baseTypeMap[val];
            }
        }

      // make the user's wanted mapping for icao
      if( ! icao.isEmpty() && c_icaoMap.contains(icao) )
        {
          QString val = c_icaoMap[icao];

          if( c_baseTypeMap.contains(val) )
            {
              afType = c_baseTypeMap[val];
            }
        }

      // airfield name
      afName = afName.toLower();

      QChar lastChar(' ');

      // convert airfield names to upper-lower
      for( int i=0; i < afName.length(); i++ )
        {
          if( lastChar == ' ' )
            {
              afName.replace( i, 1, afName.mid(i,1).toUpper() );
            }

          lastChar = afName[i];
        }

      // gps name, we use 8 characters without spaces
      QString gpsName = afName;
      gpsName.remove(QChar(' '));
      gpsName = gpsName.left(8);

      if( ! shortNameSet.contains( gpsName) )
        {
          shortNameSet.insert( gpsName );
        }
      else
        {
          // Try to generate an unique short name. The assumption is that we never have
          // more than 10 equal names.
          for( int i=0; i <= 9; i++ )
            {
              gpsName.replace( gpsName.length()-1, 1, QString::number(i) );

              if( ! shortNameSet.contains( gpsName) )
                {
                  shortNameSet.insert( gpsName );
                  break;
                }
            }
        }

      if( ulField  )
        {
          if( afName.right(3) == " Ul" )
            {
              // Convert lower l of Ul to upper case
              afName.replace( afName.length()-1, 1, "L" );
            }
          else
            {
              // append UL substring
              // afName += " UL";
            }
        }

      qint32 lat, lon;
      QString degree, min, sec;
      double d, m, s;

      // convert latitude
      degree = line.mid(46,2);
      min    = line.mid(48,2);
      sec    = line.mid(50,2);

      d = degree.toDouble(&ok);

      if( ! ok )
        {
#ifndef MAEMO
          qWarning( "W2000, Line %d: %s (%s) wrong latitude degree value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
#ifndef MAEMO
          qWarning( "W2000, Line %d: %s (%s) wrong latitude minute value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
#ifndef MAEMO
          qWarning( "W2000, Line %d: %s (%s) wrong latitude second value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
          continue;
        }

      double latTmp = (d * 600000.) + (10000. * (m + s / 60. ));

      lat = (qint32) rint(latTmp);

      if( line[45] == 'S' )
        {
          lat = -lat;
        }

      // convert longitude
      degree = line.mid(53,3);
      min    = line.mid(56,2);
      sec    = line.mid(58,2);

      d = degree.toDouble(&ok);

      if( ! ok )
        {
#ifndef MAEMO
          qWarning( "W2000, Line %d: %s (%s) wrong longitude degree value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
#ifndef MAEMO
          qWarning( "W2000, Line %d: %s (%s) wrong longitude minute value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
#ifndef MAEMO
          qWarning( "W2000, Line %d: %s (%s) wrong longitude second value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
          continue;
        }

      double lonTmp = (d * 600000.) + (10000. * (m + s / 60. ));

      lon = (qint32) rint(lonTmp);

      if( line[52] == 'W' )
        {
          lon = -lon;
        }

      if( c_homeRadius > 0.0 )
        {
          // Home radius filter is defined, we will
          // compute the distance between the home position and the
          // read point. Is the distance is over the user defined
          // value away we will ignore this point.

          QPoint home = _globalMapMatrix->getHomeCoord();
          QPoint af( lat, lon );

          double d = dist( &home, &af );

          if( d > c_homeRadius )
            {
              // Distance is greater than defined radius in GeneralConfig
              // qDebug("Ignoring Dist=%f, AF=%s", d, afName.toLatin1().data());
              continue;
            }
        }

#ifdef BOUNDING_BOX
      // update the bounding box
      _globalMapContents->AddPointToRect( boundingBox, QPoint(lat, lon) );
#endif

      WGSPoint wgsPos(lat, lon);

      // We do check here, if the coordinates of the object are already known to
      // filter out multiple entries. Only the first entry do pass the filter.
      QString corrString = WGSPoint::coordinateString( wgsPos );

      if( pointFilter.contains( corrString ) )
        {
          // An object with the same coordinates do already exist.
          // We do ignore this one.
          qWarning( "W2000, Line %d: %s (%s) skipping entry, coordinates already in use!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      // store coordinates in filter
      pointFilter.insert( corrString );

      QPoint position = _globalMapMatrix->wgsToMap(wgsPos);

      // elevation
      buf = line.mid(41,4 ).trimmed();

      ok = false;
      qint16 elevation = 0;

      if( ! buf.isEmpty() )
        {
          elevation = buf.toInt(&ok);
        }

      if( ! ok )
        {
#ifndef MAEMO
          qWarning( "W2000, Line %d: %s (%s) missing or wrong elevation, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
          elevation = 0;
        }

      // frequency
      QString frequency = line.mid(36,3) + "." + line.mid(39,2).trimmed();

      float fFrequency = frequency.toFloat(&ok);

      if( ( !ok || fFrequency < 108 || fFrequency > 137.0 ) )
        {
          if( olField == false )
            {
              // Don't display warnings for outlandings
#ifndef MAEMO
              qWarning( "W2000, Line %d: %s (%s) missing or wrong frequency, set value to 0!",
                        lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
             }

          fFrequency = 0.0; // reset frequency to unknown
        }
      else
        {
          // check, what has to be appended as last digit
          if( line[40] == '2' || line[40] == '7' )
            {
              fFrequency += 0.005;
            }
        }

      /* Runway description from Welt2000.txt file
       *
       * A: 08/26 MEANS THAT THERE IS ONLY ONE RUNWAYS 08 AND (26=08 + 18)
       * B: 17/07 MEANS THAT THERE ARE TWO RUNWAYS,
       *          BUT 17 IS THE MAIN RWY SURFACE LENGTH
       * C: IF BOTH DIRECTIONS ARE IDENTICAL (04/04),
       *    THIS DIRECTION IS STRONGLY RECOMMENDED
       */

      // runway direction have two digits, we consider both directions
      buf = line.mid(32,2).trimmed();

      ok = false;
      ok1 = false;

      ushort rwDir = 0;
      ushort rwDir1 = 0;
      ushort rwDir2 = 0;

      if( ! buf.isEmpty() )
        {
          rwDir1 = buf.toUShort(&ok);
        }

      // extract second direction
      buf = line.mid(34,2).trimmed();

      if( ! buf.isEmpty() )
        {
          rwDir2 = buf.toUShort(&ok1);
        }

      if( ! ok || ! ok1 || rwDir1 < 1 || rwDir1 > 36 || rwDir2 < 1 || rwDir2 > 36 )
        {
#ifndef MAEMO
          qWarning( "W2000, Line %d: %s (%s) missing or wrong runway direction, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
          rwDir = rwDir1 = rwDir2 = 0;
        }

      // Put both directions together in one variable, first direction in the
      // upper part.
      rwDir = rwDir1*256 + rwDir2;

      // runway length in deka-meters, must be multiplied by 10 to get meters
      buf = line.mid(29,3).trimmed();

      ok = false;
      float rwLen = 0.0;

      if( ! buf.isEmpty() )
        {
          rwLen = buf.toFloat(&ok);
        }

      if( ! ok )
        {
#ifndef MAEMO
          qWarning( "W2000, Line %d: %s (%s) missing or wrong runway length, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
#endif
          rwLen = 0.0;
        }
      else
        {
          rwLen *= 10.0;
        }

      if( c_runwayLengthFilter > 0.0 && rwLen < c_runwayLengthFilter )
        {
          qDebug( "W2000, Line %d: RWY Filter, %s (%s) RWY %.0fm too short!",
                  lineNo, afName.toLatin1().data(), country.toLatin1().data(), rwLen );
          continue;
        }

      // runway surface
      ushort rwSurface;
      QChar rwType = line[28];

      if( rwType == 'A' )
        {
          rwSurface = Runway::Asphalt;
        }
      else if( rwType == 'C' )
        {
          rwSurface = Runway::Concrete;
        }
      else if( rwType == 'G' )
        {
          rwSurface = Runway::Grass;
        }
      else if( rwType == 'S' )
        {
          rwSurface = Runway::Sand;
        }
      else
        {
          rwSurface = Runway::Unknown;
        }

      //---------------------------------------------------------------
      // append a new record to the related list
      //---------------------------------------------------------------

      // count output records separated by kind
      if( ulField )
        {
          ul++;
        }
      else if( glField )
        {
          gl++;
        }
      else if( olField )
        {
          ol++;
        }
      else
        {
          af++;
        }

      /* Runway description from Welt2000.txt file
       *
       * A: 08/26 MEANS THAT THERE IS ONLY ONE RUNWAYS 08 AND (26=08 + 18)
       * B: 17/07 MEANS THAT THERE ARE TWO RUNWAYS,
       *          BUT 17 IS THE MAIN RWY SURFACE LENGTH
       * C: IF BOTH DIRECTIONS ARE IDENTICAL (04/04),
       *    THIS DIRECTION IS STRONGLY RECOMMENDED
       */

      // create the runway objects and store them in the list
      QList<Runway> rwyList;
      Runway rwy( rwLen, rwDir, rwSurface, true );

      // Check, how many runways do we have
      if( rwDir == 0 )
        {
          // Runway directions undefined
          rwyList.append( rwy );
        }
      else if( rwDir1 == rwDir2 || abs(rwDir1-rwDir2) == 18 )
        {
          // WE have only one runway
          rwyList.append( rwy );
        }
      else
        {
          // We have two runways
          int inverseDir = rwDir1 > 18 ? rwDir1-18 : rwDir1 + 18;
          rwy.m_heading = rwDir1*256 + inverseDir;
          rwyList.append( rwy );

          inverseDir = rwDir2 > 18 ? rwDir2-18 : rwDir2 + 18;
          rwy.m_heading = rwDir2*256 + inverseDir;
          rwyList.append( rwy );
        }

      Airfield af( afName, icao.trimmed(), gpsName, afType,
                   wgsPos, position, rwyList, elevation, fFrequency,
                   country, commentLong );

      if( afType == BaseMapElement::Outlanding )
        {
          // Add an outlanding site to the list.
          outlandingList.append( af );
        }
      else if( afType == BaseMapElement::Gliderfield )
        {
          // Add a glider site to the related list.
          gliderfieldList.append( af );
        }
      else
        {
          // Add an airfield or an ultralight field to the list
          airfieldList.append( af );
        }

      if( doCompile )
        {
          counter++;
          // airfield type
          outbuf << quint8( afType );
          // airfield name with country
          ShortSave(outbuf, afName.toUtf8());
          // icao
          ShortSave(outbuf, icao.trimmed().toUtf8());
          // GPS name
          ShortSave(outbuf, gpsName.toUtf8());
          // WGS84 coordinates
          outbuf << wgsPos;
          // projected WGS84 coordinates
          outbuf << position;
          // elevation in meters
          outbuf << qint16( elevation);
          // frequency written as e.g. 126.575, is reduced to 16 bits
          if( fFrequency == 0.0 )
            {
              outbuf << quint16(0);
            }
          else
            {
              outbuf << quint16( rint((fFrequency - 100.0) * 1000.0 ));
            }

          // two runway directions packed in a word
          outbuf << quint16(rwDir);
          // runway length in meters
          outbuf << quint16(rwLen);
          // runway surface
          outbuf << quint8(rwSurface);
          // comment
          ShortSave(outbuf, commentShort.toUtf8());
          // country
          ShortSave(outbuf, country.toUtf8());
        }

    } // End of while( ! in.atEnd() )

  in.close();

  if( doCompile )
    {
      buffer.close(); // close our output buffer

      if( counter )
        {
          qDebug("W2000: writing file %s", compileFile.toLatin1().data());

          // entries are contained in buffer, write all into the compiled file
          out << quint32( KFLOG_FILE_MAGIC );
          out << qint8( FILE_TYPE_AIRFIELD_C );
          out << quint16( FILE_VERSION_AIRFIELD_C );
          out << QDateTime::currentDateTime();
          out << c_countryList;
          out << c_homeRadius;
          out << c_runwayLengthFilter;
          out << QPoint( _globalMapMatrix->getHomeCoord() ); // home position
          out << outlandings;

#ifdef BOUNDING_BOX
          // boundingbox is never used during read in, we don't need to write out it
          // qDebug("Bounding box is: (%d, %d),(%d, %d)",
          // boundingBox.left(), boundingBox.top(), boundingBox.right(), boundingBox.bottom());
          out << boundingBox;
#endif

          SaveProjection( out, _globalMapMatrix->getProjection() );

          // write data counters to file
          out << quint32( af );
          out << quint32( gl );
          out << quint32( ul );
          out << quint32( ol );

          // write data on airfields from buffer into the file
          out << bufdata;
          compFile.close();
        }
      else
        {
          // no entries are contained in buffer, remove output file
          compFile.close();
          QFile::remove( compileFile );
        }
    }

  qDebug( "W2000, Statistics from file %s: Parsing Time=%dms, Sum=%d, Airfields=%d, GL=%d, UL=%d, OL=%d",
          basename(path.toLatin1().data()), t.elapsed(), af+gl+ul+ol, af, gl, ul, ol );

  return true;
}

/**
 * Read the content of a compiled file and put it into the related
 * lists.
 */
bool Welt2000::readCompiledFile( QString &path,
                                 QList<Airfield>& airfieldList,
                                 QList<Airfield>& gliderfieldList,
                                 QList<Airfield>& outlandingList )
{
  QTime t;
  t.start();

  // get outlanding load flag from configuration data
  bool loadOls = GeneralConfig::instance()->getWelt2000LoadOutlandings();

  QFile inFile(path);

  if( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("W2000: Cannot open airfield file %s!", path.toLatin1().data());
      return false;
    }

  QDataStream in(&inFile);
  in.setVersion( QDataStream::Qt_4_7 );

  quint32 magic;
  qint8 fileType;
  quint16 fileVersion;
  QDateTime creationDateTime;
  QStringList countryList;
  double homeRadius;
  float runwayLengthFilter;
  QPoint homeCoord;
  bool outlandings;

#ifdef BOUNDING_BOX
  QRect boundingBox;
#endif

  ProjectionBase *projectionFromFile;

  // Data counter
  quint32 af;
  quint32 gl;
  quint32 ul;
  quint32 ol;

  qint32 buflen;

  quint8 afType;
  QString afName;
  QString icao;
  QString gpsName;
  WGSPoint wgsPos;
  QPoint position;
  qint16 elevation;
  quint16 inFrequency;
  quint16 rwDir; // 0...36, one value in every byte
  quint16 rwLen;
  quint8 rwSurface;
  QByteArray utf8_temp;
  QString comment;
  QString country;
  float frequency;

  in >> magic;

  if( magic != KFLOG_FILE_MAGIC )
    {
      qWarning( "W2000: wrong magic key %x read! Aborting ...", magic );
      inFile.close();
      return false;
    }

  in >> fileType;

  if( fileType != FILE_TYPE_AIRFIELD_C )
    {
      qWarning( "W2000: wrong file type %x read! Aborting ...", fileType );
      inFile.close();
      return false;
    }

  in >> fileVersion;

  if( fileVersion != FILE_VERSION_AIRFIELD_C )
    {
      qWarning( "W2000: wrong file version %x read! Aborting ...", fileVersion );
      inFile.close();
      return false;
    }

  in >> creationDateTime;
  in >> countryList;
  in >> homeRadius;
  in >> runwayLengthFilter;
  in >> homeCoord;
  in >> outlandings;

  if( loadOls == true && outlandings == false )
    {
      // We should load outlandings but there are not contained in the
      // compiled file.
      qWarning( "W2000: compiled file contains no outlandings! Aborting ..." );
      inFile.close();
      return false;
    }

#ifdef BOUNDING_BOX
  in >> boundingBox;
#endif

  projectionFromFile = LoadProjection(in);

  // projectionFromFile is allocated dynamically, we don't need it
  // here. Therefore it is immediately deleted to avoid memory leaks.
  delete projectionFromFile;
  projectionFromFile = 0;

  // read element counters
  in >> af;
  in >> gl;
  in >> ul;
  in >> ol;

  // Preallocate list memory
  if( af || ul )
    {
      airfieldList.reserve( airfieldList.size() + af + ul );
    }

  if( gl )
    {
      gliderfieldList.reserve( gliderfieldList.size() + gl );
    }

  if( ol )
    {
      outlandingList.reserve( gliderfieldList.size() + ol );
    }

  // Because we're used a buffer during output, the buffer length will
  // be written too. Now we have to read it but don't need it because
  // we access directly to the opened file.
  in >> buflen;

  uint counter = 0;

  while( ! in.atEnd() )
    {
      counter++;
      in >> afType;
      ShortLoad(in, utf8_temp);
      afName=QString::fromUtf8(utf8_temp);

      ShortLoad(in, utf8_temp);
      icao=QString::fromUtf8(utf8_temp);
      ShortLoad(in, utf8_temp);
      gpsName=QString::fromUtf8(utf8_temp);
      in >> wgsPos;
      in >> position;
      in >> elevation;
      in >> inFrequency;

      if( inFrequency == 0 )
        {
          frequency = 0.0;
        }
      else
        {
          frequency = (((float) inFrequency) / 1000.0) + 100.;
        }

      in >> rwDir;
      in >> rwLen;
      in >> rwSurface;
      // create an runway object
      Runway rwy( static_cast<float>(rwLen) ,rwDir, rwSurface, 1 );

      // read comment
      ShortLoad(in, utf8_temp);
      comment = QString::fromUtf8(utf8_temp);

      if( comment.startsWith( "FL" ) )
        {
          comment = QString( QObject::tr("Emergency Field No: ")) +
                    comment.mid( 2, 2 );
        }

      // read the 2 letter country code
      ShortLoad(in, utf8_temp);
      country = QString::fromUtf8(utf8_temp);

      if( loadOls == false && afType == BaseMapElement::Outlanding )
        {
          // do not load outlandings
          continue;
        }

      QList<Runway> rwyList;

      int rwDir1 = rwDir/256;
      int rwDir2 = rwDir%256;

      // Check, how many runways do we have
      if( rwDir == 0 )
        {
          // Runway directions undefined
          rwyList.append( rwy );
        }
      else if( rwDir1 == rwDir2 || abs(rwDir1-rwDir2) == 18 )
        {
          // WE have only one runway
          rwyList.append( rwy );
        }
      else
        {
          // We have two runways
          int inverseDir = rwDir1 > 18 ? rwDir1-18 : rwDir1 + 18;
          rwy.m_heading = rwDir1*256 + inverseDir;
          rwyList.append( rwy );

          inverseDir = rwDir2 > 18 ? rwDir2-18 : rwDir2 + 18;
          rwy.m_heading = rwDir2*256 + inverseDir;
          rwyList.append( rwy );
        }

      Airfield af( afName, icao, gpsName, (BaseMapElement::objectType) afType,
                   wgsPos, position, rwyList, elevation, frequency, country, comment );

      if( afType == BaseMapElement::Gliderfield )
        {
        // Add a glider site to the list.
          gliderfieldList.append( af );
        }
      else if( afType == BaseMapElement::Outlanding )
        {
          // Add an outlanding site to the list.
          outlandingList.append( af );
        }
      else
        {
          // Add an airfield site to the list.
          airfieldList.append( af );
        }
    }

  inFile.close();

  qDebug( "W2000: %d airfields read from %s in %dms",
          counter, basename(path.toLatin1().data()), t.elapsed() );

  return true;
}

/**
 * Get the header data of a compiled file and put it in our class
 * variables.
 */
bool Welt2000::setHeaderData( QString &path )
{
  h_headerIsValid = false; // save read result here too

  h_magic = 0;
  h_fileType = 0;
  h_fileVersion = 0;
  h_countryList.clear();
  h_homeRadius = 0.0;
  h_runwayLengthFilter = 0.0;
  h_homeCoord.setX(0);
  h_homeCoord.setY(0);
  h_outlandings = false;

  if( h_projection )
    {
      // delete an older projection object
      delete  h_projection;
      h_projection = 0;
    }

  QFile inFile(path);
  if( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("W2000: Cannot open airfield file %s!", path.toLatin1().data());
      return false;
    }

  QDataStream in(&inFile);
  in.setVersion( QDataStream::Qt_4_7 );

  in >> h_magic;

  if( h_magic != KFLOG_FILE_MAGIC )
    {
      qWarning( "W2000: wrong magic key %x read! Aborting ...", h_magic );
      inFile.close();
      return false;
    }

  in >> h_fileType;

  if( h_fileType != FILE_TYPE_AIRFIELD_C )
    {
      qWarning( "W2000: wrong file type %x read! Aborting ...", h_fileType );
      inFile.close();
      return false;
    }

  in >> h_fileVersion;

  if( h_fileVersion != FILE_VERSION_AIRFIELD_C )
    {
      qWarning( "W2000: wrong file version %x read! Aborting ...", h_fileVersion );
      inFile.close();
      return false;
    }

  in >> h_creationDateTime;
  in >> h_countryList;
  in >> h_homeRadius;
  in >> h_runwayLengthFilter;
  in >> h_homeCoord;
  in >> h_outlandings;

#ifdef BOUNDING_BOX

  in >> h_boundingBox;
#endif

  h_projection = LoadProjection(in);

  inFile.close();
  h_headerIsValid = true; // save read result here too
  return true;
}

/*-------------------------Welt2000Thread-------------------------------------*/

#include <csignal>

Welt2000Thread::Welt2000Thread( QObject *parent ) : QThread( parent )
{
  setObjectName( "Welt2000Thread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

Welt2000Thread::~Welt2000Thread()
{
}

void Welt2000Thread::run()
{
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  // Check is signal is connected to a slot.
  if( receivers( SIGNAL( loadedLists( bool,
                                      QList<Airfield>*,
                                      QList<Airfield>*,
                                      QList<Airfield>*  )) ) == 0 )
    {
      qWarning() << "Welt2000Thread: No Slot connection to Signal loadedLists";
      return;
    }

  QList<Airfield>* airfieldList    = new QList<Airfield>;
  QList<Airfield>* gliderfieldList = new QList<Airfield>;
  QList<Airfield>* outlandingList  = new QList<Airfield>;

  Welt2000 welt2000;

  bool ok = welt2000.load( *airfieldList, *gliderfieldList, *outlandingList );

  /* It is expected that a receiver slot is connected to this signal. The
   * receiver is responsible to delete the passed lists. Otherwise a big
   * memory leak will occur.
   */
  emit loadedLists( ok, airfieldList, gliderfieldList, outlandingList );
}
