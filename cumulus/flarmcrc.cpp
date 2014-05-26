/***********************************************************************
**
**   flarmcrc.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
**   Thanks to Flarm Technology GmbH, who supported us.
**
***********************************************************************/

#include "flarmcrc.h"

void FlarmCrc::update( const unsigned char b )
{
  m_crc = m_crc ^ (((unsigned short) b) << 8);

  for (int i = 0; i < 8; i++)
    {
      if (m_crc & 0x8000)
        m_crc = (m_crc << 1) ^ 0x1021;
      else
        m_crc = (m_crc << 1) & 0xffff;
    }
}
