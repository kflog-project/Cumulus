/***********************************************************************
**
**   whatsthat.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WHATSTHAT_H
#define WHATSTHAT_H

#include <QWidget>
#include <QTimer>
#include <QString>
#include <Q3SimpleRichText>

/**
 * Rip-off from Qt's QWhatsThis system in order to be able to
 * control the lifespan.
 * @author André Somers
 */
class WhatsThat : public QWidget
{
    Q_OBJECT
public:
    WhatsThat( QWidget* w, const QString& txt, QWidget* parent,
               const char* name, int timeout=5000 );

    virtual ~WhatsThat() ;

    static uint getInstance()
    {
        return instance;
    };

public slots:
    void hide();

    /**
     * Slot that can be connected to the the @ref Map
     * signal @isRedrawing. This holds deleting the popup
     * untill redrawing of the map under it is ready.
     */
    void mapIsRedrawing(bool);

protected:
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void paintEvent( QPaintEvent* );

private:
    QString text;
    Q3SimpleRichText* doc;
    QString anchor;
    bool pressed;
    QWidget* widget;
    QTimer* autohideTimer;

    /**
     * flag to block hiding of the widget. This is being
     * set by @ref mapIsRedrawing.
     */
    bool blockHide;

    /**
     * flag to indicate that the popup _should_ be closed,
     * but we are waying for the mapredrawing to finish. As
     * soon as that happens, the popup will be closed.
     */
    bool waitingForRedraw;

private: // Private methods
    /** Tries to find itself a good position to display. */
    void position();

    // instance counter
    static uint instance;

};


#endif
