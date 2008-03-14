/***********************************************************************
**
**   sound.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2008 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef Sound_H
#define Sound_H

#include <QThread>
#include <QMutex>
#include <QString>

/**
 * A class for playing of sound files in a separate thread.
 */

/**
 * @author Axel Pauli
 */

class Sound : public QThread
{
  Q_OBJECT

 public:

  Sound( QString &sound, QObject *parent = 0 );
  ~Sound();

 protected:

  void run();

  private slots:

  // called to delete the thread
  void slot_destroy();

 private:

  static QMutex mutex;
  QString _sound;
};

#endif
