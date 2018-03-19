/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef crc_h_
#define crc_h_

#include <QtGlobal>
#include "compiler.h"

extern const quint16 crc16ccittable[256];

gcc_const
static inline quint16
UpdateCRC16CCITT(quint8 octet, quint16 crc)
{
  return (crc << 8) ^ crc16ccittable[(crc >> 8) ^ octet];
}

gcc_pure
static inline quint16
UpdateCRC16CCITT(const quint8 *data, const quint8 *end, quint16 crc)
{
  while (data < end)
    crc = UpdateCRC16CCITT(*data++, crc);
  return crc;
}

gcc_pure
static inline quint16
UpdateCRC16CCITT(const void *data, quint32 length, quint16 crc)
{
  const quint8 *p = static_cast<const quint8 *>(data), *end = p + length;
  return UpdateCRC16CCITT(p, end, crc);
}

#endif
