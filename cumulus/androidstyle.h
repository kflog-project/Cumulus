/***********************************************************************
**
**   androidstyle.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008-2010 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class AndroidProxyStyle
 *
 * \author Axel Pauli
 *
 * \brief Class to adapt some Qt style settings better to Android.
 *
 * GUI adations for Android for a better user handling. The size of some
 * GUI elements will be increased by using a style proxy.
 *
 * \date 2010
 *
 * \version $Id$
 */

#ifndef AndroidStyle_h
#define AndroidStyle_h

#include <QProxyStyle>

class AndroidProxyStyle : public QProxyStyle
{

public:

  /**
   * \param style A style object to be used as base reference.
   */
  AndroidProxyStyle( QStyle* style=0 );

  virtual ~AndroidProxyStyle() {};

  /**
   * \param metric Pixel The metric where is ask for its size.
   * \param option A option parameter.
   * \param widget A pointer to a widget.
   * \return The value of the given pixel metric.
   */
  virtual int pixelMetric( PixelMetric metric,
                           const QStyleOption *option = 0,
                           const QWidget *widget = 0 ) const;

};

#endif
