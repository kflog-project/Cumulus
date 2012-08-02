/***********************************************************************
**
**   maemostyle.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008-2012 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
************************************************************************/

#include <QtGui>

#include "maemostyle.h"

// new size values to be assigned
#define SB_SIZE 15 // scrollbar size
#define CB_SIZE 20 // checkbox size
#define RB_SIZE 20 // radio button size
#define TB_SB_WIDTH 40 // tab bar scroll button width

MaemoProxyStyle::MaemoProxyStyle( QStyle* style ) : QProxyStyle( style )
{
}

int MaemoProxyStyle::pixelMetric( PixelMetric metric,
                                  const QStyleOption *option,
                                  const QWidget *widget ) const
{
  // qDebug("MaemoProxyStyle::pixelMetric(): metric=%d", metric);

  if (metric == PM_ScrollBarExtent)
    {
      // increase height of scrollbars
      return SB_SIZE;
    }
  else if( metric == PM_IndicatorWidth)
    {
      // increase width of check box
      return CB_SIZE;
    }
  else if( metric == PM_IndicatorHeight)
    {
      // increase height of check box
      return CB_SIZE;
    }
  else if( metric == PM_ExclusiveIndicatorWidth)
    {
      // increase width of radio button
      return RB_SIZE;
    }
  else if( metric == PM_ExclusiveIndicatorHeight)
    {
      // increase height of radio button
      return RB_SIZE;
    }
  else if( metric == PM_TabBarScrollButtonWidth )
    {
      // increase width of tab bar buttons
      return TB_SB_WIDTH;
    }
  else
    {
      // call default style handler
      return QProxyStyle::pixelMetric( metric, option, widget );
    }
}
