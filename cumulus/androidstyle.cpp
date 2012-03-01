/***********************************************************************
**
**   androidstyle.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
************************************************************************/

#include <QtGui>

#include "androidstyle.h"

// new size values to be assigned
#ifdef SCROLLER
#define SB_SIZE 10 // scrollbar size, now only an indicator, thanks to kinetic scrolling
#else
#define SB_SIZE 30
#endif

#define CB_SIZE 20 // checkbox size
#define RB_SIZE 20 // radio button size
#define BM_SIZE 4  // margin around button label, default 0
#define TAB_HEIGHT 30 // extra vertical px for tabs
#define TAB_SB_WIDTH 60 // tab horiz. scroll button width

AndroidProxyStyle::AndroidProxyStyle( QStyle* style ) : QProxyStyle( style )
{
}

int AndroidProxyStyle::pixelMetric( PixelMetric metric,
                                    const QStyleOption *option,
                                    const QWidget *widget ) const
{
  // qDebug("AndroidProxyStyle::pixelMetric(): metric=%d", metric);

  if (metric == PM_ScrollBarExtent)
    {
      // increase height of scrollbars
      return SB_SIZE;
    }
  else if( metric == PM_IndicatorWidth || metric == PM_IndicatorHeight)
    {
      // increase dimensions of check box
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
  else if( metric == PM_TabBarTabVSpace)
    {
      return TAB_HEIGHT;
    }
  else if( metric == PM_TabBarScrollButtonWidth )
    {
      // increase width of tab bar buttons
      return TAB_SB_WIDTH;
    }
  else if( metric == PM_ComboBoxFrameWidth )
    {
      // increase width of a combo box
      return SB_SIZE + PM_DefaultFrameWidth;
    }
  else if( metric == PM_ButtonMargin )
    {
      // increase buttons
      return BM_SIZE;
    }
  else if( metric == PM_LayoutVerticalSpacing )
    {
      // increase buttons
      return 16;
    }
  else
    {
      // call default style handler
      return QProxyStyle::pixelMetric( metric, option, widget );
    }
}
