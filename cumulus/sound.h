/***********************************************************************
**
**   sound.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2008-2012 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class Sound
 *
 * \author Axel Pauli
 *
 * \brief A class for playing of sound files in a separate thread.
 *
 * \date 2008-2012
 */

#ifndef Sound_H
#define Sound_H

#include <QThread>
#include <QMutex>
#include <QString>

class Sound : public QThread
{
  Q_OBJECT

 public:

  Sound( QString &sound, QObject *parent = 0 );
  ~Sound();

 protected:

  void run();

 private:

  static QMutex mutex;
  QString _sound;
};

#endif
