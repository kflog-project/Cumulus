/***********************************************************************
**
**   whatsthat.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers / TrollTech
**                   2008 Axel Pauli                
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QToolTip>
#include <Q3StyleSheet>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QMouseEvent>

#include "whatsthat.h"
#include "map.h"

// shadowWidth not const, for XP drop-shadow-fu turns it to 0
int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
const int vMargin = 4;
const int hMargin = 6;

uint WhatsThat::instance = 0;

WhatsThat::WhatsThat( QWidget* w, const QString& txt, QWidget* parent,
                      const char* name, int timeout )
        : QWidget( parent, name,
                   Qt::Window|Qt::WStyle_StaysOnTop|Qt::WStyle_Customize|Qt::WStyle_NoBorder ),
          text( txt ),
          pressed( FALSE ),
          widget( w )
{
    blockHide=false;
    waitingForRedraw=false;

    // qDebug("WhatsThat::WhatsThat()");
    instance++;
    doc = 0;
    autohideTimer = new QTimer(this);
    setBackgroundMode( Qt::NoBackground );
    setBackgroundColor( Qt::red );
    setPalette( QToolTip::palette() );
    setMouseTracking( FALSE );

    QRect r;

    if ( Q3StyleSheet::mightBeRichText( text ) ) {
        QFont f("Helvetica", 14, QFont::Bold  );
        doc = new Q3SimpleRichText( text, f );
        doc->adjustSize();
        int w = QApplication::desktop()->screenGeometry().width();
        // int h = QApplication::desktop()->screenGeometry().height();

        // adapt RichText to desktop window size
        if( doc->width() > (w-22) ) {
            w = w-22;
        } else {
            w = doc->width();
        }

        r.setRect( 0, 0, w, doc->height() );

    } else {
        int sw = QApplication::desktop()->width() / 3;
        if ( sw < 200 )
            sw = 200;
        else if ( sw > 300 )
            sw = 300;

        r = fontMetrics().boundingRect( 0, 0, sw, 1000,
                                        Qt::AlignTop + Qt::WordBreak + Qt::ExpandTabs,
                                        text );
    }

    resize( r.width() + 2*hMargin + shadowWidth, r.height() + 2*vMargin + shadowWidth );
    position();

    // @AP: Widget will be destroyed, if timer expired. If timeout is
    // zero, manual quit is expected by the user.

    if( timeout > 0 ) {
        connect(autohideTimer, SIGNAL(timeout()),
                this, SLOT(hide()));
        autohideTimer->start(timeout,true);
    }

    //@AS: Quick 'n dirty connection to map...
    connect(Map::getInstance(), SIGNAL(isRedrawing(bool)),
            this, SLOT(mapIsRedrawing(bool)));
}


WhatsThat::~WhatsThat()
{
    instance--;

    // @ap: allocated objects should be deleted, if they exists
    delete doc;
}


void WhatsThat::hide()
{
    // @AP: stop timer to avoid callbacks into nirvana
    autohideTimer->stop();

    if (blockHide) {
        waitingForRedraw=true;
        qDebug("Mapredraw in progress, waiting to finish...");
    } else {
        QWidget::close(true);
    }
}


void WhatsThat::mousePressEvent( QMouseEvent* e )
{
    pressed = TRUE;
    if ( e->button() == Qt::LeftButton && rect().contains( e->pos() ) ) {
        if ( doc )
            anchor = doc->anchorAt( e->pos() -  QPoint( hMargin, vMargin) );
        return;
    }
    hide();
}


void WhatsThat::mouseReleaseEvent( QMouseEvent*  )
{
    if ( !pressed )
        return;

    hide();
}


void WhatsThat::mouseMoveEvent( QMouseEvent* )
{}


void WhatsThat::keyPressEvent( QKeyEvent* )
{
    hide();
}


void WhatsThat::paintEvent( QPaintEvent* )
{
    bool drawShadow = TRUE;

    QRect r = rect();
    if ( drawShadow ) {
        r.setWidth(r.width()-shadowWidth);
        r.setHeight(r.height()-shadowWidth);
    }
    QPainter p( this);
    p.setPen( colorGroup().foreground() );
    p.drawRect( r );
    //p.setPen( colorGroup().mid() );
    // p.setBrush( colorGroup().brush( QColorGroup::Background ) );
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#FFFFE6"));
    int w = r.width();
    int h = r.height();
    p.drawRect( 1, 1, w-2, h-2 );

    if ( drawShadow ) {
        p.setPen( colorGroup().shadow() );
        p.drawPoint( w + 5, 6 );
        p.drawLine( w + 3, 6, w + 5, 8 );
        p.drawLine( w + 1, 6, w + 5, 10 );
        int i;
        for( i=7; i < h; i += 2 )
            p.drawLine( w, i, w + 5, i + 5 );
        for( i = w - i + h; i > 6; i -= 2 )
            p.drawLine( i, h, i + 5, h + 5 );
        for( ; i > 0 ; i -= 2 )
            p.drawLine( 6, h + 6 - i, i + 5, h + 5 );
    }
    p.setPen( colorGroup().foreground() );
    //r.addCoords( hMargin, vMargin, -hMargin, -vMargin );
    r.moveBy(hMargin,vMargin);
    r.setWidth(r.width()-hMargin);
    r.setHeight(r.height()-2*vMargin);

    if ( doc ) {
        doc->draw( &p, r.x(), r.y(), r, colorGroup(), 0 );
    } else {
      p.drawText( r, Qt::AlignTop + Qt::WordBreak + Qt::ExpandTabs, text );
    }
}


/** Tries to find itself a good position to display. */
void WhatsThat::position()
{
    // okay, now to find a suitable location

    QRect screen = QApplication::desktop()->rect();

    int x;
    int w = this->width();
    int h = this->height();
    int sx = screen.x();
    int sy = screen.y();
    QPoint ppos = QCursor::pos();

    // first try locating the widget immediately above/below,
    // with nice alignment if possible.
    QPoint pos;
    if ( widget )
        pos = widget->mapToGlobal( QPoint( 0,0 ) );

    if ( widget && w > widget->width() + 16 )
        x = pos.x() + widget->width()/2 - w/2;
    else
        x = ppos.x() - w/2;

    // squeeze it in if that would result in part of what's this
    // being only partially visible
    if ( x + w  + shadowWidth > sx+screen.width() )
        x = (widget? (qMin(screen.width(),
                           pos.x() + widget->width())
                     ) : screen.width() )
            - w;

    if ( x < sx )
        x = sx;

    int y;
    if ( widget && h > widget->height() + 16 ) {
        y = pos.y() + widget->height() + 2; // below, two pixels spacing
        // what's this is above or below, wherever there's most space
        if ( y + h + 10 > sy+screen.height() )
            y = pos.y() + 2 - shadowWidth - h; // above, overlap
    }
    y = ppos.y() + 2;

    // squeeze it in if that would result in part of what's this
    // being only partially visible
    if ( y + h + shadowWidth > sy+screen.height() )
        y = ( widget ? (qMin(screen.height(),
                             pos.y() + widget->height())
                       ) : screen.height() )
            - h;
    if ( y < sy )
        y = sy;

    this->move( x, y );
    //  this->show();

}


void WhatsThat::mapIsRedrawing(bool redraw)
{
    blockHide=redraw;
    if ((!redraw) && waitingForRedraw) {
        hide();
    }
}

