/***********************************************************************
**
**   maemostyle.h
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

#ifndef MaemoStyle_h
#define MaemoStyle_h

#include <QPlastiqueStyle>

class MaemoStyle : public QPlastiqueStyle
{
  Q_OBJECT

public:

  MaemoStyle();
  virtual ~MaemoStyle();

  virtual int pixelMetric( PixelMetric metric, const QStyleOption *option = 0,
                           const QWidget *widget = 0 ) const;

};

#endif
