/***********************************************************************
**
**   colordialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2014 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class ColorDialog
 *
 * \author Axel Pauli
 *
 * \brief This dialog is an extended QColorDialog.
 *
 * This dialog is an extended QColorDialog which suppresses the close
 * of the dialog, if the return key is pressed.
 *
 * \date 2014
 *
 * \version $Id$
 *
 */

#ifndef COLOR_DIALOG_H
#define COLOR_DIALOG_H

#include <QColor>
#include <QString>

class QWidget;

class ColorDialog
{
 private:

  ColorDialog();

  Q_DISABLE_COPY( ColorDialog );

 public:

  /**
   * That popup a QColorDialog, which will not be closed, if the return key
   * is pressed.
   *
   * \param initial The initial color setup by the dialog
   *
   * \param parent The parent widget
   *
   * \param title The window title to be set
   *
   * \return A valid color, if the Ok button was pressed, otherwise the
   *         color is set to invalid.
   *
   */
  static QColor getColor( const QColor& initial,
                          QWidget* parent,
                          const QString& title="" );
};

#endif
