/***********************************************************************
**
**   resource.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  1999-2000 by Heiner Lamprecht, Florian Ehinger
**                   2009-2025 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#pragma once

/*
 * Definiert Macros zur Festlegung des Datei-Headers von KFLog-Karten
 *
 * Name der Ausgabedatei:
 *
 *   X_iiii.kfl
 *
 *
 *   X:    Typ der enthaltenen Daten
 *
 *         "G" für 0-Meter-Linie ("Ground")
 *         "T" für restliche Höhenlinien ("Terrain")
 *         "M" für allgemeine Karteninformationen
 *
 *   iiii: Kennzahl der Kachel, Angabe vierstellig
 *         (Zahl liegt zwischen 0 und 8190)
 *
 *   Typkennung und Kachel-ID werden ebenfalls im Header der Datei angegeben.
 *
 *
 * Anfang der Datei:
 *
 *   Byte     Typ       Inhalt         Bedeutung
 *  -----------------------------------------------------------------
 *   0        belong    0x404b464c     KFLog-mapfile       ("@KFL")
 *   >4       byte      0x47           ground data         ("G")
 *                      0x54           terrain data        ("T")
 *                      0x4d           map data            ("M")
 *                      0x41           aeronautical data   ("A")
 *   >5       Q_UINT16  <>             Version des Dateiformates
 *   >7       Q_UINT16  <>             ID der Kachel
 *   >9       8byte     <>             Zeitstempel der Erzeugung
 */

// number of last map tile, possible range goes 0...16200
#define MAX_TILE_NUMBER 16200

// general KFLOG file token: @KFL
#define KFLOG_FILE_MAGIC   0x404b464c

//#define TYPE_TERRAIN    0x54
//#define TYPE_MAP        0x4d

// uncompiled map file types
#define FILE_TYPE_GROUND      0x47
#define FILE_TYPE_TERRAIN     0x54
#define FILE_TYPE_MAP         0x4d

// Uncompiled map file versions
#define FILE_VERSION_GROUND   102
#define FILE_VERSION_TERRAIN  102
#define FILE_VERSION_MAP      101

// Compiled map file types
#define FILE_TYPE_GROUND_C    0x67
#define FILE_TYPE_TERRAIN_C   0x74
#define FILE_TYPE_MAP_C       0x6d

// Type definition for compiled airspace files, used by Airspace parsers
#define FILE_TYPE_AIRSPACE_C  "Cumulus-Airspaces"

// Type definition for openAIP compiled airfield files.
#define FILE_TYPE_AIRFIELD_OAIP_C "OpenAIP-Airfields"

// Type definition for openAIP compiled navigation aids files.
#define FILE_TYPE_NAV_AIDS_OAIP_C "OpenAIP-NavAids"

// Type definition for openAIP compiled hotspot files.
#define FILE_TYPE_HOTSPOT_OAIP_C "OpenAIP-Hotspots"

// Type definition for openAIP compiled single point files.
#define FILE_TYPE_SINGLE_POINT_OAIP_C "OpenAIP-SinglePoints"

//=================================================================================
// Compiled file versions. Increment this value, if you change the compiled format.
//=================================================================================
#define FILE_VERSION_GROUND_C   104
#define FILE_VERSION_TERRAIN_C  104
#define FILE_VERSION_MAP_C      103

// Version definition for compiled airspace files.
#define FILE_VERSION_AIRSPACE_C 6

// Version definition for compiled airfield files.
#define FILE_VERSION_AIRFIELD_C 7

// Version definition for compiled navigation aid files.
#define FILE_VERSION_NAV_AIDS_C 3

// Version definition for compiled hotspot files.
#define FILE_VERSION_HOTSPOT_C 3

// Version definition for compiled single point files.
#define FILE_VERSION_SINGLE_POINT_C 1


/******************************************************************************
 * Definition of map element types
 ******************************************************************************/
#define NOT_SELECTED      0
#define INT_AIRPORT       1
#define AIRPORT           2
#define MIL_AIRPORT       3
#define CIVMIL_AIRPORT    4
#define AIRFIELD          5
#define CLOSED_AIRFIELD   6
#define CIV_HELIPORT      7
#define MIL_HELIPORT      8
#define AMB_HELIPORT      9
#define GLIDERFIELD      10
#define ULTRALIGHT       11
#define HANGGLIDER       12
#define PARACHUTE        13
#define BALLOON          14
#define OUTLANDING       15
#define VOR              16
#define VORDME           17
#define VORTAC           18
#define NDB              19
#define COMPPOINT        20
#define AIR_A            21
#define AIR_B            22
#define AIR_C            23
#define AIR_D            24
#define AIR_E            25
#define WAVE_WINDOW      26
#define AIR_F            27
#define CONTROL_C        28
#define CONTROL_D        29
#define DANGER           30
#define SUA              31 // Special Use Airspace (SUA)
#define RESTRICTED       32
#define TMZ              33
#define OBSTACLE         34
#define LIGHT_OBSTACLE   35
#define OBSTACLE_GROUP   36
#define LIGHT_OBSTACLE_GROUP  37
#define SPOT             38
#define ISOHYPSE         39
#define GLACIER          40
#define BORDER           41
#define CITY             42
#define VILLAGE          43
#define LANDMARK         44
#define MOTORWAY         45
#define ROAD             46
#define RAILWAY          47
#define AERIAL_CABLE     48
#define LAKE             49
#define RIVER            50
#define CANAL            51
#define FLIGHT           52
#define FLIGHT_TASK      53
#define RIVER_T          54
#define LAKE_T           55
#define PACK_ICE         56
#define RAILWAY_D        57
#define TRAIL            58
#define FOREST           60
#define TURNPOINT        61
#define THERMAL          62
#define FLIGHT_GROUP     63
#define FAI_AREA_LOW     64
#define FAI_AREA_HIGH    65
#define GLIDER_SECTOR    66 // used for glider sectors
#define EMPTY_POINT      67 // used for empty points
#define PROHIBITED       68 // used for prohibited airspace
#define AIR_G            69
#define AIR_FIR          70 // Flight Information Region
#define AIR_UKN          71
#define TACAN            72 // Tactical Air Navigation (TACAN)
#define RMZ              73 // Radio Mandatory Zone
#define CTR              74 // Control Zone
#define USER_POINT       75 // A point set by the user
#define AIR_FLARM        76 // Flarm alert zone
#define ALTIPORT         77
#define LAST_ENTRY       79 // This must be always the last entry!!!

