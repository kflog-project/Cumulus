/***********************************************************************
**
**   protocol.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004 by Axel Pauli (axel@kflog.org)
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
 * between the cumulus and the gps client process.
 */

#ifndef _Protocol_h_
#define _Protocol_h_

// Message Key word definitions

#define MSG_PROTOCOL         "Cumulus-GPS_Client_IPC_V1.0_Axel@kflog.org"

#define MSG_MAGIC            "\\Magic\\"

#define MSG_POS		     "\\Positive\\"

#define MSG_NEG		     "\\Negative\\"

// data available notitication

#define MSG_DA		     "\\Data_Available\\"

// open serial device "Open" <device> <speed>

#define MSG_OPEN	     "\\Open\\"

// close serial device

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

#define MSG_SHD	             "\\Shutdown\\"

//------- Other message strings-------//

#define MSG_CONLOST          "#GPS_Connection_lost#"

#endif  // #ifndef _Protocol_h_
