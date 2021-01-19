/***********************************************************************
**
**   protocol.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004-2021 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***********************************************************************/

/**
 * This file contains the message keys used during interprocess communication
 * between the Cumulus and the GPS client process.
 */

#ifndef _Protocol_h_
#define _Protocol_h_

// Message key word definitions

//------- Used by Command/Response channel -------//

#define MSG_PROTOCOL   "Cumulus-GPS_Client_IPC_V1.6_Axel@kflog.org"

#define MSG_MAGIC      "\\Magic\\"

#define MSG_POS	       "\\Positive\\"

#define MSG_NEG        "\\Negative\\"

// open connection to the GPS device "Open" <device> <speed>
#define MSG_OPEN       "\\Open\\"

// open one WiFi connection to the GPS device "Open_WiFi" <IP> <Port>
#define MSG_OPEN_WIFI_1 "\\Open_WiFi_1\\"

// open two WiFi connection to the GPS device "Open_WiFi_2" <IP1> <Port1> <IP2> <Port2>
#define MSG_OPEN_WIFI_2 "\\Open_WiFi_2\\"

// close connection to the GPS device
#define MSG_CLOSE      "\\Close\\"

// send message to GPS device
#define MSG_SM	       "\\Send_Msg\\"

// Switch on forwarding of GPS data
#define MSG_FGPS_ON    "\\Gps_Data_On\\"

// Switch off forwarding of GPS data
#define MSG_FGPS_OFF   "\\Gps_Data_Off\\"

// shutdown request
#define MSG_SHD	"\\Shutdown\\"

// GPS message keys to be processed.
#define MSG_GPS_KEYS   "\\Gps_Msg_Keys\\"

// Flarm Flight list is requested
#define MSG_FLARM_FLIGHT_LIST_REQ   "\\Flarm_Flight_List\\"

// Flarm Flight download is requested
#define MSG_FLARM_FLIGHT_DOWNLOAD   "\\Flarm_Flight_Download\\"

// Flarm Reset is requested
#define MSG_FLARM_RESET   "\\Flarm_Reset\\"

//------- Used by Forward data channel -------//

// GPS data message
#define MSG_GPS_DATA  "#Gps_Data#"

// Flarm flight list response
#define MSG_FLARM_FLIGHT_LIST_RES  "#FFLR#"

// Flarm flight download response, used to report results.
#define MSG_FLARM_FLIGHT_DOWNLOAD_INFO  "#FFDI#"

// Flarm flight download progress, used to report dowload progress.
#define MSG_FLARM_FLIGHT_DOWNLOAD_PROGRESS  "#FFDP#"

// Device report.
#define MSG_DEVICE_REPORT "#Device_Report#"

#define MSG_CON_OFF       "#GPS_Connection_off#"

#define MSG_CON_ON        "#GPS_Connection_on#"

#endif  // #ifndef _Protocol_h_
