/***********************************************************************
 **
 **   KRT2Constants.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2025 by Axel Pauli (kflog.cumulus@gmail.com)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
***********************************************************************/

#pragma once

#ifdef ANDROID
#define nullptr 0
#endif

#define STX 0x02 //!< Command start character.
#define ACK 0x06 //!< Command acknowledged character.
#define NAK 0x15 //!< Command not acknowledged character.
#define NO_RSP 0 //!< No response received yet.
#define RCQ 'S'  //!< Respond to connection query

  /** Unknown code, received once after power up, STX '8' */
#define UNKNOWN1 '8'
#define SET_VOLUME 'A'
#define LOW_BATTERY 'B'
#define EXCHANGE_FREQUENCIES 'C'
#define NO_LOW_BATTERY 'D'
#define PLL_ERROR 'E'
#define NO_PLL_ERROR 'F'
#define RX 'J'
#define TX 'K'
#define TE 'L'
#define RX_ON_ACTIVE_FREQUENCY 'M'
#define DUAL_ON 'O'
#define STANDBY_FREQUENCY 'R'
#define ACTIVE_FREQUENCY 'U'
#define NO_RX 'V'
#define PLL_ERROR2 'W'
#define NO_TX_RX 'Y'
#define SET_FREQUENCY 'Z'
#define DUAL_OFF 'o'
#define NO_RX_ON_ACTIVE_FREQUENCY 'm'
#define MAX_NAME_LENGTH 8 //!< Max. radio station name length.
