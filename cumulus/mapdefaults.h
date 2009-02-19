/***********************************************************************
**
**   mapdefaults.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, 2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAP_DEFAULTS_H
#define MAP_DEFAULTS_H

// Default-Home (Berlin, Brandenburger Tor)
#define HOME_DEFAULT_LAT 31509700
#define HOME_DEFAULT_LON 8026500

// the scale-borders
#define VAL_BORDER_L                      10
#define VAL_BORDER_U                    1200
#define VAL_BORDER_1                     100
#define VAL_BORDER_2                     500
#define VAL_BORDER_3                    1000
#define VAL_BORDER_S                     250

/*
 * Definierte Farbwerte bei (dazwischen wurde frueher linear geaendert):
 *
 *   <0 m :    96 / 128 / 248
 *    0 m :   174 / 208 / 129 *
 *   10 m :   201 / 230 / 178 *
 *   50 m :   231 / 255 / 231 *
 *  100 m :   221 / 245 / 183 *
 *  250 m :   240 / 240 / 168 *
 * 1000 m :   235 / 155 /  98 *
 * 4000 m :   130 /  65 /  20 *
 * 9000 m :    96 /  43 /  16 *
 */

// number of different terrain colors
#define SIZEOF_TERRAIN_COLORS 51

// Default values used for terrain colors

// color used as ground color, when isoline drawing is disabled
#define COLOR_LEVEL_GROUND QColor(237, 248, 175)

#define COLOR_LEVEL_SUB QColor(230,255,255)
#define COLOR_LEVEL_0 QColor(165, 214, 126)
#define COLOR_LEVEL_10 QColor(185, 220, 131)
#define COLOR_LEVEL_25 QColor(193, 225, 138)
#define COLOR_LEVEL_50 QColor(208, 234, 151)
#define COLOR_LEVEL_75 QColor(222, 243, 164)
#define COLOR_LEVEL_100 QColor(237, 252, 178)
#define COLOR_LEVEL_150 QColor(237, 248, 175)
#define COLOR_LEVEL_200 QColor(236, 241, 169)
#define COLOR_LEVEL_250 QColor(236, 235, 165)
#define COLOR_LEVEL_300 QColor(236, 230, 160)
#define COLOR_LEVEL_350 QColor(236, 225, 156)
#define COLOR_LEVEL_400 QColor(236, 219, 151)
#define COLOR_LEVEL_450 QColor(236, 214, 147)
#define COLOR_LEVEL_500 QColor(236, 208, 142)
#define COLOR_LEVEL_600 QColor(235, 198, 133)
#define COLOR_LEVEL_700 QColor(235, 187, 124)
#define COLOR_LEVEL_800 QColor(235, 176, 115)
#define COLOR_LEVEL_900 QColor(235, 165, 106)
#define COLOR_LEVEL_1000 QColor(235, 155, 98)
#define COLOR_LEVEL_1250 QColor(226, 147, 91)
#define COLOR_LEVEL_1500 QColor(217, 140, 85)
#define COLOR_LEVEL_1750 QColor(208, 132, 78)
#define COLOR_LEVEL_2000 QColor(200, 125, 72)
#define COLOR_LEVEL_2250 QColor(191, 117, 65)
#define COLOR_LEVEL_2500 QColor(182, 110, 59)
#define COLOR_LEVEL_2750 QColor(173, 102, 52)
#define COLOR_LEVEL_3000 QColor(165, 95, 46)
#define COLOR_LEVEL_3250 QColor(156, 87, 39)
#define COLOR_LEVEL_3500 QColor(147, 80, 33)
#define COLOR_LEVEL_3750 QColor(138, 72, 26)
#define COLOR_LEVEL_4000 QColor(130, 65, 20)
#define COLOR_LEVEL_4250 QColor(128, 63, 19)
#define COLOR_LEVEL_4500 QColor(126, 62, 19)
#define COLOR_LEVEL_4750 QColor(124, 61, 19)
#define COLOR_LEVEL_5000 QColor(123, 60, 19)
#define COLOR_LEVEL_5250 QColor(121, 59, 19)
#define COLOR_LEVEL_5500 QColor(119, 58, 18)
#define COLOR_LEVEL_5750 QColor(118, 57, 18)
#define COLOR_LEVEL_6000 QColor(116, 56, 18)
#define COLOR_LEVEL_6250 QColor(114, 55, 18)
#define COLOR_LEVEL_6500 QColor(113, 54, 18)
#define COLOR_LEVEL_6750 QColor(111, 52, 17)
#define COLOR_LEVEL_7000 QColor(109, 51, 17)
#define COLOR_LEVEL_7250 QColor(107, 50, 17)
#define COLOR_LEVEL_7500 QColor(106, 49, 17)
#define COLOR_LEVEL_7750 QColor(104, 48, 17)
#define COLOR_LEVEL_8000 QColor(102, 47, 16)
#define COLOR_LEVEL_8250 QColor(101, 46, 16)
#define COLOR_LEVEL_8500 QColor(99, 45, 16)
#define COLOR_LEVEL_8750 QColor(97, 44, 16)

// Default-Values for map-element:

// [Trail]
#define TRAIL_COLOR_1  QColor(255,100,100)
#define TRAIL_COLOR_2  QColor(255,100,100)
#define TRAIL_COLOR_3  QColor(255,100,100)
#define TRAIL_COLOR_4  QColor(255,100,100)

#define TRAIL_PEN_WIDTH_1 2
#define TRAIL_PEN_WIDTH_2 2
#define TRAIL_PEN_WIDTH_3 1
#define TRAIL_PEN_WIDTH_4 1

#define TRAIL_PEN_STYLE_1 Qt::SolidLine
#define TRAIL_PEN_STYLE_2 Qt::SolidLine
#define TRAIL_PEN_STYLE_3 Qt::SolidLine
#define TRAIL_PEN_STYLE_4 Qt::SolidLine

// [Road] color red
#define ROAD_COLOR_1  QColor(255,000,000)
#define ROAD_COLOR_2  QColor(255,000,000)
#define ROAD_COLOR_3  QColor(255,000,000)
#define ROAD_COLOR_4  QColor(255,000,000)

#define ROAD_PEN_WIDTH_1 3
#define ROAD_PEN_WIDTH_2 2
#define ROAD_PEN_WIDTH_3 1
#define ROAD_PEN_WIDTH_4 1

#define ROAD_PEN_STYLE_1 Qt::SolidLine
#define ROAD_PEN_STYLE_2 Qt::SolidLine
#define ROAD_PEN_STYLE_3 Qt::SolidLine
#define ROAD_PEN_STYLE_4 Qt::SolidLine

// [Highway] color red
#define HIGH_COLOR_1  QColor(255,000,000)
#define HIGH_COLOR_2  QColor(255,000,000)
#define HIGH_COLOR_3  QColor(255,000,000)
#define HIGH_COLOR_4  QColor(255,000,000)

#define HIGH_PEN_WIDTH_1 6
#define HIGH_PEN_WIDTH_2 5
#define HIGH_PEN_WIDTH_3 4
#define HIGH_PEN_WIDTH_4 3

#define HIGH_PEN_STYLE_1 Qt::SolidLine
#define HIGH_PEN_STYLE_2 Qt::SolidLine
#define HIGH_PEN_STYLE_3 Qt::SolidLine
#define HIGH_PEN_STYLE_4 Qt::SolidLine

// [Railway] color black
#define RAIL_COLOR_1  QColor(000,000,000)
#define RAIL_COLOR_2  QColor(000,000,000)
#define RAIL_COLOR_3  QColor(000,000,000)
#define RAIL_COLOR_4  QColor(000,000,000)

#define RAIL_PEN_WIDTH_1 3
#define RAIL_PEN_WIDTH_2 2
#define RAIL_PEN_WIDTH_3 1
#define RAIL_PEN_WIDTH_4 1

#define RAIL_PEN_STYLE_1 Qt::DashLine
#define RAIL_PEN_STYLE_2 Qt::DashLine
#define RAIL_PEN_STYLE_3 Qt::DashLine
#define RAIL_PEN_STYLE_4 Qt::DashLine

// [Railway_d]
#define RAIL_D_COLOR_1  QColor(000,000,000)
#define RAIL_D_COLOR_2  QColor(000,000,000)
#define RAIL_D_COLOR_3  QColor(000,000,000)
#define RAIL_D_COLOR_4  QColor(000,000,000)

#define RAIL_D_PEN_WIDTH_1 3
#define RAIL_D_PEN_WIDTH_2 3
#define RAIL_D_PEN_WIDTH_3 2
#define RAIL_D_PEN_WIDTH_4 2

#define RAIL_D_PEN_STYLE_1 Qt::DashLine
#define RAIL_D_PEN_STYLE_2 Qt::DashLine
#define RAIL_D_PEN_STYLE_3 Qt::DashLine
#define RAIL_D_PEN_STYLE_4 Qt::DashLine

// [Aerial Cable]
#define AERIAL_CABLE_COLOR_1  QColor(000,000,000)
#define AERIAL_CABLE_COLOR_2  QColor(000,000,000)
#define AERIAL_CABLE_COLOR_3  QColor(000,000,000)
#define AERIAL_CABLE_COLOR_4  QColor(000,000,000)

#define AERIAL_CABLE_PEN_WIDTH_1 3
#define AERIAL_CABLE_PEN_WIDTH_2 3
#define AERIAL_CABLE_PEN_WIDTH_3 2
#define AERIAL_CABLE_PEN_WIDTH_4 2

#define AERIAL_CABLE_PEN_STYLE_1 Qt::DashLine
#define AERIAL_CABLE_PEN_STYLE_2 Qt::DashLine
#define AERIAL_CABLE_PEN_STYLE_3 Qt::DashLine
#define AERIAL_CABLE_PEN_STYLE_4 Qt::DashLine

// [River]
#define RIVER_COLOR_1 QColor(70,70,195)
#define RIVER_COLOR_2 QColor(70,70,195)
#define RIVER_COLOR_3 QColor(70,70,195)
#define RIVER_COLOR_4 QColor(70,70,195)

#define RIVER_PEN_WIDTH_1 2
#define RIVER_PEN_WIDTH_2 1
#define RIVER_PEN_WIDTH_3 1
#define RIVER_PEN_WIDTH_4 1

#define RIVER_PEN_STYLE_1 Qt::SolidLine
#define RIVER_PEN_STYLE_2 Qt::SolidLine
#define RIVER_PEN_STYLE_3 Qt::SolidLine
#define RIVER_PEN_STYLE_4 Qt::SolidLine

// [Lake]
#define LAKE_COLOR_1 QColor(70,195,195)
#define LAKE_COLOR_2 QColor(70,195,195)
#define LAKE_COLOR_3 QColor(70,195,195)
#define LAKE_COLOR_4 QColor(70,195,195)

#define LAKE_PEN_WIDTH_1 0
#define LAKE_PEN_WIDTH_2 0
#define LAKE_PEN_WIDTH_3 0
#define LAKE_PEN_WIDTH_4 0

#define LAKE_PEN_STYLE_1 Qt::NoPen
#define LAKE_PEN_STYLE_2 Qt::NoPen
#define LAKE_PEN_STYLE_3 Qt::NoPen
#define LAKE_PEN_STYLE_4 Qt::NoPen

#define LAKE_BRUSH_STYLE_1 Qt::SolidPattern
#define LAKE_BRUSH_STYLE_2 Qt::SolidPattern
#define LAKE_BRUSH_STYLE_3 Qt::SolidPattern
#define LAKE_BRUSH_STYLE_4 Qt::SolidPattern

#define LAKE_BRUSH_COLOR_1 QColor(70,195,195)
#define LAKE_BRUSH_COLOR_2 QColor(70,195,195)
#define LAKE_BRUSH_COLOR_3 QColor(70,195,195)
#define LAKE_BRUSH_COLOR_4 QColor(70,195,195)

// [Canal]
#define CANAL_COLOR_1 QColor(70,70,195)
#define CANAL_COLOR_2 QColor(70,70,195)
#define CANAL_COLOR_3 QColor(70,70,195)
#define CANAL_COLOR_4 QColor(70,70,195)

#define CANAL_PEN_WIDTH_1 2
#define CANAL_PEN_WIDTH_2 2
#define CANAL_PEN_WIDTH_3 1
#define CANAL_PEN_WIDTH_4 1

#define CANAL_PEN_STYLE_1 Qt::SolidLine
#define CANAL_PEN_STYLE_2 Qt::SolidLine
#define CANAL_PEN_STYLE_3 Qt::SolidLine
#define CANAL_PEN_STYLE_4 Qt::SolidLine

// [City]
#define CITY_COLOR_1 QColor(0,0,0) // black
#define CITY_COLOR_2 QColor(0,0,0)
#define CITY_COLOR_3 QColor(0,0,0)
#define CITY_COLOR_4 QColor(0,0,0)

#define CITY_PEN_WIDTH_1 1
#define CITY_PEN_WIDTH_2 1
#define CITY_PEN_WIDTH_3 1
#define CITY_PEN_WIDTH_4 1

#define CITY_PEN_STYLE_1 Qt::SolidLine
#define CITY_PEN_STYLE_2 Qt::SolidLine
#define CITY_PEN_STYLE_3 Qt::SolidLine
#define CITY_PEN_STYLE_4 Qt::SolidLine

#define CITY_BRUSH_COLOR_1 QColor(255,250,100) // light yellow
#define CITY_BRUSH_COLOR_2 QColor(255,250,100)
#define CITY_BRUSH_COLOR_3 QColor(255,250,100)
#define CITY_BRUSH_COLOR_4 QColor(255,250,100)

#define CITY_BRUSH_STYLE_1 Qt::SolidPattern
#define CITY_BRUSH_STYLE_2 Qt::SolidPattern
#define CITY_BRUSH_STYLE_3 Qt::SolidPattern
#define CITY_BRUSH_STYLE_4 Qt::SolidPattern

// [Forest]
#define FRST_COLOR_1 QColor(155,255,127) // light green
#define FRST_COLOR_2 QColor(155,255,127)
#define FRST_COLOR_3 QColor(155,255,127)
#define FRST_COLOR_4 QColor(155,255,127)

#define FRST_PEN_WIDTH_1 3
#define FRST_PEN_WIDTH_2 2
#define FRST_PEN_WIDTH_3 1
#define FRST_PEN_WIDTH_4 1

#define FRST_PEN_STYLE_1 Qt::SolidLine
#define FRST_PEN_STYLE_2 Qt::SolidLine
#define FRST_PEN_STYLE_3 Qt::SolidLine
#define FRST_PEN_STYLE_4 Qt::SolidLine

#define FRST_BRUSH_COLOR_1 QColor(155,255,127)
#define FRST_BRUSH_COLOR_2 QColor(155,255,127)
#define FRST_BRUSH_COLOR_3 QColor(155,255,127)
#define FRST_BRUSH_COLOR_4 QColor(155,255,127)

#define FRST_BRUSH_STYLE_1 Qt::SolidPattern
#define FRST_BRUSH_STYLE_2 Qt::SolidPattern
#define FRST_BRUSH_STYLE_3 Qt::SolidPattern
#define FRST_BRUSH_STYLE_4 Qt::SolidPattern

// [Glacier]
#define GLACIER_COLOR_1 QColor(255,255,255)
#define GLACIER_COLOR_2 QColor(255,255,255)
#define GLACIER_COLOR_3 QColor(255,255,255)
#define GLACIER_COLOR_4 QColor(255,255,255)

#define GLACIER_PEN_WIDTH_1 2
#define GLACIER_PEN_WIDTH_2 2
#define GLACIER_PEN_WIDTH_3 2
#define GLACIER_PEN_WIDTH_4 2

#define GLACIER_PEN_STYLE_1 Qt::SolidLine
#define GLACIER_PEN_STYLE_2 Qt::SolidLine
#define GLACIER_PEN_STYLE_3 Qt::SolidLine
#define GLACIER_PEN_STYLE_4 Qt::SolidLine

#define GLACIER_BRUSH_COLOR_1 QColor(255,255,255)
#define GLACIER_BRUSH_COLOR_2 QColor(255,255,255)
#define GLACIER_BRUSH_COLOR_3 QColor(255,255,255)
#define GLACIER_BRUSH_COLOR_4 QColor(255,255,255)

#define GLACIER_BRUSH_STYLE_1 Qt::SolidPattern
#define GLACIER_BRUSH_STYLE_2 Qt::SolidPattern
#define GLACIER_BRUSH_STYLE_3 Qt::SolidPattern
#define GLACIER_BRUSH_STYLE_4 Qt::SolidPattern

// [Pack Ice]
#define PACK_ICE_COLOR_1 QColor(255,250,100)
#define PACK_ICE_COLOR_2 QColor(255,250,100)
#define PACK_ICE_COLOR_3 QColor(255,250,100)
#define PACK_ICE_COLOR_4 QColor(255,250,100)

#define PACK_ICE_PEN_WIDTH_1 2
#define PACK_ICE_PEN_WIDTH_2 2
#define PACK_ICE_PEN_WIDTH_3 2
#define PACK_ICE_PEN_WIDTH_4 2

#define PACK_ICE_PEN_STYLE_1 Qt::SolidLine
#define PACK_ICE_PEN_STYLE_2 Qt::SolidLine
#define PACK_ICE_PEN_STYLE_3 Qt::SolidLine
#define PACK_ICE_PEN_STYLE_4 Qt::SolidLine

#define PACK_ICE_BRUSH_COLOR_1 QColor(255,250,100)
#define PACK_ICE_BRUSH_COLOR_2 QColor(255,250,100)
#define PACK_ICE_BRUSH_COLOR_3 QColor(255,250,100)
#define PACK_ICE_BRUSH_COLOR_4 QColor(255,250,100)

#define PACK_ICE_BRUSH_STYLE_1 Qt::SolidPattern
#define PACK_ICE_BRUSH_STYLE_2 Qt::SolidPattern
#define PACK_ICE_BRUSH_STYLE_3 Qt::SolidPattern
#define PACK_ICE_BRUSH_STYLE_4 Qt::SolidPattern

// [RIVER_t]
#define RIVER_T_COLOR_1 QColor(255,250,100)
#define RIVER_T_COLOR_2 QColor(255,250,100)
#define RIVER_T_COLOR_3 QColor(255,250,100)
#define RIVER_T_COLOR_4 QColor(255,250,100)

#define RIVER_T_PEN_WIDTH_1 2
#define RIVER_T_PEN_WIDTH_2 2
#define RIVER_T_PEN_WIDTH_3 2
#define RIVER_T_PEN_WIDTH_4 2

#define RIVER_T_PEN_STYLE_1 Qt::SolidLine
#define RIVER_T_PEN_STYLE_2 Qt::SolidLine
#define RIVER_T_PEN_STYLE_3 Qt::SolidLine
#define RIVER_T_PEN_STYLE_4 Qt::SolidLine

#define RIVER_T_BRUSH_COLOR_1 QColor(255,250,100)
#define RIVER_T_BRUSH_COLOR_2 QColor(255,250,100)
#define RIVER_T_BRUSH_COLOR_3 QColor(255,250,100)
#define RIVER_T_BRUSH_COLOR_4 QColor(255,250,100)

#define RIVER_T_BRUSH_STYLE_1 Qt::SolidPattern
#define RIVER_T_BRUSH_STYLE_2 Qt::SolidPattern
#define RIVER_T_BRUSH_STYLE_3 Qt::SolidPattern
#define RIVER_T_BRUSH_STYLE_4 Qt::SolidPattern

// [Airspace A]
#define AIRA_PEN_WIDTH_1 4
#define AIRA_PEN_WIDTH_2 3
#define AIRA_PEN_WIDTH_3 3
#define AIRA_PEN_WIDTH_4 2

#define AIRA_PEN_STYLE_1 Qt::SolidLine
#define AIRA_PEN_STYLE_2 Qt::SolidLine
#define AIRA_PEN_STYLE_3 Qt::SolidLine
#define AIRA_PEN_STYLE_4 Qt::SolidLine

#define AIRA_COLOR QColor(0,128,0).name()
#define AIRA_COLOR_1 QColor(0,180,0)
#define AIRA_COLOR_2 QColor(0,180,0)
#define AIRA_COLOR_3 QColor(0,180,0)
#define AIRA_COLOR_4 QColor(0,180,0)

#define AIRA_BRUSH_COLOR QColor(0,180,0).name()
#define AIRA_BRUSH_COLOR_1 QColor(0,180,0)
#define AIRA_BRUSH_COLOR_2 QColor(0,180,0)
#define AIRA_BRUSH_COLOR_3 QColor(0,180,0)
#define AIRA_BRUSH_COLOR_4 QColor(0,180,0)

#define AIRA_BRUSH_STYLE_1 Qt::NoBrush
#define AIRA_BRUSH_STYLE_2 Qt::NoBrush
#define AIRA_BRUSH_STYLE_3 Qt::NoBrush
#define AIRA_BRUSH_STYLE_4 Qt::NoBrush

// [Airspace B]
#define AIRB_PEN_WIDTH_1 4
#define AIRB_PEN_WIDTH_2 3
#define AIRB_PEN_WIDTH_3 3
#define AIRB_PEN_WIDTH_4 2

#define AIRB_PEN_STYLE_1 Qt::SolidLine
#define AIRB_PEN_STYLE_2 Qt::SolidLine
#define AIRB_PEN_STYLE_3 Qt::SolidLine
#define AIRB_PEN_STYLE_4 Qt::SolidLine

#define AIRB_COLOR QColor(0,128,0).name()
#define AIRB_COLOR_1 QColor(0,180,0)
#define AIRB_COLOR_2 QColor(0,180,0)
#define AIRB_COLOR_3 QColor(0,180,0)
#define AIRB_COLOR_4 QColor(0,180,0)

#define AIRB_BRUSH_COLOR QColor(0,180,0).name()
#define AIRB_BRUSH_COLOR_1 QColor(0,180,0)
#define AIRB_BRUSH_COLOR_2 QColor(0,180,0)
#define AIRB_BRUSH_COLOR_3 QColor(0,180,0)
#define AIRB_BRUSH_COLOR_4 QColor(0,180,0)

#define AIRB_BRUSH_STYLE_1 Qt::NoBrush
#define AIRB_BRUSH_STYLE_2 Qt::NoBrush
#define AIRB_BRUSH_STYLE_3 Qt::NoBrush
#define AIRB_BRUSH_STYLE_4 Qt::NoBrush

// [Airspace C]
#define AIRC_PEN_WIDTH_1 4
#define AIRC_PEN_WIDTH_2 3
#define AIRC_PEN_WIDTH_3 3
#define AIRC_PEN_WIDTH_4 2

#define AIRC_PEN_STYLE_1 Qt::SolidLine
#define AIRC_PEN_STYLE_2 Qt::SolidLine
#define AIRC_PEN_STYLE_3 Qt::SolidLine
#define AIRC_PEN_STYLE_4 Qt::SolidLine

#define AIRC_COLOR QColor(0,128,0).name()
#define AIRC_COLOR_1 QColor(0,180,0)
#define AIRC_COLOR_2 QColor(0,180,0)
#define AIRC_COLOR_3 QColor(0,180,0)
#define AIRC_COLOR_4 QColor(0,180,0)

#define AIRC_BRUSH_COLOR QColor(0,180,0).name()
#define AIRC_BRUSH_COLOR_1 QColor(0,180,0)
#define AIRC_BRUSH_COLOR_2 QColor(0,180,0)
#define AIRC_BRUSH_COLOR_3 QColor(0,180,0)
#define AIRC_BRUSH_COLOR_4 QColor(0,180,0)

#define AIRC_BRUSH_STYLE_1 Qt::NoBrush
#define AIRC_BRUSH_STYLE_2 Qt::NoBrush
#define AIRC_BRUSH_STYLE_3 Qt::NoBrush
#define AIRC_BRUSH_STYLE_4 Qt::NoBrush

// [Airspace D]
#define AIRD_PEN_WIDTH_1 4
#define AIRD_PEN_WIDTH_2 3
#define AIRD_PEN_WIDTH_3 3
#define AIRD_PEN_WIDTH_4 2

#define AIRD_PEN_STYLE_1 Qt::SolidLine
#define AIRD_PEN_STYLE_2 Qt::SolidLine
#define AIRD_PEN_STYLE_3 Qt::SolidLine
#define AIRD_PEN_STYLE_4 Qt::SolidLine

#define AIRD_COLOR QColor(0,128,0).name()
#define AIRD_COLOR_1 QColor(0,180,0)
#define AIRD_COLOR_2 QColor(0,180,0)
#define AIRD_COLOR_3 QColor(0,180,0)
#define AIRD_COLOR_4 QColor(0,180,0)

#define AIRD_BRUSH_COLOR QColor(0,180,0).name()
#define AIRD_BRUSH_COLOR_1 QColor(0,180,0)
#define AIRD_BRUSH_COLOR_2 QColor(0,180,0)
#define AIRD_BRUSH_COLOR_3 QColor(0,180,0)
#define AIRD_BRUSH_COLOR_4 QColor(0,180,0)

#define AIRD_BRUSH_STYLE_1 Qt::NoBrush
#define AIRD_BRUSH_STYLE_2 Qt::NoBrush
#define AIRD_BRUSH_STYLE_3 Qt::NoBrush
#define AIRD_BRUSH_STYLE_4 Qt::NoBrush

// [Airspace E]
#define AIRE_PEN_WIDTH_1 4
#define AIRE_PEN_WIDTH_2 3
#define AIRE_PEN_WIDTH_3 3
#define AIRE_PEN_WIDTH_4 2

#define AIRE_PEN_STYLE_1 Qt::SolidLine
#define AIRE_PEN_STYLE_2 Qt::SolidLine
#define AIRE_PEN_STYLE_3 Qt::SolidLine
#define AIRE_PEN_STYLE_4 Qt::SolidLine

#define AIRE_COLOR QColor(0,0,128).name()
#define AIRE_COLOR_1 QColor(138,169,235)
#define AIRE_COLOR_2 QColor(138,169,235)
#define AIRE_COLOR_3 QColor(138,169,235)
#define AIRE_COLOR_4 QColor(138,169,235)

#define AIRE_BRUSH_COLOR QColor(138,169,235).name()
#define AIRE_BRUSH_COLOR_1 QColor(138,169,235)
#define AIRE_BRUSH_COLOR_2 QColor(138,169,235)
#define AIRE_BRUSH_COLOR_3 QColor(138,169,235)
#define AIRE_BRUSH_COLOR_4 QColor(138,169,235)

#define AIRE_BRUSH_STYLE_1 Qt::NoBrush
#define AIRE_BRUSH_STYLE_2 Qt::NoBrush
#define AIRE_BRUSH_STYLE_3 Qt::NoBrush
#define AIRE_BRUSH_STYLE_4 Qt::NoBrush

// [Airspace F]
#define AIRF_PEN_WIDTH_1 4
#define AIRF_PEN_WIDTH_2 3
#define AIRF_PEN_WIDTH_3 3
#define AIRF_PEN_WIDTH_4 2

#define AIRF_PEN_STYLE_1 Qt::SolidLine
#define AIRF_PEN_STYLE_2 Qt::SolidLine
#define AIRF_PEN_STYLE_3 Qt::SolidLine
#define AIRF_PEN_STYLE_4 Qt::SolidLine

#define AIRF_COLOR QColor(0,0,128).name()
#define AIRF_COLOR_1 QColor(138,169,235)
#define AIRF_COLOR_2 QColor(138,169,235)
#define AIRF_COLOR_3 QColor(138,169,235)
#define AIRF_COLOR_4 QColor(138,169,235)

#define AIRF_BRUSH_COLOR QColor(138,169,235).name()
#define AIRF_BRUSH_COLOR_1 QColor(138,169,235)
#define AIRF_BRUSH_COLOR_2 QColor(138,169,235)
#define AIRF_BRUSH_COLOR_3 QColor(138,169,235)
#define AIRF_BRUSH_COLOR_4 QColor(138,169,235)

#define AIRF_BRUSH_STYLE_1 Qt::Dense6Pattern
#define AIRF_BRUSH_STYLE_2 Qt::Dense6Pattern
#define AIRF_BRUSH_STYLE_3 Qt::Dense6Pattern
#define AIRF_BRUSH_STYLE_4 Qt::Dense6Pattern

// [Control C]
#define CTRC_PEN_WIDTH_1 4
#define CTRC_PEN_WIDTH_2 3
#define CTRC_PEN_WIDTH_3 3
#define CTRC_PEN_WIDTH_4 2

#define CTRC_PEN_STYLE_1 Qt::DotLine
#define CTRC_PEN_STYLE_2 Qt::DotLine
#define CTRC_PEN_STYLE_3 Qt::DotLine
#define CTRC_PEN_STYLE_4 Qt::DotLine

#define CTRC_COLOR QColor(0,0,128).name()
#define CTRC_COLOR_1 QColor(0,0,128)
#define CTRC_COLOR_2 QColor(0,0,128)
#define CTRC_COLOR_3 QColor(0,0,128)
#define CTRC_COLOR_4 QColor(0,0,128)

#define CTRC_BRUSH_COLOR QColor(255,160,162).name()
#define CTRC_BRUSH_COLOR_1 QColor(255,160,162)
#define CTRC_BRUSH_COLOR_2 QColor(255,160,162)
#define CTRC_BRUSH_COLOR_3 QColor(255,160,162)
#define CTRC_BRUSH_COLOR_4 QColor(255,160,162)

#define CTRC_BRUSH_STYLE_1 Qt::Dense4Pattern
#define CTRC_BRUSH_STYLE_2 Qt::Dense4Pattern
#define CTRC_BRUSH_STYLE_3 Qt::Dense4Pattern
#define CTRC_BRUSH_STYLE_4 Qt::Dense4Pattern

// [Control D]
#define CTRD_PEN_WIDTH_1 4
#define CTRD_PEN_WIDTH_2 3
#define CTRD_PEN_WIDTH_3 2
#define CTRD_PEN_WIDTH_4 1

#define CTRD_PEN_STYLE_1 Qt::DotLine
#define CTRD_PEN_STYLE_2 Qt::DotLine
#define CTRD_PEN_STYLE_3 Qt::DotLine
#define CTRD_PEN_STYLE_4 Qt::DotLine

#define CTRD_COLOR QColor(0,0,128).name()
#define CTRD_COLOR_1 QColor(0,0,128)
#define CTRD_COLOR_2 QColor(0,0,128)
#define CTRD_COLOR_3 QColor(0,0,128)
#define CTRD_COLOR_4 QColor(0,0,128)

#define CTRD_BRUSH_COLOR QColor(255,160,162).name()
#define CTRD_BRUSH_COLOR_1 QColor(255,160,162)
#define CTRD_BRUSH_COLOR_2 QColor(255,160,162)
#define CTRD_BRUSH_COLOR_3 QColor(255,160,162)
#define CTRD_BRUSH_COLOR_4 QColor(255,160,162)

#define CTRD_BRUSH_STYLE_1 Qt::Dense4Pattern
#define CTRD_BRUSH_STYLE_2 Qt::Dense4Pattern
#define CTRD_BRUSH_STYLE_3 Qt::Dense4Pattern
#define CTRD_BRUSH_STYLE_4 Qt::Dense4Pattern

// [Wave Window]
#define WAVE_WINDOW_PEN_WIDTH_1 4
#define WAVE_WINDOW_PEN_WIDTH_2 3
#define WAVE_WINDOW_PEN_WIDTH_3 3
#define WAVE_WINDOW_PEN_WIDTH_4 2

#define WAVE_WINDOW_PEN_STYLE_1 Qt::SolidLine
#define WAVE_WINDOW_PEN_STYLE_2 Qt::SolidLine
#define WAVE_WINDOW_PEN_STYLE_3 Qt::SolidLine
#define WAVE_WINDOW_PEN_STYLE_4 Qt::SolidLine

#define WAVE_WINDOW_COLOR QColor(0,0,200).name()
#define WAVE_WINDOW_COLOR_1 QColor(0,0,200)
#define WAVE_WINDOW_COLOR_2 QColor(0,0,200)
#define WAVE_WINDOW_COLOR_3 QColor(0,0,200)
#define WAVE_WINDOW_COLOR_4 QColor(0,0,200)

#define WAVE_WINDOW_BRUSH_COLOR QColor(0,0,200).name()
#define WAVE_WINDOW_BRUSH_COLOR_1 QColor(0,0,200)
#define WAVE_WINDOW_BRUSH_COLOR_2 QColor(0,0,200)
#define WAVE_WINDOW_BRUSH_COLOR_3 QColor(0,0,200)
#define WAVE_WINDOW_BRUSH_COLOR_4 QColor(0,0,200)

#define WAVE_WINDOW_BRUSH_STYLE_1 Qt::Dense7Pattern
#define WAVE_WINDOW_BRUSH_STYLE_2 Qt::Dense7Pattern
#define WAVE_WINDOW_BRUSH_STYLE_3 Qt::Dense7Pattern
#define WAVE_WINDOW_BRUSH_STYLE_4 Qt::Dense7Pattern

// [Low flight area]
#define LOWF_PEN_WIDTH_1 4
#define LOWF_PEN_WIDTH_2 3
#define LOWF_PEN_WIDTH_3 3
#define LOWF_PEN_WIDTH_4 2

#define LOWF_PEN_STYLE_1 Qt::DotLine
#define LOWF_PEN_STYLE_2 Qt::DotLine
#define LOWF_PEN_STYLE_3 Qt::DotLine
#define LOWF_PEN_STYLE_4 Qt::DotLine

#define LOWF_COLOR QColor(180,0,0).name()
#define LOWF_COLOR_1 QColor(180,0,0)
#define LOWF_COLOR_2 QColor(180,0,0)
#define LOWF_COLOR_3 QColor(180,0,0)
#define LOWF_COLOR_4 QColor(180,0,0)

#define LOWF_BRUSH_COLOR QColor(180,0,0).name()
#define LOWF_BRUSH_COLOR_1 QColor(180,0,0)
#define LOWF_BRUSH_COLOR_2 QColor(180,0,0)
#define LOWF_BRUSH_COLOR_3 QColor(180,0,0)
#define LOWF_BRUSH_COLOR_4 QColor(180,0,0)

#define LOWF_BRUSH_STYLE_1 Qt::Dense7Pattern
#define LOWF_BRUSH_STYLE_2 Qt::Dense7Pattern
#define LOWF_BRUSH_STYLE_3 Qt::Dense7Pattern
#define LOWF_BRUSH_STYLE_4 Qt::Dense7Pattern

// [Danger]
#define DNG_PEN_WIDTH_1 4
#define DNG_PEN_WIDTH_2 3
#define DNG_PEN_WIDTH_3 3
#define DNG_PEN_WIDTH_4 2

#define DNG_PEN_STYLE_1 Qt::SolidLine
#define DNG_PEN_STYLE_2 Qt::SolidLine
#define DNG_PEN_STYLE_3 Qt::SolidLine
#define DNG_PEN_STYLE_4 Qt::SolidLine

#define DANGER_COLOR QColor(0,0,128).name()
#define DNG_COLOR_1 QColor(0,0,128)
#define DNG_COLOR_2 QColor(0,0,128)
#define DNG_COLOR_3 QColor(0,0,128)
#define DNG_COLOR_4 QColor(0,0,128)

#define DANGER_BRUSH_COLOR QColor(138,169,235).name()
#define DNG_BRUSH_COLOR_1 QColor(0,0,128)
#define DNG_BRUSH_COLOR_2 QColor(0,0,128)
#define DNG_BRUSH_COLOR_3 QColor(0,0,128)
#define DNG_BRUSH_COLOR_4 QColor(0,0,128)

#define DNG_BRUSH_STYLE_1 Qt::BDiagPattern
#define DNG_BRUSH_STYLE_2 Qt::BDiagPattern
#define DNG_BRUSH_STYLE_3 Qt::BDiagPattern
#define DNG_BRUSH_STYLE_4 Qt::BDiagPattern

// [Restricted]
#define RES_PEN_WIDTH_1 4
#define RES_PEN_WIDTH_2 3
#define RES_PEN_WIDTH_3 2
#define RES_PEN_WIDTH_4 1

#define RES_PEN_STYLE_1 Qt::SolidLine
#define RES_PEN_STYLE_2 Qt::SolidLine
#define RES_PEN_STYLE_3 Qt::SolidLine
#define RES_PEN_STYLE_4 Qt::SolidLine

#define RESTRICTED_COLOR QColor(0,0,128).name()
#define RES_COLOR_1 QColor(0,0,128)
#define RES_COLOR_2 QColor(0,0,128)
#define RES_COLOR_3 QColor(0,0,128)
#define RES_COLOR_4 QColor(0,0,128)

#define RESTRICTED_BRUSH_COLOR QColor(138,169,235).name()
#define RES_BRUSH_COLOR_1 QColor(0,0,128)
#define RES_BRUSH_COLOR_2 QColor(0,0,128)
#define RES_BRUSH_COLOR_3 QColor(0,0,128)
#define RES_BRUSH_COLOR_4 QColor(0,0,128)

#define RES_BRUSH_STYLE_1 Qt::BDiagPattern
#define RES_BRUSH_STYLE_2 Qt::BDiagPattern
#define RES_BRUSH_STYLE_3 Qt::BDiagPattern
#define RES_BRUSH_STYLE_4 Qt::BDiagPattern

// [Transponder Mandatory Zone]
#define TMZ_PEN_WIDTH_1 4
#define TMZ_PEN_WIDTH_2 3
#define TMZ_PEN_WIDTH_3 3
#define TMZ_PEN_WIDTH_4 2

#define TMZ_PEN_STYLE_1 Qt::DashDotLine
#define TMZ_PEN_STYLE_2 Qt::DashDotLine
#define TMZ_PEN_STYLE_3 Qt::DashDotLine
#define TMZ_PEN_STYLE_4 Qt::DashDotLine

#define TMZ_COLOR QColor(0,0,128).name()
#define TMZ_COLOR_1 QColor(0,0,128)
#define TMZ_COLOR_2 QColor(0,0,128)
#define TMZ_COLOR_3 QColor(0,0,128)
#define TMZ_COLOR_4 QColor(0,0,128)

#define TMZ_BRUSH_COLOR QColor(138,169,235).name()
#define TMZ_BRUSH_COLOR_1 QColor(0,0,128)
#define TMZ_BRUSH_COLOR_2 QColor(0,0,128)
#define TMZ_BRUSH_COLOR_3 QColor(0,0,128)
#define TMZ_BRUSH_COLOR_4 QColor(0,0,128)

#define TMZ_BRUSH_STYLE_1 Qt::NoBrush
#define TMZ_BRUSH_STYLE_2 Qt::NoBrush
#define TMZ_BRUSH_STYLE_3 Qt::NoBrush
#define TMZ_BRUSH_STYLE_4 Qt::NoBrush

// [Glider Sector]
#define GLIDER_SECTOR_PEN_WIDTH_1 4
#define GLIDER_SECTOR_PEN_WIDTH_2 3
#define GLIDER_SECTOR_PEN_WIDTH_3 3
#define GLIDER_SECTOR_PEN_WIDTH_4 2

#define GLIDER_SECTOR_PEN_STYLE_1 Qt::SolidLine
#define GLIDER_SECTOR_PEN_STYLE_2 Qt::SolidLine
#define GLIDER_SECTOR_PEN_STYLE_3 Qt::SolidLine
#define GLIDER_SECTOR_PEN_STYLE_4 Qt::SolidLine

#define GLIDER_SECTOR_COLOR QColor(255,51,0).name()
#define GLIDER_SECTOR_COLOR_1 QColor(255,51,0)
#define GLIDER_SECTOR_COLOR_2 QColor(255,51,0)
#define GLIDER_SECTOR_COLOR_3 QColor(255,51,0)
#define GLIDER_SECTOR_COLOR_4 QColor(255,51,0)

#define GLIDER_SECTOR_BRUSH_COLOR QColor(255,100,0).name()
#define GLIDER_SECTOR_BRUSH_COLOR_1 QColor(255,100,0)
#define GLIDER_SECTOR_BRUSH_COLOR_2 QColor(255,100,0)
#define GLIDER_SECTOR_BRUSH_COLOR_3 QColor(255,100,0)
#define GLIDER_SECTOR_BRUSH_COLOR_4 QColor(255,100,0)

#define GLIDER_SECTOR_BRUSH_STYLE_1 Qt::Dense4Pattern
#define GLIDER_SECTOR_BRUSH_STYLE_2 Qt::Dense4Pattern
#define GLIDER_SECTOR_BRUSH_STYLE_3 Qt::Dense4Pattern
#define GLIDER_SECTOR_BRUSH_STYLE_4 Qt::Dense4Pattern

#endif
