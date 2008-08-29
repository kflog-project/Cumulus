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
**  @short Class to adapt some Qt style settings better to Maemo.
**  The size of some GUI elements will be increased.
**
** @author Axel Pauli
***********************************************************************/

#ifndef MaemoStyle_h
#define MaemoStyle_h

#include <QWindowsStyle>
#include <QCleanlooksStyle>
#include <QPlastiqueStyle>


/** Adation to Windows style */
class MaemoWindowsStyle : public QWindowsStyle
{
  Q_OBJECT

public:

  MaemoWindowsStyle() {};
  virtual ~MaemoWindowsStyle() {};

  virtual int pixelMetric( PixelMetric metric, const QStyleOption *option = 0,
                           const QWidget *widget = 0 ) const;

};

/** Adation to Cleanlook style */
class MaemoCleanlooksStyle : public QCleanlooksStyle
{
  Q_OBJECT

public:

  MaemoCleanlooksStyle() {};
  virtual ~MaemoCleanlooksStyle() {};

  virtual int pixelMetric( PixelMetric metric, const QStyleOption *option = 0,
                           const QWidget *widget = 0 ) const;

};

/** Adation to Plastique style */
class MaemoPlastiqueStyle : public QPlastiqueStyle
{
  Q_OBJECT

public:

  MaemoPlastiqueStyle() {};
  virtual ~MaemoPlastiqueStyle() {};

  virtual int pixelMetric( PixelMetric metric, const QStyleOption *option = 0,
                           const QWidget *widget = 0 ) const;

};

#endif
