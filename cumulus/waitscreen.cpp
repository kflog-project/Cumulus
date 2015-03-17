/***********************************************************************
**
**   waitscreen.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <unistd.h>
#include <QtGui>
#include <QGridLayout>

#include "generalconfig.h"
#include "layout.h"
#include "waitscreen.h"

WaitScreen::WaitScreen(QWidget *parent ) :
  QDialog( parent, Qt::FramelessWindowHint ),
  progress( 0 ),
  lastRot( 0 ),
  _screenUsage( true )
{
  setObjectName("WaitScreen");

  int scale = Layout::getIntScaledDensity();

  QGridLayout * topLayout  = new QGridLayout;
  topLayout->setMargin(5 * scale);
  topLayout->setSpacing( 10 );
  topLayout->setColumnMinimumWidth(0, 45);

  QGridLayout * backLayout = new QGridLayout(this);
  backLayout->setMargin(0);
  backLayout->addLayout(topLayout, 1, 1);
  backLayout->setRowMinimumHeight(0, 3 * scale);
  backLayout->setRowMinimumHeight(2, 3 * scale);
  backLayout->setColumnMinimumWidth(0, 3 * scale);
  backLayout->setColumnMinimumWidth(2, 3 * scale);

  QFrame* frm = new QFrame(this);
  frm->setFrameStyle(QFrame::Box | QFrame::Plain);
  frm->setLineWidth(3 * scale);
  backLayout->addWidget(frm, 0, 0, 3, 3);

  Glider = new QLabel;
  topLayout->addWidget(Glider, 0, 0, 3, 1);

  QLabel* txt = new QLabel(tr("Cumulus is working, please wait!"), this);
  topLayout->addWidget(txt, 0, 1);

  Text1 = new QLabel;
  topLayout->addWidget(Text1, 1, 1);

  Text2 = new QLabel;
  topLayout->addWidget(Text2, 2, 1);

  _gliders = GeneralConfig::instance()->loadPixmap("gliders.png");
  _glider = QPixmap(40, 40);
  _glider.fill( palette().color(backgroundRole()) );

  QPainter p(&_glider);
  p.drawPixmap( 0, 0, _gliders );

  Glider->setPixmap(_glider);
  setVisible( true );
}

WaitScreen::~WaitScreen()
{}

/** This slot is used to set the main text, such as "Loading maps..." */
void WaitScreen::slot_SetText1(const QString& text)
{
  Text1->setText(text);
  Text2->setText("");

  // qDebug("slot_SetText1(): text=%s\n", text.latin1());

  if( screenUsage() )
    {
      slot_Progress(1);
    }
  else
    {
      flush();
    }
}

/**
 * This slot is used to set the secondary text, such as the name of the
 * airspace file that is being loaded. It is also reset to an empty string
 * if SetText1 is called.
 */
void WaitScreen::slot_SetText2(const QString& text)
{
  QString shortText = text;

  if( text.length() > 32 )
    {
      shortText="..." + text.right(32);

      int pos = shortText.lastIndexOf( QChar('/') );

      if( pos != -1 )
        {
          // cut directory paths
          shortText = shortText.right(shortText.length() - pos - 1 );
        }
    }

  Text2->setText(shortText);

  if( screenUsage() )
    {
      slot_Progress(1);
    }
  else
    {
      flush();
    }
}

/**
 * This slot is called to indicate progress. It is used to rotate the
 * glider-icon to indicate to the user that something is in fact happening...
 */
void WaitScreen::slot_Progress( int stepsize )
{
  if( screenUsage() && isVisible() )
    {
      progress += stepsize;

      int rot = progress % 24;  //we are rotating in steps of 15 degrees.

      if( lastRot != rot )
        {
          _glider.fill( Glider->palette().color( QPalette::Window ) );

          QPainter p( &_glider );
          p.drawPixmap( 0, 0, _gliders, rot * 40, 0, 40, 40 );
          Glider->setPixmap( _glider );

          lastRot = rot;
          setVisible( true );
          repaint();
          flush();
        }
     }
}

void WaitScreen::flush()
{
  QCoreApplication::flush();

#ifdef ANDROID
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents|QEventLoop::ExcludeSocketNotifiers);
#endif

  //usleep(250000);
}
