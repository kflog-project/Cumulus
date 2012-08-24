/***********************************************************************
**
**   sound.cpp
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

#include <QtGui>

#include "sound.h"
#include "generalconfig.h"

QMutex Sound::mutex;

Sound::Sound( QString &sound, QObject *parent ) : QThread( parent )
{
  _sound = sound;
  setTerminationEnabled(true);

  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

Sound::~Sound()
{
  // qDebug( "Sound::~Sound() is called" );
}


/** Plays the sound file by calling the configured player, which has
 *  been passed in the constructor. If sound file or player are not
 *  accessable a simple beep is given out as fallback solution.
 */
void Sound::run()
{
  QMutexLocker locker( &mutex );

  QFileInfo info = QFileInfo( _sound );

  if( ! info.isReadable() )
    {
      QApplication::beep();
      qWarning("Sound file %s is not readable", _sound.toLatin1().data() );
      return;
    }

  QString player = GeneralConfig::instance()->getSoundPlayer();

  if( player.isEmpty() )
    {
      QApplication::beep();
      qWarning("No sound player defined by user" );
      return;
    }

  QStringList list = player.split(" ");

  info = QFileInfo( list[0] );

  if( ! info.isExecutable() )
    {
      QApplication::beep();
      qWarning("Sound player %s is not executable", player.toLatin1().data() );
      return;
    }

  QString cmd;

  if( player.contains( "%s" ) )
    {
      // the sound file is enclosed in the command string
      cmd = player.replace( "%s", _sound );
    }
  else
    {
      // the sound file can appended at the player end
      cmd = player + " " + _sound;
    }

  // qDebug( "%s", cmd.toLatin1().data() );

  // Execute the player command, to play the passed sound.
  // QProcess::execute( cmd ) did not work at this place. Maybe it
  // uses also threads. The good old c-function system() works solid.
  system( cmd.toLatin1().data() );
}
