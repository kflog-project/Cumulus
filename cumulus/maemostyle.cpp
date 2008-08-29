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
**  @short Class to adapt some Qt style settings to Maemo.
**
** @author Axel Pauli
***********************************************************************/

#include "maemostyle.h"

MaemoStyle::MaemoStyle()
{
}

MaemoStyle::~MaemoStyle()
{
}

int MaemoStyle::pixelMetric( PixelMetric metric, const QStyleOption *option,
                             const QWidget *widget ) const
{
  //qDebug("MaemoStyle::pixelMetric(): metric=%d", metric);

  if (metric == PM_ScrollBarExtent)
    {
      //qDebug("PM_ScrollBarExtent");
        // increase hight of scrollbars
        return 25;
    }
  else if( metric == PM_IndicatorWidth)
    {
      //qDebug("PM_IndicatorWidth");
        // increase width of check box
        return 20;
    }
  else if( metric == PM_IndicatorHeight)
    {
      //qDebug("PM_IndicatorHeight");
        // increase hight of check box
        return 20;
    }
  else
    {
      return QPlastiqueStyle::pixelMetric(metric, option, widget);
    }
}
