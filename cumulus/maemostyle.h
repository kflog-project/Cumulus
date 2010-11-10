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
***********************************************************************
** @short Class to adapt some Qt style settings better to Maemo.
**
** The size of some GUI elements will be increased.
**
** @author Axel Pauli
***********************************************************************/

#ifndef MaemoStyle_h
#define MaemoStyle_h

#include <QProxyStyle>

/** GUI adations for Maemo for a better user handling. */
class MaemoProxyStyle : public QProxyStyle
{

public:

  MaemoProxyStyle( QStyle* style=0 );

  virtual ~MaemoProxyStyle() {};

  /**
   * \param metric Pixel metric where is ask for its size.
   * \return The value of the given pixel metric.
   */
  virtual int pixelMetric( PixelMetric metric,
                           const QStyleOption *option = 0,
                           const QWidget *widget = 0 ) const;
};

#endif
