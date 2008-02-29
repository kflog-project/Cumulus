/***********************************************************************
**
**   waitscreen.h
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

#ifndef WAITSCREEN_H
#define WAITSCREEN_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QString>
#include <QApplication>
#include <QPixmap>

/** This class represents a semi-modal dialog to indicate what is happening to the program.
  * It is used while loading maps for instance.
  * @author André Somers
  */

class WaitScreen : public QDialog
{
    Q_OBJECT
public:
    WaitScreen(QWidget *parent=0);

    ~WaitScreen();

    void setScreenUsage( const bool newValue )
    {
        _screenUsage = newValue;
    };

    const bool screenUsage() const
    {
        return _screenUsage;
    };

public slots: // Public slots
    /**
     * This slot is used to set the main text,
     * such as "Loading maps..."
     */
    void slot_SetText1(const QString& text);

    /**
     * This slot is used to set the secondairy text,
     * such as the name of the airspacefile that is being
     * loaded. It is also reset to an empty string if
     * SetText1 is called.
     */
    void slot_SetText2(const QString& text);

    /**
     * This slot is called to indicate progress. It is
     * used to rotate the glider-icon to indicate to the
     * user that something is in fact happening...
     */
    void slot_Progress(int stepsize=1);


private: // Private attributes
    /**
     * Holds the current progress-value. Used to draw the
     * glider icon in the correct rotation.
     */
    int progress;

    QLabel * Text1;
    QLabel * Text2;
    //  QLabel * Prog;
    QLabel * Glider;

    QApplication *app;

    QPixmap _gliders;
    QPixmap _glider;
    int lastRot;
    bool _screenUsage;

};

#endif
