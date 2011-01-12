/***********************************************************************
**
**   resource.h
**
**   This file is part of cumulus
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**                   2009 modified by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef RESSOURCE_H
#define RESSOURCE_H

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

#define KFLOG_FILE_MAGIC   0x404b464c

#define TYPE_GROUND     0x47
#define TYPE_TERRAIN    0x54
#define TYPE_MAP        0x4d
#define TYPE_AERO       0x41
#define FORMAT_VERSION  100

/************************************************************************************
 * Definition of map element types
 ************************************************************************************/

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
#define GLIDERFIELD       10
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
#define LOW_FLIGHT       31
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
#define HIGHWAY          45
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

#endif // RESOURCE_H
