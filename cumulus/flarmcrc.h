/***********************************************************************
**
**   flarmcrc.h
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

#ifndef FLARM_CRC_H_
#define FLARM_CRC_H_

/**
 * \class FlarmCrc
 *
 * \author Flarm Technology GmbH, Axel Pauli
 *
 * \date 2012
 *
 * \brief Flarm CRC calculation formula.
 *
 * \version $Id$
 *
 */

class FlarmCrc
{
 public:

  FlarmCrc() : m_crc(0)
  {};

  ~FlarmCrc()
  {};

  /**
   * Add character to the CRC checksum.
   *
   * \param char Character to be added to the CRC checksum.
   */
  void update(const unsigned char b);

  /**
   * \return The calculated CRC.
   */
  unsigned short getCRC() const
  {
    return m_crc;
  };

 private:

  unsigned short m_crc;
};

#endif /* FLARM_CRC_H_ */
