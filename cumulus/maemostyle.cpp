/***********************************************************************
**
**   maemostyle.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************
**  @short Class to adapt some Qt style settings better to Maemo.
**  The size of some GUI elements will be increased.
**
** @author Axel Pauli
***********************************************************************/

#include "maemostyle.h"

// new size values to be assigned
#define SB_SIZE 25 // scrollbar size
#define CB_SIZE 20 // checkbox size
#define RB_SIZE 20 // radio button size

int MaemoWindowsStyle::pixelMetric( PixelMetric metric, const QStyleOption *option,
                                    const QWidget *widget ) const
{
  // qDebug("MaemoWindowsStyle::pixelMetric(): metric=%d", metric);

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
  else
    {
      // call default style handler
      return QWindowsStyle::pixelMetric(metric, option, widget);
    }
}

int MaemoCleanlooksStyle::pixelMetric( PixelMetric metric, const QStyleOption *option,
                                       const QWidget *widget ) const
{
  // qDebug("MaemoWindowsStyle::pixelMetric(): metric=%d", metric);

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
  else
    {
      // call default style handler
      return QCleanlooksStyle::pixelMetric(metric, option, widget);
    }
}

int MaemoPlastiqueStyle::pixelMetric( PixelMetric metric, const QStyleOption *option,
                                       const QWidget *widget ) const
{
  // qDebug("MaemoWindowsStyle::pixelMetric(): metric=%d", metric);

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
  else
    {
      // call default style handler
      return QPlastiqueStyle::pixelMetric(metric, option, widget);
    }
}
