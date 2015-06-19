/***********************************************************************
**
**   flarmbincomlinux.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2012-2015 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***********************************************************************/

#ifndef FLARM_BIN_COM_LINUX_H_
#define FLARM_BIN_COM_LINUX_H_

#include "flarmbincom.h"

/**
 * \class FlarmBinComLinux
 *
 * \author Axel Pauli
 *
 * \date 2012-2015
 *
 * \brief Flarm binary low level port routines for Linux.
 *
 * \version 1.1
 *
 */

class FlarmBinComLinux : public FlarmBinCom
{
 public:

  /**
   * Contructor of class.
   *
   * \param socket Opened socket to Flarm device.
   */
  FlarmBinComLinux(int socket);

  virtual ~FlarmBinComLinux();

 protected:

  /** Low level write character port method. */
  virtual int writeChar(const unsigned char c);

  /**
   * Low level read character port method.
   *
   * \param[out] b Character to be returned,
   *
   * \param[in] timeout Time to be wait for a character in milli seconds.
   *
   * \return 0 means timeout, -1 means error, 1 means ok
   */
  virtual int readChar(unsigned char* b, const int timeout);

 private:

  /** Socket to Flarm device. */
  int m_Socket;
};

#endif /* FLARM_BIN_COM_LINUX_H_ */
