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

#include <cstdint>

#pragma once

  static constexpr uint8_t STX{0x02}; //!< Command start character.
  static constexpr uint8_t ACK{0x06}; //!< Command acknowledged character.
  static constexpr uint8_t NAK{0x15}; //!< Command not acknowledged character.
  static constexpr uint8_t NO_RSP{0}; //!< No response received yet.
  static constexpr uint8_t RCQ{'S'};  //!< Respond to connection query

  /** Unknown code, received once after power up, STX '8' */
  static constexpr uint8_t UNKNOWN1{'8'};
  static constexpr uint8_t SET_VOLUME{'A'};
  static constexpr uint8_t LOW_BATTERY{'B'};
  static constexpr uint8_t EXCHANGE_FREQUENCIES{'C'};
  static constexpr uint8_t NO_LOW_BATTERY{'D'};
  static constexpr uint8_t PLL_ERROR{'E'};
  static constexpr uint8_t NO_PLL_ERROR{'F'};
  static constexpr uint8_t RX{'J'};
  static constexpr uint8_t TX{'K'};
  static constexpr uint8_t TE{'L'};
  static constexpr uint8_t RX_ON_ACTIVE_FREQUENCY{'M'};
  static constexpr uint8_t DUAL_ON{'O'};
  static constexpr uint8_t STANDBY_FREQUENCY{'R'};
  static constexpr uint8_t ACTIVE_FREQUENCY{'U'};
  static constexpr uint8_t NO_RX{'V'};
  static constexpr uint8_t PLL_ERROR2{'W'};
  static constexpr uint8_t NO_TX_RX{'Y'};
  static constexpr uint8_t SET_FREQUENCY{'Z'};
  static constexpr uint8_t DUAL_OFF{'o'};
  static constexpr uint8_t NO_RX_ON_ACTIVE_FREQUENCY{'m'};
  static constexpr uint8_t MAX_NAME_LENGTH = 8; //!< Max. radio station name length.
