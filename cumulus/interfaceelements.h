/***********************************************************************
**
**   interfaceelements.h
**
**   This file is part of Cumulus.
**   It contains the definitions for the list of interface elements
**   for the mapview
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef INTERFACE_ELEMENTS_H
#define INTERFACE_ELEMENTS_H

#define MVW_MAX_ELEMENT 95

//general elements
#define MVW_HOR_LAYOUT 1
#define MVW_VER_LAYOUT 2
#define MVW_LINE 3

//main widgets
#define MVW_MAP 4
#define MVW_THERMAL_CENTER 5  //Thermal centering help
#define MVW_THERMAL_STATS 6 //Graphical statistics on thermals

//7,8,9 reserved

//information on waypoint
#define MVW_WP_NAME 10
#define MVW_WP_NAME_ELEV 11
#define MVW_WP_ELEV 12
#define MVW_WP_DIST 13
#define MVW_WP_BEARING 14
#define MVW_WP_BEARING_GRAPH 15     //graphial representation of bearing to waypoint
#define MVW_WP_REL_BEARING 16  //relative bearing
#define MVW_WP_REL_BEARING_GRAPH 17 //graphial representation of relative bearing to waypoint
#define MVW_WP_DESCRIPTION 18
//19 reserved

//information on status
#define MVW_TRACK 20
#define MVW_HEAD 21
#define MVW_SPEED_GROUND 22
#define MVW_SPEED_AIR 23
//24,25 reserved

//altitude
#define MVW_POSITION 26
#define MVW_ALT_MSL 27
#define MVW_ALT_GND 28
#define MVW_ALT_AGL 29
//30-35 reserved

//ground elevation
#define MVW_GROUND_ELEV 36 //estimated elevation of ground under current position
#define MVW_GROUND_ELEV_MIN 37
#define MVW_GROUND_ELEV_MAX 38
//39 reserved

//vario
#define MVW_VARIO 40
#define MVW_VARIO_INTEGRATOR_SWITCHING 41 //integrator
//42 reserved
#define MVW_VARIO_AVG_THERMAL 43 //average over thermal
#define MVW_VARIO_AVG_FLIGHT 44 //integrator
#define MVW_VARIO_AVG_CIRCLE 45 //average over the last circle
//45-49 reserved

//settings
#define MVW_WATER 50
#define MVW_BUGS 51
#define MVW_MCCREADY 52
#define MVW_GLIDER_NAME 53
//54-59 reserved

//wind
#define MVW_WIND_SPEED 60
#define MVW_WIND_DIRECTION 61
#define MVW_WIND_HT_COMP 62 //head or tailwind component
#define MVW_WIND_DIRECTION_GRAPH 63 //graphical
#define MVW_WIND_HT_COMP_GRAPH 64 //head or tailwind component (graphical)

//time
#define MVW_TIME 65 //current time
#define MVW_TIME_GMT 66 //current time (GMT)
#define MVW_ETA_WP 67 //time that waypoint will be reached
#define MVW_ETA_TASK 68 //time that task will be finished
#define MVW_TIME_WP 69 //time to reach waypoint
#define MVW_TIME_TASK 70 //time to finish task
//71-74 reserved

//flightcomputer
#define MVW_S2F 75 //Speed to fly
#define MVW_MCCREADY_ADVISE 76 //Mc proposal according to effective climb rate
//77 reserved
#define MVW_ALT_FINAL_GLIDE 78 //Altitude needed to finish task
#define MVW_ALT_WP 79 //Altitude needed to reach waypoint
#define MVW_ARR_ALT_WP 80 //arrival altitude for current waypoint
#define MVW_ARR_ALT_TASK 81 //arrival altitude for end of task

//landingsites
#define MVW_LAND_NEAREST_NAME 85
#define MVW_LAND_NEAREST_DISTANCE 86
#define MVW_LAND_NEAREST_BEARING 87
#define MVW_LAND_NEAREST_ALTITUDE 88
#define MVW_LAND_COUNT 89

//task. Not every value may be available for each tasktype
#define MVW_TASK_DISTANCE_FLOWN 90
#define MVW_TASK_DISTANCE_LEFT 91
#define MVW_TASK_TIME_ELAPSED 92
#define MVW_TASK_TIME_LEFT_MIN 93
#define MVW_TASK_TIME_LEFT_MAX 94
#define MVW_TASK_AVG_SPEED 95

#endif
