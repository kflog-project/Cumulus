/***********************************************************************
 **
 **   mapconfig.cpp
 **
 **   This file is part of cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001 by Heiner Lamprecht, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "mapconfig.h"
#include "generalconfig.h"

#include "basemapelement.h"
#include "mapdefaults.h"
#include "reachablelist.h"

#define READ_TOPO(a,b)                          \
  topographyColorList.append(new QColor(b));

#define READ_BORDER(a)                          \
  a[0] = true;                                  \
  a[1] = true;                                  \
  a[2] = true;                                  \
  a[3] = true;

#define READ_PEN(G, A, B, C1, C2, C3, C4, P1, P2, P3, P4,       \
                 S1, S2, S3, S4)                                \
  READ_BORDER(B)                                                \
  A.append(new QPen(C1, P1, (Qt::PenStyle)S1));               \
  A.append(new QPen(C2, P2, (Qt::PenStyle)S2));                 \
  A.append(new QPen(C3, P3, (Qt::PenStyle)S3));                 \
  A.append(new QPen(C4, P4, (Qt::PenStyle)S4));


#define READ_PEN_BRUSH(G, a, B, A, C1, C2, C3, C4, P1, P2, P3, P4,      \
                       S1, S2, S3, S4, C7, C8, C9, C10, S7, S8, S9, S10) \
  READ_PEN(G, a, B, C1, C2, C3, C4, P1, P2, P3, P4,                     \
           S1, S2, S3, S4)                                              \
  A.append(new QBrush(C7,  (Qt::BrushStyle)S7));                      \
  A.append(new QBrush(C8,  (Qt::BrushStyle)S8));                        \
  A.append(new QBrush(C9,  (Qt::BrushStyle)S9));                        \
  A.append(new QBrush(C10, (Qt::BrushStyle)S10));


MapConfig::MapConfig(QObject* parent)
  : QObject(parent), scaleIndex(0), isSwitch(false)
{
  airABorder = new bool[4];
  airBBorder = new bool[4];
  airCBorder = new bool[4];
  airDBorder = new bool[4];
  airElBorder = new bool[4];
  airEhBorder = new bool[4];
  airFBorder = new bool[4];
  ctrCBorder = new bool[4];
  ctrDBorder = new bool[4];
  dangerBorder = new bool[4];
  lowFBorder = new bool[4];
  restrBorder = new bool[4];
  tmzBorder = new bool[4];
  suSectorBorder = new bool[4];

  trailBorder = new bool[4];
  roadBorder = new bool[4];
  highwayBorder = new bool[4];
  railBorder = new bool[4];
  rail_dBorder = new bool[4];
  aerialcableBorder = new bool[4];
  riverBorder = new bool[4];
  river_tBorder = new bool[4];
  canalBorder = new bool[4];
  cityBorder = new bool[4];

  forestBorder = new bool[4];
  glacierBorder = new bool[4];
  packiceBorder = new bool[4];

  // pre-create QIcons with background for copying later when needed
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
    airfieldIcon.insert(airfieldType[i],afIcon);
  }
  // qDebug("MapConfig initialized...");
}


MapConfig::~MapConfig()
{
  // @AP: make it easier for valgind, otherwise as memory leaks declarated.
  delete [] airABorder;
  delete [] airBBorder;
  delete [] airCBorder;
  delete [] airDBorder;
  delete [] airElBorder;
  delete [] airEhBorder;
  delete [] airFBorder;
  delete [] ctrCBorder;
  delete [] ctrDBorder;
  delete [] dangerBorder;
  delete [] lowFBorder;
  delete [] restrBorder;
  delete [] tmzBorder;
  delete [] suSectorBorder;
  delete [] trailBorder;
  delete [] roadBorder;
  delete [] highwayBorder;
  delete [] railBorder;
  delete [] rail_dBorder;
  delete [] aerialcableBorder;
  delete [] riverBorder;
  delete [] river_tBorder;
  delete [] canalBorder;
  delete [] cityBorder;
  delete [] forestBorder;
  delete [] glacierBorder;
  delete [] packiceBorder;

  // @AP: lists should be automatic deallocate its members during remove
  qDeleteAll(topographyColorList);
  topographyColorList.clear();
  qDeleteAll(airAPenList);
  airAPenList.clear();
  qDeleteAll(airABrushList);
  airABrushList.clear();
  qDeleteAll(airBPenList);
  airBPenList.clear();
  qDeleteAll(airBBrushList);
  airBBrushList.clear();
  qDeleteAll(airCPenList);
  airCPenList.clear();
  qDeleteAll(airCBrushList);
  airCBrushList.clear();
  qDeleteAll(airDPenList);
  airDPenList.clear();
  qDeleteAll(airDBrushList);
  airDBrushList.clear();
  qDeleteAll(airElPenList);
  airElPenList.clear();
  qDeleteAll(airElBrushList);
  airElBrushList.clear();
  qDeleteAll(airEhPenList);
  airEhPenList.clear();
  qDeleteAll(airEhBrushList);
  airEhBrushList.clear();
  qDeleteAll(airFPenList);
  airFPenList.clear();
  qDeleteAll(airFBrushList);
  airFBrushList.clear();
  qDeleteAll(ctrCPenList);
  ctrCPenList.clear();
  qDeleteAll(ctrCBrushList);
  ctrCBrushList.clear();
  qDeleteAll(ctrDPenList);
  ctrDPenList.clear();
  qDeleteAll(ctrDBrushList);
  ctrDBrushList.clear();
  qDeleteAll(lowFPenList);
  lowFPenList.clear();
  qDeleteAll(lowFBrushList);
  lowFBrushList.clear();
  qDeleteAll(dangerPenList);
  dangerPenList.clear();
  qDeleteAll(dangerBrushList);
  dangerBrushList.clear();
  qDeleteAll(restrPenList);
  restrPenList.clear();
  qDeleteAll(restrBrushList);
  restrBrushList.clear();
  qDeleteAll(tmzPenList);
  tmzPenList.clear();
  qDeleteAll(tmzBrushList);
  tmzBrushList.clear();
  qDeleteAll(suSectorPenList);
  suSectorPenList.clear();
  qDeleteAll(suSectorBrushList);
  suSectorBrushList.clear();
  qDeleteAll(highwayPenList);
  highwayPenList.clear();
  qDeleteAll(roadPenList);
  roadPenList.clear();
  qDeleteAll(trailPenList);
  trailPenList.clear();
  qDeleteAll(railPenList);
  railPenList.clear();
  qDeleteAll(rail_dPenList);
  rail_dPenList.clear();
  qDeleteAll(aerialcablePenList);
  aerialcablePenList.clear();
  qDeleteAll(riverPenList);
  riverPenList.clear();
  qDeleteAll(river_tPenList);
  river_tPenList.clear();
  qDeleteAll(river_tBrushList);
  river_tBrushList.clear();
  qDeleteAll(canalPenList);
  canalPenList.clear();
  qDeleteAll(cityPenList);
  cityPenList.clear();
  qDeleteAll(cityBrushList);
  cityBrushList.clear();
  qDeleteAll(forestPenList);
  forestPenList.clear();
  qDeleteAll(glacierPenList);
  glacierPenList.clear();
  qDeleteAll(packicePenList);
  packicePenList.clear();
  qDeleteAll(forestBrushList);
  forestBrushList.clear();
  qDeleteAll(glacierBrushList);
  glacierBrushList.clear();
  qDeleteAll(packiceBrushList);
  packiceBrushList.clear();
}


void MapConfig::slotReadConfig()
{
  qDeleteAll(topographyColorList);
  topographyColorList.clear();
  qDeleteAll(airAPenList);
  airAPenList.clear();
  qDeleteAll(airABrushList);
  airABrushList.clear();
  qDeleteAll(airBPenList);
  airBPenList.clear();
  qDeleteAll(airBBrushList);
  airBBrushList.clear();
  qDeleteAll(airCPenList);
  airCPenList.clear();
  qDeleteAll(airCBrushList);
  airCBrushList.clear();
  qDeleteAll(airDPenList);
  airDPenList.clear();
  qDeleteAll(airDBrushList);
  airDBrushList.clear();
  qDeleteAll(airElPenList);
  airElPenList.clear();
  qDeleteAll(airElBrushList);
  airElBrushList.clear();
  qDeleteAll(airEhPenList);
  airEhPenList.clear();
  qDeleteAll(airEhBrushList);
  airEhBrushList.clear();
  qDeleteAll(airFPenList);
  airFPenList.clear();
  qDeleteAll(airFBrushList);
  airFBrushList.clear();
  qDeleteAll(ctrCPenList);
  ctrCPenList.clear();
  qDeleteAll(ctrCBrushList);
  ctrCBrushList.clear();
  qDeleteAll(ctrDPenList);
  ctrDPenList.clear();
  qDeleteAll(ctrDBrushList);
  ctrDBrushList.clear();
  qDeleteAll(lowFPenList);
  lowFPenList.clear();
  qDeleteAll(lowFBrushList);
  lowFBrushList.clear();
  qDeleteAll(dangerPenList);
  dangerPenList.clear();
  qDeleteAll(dangerBrushList);
  dangerBrushList.clear();
  qDeleteAll(restrPenList);
  restrPenList.clear();
  qDeleteAll(restrBrushList);
  restrBrushList.clear();
  qDeleteAll(tmzPenList);
  tmzPenList.clear();
  qDeleteAll(tmzBrushList);
  tmzBrushList.clear();
  qDeleteAll(suSectorPenList);
  suSectorPenList.clear();
  qDeleteAll(suSectorBrushList);
  suSectorBrushList.clear();
  qDeleteAll(highwayPenList);
  highwayPenList.clear();
  qDeleteAll(roadPenList);
  roadPenList.clear();
  qDeleteAll(trailPenList);
  trailPenList.clear();
  qDeleteAll(railPenList);
  railPenList.clear();
  qDeleteAll(rail_dPenList);
  rail_dPenList.clear();
  qDeleteAll(aerialcablePenList);
  aerialcablePenList.clear();
  qDeleteAll(riverPenList);
  riverPenList.clear();
  qDeleteAll(river_tPenList);
  river_tPenList.clear();
  qDeleteAll(river_tBrushList);
  river_tBrushList.clear();
  qDeleteAll(canalPenList);
  canalPenList.clear();
  qDeleteAll(cityPenList);
  cityPenList.clear();
  qDeleteAll(cityBrushList);
  cityBrushList.clear();
  qDeleteAll(forestPenList);
  forestPenList.clear();
  qDeleteAll(glacierPenList);
  glacierPenList.clear();
  qDeleteAll(packicePenList);
  packicePenList.clear();
  qDeleteAll(forestBrushList);
  forestBrushList.clear();
  qDeleteAll(glacierBrushList);
  glacierBrushList.clear();
  qDeleteAll(packiceBrushList);
  packiceBrushList.clear();

  READ_TOPO("SubTerrain", LEVEL_SUB)
    READ_TOPO("0M", LEVEL_0)
    READ_TOPO("10M", LEVEL_10)
    READ_TOPO("25M", LEVEL_25)
    READ_TOPO("50M", LEVEL_50)
    READ_TOPO("75M", LEVEL_75)
    READ_TOPO("100M", LEVEL_100)
    READ_TOPO("150M", LEVEL_150)
    READ_TOPO("200M", LEVEL_200)
    READ_TOPO("250M", LEVEL_250)
    READ_TOPO("300M", LEVEL_300)
    READ_TOPO("350M", LEVEL_350)
    READ_TOPO("400M", LEVEL_400)
    READ_TOPO("450M", LEVEL_450)
    READ_TOPO("500M", LEVEL_500)
    READ_TOPO("600M", LEVEL_600)
    READ_TOPO("700M", LEVEL_700)
    READ_TOPO("800M", LEVEL_800)
    READ_TOPO("900M", LEVEL_900)
    READ_TOPO("1000M", LEVEL_1000)
    READ_TOPO("1250M", LEVEL_1250)
    READ_TOPO("1500M", LEVEL_1500)
    READ_TOPO("1750M", LEVEL_1750)
    READ_TOPO("2000M", LEVEL_2000)
    READ_TOPO("2250M", LEVEL_2250)
    READ_TOPO("2500M", LEVEL_2500)
    READ_TOPO("2750M", LEVEL_2750)
    READ_TOPO("3000M", LEVEL_3000)
    READ_TOPO("3250M", LEVEL_3250)
    READ_TOPO("3500M", LEVEL_3500)
    READ_TOPO("3750M", LEVEL_3750)
    READ_TOPO("4000M", LEVEL_4000)
    READ_TOPO("4250M", LEVEL_4250)
    READ_TOPO("4500M", LEVEL_4500)
    READ_TOPO("4750M", LEVEL_4750)
    READ_TOPO("5000M", LEVEL_5000)
    READ_TOPO("5250M", LEVEL_5250)
    READ_TOPO("5500M", LEVEL_5500)
    READ_TOPO("5750M", LEVEL_5750)
    READ_TOPO("6000M", LEVEL_6000)
    READ_TOPO("6250M", LEVEL_6250)
    READ_TOPO("6500M", LEVEL_6500)
    READ_TOPO("6750M", LEVEL_6750)
    READ_TOPO("7000M", LEVEL_7000)
    READ_TOPO("7250M", LEVEL_7250)
    READ_TOPO("7500M", LEVEL_7500)
    READ_TOPO("7750M", LEVEL_7750)
    READ_TOPO("8000M", LEVEL_8000)
    READ_TOPO("8250M", LEVEL_8250)
    READ_TOPO("8500M", LEVEL_8500)
    READ_TOPO("8750M", LEVEL_8750)

    READ_PEN("Road", roadPenList, roadBorder,
             ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
             ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4,
             ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4)

    READ_PEN("Trail", trailPenList, trailBorder,
             TRAIL_COLOR_1, TRAIL_COLOR_2, TRAIL_COLOR_3, TRAIL_COLOR_4,
             TRAIL_PEN_1, TRAIL_PEN_2, TRAIL_PEN_3, TRAIL_PEN_4,
             TRAIL_PEN_STYLE_1, TRAIL_PEN_STYLE_2, TRAIL_PEN_STYLE_3, TRAIL_PEN_STYLE_4)

    READ_PEN("River", riverPenList, riverBorder,
             RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3, RIVER_COLOR_4,
             RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
             RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4)

    READ_PEN("Canal", canalPenList, canalBorder,
             CANAL_COLOR_1, CANAL_COLOR_2, CANAL_COLOR_3, CANAL_COLOR_4,
             CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4,
             CANAL_PEN_STYLE_1, CANAL_PEN_STYLE_2, CANAL_PEN_STYLE_3, CANAL_PEN_STYLE_4)

    READ_PEN("Rail", railPenList, railBorder,
             RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
             RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4,
             RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4)

    READ_PEN("Rail_D", rail_dPenList, rail_dBorder,
             RAIL_D_COLOR_1, RAIL_D_COLOR_2, RAIL_D_COLOR_3, RAIL_D_COLOR_4,
             RAIL_D_PEN_1, RAIL_D_PEN_2, RAIL_D_PEN_3, RAIL_D_PEN_4,
             RAIL_D_PEN_STYLE_1, RAIL_D_PEN_STYLE_2, RAIL_D_PEN_STYLE_3, RAIL_D_PEN_STYLE_4)

    READ_PEN("Aerial_Cable", aerialcablePenList, aerialcableBorder,
             AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2, AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4,
             AERIAL_CABLE_PEN_1, AERIAL_CABLE_PEN_2, AERIAL_CABLE_PEN_3, AERIAL_CABLE_PEN_4,
             AERIAL_CABLE_PEN_STYLE_1, AERIAL_CABLE_PEN_STYLE_2, AERIAL_CABLE_PEN_STYLE_3, AERIAL_CABLE_PEN_STYLE_4)


    READ_PEN("Highway", highwayPenList, highwayBorder,
             HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
             HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4,
             HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4)


    // PenStyle and BrushStyle are not used for cities ...
    READ_PEN_BRUSH("City", cityPenList, cityBorder, cityBrushList,
                   CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3,CITY_COLOR_4,
                   CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
                   Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine,
                   CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
                   CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4,
                   Qt::SolidPattern, Qt::SolidPattern,
                   Qt::SolidPattern, Qt::SolidPattern)

    READ_PEN_BRUSH("Forest", forestPenList, forestBorder, forestBrushList,
                   FRST_COLOR_1, FRST_COLOR_2, FRST_COLOR_3, FRST_COLOR_4,
                   FRST_PEN_1, FRST_PEN_2, FRST_PEN_3, FRST_PEN_4,
                   FRST_PEN_STYLE_1, FRST_PEN_STYLE_2, FRST_PEN_STYLE_3, FRST_PEN_STYLE_4,
                   FRST_BRUSH_COLOR_1, FRST_BRUSH_COLOR_2,
                   FRST_BRUSH_COLOR_3, FRST_BRUSH_COLOR_4,
                   FRST_BRUSH_STYLE_1, FRST_BRUSH_STYLE_2,
                   FRST_BRUSH_STYLE_3, FRST_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Glacier", glacierPenList, glacierBorder, glacierBrushList,
                   GLACIER_COLOR_1, GLACIER_COLOR_2, GLACIER_COLOR_3, GLACIER_COLOR_4,
                   GLACIER_PEN_1, GLACIER_PEN_2, GLACIER_PEN_3, GLACIER_PEN_4,
                   GLACIER_PEN_STYLE_1, GLACIER_PEN_STYLE_2, GLACIER_PEN_STYLE_3, GLACIER_PEN_STYLE_4,
                   GLACIER_BRUSH_COLOR_1, GLACIER_BRUSH_COLOR_2,
                   GLACIER_BRUSH_COLOR_3, GLACIER_BRUSH_COLOR_4,
                   GLACIER_BRUSH_STYLE_1, GLACIER_BRUSH_STYLE_2,
                   GLACIER_BRUSH_STYLE_3, GLACIER_BRUSH_STYLE_4)

    READ_PEN_BRUSH("PackIce", packicePenList, packiceBorder, packiceBrushList,
                   PACK_ICE_COLOR_1, PACK_ICE_COLOR_2, PACK_ICE_COLOR_3, PACK_ICE_COLOR_4,
                   PACK_ICE_PEN_1, PACK_ICE_PEN_2, PACK_ICE_PEN_3, PACK_ICE_PEN_4,
                   PACK_ICE_PEN_STYLE_1, PACK_ICE_PEN_STYLE_2, PACK_ICE_PEN_STYLE_3, PACK_ICE_PEN_STYLE_4,
                   PACK_ICE_BRUSH_COLOR_1, PACK_ICE_BRUSH_COLOR_2,
                   PACK_ICE_BRUSH_COLOR_3, PACK_ICE_BRUSH_COLOR_4,
                   PACK_ICE_BRUSH_STYLE_1, PACK_ICE_BRUSH_STYLE_2,
                   PACK_ICE_BRUSH_STYLE_3, PACK_ICE_BRUSH_STYLE_4)

    READ_PEN_BRUSH("River_T", river_tPenList, river_tBorder, river_tBrushList,
                   RIVER_T_COLOR_1, RIVER_T_COLOR_2, RIVER_T_COLOR_3, RIVER_T_COLOR_4,
                   RIVER_T_PEN_1, RIVER_T_PEN_2, RIVER_T_PEN_3, RIVER_T_PEN_4,
                   RIVER_T_PEN_STYLE_1, RIVER_T_PEN_STYLE_2, RIVER_T_PEN_STYLE_3, RIVER_T_PEN_STYLE_4,
                   RIVER_T_BRUSH_COLOR_1, RIVER_T_BRUSH_COLOR_2,
                   RIVER_T_BRUSH_COLOR_3, RIVER_T_BRUSH_COLOR_4,
                   RIVER_T_BRUSH_STYLE_1, RIVER_T_BRUSH_STYLE_2,
                   RIVER_T_BRUSH_STYLE_3, RIVER_T_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Airspace A", airAPenList, airABorder, airABrushList,
                   AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
                   AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
                   AIRA_PEN_STYLE_1, AIRA_PEN_STYLE_2, AIRA_PEN_STYLE_3, AIRA_PEN_STYLE_4,
                   AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
                   AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4,
                   AIRA_BRUSH_STYLE_1, AIRA_BRUSH_STYLE_2,
                   AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Airspace B", airBPenList, airBBorder, airBBrushList,
                   AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
                   AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
                   AIRB_PEN_STYLE_1, AIRB_PEN_STYLE_2, AIRB_PEN_STYLE_3, AIRB_PEN_STYLE_4,
                   AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
                   AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4,
                   AIRB_BRUSH_STYLE_1, AIRB_BRUSH_STYLE_2,
                   AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Airspace C", airCPenList, airCBorder, airCBrushList,
                   AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
                   AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
                   AIRC_PEN_STYLE_1, AIRC_PEN_STYLE_2, AIRC_PEN_STYLE_3, AIRC_PEN_STYLE_4,
                   AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
                   AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4,
                   AIRC_BRUSH_STYLE_1, AIRC_BRUSH_STYLE_2,
                   AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Airspace D", airDPenList, airDBorder, airDBrushList,
                   AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
                   AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
                   AIRD_PEN_STYLE_1, AIRD_PEN_STYLE_2, AIRD_PEN_STYLE_3, AIRD_PEN_STYLE_4,
                   AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
                   AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4,
                   AIRD_BRUSH_STYLE_1, AIRD_BRUSH_STYLE_2,
                   AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Airspace E low", airElPenList, airElBorder, airElBrushList,
                   AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3, AIREL_COLOR_4,
                   AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
                   AIREL_PEN_STYLE_1, AIREL_PEN_STYLE_2, AIREL_PEN_STYLE_3, AIREL_PEN_STYLE_4,
                   AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
                   AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4,
                   AIREL_BRUSH_STYLE_1, AIREL_BRUSH_STYLE_2,
                   AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Airspace E high", airEhPenList, airEhBorder, airEhBrushList,
                   AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3, AIREH_COLOR_4,
                   AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
                   AIREH_PEN_STYLE_1, AIREH_PEN_STYLE_2, AIREH_PEN_STYLE_3, AIREH_PEN_STYLE_4,
                   AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
                   AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4,
                   AIREH_BRUSH_STYLE_1, AIREH_BRUSH_STYLE_2,
                   AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Airspace F", airFPenList, airFBorder, airFBrushList,
                   AIRF_COLOR_1, AIRF_COLOR_2, AIRF_COLOR_3, AIRF_COLOR_4,
                   AIRF_PEN_1, AIRF_PEN_2, AIRF_PEN_3, AIRF_PEN_4,
                   AIRF_PEN_STYLE_1, AIRF_PEN_STYLE_2, AIRF_PEN_STYLE_3, AIRF_PEN_STYLE_4,
                   AIRF_BRUSH_COLOR_1, AIRF_BRUSH_COLOR_2,
                   AIRF_BRUSH_COLOR_3, AIRF_BRUSH_COLOR_4,
                   AIRF_BRUSH_STYLE_1, AIRF_BRUSH_STYLE_2,
                   AIRF_BRUSH_STYLE_3, AIRF_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Control C", ctrCPenList, ctrCBorder,ctrCBrushList,
                   CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
                   CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
                   CTRC_PEN_STYLE_1, CTRC_PEN_STYLE_2, CTRC_PEN_STYLE_3, CTRC_PEN_STYLE_4,
                   CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
                   CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4,
                   CTRC_BRUSH_STYLE_1, CTRC_BRUSH_STYLE_2,
                   CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Control D", ctrDPenList, ctrDBorder, ctrDBrushList,
                   CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
                   CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
                   CTRD_PEN_STYLE_1, CTRD_PEN_STYLE_2, CTRD_PEN_STYLE_3, CTRD_PEN_STYLE_4,
                   CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
                   CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4,
                   CTRD_BRUSH_STYLE_1, CTRD_BRUSH_STYLE_2,
                   CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Danger", dangerPenList, dangerBorder, dangerBrushList,
                   DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
                   DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
                   DNG_PEN_STYLE_1, DNG_PEN_STYLE_2, DNG_PEN_STYLE_3, DNG_PEN_STYLE_4,
                   DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
                   DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4,
                   DNG_BRUSH_STYLE_1, DNG_BRUSH_STYLE_2,
                   DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Low Flight", lowFPenList, lowFBorder,lowFBrushList,
                   LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
                   LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
                   LOWF_PEN_STYLE_1, LOWF_PEN_STYLE_2, LOWF_PEN_STYLE_3, LOWF_PEN_STYLE_4,
                   LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
                   LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4,
                   LOWF_BRUSH_STYLE_1, LOWF_BRUSH_STYLE_2,
                   LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Restricted Area", restrPenList, restrBorder, restrBrushList,
                   RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
                   RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
                   RES_PEN_STYLE_1, RES_PEN_STYLE_2, RES_PEN_STYLE_3, RES_PEN_STYLE_4,
                   RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
                   RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4,
                   RES_BRUSH_STYLE_1, RES_BRUSH_STYLE_2,
                   RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4)

    READ_PEN_BRUSH("TMZ", tmzPenList, tmzBorder, tmzBrushList,
                   TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
                   TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
                   TMZ_PEN_STYLE_1, TMZ_PEN_STYLE_2, TMZ_PEN_STYLE_3, TMZ_PEN_STYLE_4,
                   TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
                   TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4,
                   TMZ_BRUSH_STYLE_1, TMZ_BRUSH_STYLE_2,
                   TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4)

    READ_PEN_BRUSH("Special Use Sector", suSectorPenList, suSectorBorder, suSectorBrushList,
                   SU_SECTOR_COLOR_1, SU_SECTOR_COLOR_2, SU_SECTOR_COLOR_3, SU_SECTOR_COLOR_4,
                   SU_SECTOR_PEN_1, SU_SECTOR_PEN_2, SU_SECTOR_PEN_3, SU_SECTOR_PEN_4,
                   SU_SECTOR_PEN_STYLE_1, SU_SECTOR_PEN_STYLE_2, SU_SECTOR_PEN_STYLE_3, SU_SECTOR_PEN_STYLE_4,
                   SU_SECTOR_BRUSH_COLOR_1, SU_SECTOR_BRUSH_COLOR_2,
                   SU_SECTOR_BRUSH_COLOR_3, SU_SECTOR_BRUSH_COLOR_4,
                   SU_SECTOR_BRUSH_STYLE_1, SU_SECTOR_BRUSH_STYLE_2,
                   SU_SECTOR_BRUSH_STYLE_3, SU_SECTOR_BRUSH_STYLE_4)


    GeneralConfig *conf = GeneralConfig::instance();

  drawBearing            = conf->getMapBearLine();
  drawIsoLines           = conf->getMapLoadIsoLines();
  bShowWpLabels          = conf->getMapShowWaypointLabels();
  bShowWpLabelsExtraInfo = conf->getMapShowWaypointLabelsExtraInfo();
  bLoadIsolines          = conf->getMapLoadIsoLines();
  bShowIsolineBorders    = conf->getMapShowIsoLineBorders();
  bLoadRoads             = conf->getMapLoadRoads();
  bLoadHighways          = conf->getMapLoadHighways();
  bLoadRailroads         = conf->getMapLoadRailroads();
  bLoadCities            = conf->getMapLoadCities();
  bLoadWaterways         = conf->getMapLoadWaterways();
  bLoadForests           = conf->getMapLoadForests();
  bDeleteAfterMapCompile = conf->getMapDeleteAfterCompile();
  bUnloadUnneededMap     = conf->getMapUnload();

  emit configChanged();
}


void MapConfig::slotSetMatrixValues(int index, bool sw)
{
  isSwitch = sw;
  scaleIndex = index;
}


QPen MapConfig::getDrawPen(unsigned int typeID)
{
  return __getPen(typeID, scaleIndex);
}


QPen MapConfig::__getPen(unsigned int typeID, int sIndex)
{
  switch(typeID) {
  case BaseMapElement::Trail:
    return *trailPenList.at(sIndex);
  case BaseMapElement::Road:
    return *roadPenList.at(sIndex);
  case BaseMapElement::Highway:
    return *highwayPenList.at(sIndex);
  case BaseMapElement::Railway:
    return *railPenList.at(sIndex);
  case BaseMapElement::Railway_D:
    return *rail_dPenList.at(sIndex);
  case BaseMapElement::Aerial_Cable:
    return *aerialcablePenList.at(sIndex);
  case BaseMapElement::River:
  case BaseMapElement::Lake:
    return *riverPenList.at(sIndex);
  case BaseMapElement::River_T:
  case BaseMapElement::Lake_T:
    return *river_tPenList.at(sIndex);
  case BaseMapElement::Canal:
    return *canalPenList.at(sIndex);
  case BaseMapElement::City:
    return *cityPenList.at(sIndex);
  case BaseMapElement::AirA:
    return *airAPenList.at(sIndex);
  case BaseMapElement::AirB:
    return *airBPenList.at(sIndex);
  case BaseMapElement::AirC:
    return *airCPenList.at(sIndex);
  case BaseMapElement::AirD:
    return *airDPenList.at(sIndex);
  case BaseMapElement::AirElow:
    return *airElPenList.at(sIndex);
  case BaseMapElement::AirEhigh:
    return *airEhPenList.at(sIndex);
  case BaseMapElement::AirF:
    return *airFPenList.at(sIndex);
  case BaseMapElement::ControlC:
    return *ctrCPenList.at(sIndex);
  case BaseMapElement::ControlD:
    return *ctrDPenList.at(sIndex);
  case BaseMapElement::Danger:
    return *dangerPenList.at(sIndex);
  case BaseMapElement::LowFlight:
    return *lowFPenList.at(sIndex);
  case BaseMapElement::Restricted:
    return *restrPenList.at(sIndex);
  case BaseMapElement::Tmz:
    return *tmzPenList.at(sIndex);
  case BaseMapElement::Forest:
    return *forestPenList.at(sIndex);
  case BaseMapElement::Glacier:
    return *glacierPenList.at(sIndex);
  case BaseMapElement::PackIce:
    return *packicePenList.at(sIndex);
  default:
    return *roadPenList.at(sIndex);
  }
}


bool MapConfig::isBorder(unsigned int typeID)
{
  //  return true;
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
  case BaseMapElement::AirElow:
    return airElBorder[scaleIndex];
  case BaseMapElement::AirEhigh:
    return airEhBorder[scaleIndex];
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
  case BaseMapElement::Glacier:
    return glacierBorder[scaleIndex];
  case BaseMapElement::PackIce:
    return packiceBorder[scaleIndex];

  }

  /* Should never happen ... */
  return true;
}


QColor MapConfig::getIsoColor(unsigned int heightIndex)
{
  return *topographyColorList.at(heightIndex);
}


QBrush MapConfig::getDrawBrush(unsigned int typeID)
{
  return __getBrush(typeID, scaleIndex);
}


QBrush MapConfig::__getBrush(unsigned int typeID, int sIndex)
{
  switch(typeID) {
  case BaseMapElement::City:
    return *cityBrushList.at(sIndex);
  case BaseMapElement::Lake:
    return QBrush(riverPenList.at(sIndex)->color(), Qt::SolidPattern);
  case BaseMapElement::AirA:
    return *airABrushList.at(sIndex);
  case BaseMapElement::AirB:
    return *airBBrushList.at(sIndex);
  case BaseMapElement::AirC:
    return *airCBrushList.at(sIndex);
  case BaseMapElement::AirD:
    return *airDBrushList.at(sIndex);
  case BaseMapElement::AirElow:
    return *airElBrushList.at(sIndex);
  case BaseMapElement::AirEhigh:
    return *airEhBrushList.at(sIndex);
  case BaseMapElement::AirF:
    return *airFBrushList.at(sIndex);
  case BaseMapElement::ControlC:
    return *ctrCBrushList.at(sIndex);
  case BaseMapElement::ControlD:
    return *ctrDBrushList.at(sIndex);
  case BaseMapElement::Danger:
    return *dangerBrushList.at(sIndex);
  case BaseMapElement::LowFlight:
    return *lowFBrushList.at(sIndex);
  case BaseMapElement::Restricted:
    return *restrBrushList.at(sIndex);
  case BaseMapElement::Tmz:
    return *tmzBrushList.at(sIndex);
  case BaseMapElement::Forest:
    return *forestBrushList.at(sIndex);
  }
  return QBrush();
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

QIcon MapConfig::getListIcon(unsigned int typeID)
{
  return QIcon( airfieldIcon[typeID] );
}


bool MapConfig::isRotatable( unsigned int typeID )
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
    iconName += "-18.png";  // airfield icons can be rotated 10 deg wise
  else
    iconName += ".xpm";
  return iconName;
}


/** Returns true if small icons are used, else returns false. */
bool MapConfig::useSmallIcons() const
{
  return !isSwitch;
}


/** Read property of bool drawBearing. */
bool MapConfig::getdrawBearing() const
{
  return drawBearing;
}


/** Read property of bool drawBearing. */
bool MapConfig::getdrawIsoLines() const
{
  return drawIsoLines;
}


bool MapConfig::getShowWpLabels() const
{
  return bShowWpLabels;
}

void MapConfig::setShowWpLabels(bool show)
{
  bShowWpLabels=show;
  GeneralConfig::instance()->setMapShowWaypointLabels( show );
}


bool MapConfig::getShowWpLabelsExtraInfo() const
{
  return bShowWpLabelsExtraInfo;
}

void MapConfig::setShowWpLabelsExtraInfo(bool show)
{
  bShowWpLabelsExtraInfo = show;
  GeneralConfig::instance()->setMapShowWaypointLabelsExtraInfo( show );
}


/** Returns whether or not the object should be loaded. */
bool MapConfig::getLoadIsolines() const
{
  return bLoadIsolines;
}


bool MapConfig::getShowIsolineBorders() const
{
  return bShowIsolineBorders;
}


bool MapConfig::getLoadRoads() const
{
  return bLoadRoads;
}


bool MapConfig::getLoadHighways() const
{
  return bLoadHighways;
}


bool MapConfig::getLoadRailroads() const
{
  return bLoadRailroads;
}


bool MapConfig::getLoadCities() const
{
  return bLoadCities;
}


bool MapConfig::getLoadWaterways() const
{
  return bLoadWaterways;
}


bool MapConfig::getLoadForests() const
{
  return bLoadForests;
}
