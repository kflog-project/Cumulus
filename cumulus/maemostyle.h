/***********************************************************************
**
**   maemostyle.h
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
 * \class MaemoProxyStyle
 *
 * \author Axel Pauli
 *
 * \brief Class to adapt some Qt style settings better to Maemo.
 *
 * GUI adations for Maemo for a better user handling. The size of some
 * GUI elements will be increased by using a style proxy.
 *
 * \date 2010
 */

#ifndef MaemoStyle_h
#define MaemoStyle_h

#include <QProxyStyle>

class MaemoProxyStyle : public QProxyStyle
{

public:

  /**
   * \param style A style object to be used as base reference.
   */
  MaemoProxyStyle( QStyle* style=0 );

  virtual ~MaemoProxyStyle() {};

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
