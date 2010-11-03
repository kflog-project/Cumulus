/***********************************************************************
**
**   protocol.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004-2010 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

/**
 * This file contains the messages used during interprocess communication
 * between the Cumulus and the GPS client process.
 */

#ifndef _Protocol_h_
#define _Protocol_h_

// Message key word definitions

#define MSG_PROTOCOL   "Cumulus-GPS_Client_IPC_V1.2_Axel@kflog.org"

#define MSG_MAGIC      "\\Magic\\"

#define MSG_POS		     "\\Positive\\"

#define MSG_NEG		     "\\Negative\\"

// data available notification

#define MSG_DA		     "\\Data_Available\\"

// open connection to the GPS device "Open" <device> <speed>

#define MSG_OPEN	     "\\Open\\"

// close connection to the GPS device

#define MSG_CLOSE	     "\\Close\\"

// get next available message from queue

#define MSG_GM		     "\\Get_Message\\"

// reply to get next available message from queue

#define MSG_RM		     "\\Reply_Message\\"

// send message to GPS device

#define MSG_SM		     "\\Send_Message\\"

// notify request

#define MSG_NTY		     "\\Notify\\"

// shutdown request

#define MSG_SHD	       "\\Shutdown\\"

// GPS message keys to be processed.

#define MSG_GPS_KEYS   "\\GPS_Msg_Keys\\"

//------- Other message strings-------//

#define MSG_CON_OFF     "#GPS_Connection_off#"

#define MSG_CON_ON      "#GPS_Connection_on#"

#endif  // #ifndef _Protocol_h_
