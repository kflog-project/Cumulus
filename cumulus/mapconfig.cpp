/***********************************************************************
 **
 **   mapconfig.cpp
 **
 **   This file is part of cumulus.
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

#include "mapconfig.h"
#include "basemapelement.h"
#include "mapdefaults.h"
#include "generalconfig.h"

// Different macros used by read method for configuration data
#define READ_BORDER(a) \
  a[0] = true;         \
  a[1] = true;         \
  a[2] = true;         \
  a[3] = true;

#define READ_PEN(G, A, B, C1, C2, C3, C4, P1, P2, P3, P4,   \
                 S1, S2, S3, S4)                            \
  READ_BORDER(B)                                            \
  A.append(QPen(C1, P1, (Qt::PenStyle)S1));                 \
  A.append(QPen(C2, P2, (Qt::PenStyle)S2));                 \
  A.append(QPen(C3, P3, (Qt::PenStyle)S3));                 \
  A.append(QPen(C4, P4, (Qt::PenStyle)S4));

#define READ_PEN_BRUSH(G, a, B, A, C1, C2, C3, C4, P1, P2, P3, P4,       \
                       S1, S2, S3, S4, C7, C8, C9, C10, S7, S8, S9, S10) \
  READ_PEN(G, a, B, C1, C2, C3, C4, P1, P2, P3, P4,                      \
           S1, S2, S3, S4)                                               \
  A.append(QBrush(C7,  (Qt::BrushStyle)S7));                         \
  A.append(QBrush(C8,  (Qt::BrushStyle)S8));                         \
  A.append(QBrush(C9,  (Qt::BrushStyle)S9));                         \
  A.append(QBrush(C10, (Qt::BrushStyle)S10));

// number of created class instances
short MapConfig::instances = 0;

MapConfig::MapConfig(QObject* parent)
  : QObject(parent), scaleIndex(0), isSwitch(false)
{
  if ( ++instances > 1 )
    {
      // There exists already a class instance as singleton.
      return;
    }

  ++instances;

  // create QIcons with background for copying later when needed
  // in airfield list; speeds up list display

  unsigned int airfieldType[12] = { BaseMapElement::IntAirport, BaseMapElement::Airport, BaseMapElement::MilAirport,
    BaseMapElement::CivMilAirport, BaseMapElement::Airfield, BaseMapElement::ClosedAirfield, BaseMapElement::CivHeliport,
    BaseMapElement::MilHeliport, BaseMapElement::AmbHeliport, BaseMapElement::Glidersite, BaseMapElement::UltraLight,
    BaseMapElement::HangGlider };

  QPixmap selectPixmap;
  QIcon afIcon;
  QPainter pnt;

  for ( int i=0; i<12; i++ ) {
    selectPixmap = QPixmap(18,18);
    pnt.begin(&selectPixmap);
    selectPixmap.fill( Qt::white );
    pnt.drawPixmap(1, 1, getPixmap(airfieldType[i],false,true) );
    pnt.end();
    afIcon = QIcon();
    afIcon.addPixmap( getPixmap(airfieldType[i],false,true) );
    afIcon.addPixmap( selectPixmap, QIcon::Selected );
    airfieldIcon.insert(airfieldType[i], afIcon);
  }

  slotReadConfig();
  // qDebug("MapConfig initialized...");
}

MapConfig::~MapConfig()
{
  // decrement instance counter
  --instances;

  // @AP: all lists should be automatically deallocate its members
  // during destruction
}

void MapConfig::slotReadConfig()
{
  topographyColorList.clear();

  airAPenList.clear();
  airABrushList.clear();
  airBPenList.clear();
  airBBrushList.clear();
  airCPenList.clear();
  airCBrushList.clear();
  airDPenList.clear();
  airDBrushList.clear();
  airEPenList.clear();
  airEBrushList.clear();
  airFPenList.clear();
  airFBrushList.clear();
  ctrCPenList.clear();
  ctrCBrushList.clear();
  ctrDPenList.clear();
  ctrDBrushList.clear();
  lowFPenList.clear();
  lowFBrushList.clear();
  dangerPenList.clear();
  dangerBrushList.clear();
  restrPenList.clear();
  restrBrushList.clear();
  tmzPenList.clear();
  tmzBrushList.clear();
  waveWindowPenList.clear();
  waveWindowBrushList.clear();
  gliderSectorPenList.clear();
  gliderSectorBrushList.clear();
  highwayPenList.clear();
  roadPenList.clear();
  trailPenList.clear();
  railPenList.clear();
  rail_dPenList.clear();
  aerialcablePenList.clear();
  riverPenList.clear();
  river_tPenList.clear();
  river_tBrushList.clear();
  canalPenList.clear();
  cityPenList.clear();
  cityBrushList.clear();
  lakePenList.clear();
  lakeBrushList.clear();
  forestPenList.clear();
  glacierPenList.clear();
  packicePenList.clear();
  forestBrushList.clear();
  glacierBrushList.clear();
  packiceBrushList.clear();

  // Load colors for topography
  topographyColorList.append(COLOR_LEVEL_SUB);
  topographyColorList.append(COLOR_LEVEL_0);
  topographyColorList.append(COLOR_LEVEL_10);
  topographyColorList.append(COLOR_LEVEL_25);
  topographyColorList.append(COLOR_LEVEL_50);
  topographyColorList.append(COLOR_LEVEL_75);
  topographyColorList.append(COLOR_LEVEL_100);
  topographyColorList.append(COLOR_LEVEL_150);
  topographyColorList.append(COLOR_LEVEL_200);
  topographyColorList.append(COLOR_LEVEL_250);
  topographyColorList.append(COLOR_LEVEL_300);
  topographyColorList.append(COLOR_LEVEL_350);
  topographyColorList.append(COLOR_LEVEL_400);
  topographyColorList.append(COLOR_LEVEL_450);
  topographyColorList.append(COLOR_LEVEL_500);
  topographyColorList.append(COLOR_LEVEL_600);
  topographyColorList.append(COLOR_LEVEL_700);
  topographyColorList.append(COLOR_LEVEL_800);
  topographyColorList.append(COLOR_LEVEL_900);
  topographyColorList.append(COLOR_LEVEL_1000);
  topographyColorList.append(COLOR_LEVEL_1250);
  topographyColorList.append(COLOR_LEVEL_1500);
  topographyColorList.append(COLOR_LEVEL_1750);
  topographyColorList.append(COLOR_LEVEL_2000);
  topographyColorList.append(COLOR_LEVEL_2250);
  topographyColorList.append(COLOR_LEVEL_2500);
  topographyColorList.append(COLOR_LEVEL_2750);
  topographyColorList.append(COLOR_LEVEL_3000);
  topographyColorList.append(COLOR_LEVEL_3250);
  topographyColorList.append(COLOR_LEVEL_3500);
  topographyColorList.append(COLOR_LEVEL_3750);
  topographyColorList.append(COLOR_LEVEL_4000);
  topographyColorList.append(COLOR_LEVEL_4250);
  topographyColorList.append(COLOR_LEVEL_4500);
  topographyColorList.append(COLOR_LEVEL_4750);
  topographyColorList.append(COLOR_LEVEL_5000);
  topographyColorList.append(COLOR_LEVEL_5250);
  topographyColorList.append(COLOR_LEVEL_5500);
  topographyColorList.append(COLOR_LEVEL_5750);
  topographyColorList.append(COLOR_LEVEL_6000);
  topographyColorList.append(COLOR_LEVEL_6250);
  topographyColorList.append(COLOR_LEVEL_6500);
  topographyColorList.append(COLOR_LEVEL_6750);
  topographyColorList.append(COLOR_LEVEL_7000);
  topographyColorList.append(COLOR_LEVEL_7250);
  topographyColorList.append(COLOR_LEVEL_7500);
  topographyColorList.append(COLOR_LEVEL_7750);
  topographyColorList.append(COLOR_LEVEL_8000);
  topographyColorList.append(COLOR_LEVEL_8250);
  topographyColorList.append(COLOR_LEVEL_8500);
  topographyColorList.append(COLOR_LEVEL_8750);

  GeneralConfig *conf = GeneralConfig::instance();

  READ_PEN("Road", roadPenList, roadBorder,
           ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
           ROAD_PEN_WIDTH_1, ROAD_PEN_WIDTH_2, ROAD_PEN_WIDTH_3, ROAD_PEN_WIDTH_4,
           ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4)

  READ_PEN("Trail", trailPenList, trailBorder,
           TRAIL_COLOR_1, TRAIL_COLOR_2, TRAIL_COLOR_3, TRAIL_COLOR_4,
           TRAIL_PEN_WIDTH_1, TRAIL_PEN_WIDTH_2, TRAIL_PEN_WIDTH_3, TRAIL_PEN_WIDTH_4,
           TRAIL_PEN_STYLE_1, TRAIL_PEN_STYLE_2, TRAIL_PEN_STYLE_3, TRAIL_PEN_STYLE_4)

  READ_PEN("River", riverPenList, riverBorder,
           RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3, RIVER_COLOR_4,
           RIVER_PEN_WIDTH_1, RIVER_PEN_WIDTH_2, RIVER_PEN_WIDTH_3, RIVER_PEN_WIDTH_4,
           RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4)

  READ_PEN("Canal", canalPenList, canalBorder,
           CANAL_COLOR_1, CANAL_COLOR_2, CANAL_COLOR_3, CANAL_COLOR_4,
           CANAL_PEN_WIDTH_1, CANAL_PEN_WIDTH_2, CANAL_PEN_WIDTH_3, CANAL_PEN_WIDTH_4,
           CANAL_PEN_STYLE_1, CANAL_PEN_STYLE_2, CANAL_PEN_STYLE_3, CANAL_PEN_STYLE_4)

  READ_PEN("Rail", railPenList, railBorder,
           RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
           RAIL_PEN_WIDTH_1, RAIL_PEN_WIDTH_2, RAIL_PEN_WIDTH_3, RAIL_PEN_WIDTH_4,
           RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4)

  READ_PEN("Rail_D", rail_dPenList, rail_dBorder,
           RAIL_D_COLOR_1, RAIL_D_COLOR_2, RAIL_D_COLOR_3, RAIL_D_COLOR_4,
           RAIL_D_PEN_WIDTH_1, RAIL_D_PEN_WIDTH_2, RAIL_D_PEN_WIDTH_3, RAIL_D_PEN_WIDTH_4,
           RAIL_D_PEN_STYLE_1, RAIL_D_PEN_STYLE_2, RAIL_D_PEN_STYLE_3, RAIL_D_PEN_STYLE_4)

  READ_PEN("Aerial_Cable", aerialcablePenList, aerialcableBorder,
           AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2, AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4,
           AERIAL_CABLE_PEN_WIDTH_1, AERIAL_CABLE_PEN_WIDTH_2, AERIAL_CABLE_PEN_WIDTH_3, AERIAL_CABLE_PEN_WIDTH_4,
           AERIAL_CABLE_PEN_STYLE_1, AERIAL_CABLE_PEN_STYLE_2, AERIAL_CABLE_PEN_STYLE_3, AERIAL_CABLE_PEN_STYLE_4)

  READ_PEN("Highway", highwayPenList, highwayBorder,
           HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
           HIGH_PEN_WIDTH_1, HIGH_PEN_WIDTH_2, HIGH_PEN_WIDTH_3, HIGH_PEN_WIDTH_4,
           HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4)

  READ_PEN_BRUSH("Lake", lakePenList, lakeBorder, lakeBrushList,
             LAKE_COLOR_1, LAKE_COLOR_2, LAKE_COLOR_3,LAKE_COLOR_4,
             LAKE_PEN_WIDTH_1, LAKE_PEN_WIDTH_2, LAKE_PEN_WIDTH_3, LAKE_PEN_WIDTH_4,
             LAKE_PEN_STYLE_1, LAKE_PEN_STYLE_2, LAKE_PEN_STYLE_3, LAKE_PEN_STYLE_4,
             LAKE_BRUSH_COLOR_1, LAKE_BRUSH_COLOR_2,
             LAKE_BRUSH_COLOR_3, LAKE_BRUSH_COLOR_4,
             LAKE_BRUSH_STYLE_1, LAKE_BRUSH_STYLE_2,
             LAKE_BRUSH_STYLE_3, LAKE_BRUSH_STYLE_4)

  READ_PEN_BRUSH("City", cityPenList, cityBorder, cityBrushList,
             CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3,CITY_COLOR_4,
             CITY_PEN_WIDTH_1, CITY_PEN_WIDTH_2, CITY_PEN_WIDTH_3, CITY_PEN_WIDTH_4,
             CITY_PEN_STYLE_1, CITY_PEN_STYLE_2, CITY_PEN_STYLE_3, CITY_PEN_STYLE_4,
             CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
             CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4,
             CITY_BRUSH_STYLE_1, CITY_BRUSH_STYLE_2,
             CITY_BRUSH_STYLE_3, CITY_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Forest", forestPenList, forestBorder, forestBrushList,
                 FRST_COLOR_1, FRST_COLOR_2, FRST_COLOR_3, FRST_COLOR_4,
                 FRST_PEN_WIDTH_1, FRST_PEN_WIDTH_2, FRST_PEN_WIDTH_3, FRST_PEN_WIDTH_4,
                 FRST_PEN_STYLE_1, FRST_PEN_STYLE_2, FRST_PEN_STYLE_3, FRST_PEN_STYLE_4,
                 FRST_BRUSH_COLOR_1, FRST_BRUSH_COLOR_2,
                 FRST_BRUSH_COLOR_3, FRST_BRUSH_COLOR_4,
                 FRST_BRUSH_STYLE_1, FRST_BRUSH_STYLE_2,
                 FRST_BRUSH_STYLE_3, FRST_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Glacier", glacierPenList, glacierBorder, glacierBrushList,
                 GLACIER_COLOR_1, GLACIER_COLOR_2, GLACIER_COLOR_3, GLACIER_COLOR_4,
                 GLACIER_PEN_WIDTH_1, GLACIER_PEN_WIDTH_2, GLACIER_PEN_WIDTH_3, GLACIER_PEN_WIDTH_4,
                 GLACIER_PEN_STYLE_1, GLACIER_PEN_STYLE_2, GLACIER_PEN_STYLE_3, GLACIER_PEN_STYLE_4,
                 GLACIER_BRUSH_COLOR_1, GLACIER_BRUSH_COLOR_2,
                 GLACIER_BRUSH_COLOR_3, GLACIER_BRUSH_COLOR_4,
                 GLACIER_BRUSH_STYLE_1, GLACIER_BRUSH_STYLE_2,
                 GLACIER_BRUSH_STYLE_3, GLACIER_BRUSH_STYLE_4)

  READ_PEN_BRUSH("PackIce", packicePenList, packiceBorder, packiceBrushList,
                 PACK_ICE_COLOR_1, PACK_ICE_COLOR_2, PACK_ICE_COLOR_3, PACK_ICE_COLOR_4,
                 PACK_ICE_PEN_WIDTH_1, PACK_ICE_PEN_WIDTH_2, PACK_ICE_PEN_WIDTH_3, PACK_ICE_PEN_WIDTH_4,
                 PACK_ICE_PEN_STYLE_1, PACK_ICE_PEN_STYLE_2, PACK_ICE_PEN_STYLE_3, PACK_ICE_PEN_STYLE_4,
                 PACK_ICE_BRUSH_COLOR_1, PACK_ICE_BRUSH_COLOR_2,
                 PACK_ICE_BRUSH_COLOR_3, PACK_ICE_BRUSH_COLOR_4,
                 PACK_ICE_BRUSH_STYLE_1, PACK_ICE_BRUSH_STYLE_2,
                 PACK_ICE_BRUSH_STYLE_3, PACK_ICE_BRUSH_STYLE_4)

  READ_PEN_BRUSH("River_T", river_tPenList, river_tBorder, river_tBrushList,
                 RIVER_T_COLOR_1, RIVER_T_COLOR_2, RIVER_T_COLOR_3, RIVER_T_COLOR_4,
                 RIVER_T_PEN_WIDTH_1, RIVER_T_PEN_WIDTH_2, RIVER_T_PEN_WIDTH_3, RIVER_T_PEN_WIDTH_4,
                 RIVER_T_PEN_STYLE_1, RIVER_T_PEN_STYLE_2, RIVER_T_PEN_STYLE_3, RIVER_T_PEN_STYLE_4,
                 RIVER_T_BRUSH_COLOR_1, RIVER_T_BRUSH_COLOR_2,
                 RIVER_T_BRUSH_COLOR_3, RIVER_T_BRUSH_COLOR_4,
                 RIVER_T_BRUSH_STYLE_1, RIVER_T_BRUSH_STYLE_2,
                 RIVER_T_BRUSH_STYLE_3, RIVER_T_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace A", airAPenList, airABorder, airABrushList,
                 conf->getBorderColorAirspaceA(), conf->getBorderColorAirspaceA(), conf->getBorderColorAirspaceA(), conf->getBorderColorAirspaceA(),
                 AIRA_PEN_WIDTH_1, AIRA_PEN_WIDTH_2, AIRA_PEN_WIDTH_3, AIRA_PEN_WIDTH_4,
                 AIRA_PEN_STYLE_1, AIRA_PEN_STYLE_2, AIRA_PEN_STYLE_3, AIRA_PEN_STYLE_4,
                 AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
                 AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4,
                 AIRA_BRUSH_STYLE_1, AIRA_BRUSH_STYLE_2,
                 AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace B", airBPenList, airBBorder, airBBrushList,
                 conf->getBorderColorAirspaceB(), conf->getBorderColorAirspaceB(), conf->getBorderColorAirspaceB(), conf->getBorderColorAirspaceB(),
                 AIRB_PEN_WIDTH_1, AIRB_PEN_WIDTH_2, AIRB_PEN_WIDTH_3, AIRB_PEN_WIDTH_4,
                 AIRB_PEN_STYLE_1, AIRB_PEN_STYLE_2, AIRB_PEN_STYLE_3, AIRB_PEN_STYLE_4,
                 AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
                 AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4,
                 AIRB_BRUSH_STYLE_1, AIRB_BRUSH_STYLE_2,
                 AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace C", airCPenList, airCBorder, airCBrushList,
                 conf->getBorderColorAirspaceC(), conf->getBorderColorAirspaceC(), conf->getBorderColorAirspaceC(), conf->getBorderColorAirspaceC(),
                 AIRC_PEN_WIDTH_1, AIRC_PEN_WIDTH_2, AIRC_PEN_WIDTH_3, AIRC_PEN_WIDTH_4,
                 AIRC_PEN_STYLE_1, AIRC_PEN_STYLE_2, AIRC_PEN_STYLE_3, AIRC_PEN_STYLE_4,
                 AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
                 AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4,
                 AIRC_BRUSH_STYLE_1, AIRC_BRUSH_STYLE_2,
                 AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace D", airDPenList, airDBorder, airDBrushList,
                 conf->getBorderColorAirspaceD(), conf->getBorderColorAirspaceD(), conf->getBorderColorAirspaceD(), conf->getBorderColorAirspaceD(),
                 AIRD_PEN_WIDTH_1, AIRD_PEN_WIDTH_2, AIRD_PEN_WIDTH_3, AIRD_PEN_WIDTH_4,
                 AIRD_PEN_STYLE_1, AIRD_PEN_STYLE_2, AIRD_PEN_STYLE_3, AIRD_PEN_STYLE_4,
                 AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
                 AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4,
                 AIRD_BRUSH_STYLE_1, AIRD_BRUSH_STYLE_2,
                 AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace E", airEPenList, airEBorder, airEBrushList,
                 conf->getBorderColorAirspaceE(), conf->getBorderColorAirspaceE(), conf->getBorderColorAirspaceE(), conf->getBorderColorAirspaceE(),
                 AIRE_PEN_WIDTH_1, AIRE_PEN_WIDTH_2, AIRE_PEN_WIDTH_3, AIRE_PEN_WIDTH_4,
                 AIRE_PEN_STYLE_1, AIRE_PEN_STYLE_2, AIRE_PEN_STYLE_3, AIRE_PEN_STYLE_4,
                 AIRE_BRUSH_COLOR_1, AIRE_BRUSH_COLOR_2,
                 AIRE_BRUSH_COLOR_3, AIRE_BRUSH_COLOR_4,
                 AIRE_BRUSH_STYLE_1, AIRE_BRUSH_STYLE_2,
                 AIRE_BRUSH_STYLE_3, AIRE_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace F", airFPenList, airFBorder, airFBrushList,
                 conf->getBorderColorAirspaceF(), conf->getBorderColorAirspaceF(), conf->getBorderColorAirspaceF(), conf->getBorderColorAirspaceF(),
                 AIRF_PEN_WIDTH_1, AIRF_PEN_WIDTH_2, AIRF_PEN_WIDTH_3, AIRF_PEN_WIDTH_4,
                 AIRF_PEN_STYLE_1, AIRF_PEN_STYLE_2, AIRF_PEN_STYLE_3, AIRF_PEN_STYLE_4,
                 AIRF_BRUSH_COLOR_1, AIRF_BRUSH_COLOR_2,
                 AIRF_BRUSH_COLOR_3, AIRF_BRUSH_COLOR_4,
                 AIRF_BRUSH_STYLE_1, AIRF_BRUSH_STYLE_2,
                 AIRF_BRUSH_STYLE_3, AIRF_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Wave Window", waveWindowPenList, waveWindowBorder, waveWindowBrushList,
                 conf->getBorderColorWaveWindow(), conf->getBorderColorWaveWindow(), conf->getBorderColorWaveWindow(), conf->getBorderColorWaveWindow(),
                 WAVE_WINDOW_PEN_WIDTH_1, WAVE_WINDOW_PEN_WIDTH_2, WAVE_WINDOW_PEN_WIDTH_3, WAVE_WINDOW_PEN_WIDTH_4,
                 WAVE_WINDOW_PEN_STYLE_1, WAVE_WINDOW_PEN_STYLE_2, WAVE_WINDOW_PEN_STYLE_3, WAVE_WINDOW_PEN_STYLE_4,
                 WAVE_WINDOW_BRUSH_COLOR_1, WAVE_WINDOW_BRUSH_COLOR_2,
                 WAVE_WINDOW_BRUSH_COLOR_3, WAVE_WINDOW_BRUSH_COLOR_4,
                 WAVE_WINDOW_BRUSH_STYLE_1, WAVE_WINDOW_BRUSH_STYLE_2,
                 WAVE_WINDOW_BRUSH_STYLE_3, WAVE_WINDOW_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Control C", ctrCPenList, ctrCBorder,ctrCBrushList,
                 conf->getBorderColorControlC(), conf->getBorderColorControlC(), conf->getBorderColorControlC(), conf->getBorderColorControlC(),
                 CTRC_PEN_WIDTH_1, CTRC_PEN_WIDTH_2, CTRC_PEN_WIDTH_3, CTRC_PEN_WIDTH_4,
                 CTRC_PEN_STYLE_1, CTRC_PEN_STYLE_2, CTRC_PEN_STYLE_3, CTRC_PEN_STYLE_4,
                 CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
                 CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4,
                 CTRC_BRUSH_STYLE_1, CTRC_BRUSH_STYLE_2,
                 CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Control D", ctrDPenList, ctrDBorder, ctrDBrushList,
                 conf->getBorderColorControlD(), conf->getBorderColorControlD(), conf->getBorderColorControlD(), conf->getBorderColorControlD(),
                 CTRD_PEN_WIDTH_1, CTRD_PEN_WIDTH_2, CTRD_PEN_WIDTH_3, CTRD_PEN_WIDTH_4,
                 CTRD_PEN_STYLE_1, CTRD_PEN_STYLE_2, CTRD_PEN_STYLE_3, CTRD_PEN_STYLE_4,
                 CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
                 CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4,
                 CTRD_BRUSH_STYLE_1, CTRD_BRUSH_STYLE_2,
                 CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Danger", dangerPenList, dangerBorder, dangerBrushList,
                 conf->getBorderColorDanger(), conf->getBorderColorDanger(), conf->getBorderColorDanger(), conf->getBorderColorDanger(),
                 DNG_PEN_WIDTH_1, DNG_PEN_WIDTH_2, DNG_PEN_WIDTH_3, DNG_PEN_WIDTH_4,
                 DNG_PEN_STYLE_1, DNG_PEN_STYLE_2, DNG_PEN_STYLE_3, DNG_PEN_STYLE_4,
                 DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
                 DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4,
                 DNG_BRUSH_STYLE_1, DNG_BRUSH_STYLE_2,
                 DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Low Flight", lowFPenList, lowFBorder,lowFBrushList,
                 conf->getBorderColorLowFlight(), conf->getBorderColorLowFlight(), conf->getBorderColorLowFlight(), conf->getBorderColorLowFlight(),
                 LOWF_PEN_WIDTH_1, LOWF_PEN_WIDTH_2, LOWF_PEN_WIDTH_3, LOWF_PEN_WIDTH_4,
                 LOWF_PEN_STYLE_1, LOWF_PEN_STYLE_2, LOWF_PEN_STYLE_3, LOWF_PEN_STYLE_4,
                 LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
                 LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4,
                 LOWF_BRUSH_STYLE_1, LOWF_BRUSH_STYLE_2,
                 LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Restricted Area", restrPenList, restrBorder, restrBrushList,
                 conf->getBorderColorRestricted(), conf->getBorderColorRestricted(), conf->getBorderColorRestricted(), conf->getBorderColorRestricted(),
                 RES_PEN_WIDTH_1, RES_PEN_WIDTH_2, RES_PEN_WIDTH_3, RES_PEN_WIDTH_4,
                 RES_PEN_STYLE_1, RES_PEN_STYLE_2, RES_PEN_STYLE_3, RES_PEN_STYLE_4,
                 RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
                 RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4,
                 RES_BRUSH_STYLE_1, RES_BRUSH_STYLE_2,
                 RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4)

  READ_PEN_BRUSH("TMZ", tmzPenList, tmzBorder, tmzBrushList,
                 conf->getBorderColorTMZ(), conf->getBorderColorTMZ(), conf->getBorderColorTMZ(), conf->getBorderColorTMZ(),
                 TMZ_PEN_WIDTH_1, TMZ_PEN_WIDTH_2, TMZ_PEN_WIDTH_3, TMZ_PEN_WIDTH_4,
                 TMZ_PEN_STYLE_1, TMZ_PEN_STYLE_2, TMZ_PEN_STYLE_3, TMZ_PEN_STYLE_4,
                 TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
                 TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4,
                 TMZ_BRUSH_STYLE_1, TMZ_BRUSH_STYLE_2,
                 TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Glider Sector", gliderSectorPenList, gliderSectorBorder, gliderSectorBrushList,
                 conf->getBorderColorGliderSector(), conf->getBorderColorGliderSector(), conf->getBorderColorGliderSector(), conf->getBorderColorGliderSector(),
                 GLIDER_SECTOR_PEN_WIDTH_1, GLIDER_SECTOR_PEN_WIDTH_2, GLIDER_SECTOR_PEN_WIDTH_3, GLIDER_SECTOR_PEN_WIDTH_4,
                 GLIDER_SECTOR_PEN_STYLE_1, GLIDER_SECTOR_PEN_STYLE_2, GLIDER_SECTOR_PEN_STYLE_3, GLIDER_SECTOR_PEN_STYLE_4,
                 GLIDER_SECTOR_BRUSH_COLOR_1, GLIDER_SECTOR_BRUSH_COLOR_2,
                 GLIDER_SECTOR_BRUSH_COLOR_3, GLIDER_SECTOR_BRUSH_COLOR_4,
                 GLIDER_SECTOR_BRUSH_STYLE_1, GLIDER_SECTOR_BRUSH_STYLE_2,
                 GLIDER_SECTOR_BRUSH_STYLE_3, GLIDER_SECTOR_BRUSH_STYLE_4)
}

void MapConfig::slotSetMatrixValues(int index, bool sw)
{
  isSwitch = sw;
  scaleIndex = index;
}

const QPen& MapConfig::__getPen(unsigned int typeID, int sIndex)
{
  switch(typeID) {
  case BaseMapElement::Trail:
    return trailPenList.at(sIndex);
  case BaseMapElement::Road:
    return roadPenList.at(sIndex);
  case BaseMapElement::Highway:
    return highwayPenList.at(sIndex);
  case BaseMapElement::Railway:
    return railPenList.at(sIndex);
  case BaseMapElement::Railway_D:
    return rail_dPenList.at(sIndex);
  case BaseMapElement::Aerial_Cable:
    return aerialcablePenList.at(sIndex);
  case BaseMapElement::Lake:
    return lakePenList.at(sIndex);
  case BaseMapElement::River:
    return riverPenList.at(sIndex);
  case BaseMapElement::River_T:
  case BaseMapElement::Lake_T:
    return river_tPenList.at(sIndex);
  case BaseMapElement::Canal:
    return canalPenList.at(sIndex);
  case BaseMapElement::City:
    return cityPenList.at(sIndex);
  case BaseMapElement::AirA:
    return airAPenList.at(sIndex);
  case BaseMapElement::AirB:
    return airBPenList.at(sIndex);
  case BaseMapElement::AirC:
    return airCPenList.at(sIndex);
  case BaseMapElement::AirD:
    return airDPenList.at(sIndex);
  case BaseMapElement::AirE:
    return airEPenList.at(sIndex);
  case BaseMapElement::WaveWindow:
    return waveWindowPenList.at(sIndex);
  case BaseMapElement::AirF:
    return airFPenList.at(sIndex);
  case BaseMapElement::ControlC:
    return ctrCPenList.at(sIndex);
  case BaseMapElement::ControlD:
    return ctrDPenList.at(sIndex);
  case BaseMapElement::Danger:
    return dangerPenList.at(sIndex);
  case BaseMapElement::LowFlight:
    return lowFPenList.at(sIndex);
  case BaseMapElement::Restricted:
    return restrPenList.at(sIndex);
  case BaseMapElement::Tmz:
    return tmzPenList.at(sIndex);
  case BaseMapElement::Forest:
    return forestPenList.at(sIndex);
  case BaseMapElement::GliderSector:
    return gliderSectorPenList.at(sIndex);
  case BaseMapElement::Glacier:
    return glacierPenList.at(sIndex);
  case BaseMapElement::PackIce:
    return packicePenList.at(sIndex);
  default:
    qWarning( "No Pen found for BaseMapElement=%d and Index=%d", typeID, sIndex );
    return roadPenList.at(sIndex);
  }
}

bool MapConfig::isBorder(unsigned int typeID)
{
  switch(typeID) {
  case BaseMapElement::Trail:
    return trailBorder[scaleIndex];
  case BaseMapElement::Road:
    return roadBorder[scaleIndex];
  case BaseMapElement::Highway:
    return highwayBorder[scaleIndex];
  case BaseMapElement::Railway:
    return railBorder[scaleIndex];
  case BaseMapElement::Railway_D:
    return rail_dBorder[scaleIndex];
  case BaseMapElement::Aerial_Cable:
    return aerialcableBorder[scaleIndex];
  case BaseMapElement::Canal:
    return canalBorder[scaleIndex];
  case BaseMapElement::River:
  case BaseMapElement::Lake:
    return riverBorder[scaleIndex];
  case BaseMapElement::River_T:
  case BaseMapElement::Lake_T:
    return river_tBorder[scaleIndex];
  case BaseMapElement::City:
    return cityBorder[scaleIndex];
  case BaseMapElement::AirA:
    return airABorder[scaleIndex];
  case BaseMapElement::AirB:
    return airBBorder[scaleIndex];
  case BaseMapElement::AirC:
    return airCBorder[scaleIndex];
  case BaseMapElement::AirD:
    return airDBorder[scaleIndex];
  case BaseMapElement::AirE:
    return airEBorder[scaleIndex];
  case BaseMapElement::WaveWindow:
    return waveWindowBorder[scaleIndex];
  case BaseMapElement::AirF:
    return airFBorder[scaleIndex];
  case BaseMapElement::ControlC:
    return ctrCBorder[scaleIndex];
  case BaseMapElement::ControlD:
    return ctrDBorder[scaleIndex];
  case BaseMapElement::Danger:
    return dangerBorder[scaleIndex];
  case BaseMapElement::LowFlight:
    return lowFBorder[scaleIndex];
  case BaseMapElement::Restricted:
    return restrBorder[scaleIndex];
  case BaseMapElement::Tmz:
    return tmzBorder[scaleIndex];
  case BaseMapElement::Forest:
    return forestBorder[scaleIndex];
  case BaseMapElement::GliderSector:
    return gliderSectorBorder[scaleIndex];
  case BaseMapElement::Glacier:
    return glacierBorder[scaleIndex];
  case BaseMapElement::PackIce:
    return packiceBorder[scaleIndex];
  }

  qWarning( "No Border found for BaseMapElement=%d", typeID );

  /* Should never happen ... */
  return true;
}

const QBrush& MapConfig::__getBrush(unsigned int typeID, int sIndex)
{
  static const QBrush defaultBrush; // default brush

  switch(typeID) {
  case BaseMapElement::City:
    return cityBrushList.at(sIndex);
  case BaseMapElement::Lake:
    return lakeBrushList.at(sIndex);
  case BaseMapElement::AirA:
    return airABrushList.at(sIndex);
  case BaseMapElement::AirB:
    return airBBrushList.at(sIndex);
  case BaseMapElement::AirC:
    return airCBrushList.at(sIndex);
  case BaseMapElement::AirD:
    return airDBrushList.at(sIndex);
  case BaseMapElement::AirE:
    return airEBrushList.at(sIndex);
  case BaseMapElement::WaveWindow:
    return waveWindowBrushList.at(sIndex);
  case BaseMapElement::AirF:
    return airFBrushList.at(sIndex);
  case BaseMapElement::ControlC:
    return ctrCBrushList.at(sIndex);
  case BaseMapElement::ControlD:
    return ctrDBrushList.at(sIndex);
  case BaseMapElement::Danger:
    return dangerBrushList.at(sIndex);
  case BaseMapElement::LowFlight:
    return lowFBrushList.at(sIndex);
  case BaseMapElement::Restricted:
    return restrBrushList.at(sIndex);
  case BaseMapElement::Tmz:
    return tmzBrushList.at(sIndex);
  case BaseMapElement::Forest:
    return forestBrushList.at(sIndex);
  case BaseMapElement::GliderSector:
    return gliderSectorBrushList.at(sIndex);
  case BaseMapElement::River_T:
    return river_tBrushList.at(sIndex);
  case BaseMapElement::Glacier:
    return glacierBrushList.at(sIndex);
  case BaseMapElement::PackIce:
    return packiceBrushList.at(sIndex);
  }

  qWarning( "No Brush found for BaseMapElement=%d", typeID );
  return defaultBrush;
}

QPixmap MapConfig::getPixmapRotatable(unsigned int typeID, bool isWinch)
{
  QString iconName(getPixmapName(typeID, isWinch, true));

  // qDebug("PixmapNameRot: %d %s",typeID, iconName.latin1() );

  if(isSwitch)
    return GeneralConfig::instance()->loadPixmap(iconName);
  else
    return GeneralConfig::instance()->loadPixmap("small/" + iconName);
}

QPixmap MapConfig::getPixmap(unsigned int typeID, bool isWinch, bool smallIcon)
{
  QString iconName(getPixmapName(typeID, isWinch));

  // qDebug("getPixmapName,Winch,SmallIcon: %d %s",typeID, iconName.latin1() );

  if(smallIcon)
    return GeneralConfig::instance()->loadPixmap("small/" + iconName);
  else
    return GeneralConfig::instance()->loadPixmap(iconName);
}

QPixmap MapConfig::getPixmap(unsigned int typeID, bool isWinch, QColor color)
{
  QString iconName(getPixmapName(typeID, isWinch, false, color));

  // qDebug("getPixmapName,Winch,Color: %d %s",typeID, iconName.latin1() );

  if(isSwitch)
    return GeneralConfig::instance()->loadPixmap(iconName);
  else
    return GeneralConfig::instance()->loadPixmap("small/" + iconName);
}

QPixmap MapConfig::getPixmap(QString iconName)
{
  // qDebug("getPixmapName: %s", iconName.latin1() );

  if(isSwitch)
    return GeneralConfig::instance()->loadPixmap(iconName);
  else
    return GeneralConfig::instance()->loadPixmap("small/" + iconName);
}

bool MapConfig::isRotatable( unsigned int typeID ) const
{
  switch(typeID) {
  case BaseMapElement::Airport:
  case BaseMapElement::IntAirport:
  case BaseMapElement::CivMilAirport:
  case BaseMapElement::Airfield:
  case BaseMapElement::Glidersite:
    return true;
  default:
    return false;
  }
}

QString MapConfig::getPixmapName(unsigned int typeID, bool isWinch, bool rotatable, QColor /*color*/ )
{
  QString iconName;

  switch(typeID) {
  case BaseMapElement::Airport:
  case BaseMapElement::IntAirport:
    iconName = "airport";
    break;
  case BaseMapElement::MilAirport:
    iconName = "milairport";
    break;
  case BaseMapElement::CivMilAirport:
    iconName = "civmilair";
    break;
  case BaseMapElement::Airfield:
    iconName = "airfield";
    break;
  case BaseMapElement::ClosedAirfield:
    iconName = "closed";
    break;
  case BaseMapElement::CivHeliport:
    iconName = "civheliport";
    break;
  case BaseMapElement::MilHeliport:
    iconName = "milheliport";
    break;
  case BaseMapElement::AmbHeliport:
    iconName = "ambheliport";
    break;
  case BaseMapElement::Glidersite:
    if(isWinch)
      iconName = "glider_winch";
    else
      iconName = "glider";
    break;
  case BaseMapElement::UltraLight:
    iconName = "ul";
    break;
  case BaseMapElement::HangGlider:
    iconName = "paraglider";
    break;
  case BaseMapElement::Parachute:
    iconName = "jump";
    break;
  case BaseMapElement::Balloon:
    iconName = "balloon";
    break;
  case BaseMapElement::CompPoint:
    iconName = "compoint";
    break;
  case BaseMapElement::Landmark:
    iconName = "landmark";
    break;
  case BaseMapElement::Outlanding:
    iconName = "outlanding";
    break;
  case BaseMapElement::Vor:
    iconName = "vor";
    break;
  case BaseMapElement::VorDme:
    iconName = "vordme";
    break;
  case BaseMapElement::VorTac:
    iconName = "vortac";
    break;
  case BaseMapElement::Ndb:
    iconName = "ndb";
    break;
  case BaseMapElement::Obstacle:
    iconName = "obstacle";
    break;
  case BaseMapElement::LightObstacle:
    iconName = "obst_light";
    break;
  case BaseMapElement::ObstacleGroup:
    iconName = "obst_group";
    break;
  case BaseMapElement::LightObstacleGroup:
    iconName = "obst_group_light";
    break;
  case BaseMapElement::Village:
    iconName = "village";
    break;
  case BaseMapElement::Railway:
    iconName = "railway";
    break;
  case BaseMapElement::AerialRailway:
    iconName = "waypoint";
    break;
  case BaseMapElement::Turnpoint:
    iconName = "waypoint";
    break;
  case BaseMapElement::Thermal:
    iconName = "thermal";
    break;
  case BaseMapElement::City:
    iconName = "waypoint";
    break;
  case BaseMapElement::EmptyPoint:
    iconName = "empty";
    break;

  default:
    iconName = "empty";
    break;
  }
  if( rotatable )
    iconName += "-18.png";  // airfield icons can be rotated 10 degree wise
  else
    iconName += ".xpm";
  return iconName;
}
