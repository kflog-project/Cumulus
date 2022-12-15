/***********************************************************************
 **
 **   mapconfig.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001      by Heiner Lamprecht,
 **                   2002      by André Somers
 **                   2008-2018 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <cmath>

#include <QtGui>

#include "basemapelement.h"
#include "generalconfig.h"
#include "layout.h"
#include "mapcalc.h"
#include "mapconfig.h"
#include "mapdefaults.h"

// Different macros used by read method for configuration data
#define READ_BORDER(a) \
  a[0] = true;         \
  a[1] = true;         \
  a[2] = true;         \
  a[3] = true;

#define READ_PEN(G, A, B, C1, C2, C3, C4, P1, P2, P3, P4,  \
                 S1, S2, S3, S4)                           \
  READ_BORDER(B)                                           \
  A[0] = (QPen(C1, P1, (Qt::PenStyle)S1));                 \
  A[1] = (QPen(C2, P2, (Qt::PenStyle)S2));                 \
  A[2] = (QPen(C3, P3, (Qt::PenStyle)S3));                 \
  A[3] = (QPen(C4, P4, (Qt::PenStyle)S4));

#define READ_PEN_BRUSH(G, a, B, A, C1, C2, C3, C4, P1, P2, P3, P4,       \
                       S1, S2, S3, S4, C7, C8, C9, C10, S7, S8, S9, S10) \
  READ_PEN(G, a, B, C1, C2, C3, C4, P1, P2, P3, P4,                      \
           S1, S2, S3, S4)                                               \
  A[0] = (QBrush(C7,  (Qt::BrushStyle)S7));                              \
  A[1] = (QBrush(C8,  (Qt::BrushStyle)S8));                              \
  A[2] = (QBrush(C9,  (Qt::BrushStyle)S9));                              \
  A[3] = (QBrush(C10, (Qt::BrushStyle)S10));

// number of created class instances
short MapConfig::instances = 0;

MapConfig::MapConfig(QObject* parent) :
  QObject(parent),
  scaleIndex(0),
  isSwitch(false)
{
  if ( ++instances > 1 )
    {
      // There exists already a class instance as singleton.
      return;
    }

  ++instances;

  // create QIcons with background for copying later when needed
  // in airfield list; speeds up list display

  unsigned int airfieldType[13] = { BaseMapElement::IntAirport,
				    BaseMapElement::Airport,
				    BaseMapElement::MilAirport,
				    BaseMapElement::CivMilAirport,
				    BaseMapElement::Airfield,
				    BaseMapElement::ClosedAirfield,
				    BaseMapElement::CivHeliport,
				    BaseMapElement::MilHeliport,
				    BaseMapElement::AmbHeliport,
				    BaseMapElement::Gliderfield,
				    BaseMapElement::UltraLight,
				    BaseMapElement::HangGlider,
				    BaseMapElement::Outlanding
				  };
  QPixmap selectPixmap;
  QIcon afIcon;
  QPainter pnt;

  for ( int i=0; i<13; i++ )
    {
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

  READ_PEN("Motorway", motorwayPenList, motorwayBorder,
           MOTORWAY_COLOR_1, MOTORWAY_COLOR_2, MOTORWAY_COLOR_3, MOTORWAY_COLOR_4,
           MOTORWAY_PEN_WIDTH_1, MOTORWAY_PEN_WIDTH_2, MOTORWAY_PEN_WIDTH_3, MOTORWAY_PEN_WIDTH_4,
           MOTORWAY_PEN_STYLE_1, MOTORWAY_PEN_STYLE_2, MOTORWAY_PEN_STYLE_3, MOTORWAY_PEN_STYLE_4)

  READ_PEN_BRUSH("Lake", lakePenList, lakeBorder, lakeBrushList,
                 LAKE_COLOR_1, LAKE_COLOR_2, LAKE_COLOR_3, LAKE_COLOR_4,
                 LAKE_PEN_WIDTH_1, LAKE_PEN_WIDTH_2, LAKE_PEN_WIDTH_3, LAKE_PEN_WIDTH_4,
                 LAKE_PEN_STYLE_1, LAKE_PEN_STYLE_2, LAKE_PEN_STYLE_3, LAKE_PEN_STYLE_4,
                 LAKE_BRUSH_COLOR_1, LAKE_BRUSH_COLOR_2,
                 LAKE_BRUSH_COLOR_3, LAKE_BRUSH_COLOR_4,
                 LAKE_BRUSH_STYLE_1, LAKE_BRUSH_STYLE_2,
                 LAKE_BRUSH_STYLE_3, LAKE_BRUSH_STYLE_4)

  READ_PEN_BRUSH("City", cityPenList, cityBorder, cityBrushList,
                 CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3, CITY_COLOR_4,
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

  /* That loads all airspace colors and brush data. */
  slotReloadAirspaceColors();
}

/**
 * Airspace colors can be modified by the user in the airspace settings
 * configuration widget. In such a case all color data in the lists must
 * be reloaded.
 */
void MapConfig::slotReloadAirspaceColors()
{
  // reload all color data
  GeneralConfig *conf = GeneralConfig::instance();

  READ_PEN_BRUSH("Airspace A", airAPenList, airABorder, airABrushList,
                 conf->getBorderColorAirspaceA(), conf->getBorderColorAirspaceA(),
                 conf->getBorderColorAirspaceA(), conf->getBorderColorAirspaceA(),
                 AIRA_PEN_WIDTH_1, AIRA_PEN_WIDTH_2, AIRA_PEN_WIDTH_3, AIRA_PEN_WIDTH_4,
                 AIRA_PEN_STYLE_1, AIRA_PEN_STYLE_2, AIRA_PEN_STYLE_3, AIRA_PEN_STYLE_4,
                 conf->getFillColorAirspaceA(), conf->getFillColorAirspaceA(),
                 conf->getFillColorAirspaceA(), conf->getFillColorAirspaceA(),
                 AIRA_BRUSH_STYLE_1, AIRA_BRUSH_STYLE_2,
                 AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace B", airBPenList, airBBorder, airBBrushList,
                 conf->getBorderColorAirspaceB(), conf->getBorderColorAirspaceB(),
                 conf->getBorderColorAirspaceB(), conf->getBorderColorAirspaceB(),
                 AIRB_PEN_WIDTH_1, AIRB_PEN_WIDTH_2, AIRB_PEN_WIDTH_3, AIRB_PEN_WIDTH_4,
                 AIRB_PEN_STYLE_1, AIRB_PEN_STYLE_2, AIRB_PEN_STYLE_3, AIRB_PEN_STYLE_4,
                 conf->getFillColorAirspaceB(), conf->getFillColorAirspaceB(),
                 conf->getFillColorAirspaceB(), conf->getFillColorAirspaceB(),
                 AIRB_BRUSH_STYLE_1, AIRB_BRUSH_STYLE_2,
                 AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace C", airCPenList, airCBorder, airCBrushList,
                 conf->getBorderColorAirspaceC(), conf->getBorderColorAirspaceC(),
                 conf->getBorderColorAirspaceC(), conf->getBorderColorAirspaceC(),
                 AIRC_PEN_WIDTH_1, AIRC_PEN_WIDTH_2, AIRC_PEN_WIDTH_3, AIRC_PEN_WIDTH_4,
                 AIRC_PEN_STYLE_1, AIRC_PEN_STYLE_2, AIRC_PEN_STYLE_3, AIRC_PEN_STYLE_4,
                 conf->getFillColorAirspaceC(), conf->getFillColorAirspaceC(),
                 conf->getFillColorAirspaceC(), conf->getFillColorAirspaceC(),
                 AIRC_BRUSH_STYLE_1, AIRC_BRUSH_STYLE_2,
                 AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace D", airDPenList, airDBorder, airDBrushList,
                 conf->getBorderColorAirspaceD(), conf->getBorderColorAirspaceD(),
                 conf->getBorderColorAirspaceD(), conf->getBorderColorAirspaceD(),
                 AIRD_PEN_WIDTH_1, AIRD_PEN_WIDTH_2, AIRD_PEN_WIDTH_3, AIRD_PEN_WIDTH_4,
                 AIRD_PEN_STYLE_1, AIRD_PEN_STYLE_2, AIRD_PEN_STYLE_3, AIRD_PEN_STYLE_4,
                 conf->getFillColorAirspaceD(), conf->getFillColorAirspaceD(),
                 conf->getFillColorAirspaceD(), conf->getFillColorAirspaceD(),
                 AIRD_BRUSH_STYLE_1, AIRD_BRUSH_STYLE_2,
                 AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace E", airEPenList, airEBorder, airEBrushList,
                 conf->getBorderColorAirspaceE(), conf->getBorderColorAirspaceE(),
                 conf->getBorderColorAirspaceE(), conf->getBorderColorAirspaceE(),
                 AIRE_PEN_WIDTH_1, AIRE_PEN_WIDTH_2, AIRE_PEN_WIDTH_3, AIRE_PEN_WIDTH_4,
                 AIRE_PEN_STYLE_1, AIRE_PEN_STYLE_2, AIRE_PEN_STYLE_3, AIRE_PEN_STYLE_4,
                 conf->getFillColorAirspaceE(), conf->getFillColorAirspaceE(),
                 conf->getFillColorAirspaceE(), conf->getFillColorAirspaceE(),
                 AIRE_BRUSH_STYLE_1, AIRE_BRUSH_STYLE_2,
                 AIRE_BRUSH_STYLE_3, AIRE_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace F", airFPenList, airFBorder, airFBrushList,
                 conf->getBorderColorAirspaceF(), conf->getBorderColorAirspaceF(),
                 conf->getBorderColorAirspaceF(), conf->getBorderColorAirspaceF(),
                 AIRF_PEN_WIDTH_1, AIRF_PEN_WIDTH_2, AIRF_PEN_WIDTH_3, AIRF_PEN_WIDTH_4,
                 AIRF_PEN_STYLE_1, AIRF_PEN_STYLE_2, AIRF_PEN_STYLE_3, AIRF_PEN_STYLE_4,
                 conf->getFillColorAirspaceF(), conf->getFillColorAirspaceF(),
                 conf->getFillColorAirspaceF(), conf->getFillColorAirspaceF(),
                 AIRF_BRUSH_STYLE_1, AIRF_BRUSH_STYLE_2,
                 AIRF_BRUSH_STYLE_3, AIRF_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace FLARM", airFlarmPenList, airFlarmBorder, airFlarmBrushList,
                 conf->getBorderColorAirspaceFlarm(), conf->getBorderColorAirspaceFlarm(),
                 conf->getBorderColorAirspaceFlarm(), conf->getBorderColorAirspaceFlarm(),
                 AIRFLARM_PEN_WIDTH_1, AIRFLARM_PEN_WIDTH_2,
                 AIRFLARM_PEN_WIDTH_3, AIRFLARM_PEN_WIDTH_4,
                 AIRFLARM_PEN_STYLE_1, AIRFLARM_PEN_STYLE_2,
                 AIRFLARM_PEN_STYLE_3, AIRFLARM_PEN_STYLE_4,
                 conf->getFillColorAirspaceFlarm(), conf->getFillColorAirspaceFlarm(),
                 conf->getFillColorAirspaceFlarm(), conf->getFillColorAirspaceFlarm(),
                 AIRFLARM_BRUSH_STYLE_1, AIRFLARM_BRUSH_STYLE_2,
                 AIRFLARM_BRUSH_STYLE_3, AIRFLARM_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace FIR", airFirPenList, airFirBorder, airFirBrushList,
                  conf->getBorderColorAirspaceFir(), conf->getBorderColorAirspaceFir(),
                  conf->getBorderColorAirspaceFir(), conf->getBorderColorAirspaceFir(),
                  AIRFIR_PEN_WIDTH_1, AIRFIR_PEN_WIDTH_2,
                  AIRFIR_PEN_WIDTH_3, AIRFIR_PEN_WIDTH_4,
                  AIRFIR_PEN_STYLE_1, AIRFIR_PEN_STYLE_2,
                  AIRFIR_PEN_STYLE_3, AIRFIR_PEN_STYLE_4,
                  conf->getFillColorAirspaceFir(), conf->getFillColorAirspaceFir(),
                  conf->getFillColorAirspaceFir(), conf->getFillColorAirspaceFir(),
                  AIRFIR_BRUSH_STYLE_1, AIRFIR_BRUSH_STYLE_2,
                  AIRFIR_BRUSH_STYLE_3, AIRFIR_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Airspace G", airGPenList, airGBorder, airGBrushList,
                 conf->getBorderColorAirspaceG(), conf->getBorderColorAirspaceG(),
                 conf->getBorderColorAirspaceG(), conf->getBorderColorAirspaceG(),
                 AIRG_PEN_WIDTH_1, AIRG_PEN_WIDTH_2, AIRG_PEN_WIDTH_3, AIRG_PEN_WIDTH_4,
                 AIRG_PEN_STYLE_1, AIRG_PEN_STYLE_2, AIRG_PEN_STYLE_3, AIRG_PEN_STYLE_4,
                 conf->getFillColorAirspaceG(), conf->getFillColorAirspaceG(),
                 conf->getFillColorAirspaceG(), conf->getFillColorAirspaceG(),
                 AIRG_BRUSH_STYLE_1, AIRG_BRUSH_STYLE_2,
                 AIRG_BRUSH_STYLE_3, AIRG_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Wave Window", waveWindowPenList, waveWindowBorder, waveWindowBrushList,
                 conf->getBorderColorWaveWindow(), conf->getBorderColorWaveWindow(),
                 conf->getBorderColorWaveWindow(), conf->getBorderColorWaveWindow(),
                 WAVE_WINDOW_PEN_WIDTH_1, WAVE_WINDOW_PEN_WIDTH_2, WAVE_WINDOW_PEN_WIDTH_3, WAVE_WINDOW_PEN_WIDTH_4,
                 WAVE_WINDOW_PEN_STYLE_1, WAVE_WINDOW_PEN_STYLE_2, WAVE_WINDOW_PEN_STYLE_3, WAVE_WINDOW_PEN_STYLE_4,
                 conf->getFillColorWaveWindow(), conf->getFillColorWaveWindow(),
                 conf->getFillColorWaveWindow(), conf->getFillColorWaveWindow(),
                 WAVE_WINDOW_BRUSH_STYLE_1, WAVE_WINDOW_BRUSH_STYLE_2,
                 WAVE_WINDOW_BRUSH_STYLE_3, WAVE_WINDOW_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Control", ctrPenList, ctrBorder,ctrBrushList,
		 conf->getBorderColorControl(), conf->getBorderColorControl(),
		 conf->getBorderColorControl(), conf->getBorderColorControl(),
		 CTR_PEN_WIDTH_1, CTR_PEN_WIDTH_2, CTR_PEN_WIDTH_3, CTR_PEN_WIDTH_4,
		 CTR_PEN_STYLE_1, CTR_PEN_STYLE_2, CTR_PEN_STYLE_3, CTR_PEN_STYLE_4,
		 conf->getFillColorControl(), conf->getFillColorControl(),
		 conf->getFillColorControl(), conf->getFillColorControl(),
		 CTR_BRUSH_STYLE_1, CTR_BRUSH_STYLE_2,
		 CTR_BRUSH_STYLE_3, CTR_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Control C", ctrCPenList, ctrCBorder,ctrCBrushList,
                 conf->getBorderColorControlC(), conf->getBorderColorControlC(),
                 conf->getBorderColorControlC(), conf->getBorderColorControlC(),
                 CTRC_PEN_WIDTH_1, CTRC_PEN_WIDTH_2, CTRC_PEN_WIDTH_3, CTRC_PEN_WIDTH_4,
                 CTRC_PEN_STYLE_1, CTRC_PEN_STYLE_2, CTRC_PEN_STYLE_3, CTRC_PEN_STYLE_4,
                 conf->getFillColorControlC(), conf->getFillColorControlC(),
                 conf->getFillColorControlC(), conf->getFillColorControlC(),
                 CTRC_BRUSH_STYLE_1, CTRC_BRUSH_STYLE_2,
                 CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Control D", ctrDPenList, ctrDBorder, ctrDBrushList,
                 conf->getBorderColorControlD(), conf->getBorderColorControlD(),
                 conf->getBorderColorControlD(), conf->getBorderColorControlD(),
                 CTRD_PEN_WIDTH_1, CTRD_PEN_WIDTH_2, CTRD_PEN_WIDTH_3, CTRD_PEN_WIDTH_4,
                 CTRD_PEN_STYLE_1, CTRD_PEN_STYLE_2, CTRD_PEN_STYLE_3, CTRD_PEN_STYLE_4,
                 conf->getFillColorControlD(), conf->getFillColorControlD(),
                 conf->getFillColorControlD(), conf->getFillColorControlD(),
                 CTRD_BRUSH_STYLE_1, CTRD_BRUSH_STYLE_2,
                 CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Danger", dangerPenList, dangerBorder, dangerBrushList,
                 conf->getBorderColorDanger(), conf->getBorderColorDanger(),
                 conf->getBorderColorDanger(), conf->getBorderColorDanger(),
                 DNG_PEN_WIDTH_1, DNG_PEN_WIDTH_2, DNG_PEN_WIDTH_3, DNG_PEN_WIDTH_4,
                 DNG_PEN_STYLE_1, DNG_PEN_STYLE_2, DNG_PEN_STYLE_3, DNG_PEN_STYLE_4,
                 conf->getFillColorDanger(), conf->getFillColorDanger(),
                 conf->getFillColorDanger(), conf->getFillColorDanger(),
                 DNG_BRUSH_STYLE_1, DNG_BRUSH_STYLE_2,
                 DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4)

   READ_PEN_BRUSH("Prohibited", prohibitedPenList, prohibitedBorder, prohibitedBrushList,
                 conf->getBorderColorProhibited(), conf->getBorderColorProhibited(),
                 conf->getBorderColorProhibited(), conf->getBorderColorProhibited(),
                 PRO_PEN_WIDTH_1, PRO_PEN_WIDTH_2, PRO_PEN_WIDTH_3, PRO_PEN_WIDTH_4,
                 PRO_PEN_STYLE_1, PRO_PEN_STYLE_2, PRO_PEN_STYLE_3, PRO_PEN_STYLE_4,
                 conf->getFillColorProhibited(), conf->getFillColorProhibited(),
                 conf->getFillColorProhibited(), conf->getFillColorProhibited(),
                 PRO_BRUSH_STYLE_1, PRO_BRUSH_STYLE_2,
                 PRO_BRUSH_STYLE_3, PRO_BRUSH_STYLE_4)

  READ_PEN_BRUSH("SUA", suaPenList, suaBorder, suaBrushList,
                 conf->getBorderColorSUA(), conf->getBorderColorSUA(),
                 conf->getBorderColorSUA(), conf->getBorderColorSUA(),
                 SUA_PEN_WIDTH_1, SUA_PEN_WIDTH_2, SUA_PEN_WIDTH_3, SUA_PEN_WIDTH_4,
                 SUA_PEN_STYLE_1, SUA_PEN_STYLE_2, SUA_PEN_STYLE_3, SUA_PEN_STYLE_4,
                 conf->getFillColorSUA(), conf->getFillColorSUA(),
                 conf->getFillColorSUA(), conf->getFillColorSUA(),
                 SUA_BRUSH_STYLE_1, SUA_BRUSH_STYLE_2,
                 SUA_BRUSH_STYLE_3, SUA_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Restricted Area", restrPenList, restrBorder, restrBrushList,
                 conf->getBorderColorRestricted(), conf->getBorderColorRestricted(),
                 conf->getBorderColorRestricted(), conf->getBorderColorRestricted(),
                 RES_PEN_WIDTH_1, RES_PEN_WIDTH_2, RES_PEN_WIDTH_3, RES_PEN_WIDTH_4,
                 RES_PEN_STYLE_1, RES_PEN_STYLE_2, RES_PEN_STYLE_3, RES_PEN_STYLE_4,
                 conf->getFillColorRestricted(), conf->getFillColorRestricted(),
                 conf->getFillColorRestricted(), conf->getFillColorRestricted(),
                 RES_BRUSH_STYLE_1, RES_BRUSH_STYLE_2,
                 RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4)

  READ_PEN_BRUSH("RMZ", rmzPenList, rmzBorder, rmzBrushList,
		 conf->getBorderColorRMZ(), conf->getBorderColorRMZ(),
		 conf->getBorderColorRMZ(), conf->getBorderColorRMZ(),
		 RMZ_PEN_WIDTH_1, RMZ_PEN_WIDTH_2, RMZ_PEN_WIDTH_3, RMZ_PEN_WIDTH_4,
		 RMZ_PEN_STYLE_1, RMZ_PEN_STYLE_2, RMZ_PEN_STYLE_3, RMZ_PEN_STYLE_4,
		 conf->getFillColorRMZ(), conf->getFillColorRMZ(),
		 conf->getFillColorRMZ(), conf->getFillColorRMZ(),
		 RMZ_BRUSH_STYLE_1, RMZ_BRUSH_STYLE_2,
		 RMZ_BRUSH_STYLE_3, RMZ_BRUSH_STYLE_4)

  READ_PEN_BRUSH("TMZ", tmzPenList, tmzBorder, tmzBrushList,
                 conf->getBorderColorTMZ(), conf->getBorderColorTMZ(),
                 conf->getBorderColorTMZ(), conf->getBorderColorTMZ(),
                 TMZ_PEN_WIDTH_1, TMZ_PEN_WIDTH_2, TMZ_PEN_WIDTH_3, TMZ_PEN_WIDTH_4,
                 TMZ_PEN_STYLE_1, TMZ_PEN_STYLE_2, TMZ_PEN_STYLE_3, TMZ_PEN_STYLE_4,
                 conf->getFillColorTMZ(), conf->getFillColorTMZ(),
                 conf->getFillColorTMZ(), conf->getFillColorTMZ(),
                 TMZ_BRUSH_STYLE_1, TMZ_BRUSH_STYLE_2,
                 TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4)

  READ_PEN_BRUSH("Glider Sector", gliderSectorPenList, gliderSectorBorder, gliderSectorBrushList,
                 conf->getBorderColorGliderSector(), conf->getBorderColorGliderSector(),
                 conf->getBorderColorGliderSector(), conf->getBorderColorGliderSector(),
                 GLIDER_SECTOR_PEN_WIDTH_1, GLIDER_SECTOR_PEN_WIDTH_2, GLIDER_SECTOR_PEN_WIDTH_3, GLIDER_SECTOR_PEN_WIDTH_4,
                 GLIDER_SECTOR_PEN_STYLE_1, GLIDER_SECTOR_PEN_STYLE_2, GLIDER_SECTOR_PEN_STYLE_3, GLIDER_SECTOR_PEN_STYLE_4,
                 conf->getFillColorGliderSector(), conf->getFillColorGliderSector(),
                 conf->getFillColorGliderSector(), conf->getFillColorGliderSector(),
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
  if ( sIndex < 0 && sIndex > 3 )
    {
      qWarning( "MapConfig::__getBrush: sIndex(%d) out of range 1...3", sIndex );
      sIndex = 1;
    }

  switch (typeID)
    {
    case BaseMapElement::Trail:
      return trailPenList[sIndex];
    case BaseMapElement::Road:
      return roadPenList[sIndex];
    case BaseMapElement::Motorway:
      return motorwayPenList[sIndex];
    case BaseMapElement::Railway:
      return railPenList[sIndex];
    case BaseMapElement::Railway_D:
      return rail_dPenList[sIndex];
    case BaseMapElement::Aerial_Cable:
      return aerialcablePenList[sIndex];
    case BaseMapElement::Lake:
      return lakePenList[sIndex];
    case BaseMapElement::River:
      return riverPenList[sIndex];
    case BaseMapElement::River_T:
    case BaseMapElement::Lake_T:
      return river_tPenList[sIndex];
    case BaseMapElement::Canal:
      return canalPenList[sIndex];
    case BaseMapElement::City:
      return cityPenList[sIndex];
    case BaseMapElement::AirA:
      return airAPenList[sIndex];
    case BaseMapElement::AirB:
      return airBPenList[sIndex];
    case BaseMapElement::AirC:
      return airCPenList[sIndex];
    case BaseMapElement::AirD:
      return airDPenList[sIndex];
    case BaseMapElement::AirE:
      return airEPenList[sIndex];
    case BaseMapElement::WaveWindow:
      return waveWindowPenList[sIndex];
    case BaseMapElement::AirF:
      return airFPenList[sIndex];
    case BaseMapElement::AirFlarm:
      return airFlarmPenList[sIndex];
    case BaseMapElement::AirFir:
      return airFirPenList[sIndex];
    case BaseMapElement::AirG:
      return airGPenList[sIndex];
    case BaseMapElement::Ctr:
      return ctrPenList[sIndex];
    case BaseMapElement::ControlC:
      return ctrCPenList[sIndex];
    case BaseMapElement::ControlD:
      return ctrDPenList[sIndex];
    case BaseMapElement::Danger:
      return dangerPenList[sIndex];
    case BaseMapElement::Prohibited:
      return prohibitedPenList[sIndex];
    case BaseMapElement::Sua:
      return suaPenList[sIndex];
    case BaseMapElement::Restricted:
      return restrPenList[sIndex];
    case BaseMapElement::Rmz:
      return rmzPenList[sIndex];
    case BaseMapElement::Tmz:
      return tmzPenList[sIndex];
    case BaseMapElement::Forest:
      return forestPenList[sIndex];
    case BaseMapElement::GliderSector:
      return gliderSectorPenList[sIndex];
    case BaseMapElement::Glacier:
      return glacierPenList[sIndex];
    case BaseMapElement::PackIce:
      return packicePenList[sIndex];
    default:
      qWarning( "No Pen found for BaseMapElement=%d and Index=%d", typeID, sIndex );
      return roadPenList[sIndex];
    }
}

bool MapConfig::isBorder(unsigned int typeID)
{
  switch (typeID)
    {
    case BaseMapElement::Trail:
      return trailBorder[scaleIndex];
    case BaseMapElement::Road:
      return roadBorder[scaleIndex];
    case BaseMapElement::Motorway:
      return motorwayBorder[scaleIndex];
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
    case BaseMapElement::AirFlarm:
      return airFlarmBorder[scaleIndex];
    case BaseMapElement::AirFir:
      return airFirBorder[scaleIndex];
    case BaseMapElement::AirG:
      return airGBorder[scaleIndex];
    case BaseMapElement::Ctr:
      return ctrBorder[scaleIndex];
    case BaseMapElement::ControlC:
      return ctrCBorder[scaleIndex];
    case BaseMapElement::ControlD:
      return ctrDBorder[scaleIndex];
    case BaseMapElement::Danger:
      return dangerBorder[scaleIndex];
    case BaseMapElement::Prohibited:
      return prohibitedBorder[scaleIndex];
    case BaseMapElement::Sua:
      return suaBorder[scaleIndex];
    case BaseMapElement::Restricted:
      return restrBorder[scaleIndex];
    case BaseMapElement::Rmz:
      return rmzBorder[scaleIndex];
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

  if ( sIndex < 0 && sIndex > 3 )
    {
      qWarning( "MapConfig::__getBrush: sIndex(%d) out of range 1...3", sIndex );
      return defaultBrush;
    }

  switch (typeID)
    {
    case BaseMapElement::City:
      return cityBrushList[sIndex];
    case BaseMapElement::Lake:
      return lakeBrushList[sIndex];
    case BaseMapElement::AirA:
      return airABrushList[sIndex];
    case BaseMapElement::AirB:
      return airBBrushList[sIndex];
    case BaseMapElement::AirC:
      return airCBrushList[sIndex];
    case BaseMapElement::AirD:
      return airDBrushList[sIndex];
    case BaseMapElement::AirE:
      return airEBrushList[sIndex];
    case BaseMapElement::WaveWindow:
      return waveWindowBrushList[sIndex];
    case BaseMapElement::AirF:
      return airFBrushList[sIndex];
    case BaseMapElement::AirFlarm:
      return airFlarmBrushList[sIndex];
    case BaseMapElement::AirFir:
      return airFirBrushList[sIndex];
    case BaseMapElement::AirG:
      return airGBrushList[sIndex];
    case BaseMapElement::Ctr:
      return ctrBrushList[sIndex];
    case BaseMapElement::ControlC:
      return ctrCBrushList[sIndex];
    case BaseMapElement::ControlD:
      return ctrDBrushList[sIndex];
    case BaseMapElement::Danger:
      return dangerBrushList[sIndex];
    case BaseMapElement::Prohibited:
      return prohibitedBrushList[sIndex];
    case BaseMapElement::Sua:
      return suaBrushList[sIndex];
    case BaseMapElement::Restricted:
      return restrBrushList[sIndex];
    case BaseMapElement::Rmz:
      return rmzBrushList[sIndex];
    case BaseMapElement::Tmz:
      return tmzBrushList[sIndex];
    case BaseMapElement::Forest:
      return forestBrushList[sIndex];
    case BaseMapElement::GliderSector:
      return gliderSectorBrushList[sIndex];
    case BaseMapElement::River_T:
      return river_tBrushList[sIndex];
    case BaseMapElement::Glacier:
      return glacierBrushList[sIndex];
    case BaseMapElement::PackIce:
      return packiceBrushList[sIndex];
    }

  qWarning( "No Brush found for BaseMapElement=%d", typeID );
  return defaultBrush;
}

QPixmap MapConfig::getPixmap(unsigned int typeID, bool isWinch, bool smallIcon)
{
  QString iconName( getPixmapName( typeID, isWinch ) );

  // qDebug("getPixmapName,Winch,SmallIcon: %d %s",typeID, iconName.latin1() );

  if( smallIcon )
    {
      return GeneralConfig::instance()->loadPixmap( "small/" + iconName );
    }
  else
    {
      return GeneralConfig::instance()->loadPixmap( iconName );
    }
}

QPixmap MapConfig::getPixmap(unsigned int typeID, bool isWinch)
{
  QString iconName( getPixmapName( typeID, isWinch ) );

  return GeneralConfig::instance()->loadPixmapAutoScaled( iconName );
}

QPixmap MapConfig::getPixmap( QString iconName, const bool doScale )
{
  // qDebug("getPixmapName: %s", iconName.latin1() );

  if( isSwitch )
    {
      return GeneralConfig::instance()->loadPixmap( iconName, doScale );
    }
  else
    {
      return GeneralConfig::instance()->loadPixmap( "small/" + iconName );
    }
}

bool MapConfig::isRotatable( unsigned int typeID ) const
  {
    switch (typeID)
      {
      case BaseMapElement::Airfield:
      case BaseMapElement::Airport:
      case BaseMapElement::CivMilAirport:
      case BaseMapElement::IntAirport:
      case BaseMapElement::Gliderfield:
      case BaseMapElement::MilAirport:
      case BaseMapElement::UltraLight:
      case BaseMapElement::Outlanding:
        return true;
      default:
        return false;
      }
  }

QString MapConfig::getPixmapName(unsigned int typeID, bool isWinch )
{
  QString iconName;

  switch (typeID)
    {
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
    case BaseMapElement::Gliderfield:
      if (isWinch)
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
    case BaseMapElement::UserPoint:
      iconName = "userpoint";
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
    case BaseMapElement::Tacan:
      iconName = "tacan";
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
    case BaseMapElement::Road:
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
      qWarning() << "MapConfig::getPixmapName: No pixmap mapping found for typeId"
                 << typeID;
      iconName = "empty";
      break;
    }

  iconName += ".xpm";

  return iconName;
}

/**
 * Returns a pixmap containing a circle in the wanted size
 * and filled with green color. The circle has no border and
 * is transparent.
 */
QPixmap& MapConfig::getGreenCircle( int diameter )
{
  if( greenCircle.width() == diameter && greenCircle.height() == diameter )
    {
      // take the last one
      return greenCircle;
    }

  createCircle( greenCircle, diameter, QColor( Qt::green ) );

  return greenCircle;
}

/**
  * Returns a pixmap containing a circle in the wanted size
  * and filled with magenta color. The circle has no border and
  * is transparent.
  */
QPixmap& MapConfig::getMagentaCircle( int diameter )
{
  if( magentaCircle.width() == diameter && magentaCircle.height() == diameter )
    {
      // take the last one
       return magentaCircle;
    }

  createCircle( magentaCircle, diameter, QColor(Qt::magenta) );

  return magentaCircle;
}

/**
  * Returns a pixmap containing a circle in the wanted size
  * and filled with wanted color. The circle has no border and
  * is normally semi-transparent.
  */
void MapConfig::createCircle( QPixmap& pixmap, int diameter,
                              QColor color, double opacity,
                              QColor bg,
                              QPen pen )
{
  if( diameter % 2 )
    {
      // increase size, if unsymmetrically
      diameter++;
    }

  pixmap = QPixmap( diameter, diameter );
  pixmap.fill( bg );

  QPainter painter(&pixmap);
  painter.setPen( pen );
  painter.setBrush( QBrush( color, Qt::SolidPattern ) );
#ifndef MAEMO
  // @AP: that did not work under Maemo. No idea why?
  painter.setOpacity ( opacity ); // 50% opacity
#endif
  painter.drawEllipse( 0, 0, diameter, diameter );
}

/**
  * Returns a pixmap containing a square in the wanted size
  * and filled with wanted color. The square has no border and
  * is semi-transparent.
  */
void MapConfig::createSquare( QPixmap& pixmap, int size,
                              QColor color, double opacity,
                              QPen pen )
{
  if( size % 2 )
    {
      // increase size, if unsymmetrically
      size++;
    }

  pixmap = QPixmap( size, size );

  QPainter painter(&pixmap);
  painter.setPen( pen );
  painter.setBrush( QBrush( color, Qt::SolidPattern ) );
#ifndef MAEMO
  // @AP: that did not work under Maemo. No idea why?
  painter.setOpacity ( opacity ); // 50% opacity
#endif
  painter.drawRect( 0, 0, size, size );
}

/**
  * Draws on a pixmap a triangle in the wanted size,
  * filled with the wanted color and rotated in the wanted direction.
  * The triangle has no border and is semi-transparent. The top of the
  * triangle is north oriented if rotation is zero.
  *
  * @param pixmap Reference to pixmap which contains the drawn triangle
  * @param size Size of the pixmap to be drawn
  * @param color fill color of triangle
  * @param rotate rotation angle in degree of triangle
  * @param opacity a value between 0.0 ... 1.0
  * @param bg background color of pixmap. Default is set to transparent
  * @param pen to be used for outlining
  */
void MapConfig::createTriangle( QPixmap& pixmap, int size,
                                QColor color, int rotate,
                                double opacity,
                                QColor bg,
                                QPen pen )
{
  if( size % 2 )
    {
      // increase size, if unsymmetrically
      size++;
    }

  pixmap = QPixmap( size, size );

  pixmap.fill( bg );

  QPainter painter( &pixmap );

  painter.setPen( pen );
  painter.setBrush( QBrush( color, Qt::SolidPattern ) );
#ifndef MAEMO
  // @AP: that did not work under Maemo. No idea why?
  painter.setOpacity ( opacity ); // 50% opacity
#endif

  static const double rad = M_PI / 180.;

  // Note, that the Cartesian coordinate system must be mirrored at the
  // the X-axis to get the painter's coordinate system. That means all
  // angles must be multiplied by -1.
  double angle1 = -rad * MapCalc::normalize(90  - rotate);
  double angle2 = -rad * MapCalc::normalize(230 - rotate);
  double angle3 = -rad * MapCalc::normalize(310 - rotate);

  int radius = size/2;

  // Calculate the polygon points by using polar coordinates. So the triangle
  // is turned always around the same center point.
  int px1 = static_cast<int> (rint(cos(angle1) * radius));
  int py1 = static_cast<int> (rint(sin(angle1) * radius));

  int px2 = static_cast<int> (rint(cos(angle2) * radius));
  int py2 = static_cast<int> (rint(sin(angle2) * radius));

  int px3 = static_cast<int> (rint(cos(angle3) * radius));
  int py3 = static_cast<int> (rint(sin(angle3) * radius));

  QPoint points[3];

  points[0] = QPoint(px1, py1);
  points[1] = QPoint(px2, py2);
  points[2] = QPoint(px3, py3);

  // Move Cartesian middle point into painter's coordinate system.
  painter.translate( radius, radius );

  // Draw the triangle
  painter.drawPolygon( points, 3);
}

/**
  * Returns a pixmap containing a plus button. The button has rounded
  * corners and is transparent.
  */
QPixmap& MapConfig::getPlusButton()
{
  static bool first = true;

  if( first )
    {
      first = false;
      int size = Layout::getMapZoomButtonSize();

      plusButton = QPixmap( size + 8, size + 8 );
      plusButton.fill(Qt::transparent);
      QPainter painter(&plusButton);
      QPen pen(Qt::darkGray);
      pen.setWidth(4 + Layout::getIntScaledDensity() );
      painter.setPen( pen );
      painter.setBrush(Qt::NoBrush);
      painter.translate( 4, 4 );
      painter.drawRoundedRect( 0, 0, size, size, 10, 10 );
      painter.drawLine( 10, size/2, size-10, size/2 );
      painter.drawLine( size/2, 10, size/2, size-10 );
    }

  return plusButton;
}

/**
  * Returns a pixmap containing a minus button. The button has rounded
  * corners and is transparent.
  */
QPixmap& MapConfig::getMinusButton()
{
  static bool first = true;

  if( first )
    {
      first = false;
      int size = Layout::getMapZoomButtonSize();

      minusButton = QPixmap( size + 8, size + 8 );
      minusButton.fill(Qt::transparent);
      QPainter painter(&minusButton);
      QPen pen(Qt::darkGray);
      pen.setWidth(4 + Layout::getIntScaledDensity());
      painter.setPen( pen );
      painter.setBrush(Qt::NoBrush);
      painter.translate( 4, 4 );
      painter.drawRoundedRect( 0, 0, size, size, 10, 10 );
      painter.drawLine( 10, size/2, size-10, size/2 );
    }

  return minusButton;
}

QPixmap& MapConfig::getCross()
{
  static QPixmap pm;

  if( pm.isNull() )
    {
      int scale = Layout::getIntScaledDensity();
      int offset = 5 * scale;
      int w = 40 * scale;
      int h = 40 * scale;
      int cw = 4 * scale;

      int points[24] = { cw, 0, w/2, h/2-cw, w-cw, 0, w, cw, w/2+cw, h/2, w, h-cw,
			 w-cw, h, w/2, h/2+cw, cw, h, 0, h-cw, w/2-cw, h/2, 0, cw };
      QPolygon pn;

      pn.setPoints( 12, points );

      pm = QPixmap( w + offset, h + offset );
      pm.fill(Qt::transparent);

      QPainter painter;
      painter.begin(&pm);
      QPen pen(Qt::black);
      pen.setWidth(1 + Layout::getIntScaledDensity());
      painter.setPen( pen );
      painter.setBrush(Qt::white);
      painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
      painter.translate( offset / 2, offset / 2 );
      painter.drawPolygon(pn);
      painter.setBrush(Qt::black);
      painter.drawEllipse( QPoint(w/2, h/2), cw, cw );
      painter.end();
    }

  return pm;
}

QPixmap MapConfig::createGlider( const int heading, float scale )
{
  float s = scale * Layout::getIntScaledDensity();
  float offset = 5 * s;
  float w = 20 * s;
  float h = 20 * s;

  QPolygonF pn;

  pn << QPointF( w/2 + 0.5*s, h/2 - 4*s )
     << QPointF( w/2 + 0.5*s, h/2 - 1*s )
     << QPointF( w, h/2 - 1*s )
     << QPointF( w, h/2 )
     << QPointF( w/2 + 0.5*s, h/2 + 1*s )
     << QPointF( w/2 + 0.5*s, h - 3*s )
     << QPointF( w/2 + 3*s, h - 3*s )
     << QPointF( w/2 + 3*s, h - 2*s )
     << QPointF( w/2 - 3*s, h - 2*s )
     << QPointF( w/2 - 3*s, h - 3*s )
     << QPointF( w/2 - 0.5*s, h - 3*s )
     << QPointF( w/2 - 0.5*s, h/2 + 1*s )
     << QPointF( 0, h/2 )
     << QPointF( 0, h/2 - 1*s )
     << QPointF( w/2 - 0.5*s, h/2 - 1*s )
     << QPointF( w/2 - 0.5*s, h/2 - 4*s );

  QPixmap pm = QPixmap( w + offset, h + offset);
  pm.fill(Qt::transparent);

  QPainter painter;
  painter.begin(&pm);
  QPen pen(Qt::black);
  pen.setWidth(1 + Layout::getIntScaledDensity());
  painter.setPen( pen );
  painter.setBrush(Qt::white);
  painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );

  if( heading % 360 )
    {
      painter.translate( pm.width() / 2 , pm.height() / 2 );
      painter.rotate( heading % 360 );
      painter.translate( -pm.width() / 2 , -pm.height() / 2 );
    }

  painter.translate( offset / 2, offset / 2 );
  painter.drawPolygon( pn, Qt::WindingFill );
  painter.end();

  return pm;
}

QPixmap MapConfig::createAirfield( const int heading, float size, bool small )
{
  if( int(size) % 2 )
    {
      // increase size, if unsymmetrically
      size++;
    }

  float s = Layout::getIntScaledDensity();

  // scale the given size
  size = size * s;

  QPixmap pm = QPixmap( size, size );
  pm.fill(Qt::transparent);

  QRectF runway = QRectF( size/2 - 2*s, 2*s, 4*s, size - 4*s );

  float r = size * 0.6 / 2;

  int penWidth = size / 10;

  QPainter painter;
  painter.begin(&pm);
  painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  QPen pen(Qt::darkBlue);
  pen.setWidth( penWidth );
  painter.setPen( pen );

  if( small )
    {
      painter.setBrush(Qt::darkBlue);
    }
  else
    {
      painter.setBrush(Qt::NoBrush);
    }

  painter.drawEllipse( QPointF( size/2, size/2 ), r, r );

  painter.setBrush(Qt::white);
  pen.setWidth( 1 * Layout::getIntScaledDensity() );
  painter.setPen( pen );

  if( heading % 360 )
    {
      painter.translate( pm.width() / 2 , pm.height() / 2 );
      painter.rotate( heading % 360 );
      painter.translate( -pm.width() / 2 , -pm.height() / 2 );
    }

  painter.translate( 0, 0 );
  painter.drawRect( runway );
  painter.end();

  return pm;
}

QPixmap MapConfig::createLandingField( const int heading, float size, bool small )
{
  if( int(size) % 2 )
    {
      // increase size, if unsymmetrically
      size++;
    }

  float s = Layout::getIntScaledDensity();

  // scale the given size
  size = size * s;

  QPixmap pm = QPixmap( size, size );
  pm.fill(Qt::transparent);

  QRectF runway = QRectF( size/2 - 2*s, 2*s, 4*s, size - 4*s );

  float r = size * 0.6 / 2;

  int penWidth = size / 10;

  QPainter painter;
  painter.begin(&pm);
  painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  QPen pen(Qt::darkBlue);
  pen.setWidth( penWidth );
  painter.setPen( pen );

  if( small )
    {
      painter.setBrush(Qt::darkBlue);
    }
  else
    {
      painter.setBrush(Qt::NoBrush);
    }

  if( heading % 360 )
    {
      painter.translate( pm.width() / 2 , pm.height() / 2 );
      painter.rotate( heading % 360 );
      painter.translate( -pm.width() / 2 , -pm.height() / 2 );
    }

  painter.translate( 0, 0 );

  // Draw outer rectangle
  painter.drawRect( QRectF( size/2 - r, size/2 - r , 2*r, 2*r ) );

  // draw runway rectangle
  painter.setBrush(Qt::white);
  pen.setWidth( 1 * Layout::getIntScaledDensity() );
  painter.setPen( pen );
  painter.drawRect( runway );

  painter.end();

  return pm;
}
