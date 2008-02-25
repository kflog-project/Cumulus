/***********************************************************************
 **
 **   welt2000.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2006-2008 by Axel Pauli, axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <math.h>
#include <unistd.h>

#include <QBuffer>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>
#include <QByteArray>
#include <QHash>

#include "airport.h"
#include "basemapelement.h"
#include "filetools.h"
#include "glidersite.h"
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


// type definition for compiled airfield files
#define FILE_TYPE_AIRFIELD_C 0x62

// version used for files created from welt2000 data
#define FILE_VERSION_AIRFIELD_C 201

extern MapContents*  _globalMapContents;
extern MapMatrix*    _globalMapMatrix;

Welt2000::Welt2000()
{
  h_projection = (ProjectionBase *) 0;

  // prepare base mappings of cumulus
  c_baseTypeMap.insert( "IntAirport", BaseMapElement::IntAirport );
  c_baseTypeMap.insert( "Airport", BaseMapElement::Airport );
  c_baseTypeMap.insert( "MilAirport", BaseMapElement::MilAirport );
  c_baseTypeMap.insert( "CivMilAirport", BaseMapElement::CivMilAirport );
  c_baseTypeMap.insert( "Airfield", BaseMapElement::Airfield );
  c_baseTypeMap.insert( "ClosedAirfield", BaseMapElement::ClosedAirfield );
  c_baseTypeMap.insert( "CivHeliport", BaseMapElement::CivHeliport );
  c_baseTypeMap.insert( "MilHeliport", BaseMapElement::MilHeliport );
  c_baseTypeMap.insert( "AmbHeliport", BaseMapElement::AmbHeliport );
  c_baseTypeMap.insert( "Glidersite", BaseMapElement::Glidersite );
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
 * be the original ascii file or a compiled version of it. The results
 * are put in the passed lists
 */
bool Welt2000::load( MapElementList& airportList, MapElementList& gliderList )
{
  // Rename WELT2000.TXT -> welt2000.txt.
  QString wl = "welt2000.txt";
  QString wu = "WELT2000.TXT";
  QString sd = "/airfields/";

  QString p1l = MapContents::mapDir1 + sd + wl;
  QString p1u = MapContents::mapDir1 + sd + wu;
  rename( p1u.toLatin1().data(), p1l.toLatin1().data() );

  QString p2l = MapContents::mapDir2 + sd + wl;
  QString p2u = MapContents::mapDir2 + sd + wu;
  rename( p2u.toLatin1().data(), p2l.toLatin1().data() );

  QString p3l = MapContents::mapDir3 + sd + wl;
  QString p3u = MapContents::mapDir3 + sd + wu;
  rename( p3u.toLatin1().data(), p3l.toLatin1().data() );

  QString w2PathTxt;
  QString w2PathTxc;

  // Search for welt2000 source resp. compiled files.
  bool resTxt = MapContents::locateFile( "airfields/welt2000.txt", w2PathTxt );
  bool resTxc = MapContents::locateFile( "airfields/welt2000.txc", w2PathTxc );

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
          unlink( w2PathTxc.toLatin1().data() );
          return parse( w2PathTxt, airportList, gliderList, true );
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
              unlink( w2PathTxc.toLatin1().data() );
              return parse( w2PathTxt, airportList, gliderList, true );
            }
        }

      // Check against the config file
      QFileInfo fi(w2PathTxt);
      QString path = fi.dirPath(true) + "/welt2000.conf";
      QFileInfo fiConf(path);

      if( fiConf.exists() && fi.isReadable() &&
          h_creationDateTime < fiConf.lastModified() )
        {
          qDebug( "W2000: welt2000.conf has been changed --> reparse welt2000.txt" );
          // Conf file was modified, make a new compilation. It is not
          // deeper checked, what was modified due to the effort and
          // in the assumption that a config file will not be changed
          // every minute.
          unlink( w2PathTxc.toLatin1().data() );
          return parse( w2PathTxt, airportList, gliderList, true );
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
          unlink( w2PathTxc.toLatin1().data() );
          return parse( w2PathTxt, airportList, gliderList, true );
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
              unlink( w2PathTxc.toLatin1().data() );
              return parse( w2PathTxt, airportList, gliderList, true );             
            }
        }
      else
        {
          // No country definitions are contained in the compiled
          // file. File was created by using home radius. Check, if
          // home position has been changed in the meantime. In this
          // case a new parsing of the source must be started.
          QPoint curHome = _globalMapMatrix->getHomeCoord();

          if( curHome != h_homeCoord )
            {
              qDebug( "W2000: Home coordinates have been changed --> reparse welt2000.txt" );
              // Home coordinates have been changed, make a reparsing of
              // source file
              unlink( w2PathTxc.toLatin1().data() );
              return parse( w2PathTxt, airportList, gliderList, true );
            }

          // Furthermore we do check, if the home radius has been
          // changed in the mean time. In this case a new parsing of
          // the source must be started.

          // get home radius from config data
          int iRadius = GeneralConfig::instance()->getWelt2000HomeRadius();
          double dRadius;

          if( iRadius == 0 )
            {
              // default is 1000 kilometers around home position
              dRadius = 1000.0;
            }
          else
            {
              // we must look, what unit the user has choosen. This unit must
              // be considered during load of airfield data.
              dRadius = getDistanceInKm( iRadius );
            }

          if( fabs( dRadius - h_homeRadius ) > 0.5 )
            {
              qDebug( "W2000: Home radius has been changed --> reparse welt2000.txt" );
              // Home radius has been changed, make a reparsing of
              // source file
              unlink( w2PathTxc.toLatin1().data() );
              return parse( w2PathTxt, airportList, gliderList, true );
            }
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

          unlink( w2PathTxc.toLatin1().data() );
          return parse( w2PathTxt, airportList, gliderList, true );
        }

      // Nothing has been changed, read in compiled file
      if( ! readCompiledFile( w2PathTxc, airportList, gliderList ) )
        {
          // reading of compiled file failed, let's parse the source
          unlink( w2PathTxc.toLatin1().data() );
          return parse( w2PathTxt, airportList, gliderList, true );
        }

      return true;
    } // End of if( resTxc )

  // parse source file
  return parse( w2PathTxt, airportList, gliderList, true );
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
          outs << header1.toLatin1().data() << QDateTime::currentDateTime().toString().toLatin1().data() << '\n'
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

      // Check, if we have an outlanding point. Under this we have to
      // expect also ul fields. If no ul field is defined we will
      // ignore the line.
      if( kind == "2" && line.mid( 23, 4 ) != "*ULM" )
        {
          // ignore outlandings
          continue;
        }

      outs << line.toLatin1().data() << '\n';
      outLines++;
    }

  in.close();
  out.close();

  if( outLines > 2 )
    {
      // overwrite old file with new extracted file
      rename( fout.toLatin1().data(), path.toLatin1().data() );
    }
  else
    {
      // remove unneeded file, if nothing could be extracted
      unlink( fout.toLatin1().data() );
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
 *              MAP_ICAO <name>=[IntAirport|Airport|MilAirport|CivMilAirport|Airfield|ClosedAirfield|CivHeliport|MilHeliport|AmbHeliport|Glidersite|UltraLight|HangGlider]
 *              MAP_SHORT_NAME <name>=[IntAirport|Airport|MilAirport|CivMilAirport|Airfield|ClosedAirfield|CivHeliport|MilHeliport|AmbHeliport|Glidersite|UltraLight|HangGlider]
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
      qWarning("W2000: User has not provided a configuration file %s!", path.toLatin1().data());
      return false;
    }

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
          list.remove( list.begin() );

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

          if( list[0].contains("MAP_ICAO", false) )
            {
              list[0].remove( 0, 8 );
              list[0] = list[0].trimmed().toUpper(); // icao name of airfield
              list[1] = list[1].trimmed(); // new map type for airfield
              c_icaoMap.insert( list[0], list[1] );
              // qDebug("W2000: c_icaoMap.insert(%s, %s)", list[0].toLatin1().data(), list[1].toLatin1().data());
            }
          else if( list[0].contains("MAP_SHORT_NAME", false) )
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
 * Parses the passed file in welt 2000 format and put the approriate
 * entries in the related lists.
 * 
 * arg1 path: Full name with path of welt2000 file
 * arg2 airportList: All airports have to be stored in this list
 * arg3 glidertList: All gilder fields have to be stored in this list
 * arg4 doCompile: create a binary file of the parser results,
 *                 if flag is set to true. Default is false.
 * returns true (success) or false (error occured)
 */
bool Welt2000::parse( QString& path,
                      MapElementList& airportList,
                      MapElementList& gliderList,
                      bool doCompile )
{
  QTime t;
  t.start();

  // Filter out the needed extract for us from the welt 2000
  // file. That will reduce the file size over the half.
  if( filter( path ) == false )
    {
      // It seems, that no welt 2000 file has been passed
      return false;
    }

  QFile in(path);

  if( !in.open(QIODevice::ReadOnly) )
    {
      qWarning("W2000: Cannot open airfield file %s!", path.toLatin1().data());
      return false;
    }

  QTextStream ins(&in);

  // look, if a config file is accessable. If yes read out its data.
  QFileInfo fi( path );
  QString confFile = fi.dirPath(TRUE) + "/welt2000.conf";

  // It is expected that the filter file is located in the same
  // directory as the welt2000.txt file and carries the name
  // welt2000.conf
  readConfigEntries( confFile );

  // Check, if in GeneralConfig other definitions exist. These will
  // overwrite the definitions in the config file.

  GeneralConfig *conf = GeneralConfig::instance();
  QString cFilter = conf->getWelt2000CountryFilter();

  if( cFilter.length() > 0 )
    {
      // load new country filter definitions
      c_countryList.clear();

      QStringList clist = cFilter.split( QRegExp("[, ]"), QString::SkipEmptyParts );

      for( int i = 0; i < clist.count(); i++ )
        {
          QString e = clist[i].trimmed().toUpper();
      
          if( c_countryList.contains(e) )
            continue;

          c_countryList += e;
        }

      c_countryList.sort();
    }

  // get home radius from config data
  int radius = conf->getWelt2000HomeRadius();

  if( radius == 0 )
    {
      // default is 1000 kilometers
      c_homeRadius = 1000.0;
    }
  else
    {
      // we must look, what unit the user has choosen. This unit must
      // be considered during load of airfield data.
      c_homeRadius = getDistanceInKm( radius );
    }

  qDebug( "W2000: File welt2000.conf contains %d country entries", c_countryList.count() );
  qDebug( "W2000: File welt2000 defines the home radius to %.1f Km", c_homeRadius );


  // put all entries of contry list into a dictionary for faster
  // access
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
      compileFile = fi.dirPath(TRUE) + "/welt2000.txc";
      compFile.setName( compileFile );
      out.setDevice( &compFile );

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
  QString lastName = "";
  uint counter = 0;
  uint lastCounter = 0;

  // statistics counter
  uint ul, gl, af;
  ul = gl = af = 0;

  // Input file was taken from Michael Meiers welt 2000 data dase
  //
  // 0         1         2         3         4         5         6
  // 0123456789012345678901234567890123456789012345678901234567890123
  // 1234567890123456789012345678901234567890123456789012345678901230
  // AACHE1 AACHEN  MERZBRUC#EDKAA 53082612287 189N504923E0061111DEO5
  // AICHA1 AICHACH         # S !G 43022012230 440N482824E0110807DEX
  // ARGEN2 ARGENBUEHL EISE?*ULM G 40082612342 686N474128E0095744DEN
  // BASAL2 BAD SALZUNGEN UL*ULM G 65092712342 233N504900E0101305DEN
  // FUERS1 FUERSTENWALDE   #EDALG 80112912650  55N522323E0140539DEO3
  // BERLT1 BERLIN  TEGEL   #EDDTA303082611870  37N523335E0131716DEO
  // BERSC1 BERLIN SCHOENFEL#EDDBC300072512002  49N522243E0133114DEO
  // BERTE1 BERLIN TEMPELHOF#EDDIC208092711810  52N522825E0132406DEO

  while( ! in.atEnd() )
    {
      bool ok;
      QString line, buf;
      line= in.readLine(128);
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
            continue;
        }

      // look, what kind of line was read.
      // COL5 = 1 Airfield
      // COL5 = 2 Outlanding, contains also UL places

      QString kind = line.mid( 5, 1 );

      if( kind != "1" && kind != "2" )
        {
          continue; // not of interest for us
        }

      bool ulField = false;
      bool glField = false;
      bool afField = false;
      QString icao;

      if( kind == "2" ) // can be an UL field
        {
          if( line.mid( 23, 4 ) == "*ULM" )
            {
              ulField = true;
            }
          else
            {
              // step over other outlandings
              continue;
            }
        }
      else if( line.mid( 23, 3 ) == "# S" )
        {
          // Glider field
          glField = true;
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
      afName = afName.simplifyWhiteSpace();

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
          afType = BaseMapElement::Glidersite;
        }
      else if( afField == true )
        {
          if( icao.startsWith("ET") )
            {
              // German military airfield
              afType = BaseMapElement::MilAirport;
            }
          else if( afName.contains(QRegExp(" MIL$")) )
            {
              // should be an military airfiled but not 100% sure
              afType = BaseMapElement::MilAirport;
            }
          else
            {
              afType = BaseMapElement::Airfield;
            }
        }

      // make the user's wanted mapping for short name
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
      QString gpsName = afName.left(8);

      if( lastName == gpsName )
        {
          gpsName.replace( gpsName.length()-1, 1, QString::number(++lastCounter) );
        }
      else
        {
          lastName = gpsName;
          lastCounter = 0;
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
          qWarning( "W2000, Line %d: %s (%s) wrong latitude degree value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong latitude minute value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong latitude second value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
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
          qWarning( "W2000, Line %d: %s (%s) wrong longitude degree value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong longitude minute value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong longitude second value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      double lonTmp = (d * 600000.) + (10000. * (m + s / 60. ));

      lon = (qint32) rint(lonTmp);

      if( line[52] == 'W' )
        {
          lon = -lon;
        }

      if( countryDict.isEmpty() )
        {
          // No countries are defined to be filtered out, we will
          // compute the distance between the home position and the
          // read point. Is the distance is over the user defined
          // value away we will ignore this point.

          QPoint home = _globalMapMatrix->getHomeCoord();
          QPoint af( lat, lon );

          double d = dist( &home, &af );

          if( d > c_homeRadius )
            {
              // Distance is greater than defined radius in generalconfig
              // qDebug("Ignoring Dist=%f, AF=%s", d, afName.toLatin1().data());
              continue;
            }
        }

#ifdef BOUNDING_BOX
      // update the bounding box
      _globalMapContents->AddPointToRect( boundingBox, QPoint(lat, lon) );
#endif

      WGSPoint wgsPos(lat, lon);
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
          qWarning( "W2000, Line %d: %s (%s) missing or wrong elevation, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          elevation = 0;
        }

      // frequency
      QString frequency = line.mid(36,3) + "." +
        line.mid(39,2).trimmed();

      double f = frequency.toDouble(&ok);

      if( !ok || f < 117.97 || f > 137.0 )
        {
          qWarning( "W2000, Line %d: %s (%s) missing or wrong frequency, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          frequency = "000.000";
        }

      // check, what has to be appended as last digit
      if( line[40] == '2' || line[40] == '7' )
        {
          frequency += "5";
        }
      else
        {
          frequency += "0";
        }

      // runway direction as two digits, we consider only the first entry
      buf = line.mid(32,2).trimmed();

      ok = false;
      uint rwDir = 0;

      if( ! buf.isEmpty() )
        {
          rwDir = buf.toUInt(&ok);
        }

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) missing or wrong runway direction, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          rwDir = 0;
        }

      // runway length in meters, must be multiplied by 10
      buf = line.mid(29,3).trimmed();

      ok = false;
      uint rwLen = 0;

      if( ! buf.isEmpty() )
        {
          rwLen = buf.toUInt(&ok);
        }

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) missing or wrong runway length, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          rwLen = 0;
        }
      else
        {
          rwLen *= 10;
        }

      // runway surface
      uint rwSurface;
      QChar rwType = line[28];

      if( rwType == 'A' )
        {
          rwSurface = Airport::Asphalt;
        }
      else if( rwType == 'C' )
        {
          rwSurface = Airport::Concrete;
        }
      else if( rwType == 'G' )
        {
          rwSurface = Airport::Grass;
        }
      else
        {
          rwSurface = Airport::Unknown;
        }

      //---------------------------------------------------------------
      // append a new record to the related list
      //---------------------------------------------------------------

      // count output records separated by kind
      if( ulField )
        ul++;
      else if( glField )
        gl++;
      else
        af++;

      // create an runway object
      runway *rw = new runway( rwLen ,rwDir*10, rwSurface, 1 );

      if( afType != BaseMapElement::Glidersite )
        {
          // Add a non glider site to the list. That can be an
          // airfield or an ultralight field
          Airport *ap = new Airport( afName, icao.trimmed(), gpsName.toUpper(), afType,
                                     wgsPos, position, elevation, frequency, 0, rw );

          airportList.append( ap );
        }
      else
        {
          // Add a glider site to the list.
          GliderSite *gl = new GliderSite( afName, icao.trimmed(), gpsName.toUpper(),
                                           wgsPos, position, elevation, frequency, 0, rw );

          gliderList.append( gl );
        }

      if( doCompile )
        {
          // This was the order used by ealier cumulus
          // implementations. Because welt2000 does not support these
          // all a subset from the original implementation is only
          // used to spare memory and to get a better performance. All
          // unused values are commented out.

          counter++;
          // airfield type
          outbuf << quint8( afType );
          // airfield name with country
          ShortSave(outbuf, afName.toUtf8());

          // airfield id
          // ShortSave(outbuf, QString::number(++counter).toUtf8());

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

          // winch available is always set to no
          // if( afType == BaseMapElement::Glidersite ) out << qint8(0);

          // number of contact data, always set to one
          // outbuf << qint8(1);

          // frequency written as e.g. 126.500, must be put into 16 bits
          outbuf << quint16(frequency.left(3).toInt()*1000+frequency.right(3).toInt()-100000);

          // contact type
          // outbuf << qint8(0);

          // call sign
          // ShortSave(outbuf, QString("").toUtf8());

          // number of runways always set to one
          // outbuf << qint8(1);

          // runway direction
          outbuf << quint16(rwDir);
          // runway length in meters
          outbuf << quint16(rwLen);
          // runway surface
          outbuf << quint8(rwSurface);

          // runway open, always true assumed
          // outbuf << qint8(1);
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
          out << QPoint( _globalMapMatrix->getHomeCoord() ); // home position

#ifdef BOUNDING_BOX
          // boundingbox is never used during read in, we don't need to write out it
          // qDebug("Bounding box is: (%d, %d),(%d, %d)",
          // boundingBox.left(), boundingBox.top(), boundingBox.right(), boundingBox.bottom());
          out << boundingBox;
#endif

          SaveProjection( out, _globalMapMatrix->getProjection() );
          // write data on airfields from buffer
          out << bufdata;
          compFile.close();
        }
      else
        {
          // no entries are contained in buffer, remove output file
          compFile.close();
          unlink( compileFile.toLatin1().data() );
        }
    }

  qDebug( "W2000, Statistics from file %s: Parsing Time=%dms, Sum=%d, Airfields=%d, GL=%d, UL=%d",
          basename(path.toLatin1().data()), t.elapsed(), af+gl+ul, af, gl, ul );

  return true;
}


/**
 * Read the content of a compiled file and put it into the related
 * lists.
 */
bool Welt2000::readCompiledFile( QString &path,
                                 MapElementList& airportList,
                                 MapElementList& gliderList )
{
  QTime t;
  t.start();

  QFile inFile(path);

  if( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("W2000: Cannot open airfield file %s!", path.toLatin1().data());
      return false;
    }

  QDataStream in(&inFile);

  // This was the order used by ealier cumulus
  // implementations. Because welt2000 does not support these all a
  // subset from the original implementation is only used to spare
  // memory and to get a better performance.

  quint32 magic;
  qint8 fileType;
  quint16 fileVersion;
  QDateTime creationDateTime;
  QStringList countryList;
  double homeRadius;
  QPoint homeCoord;

#ifdef BOUNDING_BOX
  QRect boundingBox;
#endif

  ProjectionBase *projectionFromFile;
  qint32 buflen;

  quint8 afType;
  QString afName;
  // QString afNumber;
  QString icao;
  QString gpsName;
  WGSPoint wgsPos;
  QPoint position;
  qint16 elevation;
  // qint8 winch;
  // quint8 contactCount;
  quint16 inFrequency;
  // qint8 contactType;
  // QString callSign;
  // quint8 rwCount;
  quint16 rwDir; // 0 -> 360
  quint16 rwLen;
  quint8 rwSurface;
  // qint8 rwOpen;

  QByteArray utf8_temp;
  QString frequency;

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
      qWarning( "W2000: wrong file version %x read! Aborting ...", fileType );
      inFile.close();
      return false;
    }

  in >> creationDateTime;
  in >> countryList;
  in >> homeRadius;
  in >> homeCoord;

#ifdef BOUNDING_BOX
  in >> boundingBox;
#endif

  projectionFromFile = LoadProjection(in);

  // projectionFromFile is allocated dynamically, we don't need it
  // here. Therefore it is immediately deleted to avoid memory leaks.
  delete projectionFromFile;
  projectionFromFile = 0;

  // because we're used a buffer during output, the buffer length will
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

      // ShortLoad(in, utf8_temp);
      // afNumber=QString::fromUtf8(utf8_temp);

      ShortLoad(in, utf8_temp);
      icao=QString::fromUtf8(utf8_temp);
      ShortLoad(in, utf8_temp);
      gpsName=QString::fromUtf8(utf8_temp);
      in >> wgsPos;
      in >> position;
      in >> elevation;

      // if( afType == BaseMapElement::Glidersite ) in >> winch;
      // in >> contactCount;

      in >> inFrequency;
      frequency.sprintf("%3d.%03d",(inFrequency+100000)/1000,(inFrequency)%1000);

      // in >> contactType;
      // ShortLoad (in, utf8_temp);
      // callSign.fromUtf8(utf8_temp);
      // in >> rwCount;

      in >> rwDir;
      in >> rwLen;
      in >> rwSurface;

      // in >> rwOpen;

      // create an runway object
      runway *rw = new runway( rwLen ,rwDir*10, rwSurface, 1 );

      if( afType != BaseMapElement::Glidersite )
        {
          // Add a non glider site to the list. That can be an
          // airfield or an ultralight field
          Airport *ap = new Airport( afName, icao, gpsName, (BaseMapElement::objectType) afType,
                                     wgsPos, position, (uint) elevation, frequency, false, rw );
          airportList.append( ap );
        }
      else
        {
          // Add a glider site to the list.
          GliderSite *gl = new GliderSite( afName, icao, gpsName,
                                           wgsPos, position, (uint) elevation, frequency, false, rw );

          gliderList.append( gl );
        }
    }

  inFile.close();

  //qDebug( "W2000: %d airfield objects read from file %s in %dms",
  //        counter, basename(path.toLatin1().data()), t.elapsed() );

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
  h_homeCoord.setX(0);
  h_homeCoord.setY(0);

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
  in >> h_homeCoord;

#ifdef BOUNDING_BOX

  in >> h_boundingBox;
#endif

  h_projection = LoadProjection(in);

  inFile.close();
  h_headerIsValid = true; // save read result here too
  return true;
}


/**
 * Get the distance back in kilometers according to the set unit by
 * the user.
 *
 * @param distance as number
 * @returns distance as double in kilometers
 */
double Welt2000::getDistanceInKm( const int distance )
{
  // we must look, what unit the user has choosen.
  Distance::distanceUnit distUnit = Distance::getUnit();
  Distance dist;
  double unit = 0.0;

  if( distUnit == Distance::kilometers )
    {
      dist.setKilometers( distance );
      unit = dist.getKilometers();
    }
  else if( distUnit == Distance::miles )
    {
      dist.setMiles( distance );
      unit = dist.getKilometers();
    }
  else // if( distUnit == Distance::nautmiles )
    {
      dist.setNautMiles( distance );
      unit = dist.getKilometers();
    }
  
  return unit;
}
