/***********************************************************************
**
**   coordeditnumpad.cpp - Editor for WGS84 coordinates, supports three formats.
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QtGui>

#include "coordeditnumpad.h"
#include "numberEditor.h"
#include "wgspoint.h"

CoordEditNumPad::CoordEditNumPad(QWidget *parent) : QWidget( parent )
{
  iniKflogDegree = 0;
  iniDegree      = "";
  iniMinute      = "";
  iniSecond      = "";
  iniDirection   = "";

  setObjectName("CoordEditNumPad");

  const int spaceItem1 = 5;
  const int spaceItem2 = 10;

  QLabel *label;
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setSpacing(0);
  hbox->setContentsMargins ( 0, 0, 0, 0 );

  // In dependency of the selected coordinate format three different layouts
  // are provided by this widget.
  minuteBox = static_cast<NumberEditor *> (0);
  secondBox = static_cast<NumberEditor *> (0);

  // Degree input is always needed
  degreeBox = new NumberEditor( this );
  degreeBox->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  hbox->addWidget( degreeBox );
  hbox->addSpacing( spaceItem1 );
  label = new QLabel( "\260", this );
  hbox->addWidget( label );
  hbox->addSpacing( spaceItem2 );

  if ( WGSPoint::getFormat() == WGSPoint::DDM ||
       WGSPoint::getFormat() == WGSPoint::DMS )
    {
      minuteBox = new NumberEditor( this );
      minuteBox->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
      hbox->addWidget( minuteBox );
      hbox->addSpacing( spaceItem1 );
      label = new QLabel( "'", this );
      hbox->addWidget( label );
      hbox->addSpacing( spaceItem2 );
    }

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      secondBox = new NumberEditor( this );
      secondBox->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
      hbox->addWidget( secondBox );
      hbox->addSpacing( spaceItem1 );
      label = new QLabel( "\"", this );
      hbox->addWidget( label );
      hbox->addSpacing( spaceItem2 );
    }

  // add push button as toggle for the sky directions
  skyDirection = new QPushButton( this );
  hbox->addWidget( skyDirection );
  hbox->addStretch( 10 );

  connect( skyDirection, SIGNAL(pressed()), SLOT(slot_changeSkyDirection()) );

  setLayout( hbox );

  if ( WGSPoint::getFormat() != WGSPoint::DDD )
    {
      // If the coordinate format is not equal to decimal degree, we have
      // to check all the input boxes to prevent senseless values there.
      connect( degreeBox, SIGNAL(numberEdited( const QString&)),
               this, SLOT(slot_numberEdited( const QString& )) );

      connect( minuteBox, SIGNAL(numberEdited( const QString&)),
               this, SLOT(slot_numberEdited( const QString& )) );

      if( WGSPoint::getFormat() == WGSPoint::DMS )
        {
          connect( secondBox, SIGNAL(numberEdited( const QString&)),
                   this, SLOT(slot_numberEdited( const QString& )) );
        }
    }
}

CoordEditNumPad::~CoordEditNumPad()
{
}

/**
 * Catch show events in this class to set a uniform width for different
 * widgets depending on the used font.
 *
 */
void CoordEditNumPad::showEvent( QShowEvent * )
{
  QFontMetrics fm( font() );

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      int strWidth = fm.width(QString("00000"));

      degreeBox->setMinimumWidth( strWidth );
      degreeBox->setMaximumWidth( strWidth );
      minuteBox->setMinimumWidth( strWidth );
      minuteBox->setMaximumWidth( strWidth );
      secondBox->setMinimumWidth( strWidth );
      secondBox->setMaximumWidth( strWidth );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      int strWidth1 = fm.width(QString("00000"));
      int strWidth2 = fm.width(QString("00.00000"));

      degreeBox->setMinimumWidth( strWidth1 );
      degreeBox->setMaximumWidth( strWidth1 );
      minuteBox->setMinimumWidth( strWidth2 );
      minuteBox->setMaximumWidth( strWidth2 );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      int strWidth = fm.width(QString("000.0000000"));

      degreeBox->setMinimumWidth( strWidth );
      degreeBox->setMaximumWidth( strWidth );
    }

  int height = degreeBox->minimumSizeHint().height();

  skyDirection->setMinimumSize( height + height / 2, height );
  skyDirection->setMaximumSize( height + height / 2, height );
}

void CoordEditNumPad::slot_changeSkyDirection()
{
  if( skyDirection->text() == "N" )
    {
      skyDirection->setText( "S" );
    }
  else if( skyDirection->text() == "S" )
    {
      skyDirection->setText( "N" );
    }
  else if( skyDirection->text() == "E" )
    {
      skyDirection->setText( "W" );
    }
  else if( skyDirection->text() == "W" )
    {
      skyDirection->setText( "E" );
    }
}

/**
 * Returns true, if initial input values have been modified by the user.
 */
bool CoordEditNumPad::isInputChanged()
{
  bool changed = false;

  if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      changed |= iniDegree != degreeBox->text();
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      changed |= iniDegree != degreeBox->text();
      changed |= iniMinute != minuteBox->text();
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      changed |= iniDegree != degreeBox->text();
      changed |= iniMinute != minuteBox->text();
      changed |= iniSecond != secondBox->text();
    }

  changed |= iniDirection != skyDirection->text();

  return changed;
}

/**
 * Sets the controls for the latitude editor.
 */
LatEditNumPad::LatEditNumPad(QWidget *parent, const int base) : CoordEditNumPad(parent)
{
  setObjectName("LatEditNumPad");

  QRegExpValidator *eValidator;

  if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      QString inputMaskD("99.99999");

      degreeBox->setMaxLength(inputMaskD.size());
      eValidator = new QRegExpValidator( QRegExp( "([0-8][0-9]\\.[0-9]{1,5})|(90\\.0{1,5})" ), this );
      degreeBox->setValidator( eValidator );
      degreeBox->setTip("00.0...90.0");
      degreeBox->setPmVisible( false );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      QString inputMaskD("99");
      QString inputMaskM("99.999");

      degreeBox->setMaxLength(inputMaskD.size());
      eValidator = new QRegExpValidator( QRegExp( "([0-8][0-9])|90" ), this );
      degreeBox->setValidator( eValidator );
      degreeBox->setTip("00...90");
      degreeBox->setPmVisible( false );
      degreeBox->setDecimalVisible( false );

      minuteBox->setMaxLength(inputMaskM.size());
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]\\.[0-9]{1,3}" ), this );
      minuteBox->setValidator( eValidator );
      minuteBox->setTip("00.0...59.999");
      minuteBox->setPmVisible( false );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      QString inputMask("99");

      degreeBox->setMaxLength(inputMask.size());
      eValidator = new QRegExpValidator( QRegExp( "([0-8][0-9])|90" ), this );
      degreeBox->setValidator( eValidator );
      degreeBox->setTip("00...90");
      degreeBox->setPmVisible( false );
      degreeBox->setDecimalVisible( false );

      minuteBox->setMaxLength(inputMask.size());
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]" ), this );
      minuteBox->setValidator( eValidator );
      minuteBox->setTip("00...59");
      minuteBox->setPmVisible( false );
      minuteBox->setDecimalVisible( false );

      secondBox->setMaxLength(inputMask.size());
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]" ), this );
      secondBox->setValidator( eValidator );
      secondBox->setTip("00...59");
      secondBox->setPmVisible( false );
      secondBox->setDecimalVisible( false );
    }

  // Set all edit fields to zero.
  setKFLogDegree(0);

  // Set default sky direction.
  if (base >= 0)
    {
      skyDirection->setText( "N" );
    }
  else
    {
      skyDirection->setText( "S" );
    }
}

/**
 * Sets the controls for the longitude editor.
 */
LongEditNumPad::LongEditNumPad(QWidget *parent, const int base) : CoordEditNumPad(parent)
{
  setObjectName("LongEditNumPad");

  QRegExpValidator *eValidator;

  if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      QString inputMaskD("999.99999");

      degreeBox->setMaxLength(inputMaskD.size());
      eValidator = new QRegExpValidator( QRegExp( "(0[0-9][0-9]\\.[0-9]{1,5})|([0-1][0-7][0-9]\\.[0-9]{1,5})|(180\\.0{1,5})" ), this );
      degreeBox->setValidator( eValidator );
      degreeBox->setTip("000.0...180.0");
      degreeBox->setPmVisible( false );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      QString inputMaskD("999");
      QString inputMaskM("99.999");

      degreeBox->setMaxLength(inputMaskD.size());
      eValidator = new QRegExpValidator( QRegExp( "(0[0-9][0-9])|(1[0-7][0-9])|180" ), this );
      degreeBox->setValidator( eValidator );
      degreeBox->setTip("000...180");
      degreeBox->setPmVisible( false );
      degreeBox->setDecimalVisible( false );

      minuteBox->setMaxLength(inputMaskM.size());
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]\\.[0-9]{1,3}" ), this );
      minuteBox->setValidator( eValidator );
      minuteBox->setTip("00.0...59.999");
      minuteBox->setPmVisible( false );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      degreeBox->setMaxLength(3);
      eValidator = new QRegExpValidator( QRegExp( "(0[0-9][0-9])|(1[0-7][0-9])|180" ), this );
      degreeBox->setValidator( eValidator );
      degreeBox->setTip("000...180");
      degreeBox->setPmVisible( false );
      degreeBox->setDecimalVisible( false );

      minuteBox->setMaxLength(2);
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]" ), this );
      minuteBox->setValidator( eValidator );
      minuteBox->setTip("00...59");
      minuteBox->setPmVisible( false );
      minuteBox->setDecimalVisible( false );

      secondBox->setMaxLength(2);
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]" ), this );
      secondBox->setValidator( eValidator );
      secondBox->setTip("00...59");
      secondBox->setPmVisible( false );
      secondBox->setDecimalVisible( false );
    }

  // Set all edit fields to zero
  setKFLogDegree(0);

  // Set the default sky direction.
  if (base >= 0)
    {
      skyDirection->setText( "E" );
    }
  else
    {
      skyDirection->setText( "W" );
    }
}

/** Used to check the user input in the editor fields. */
void CoordEditNumPad::slot_numberEdited( const QString& )
{
  if( degreeBox->text() == "90" || degreeBox->text() == "180" )
    {
      // If the degree box is set to the possible maximum, the other
      // boxes must be set to zero to prevent senseless results.
      if ( WGSPoint::getFormat() == WGSPoint::DDM )
          {
            minuteBox->setText( "00.000" );
            return;
         }

      if ( WGSPoint::getFormat() == WGSPoint::DMS )
        {
          minuteBox->setText( "00");
          secondBox->setText( "00");
          return;
        }
    }
}

/**
 * Calculates a degree value in the KFLog internal format for degrees from
 * the input data fields.
 */
int CoordEditNumPad::KFLogDegree()
{
  if( isInputChanged() == false )
    {
      // Nothing was modified, return initial value to avoid rounding
      // errors during conversions.
      return iniKflogDegree;
    }

  QString input = "";
  QChar degreeChar(Qt::Key_degree);

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      input = degreeBox->text() + degreeChar + " " +
              minuteBox->text() + "' " +
              secondBox->text() + "\"";
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      // Add missing zeros, if needed
      QString number = minuteBox->text();
      int maxLen     = minuteBox->maxLength();

      if( maxLen != 32767 && maxLen > 0 && number.size() < maxLen )
        {
          int end = maxLen - number.size();

          for( int i = 0; i < end; i++ )
            {
              number.append("0");
            }
        }

      input = degreeBox->text() + degreeChar + " " + number + "'";
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      // Add missing zeros, if needed
      QString number = degreeBox->text();
      int maxLen     = degreeBox->maxLength();

      int end = maxLen - number.size();

      if( maxLen != 32767 && maxLen > 0 && number.size() < maxLen )
        {
          for( int i = 0; i < end; i++ )
            {
              number.append("0");
            }
        }

      input = number + degreeChar;
    }

  input += " " + skyDirection->text().trimmed();

  // This method make the conversion to the internal KFLog degree format.
  return WGSPoint::degreeToNum( input );
}

/**
 * Sets all edit fields according to the passed coordinate value.
 * The coordinate value is encoded in the KFLog internal format for degrees.
 */
void CoordEditNumPad::setKFLogDegree( const int coord, const bool isLat )
{
  QString posDeg, posMin, posSec;
  int degree, min, sec;
  double decDegree, decMin;

  iniKflogDegree = coord; // save initial coordinate value

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      // degrees, minutes, seconds is used as format
      WGSPoint::calcPos (coord, degree, min, sec);

      if (isLat)
        {
          posDeg.sprintf("%02d", (degree < 0)  ? -degree : degree);
        }
      else
        {
          posDeg.sprintf("%03d", (degree < 0)  ? -degree : degree);
        }

      min = abs(min);
      posMin.sprintf("%02d", min);

      sec = abs(sec);
      posSec.sprintf("%02d", sec);

      degreeBox->setText( posDeg );
      minuteBox->setText( posMin );
      secondBox->setText( posSec );

      // save initial values
      iniDegree = posDeg;
      iniMinute = posMin;
      iniSecond = posSec;
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      // degrees and decimal minutes is used as format
      WGSPoint::calcPos (coord, degree, decMin);

      if (isLat)
        {
          posDeg.sprintf("%02d", (degree < 0)  ? -degree : degree);
        }
      else
        {
          posDeg.sprintf("%03d", (degree < 0)  ? -degree : degree);
        }

      decMin = fabs(decMin);

      posMin.sprintf("%.3f", decMin);

      // Unfortunately sprintf does not support leading zero in float
      // formating. So we must do it alone.
      if ( decMin < 10.0 )
        {
          posMin.insert(0, "0");
        }

      degreeBox->setText( posDeg );
      minuteBox->setText( posMin );

      // save initial values
      iniDegree = posDeg;
      iniMinute = posMin;
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      // decimal degrees is used as format
      WGSPoint::calcPos (coord, decDegree);

      posDeg.sprintf("%.5f", (decDegree < 0)  ? -decDegree : decDegree);

      // Unfortunately sprintf does not support leading zero in float
      // formating. So we must do it alone.
      if (isLat)
        {
          if ( decDegree < 10.0 )
            {
              posDeg.insert(0, "0");
            }
        }
      else
        {
          if ( decDegree < 10.0 )
            {
              posDeg.insert(0, "00");
            }
          else if ( decDegree < 100.0 )
            {
              posDeg.insert(0, "0");
            }
        }

      degreeBox->setText( posDeg );

      // save initial value
      iniDegree = posDeg;
    }

  // Set the sky direction as label in the push button
  if (coord < 0)
    {
      if( isLat )
        {
          skyDirection->setText( "S" );
        }
      else
        {
          skyDirection->setText( "W" );
        }
    }
  else
    {
      if( isLat )
        {
          skyDirection->setText( "N" );
        }
      else
        {
          skyDirection->setText( "E" );
        }
    }

  // Save initial value of sky direction.
  iniDirection = skyDirection->text();
}

/** Sets all edit fields to the passed coordinate value in KFLog format. */
void LatEditNumPad::setKFLogDegree( const int coord )
{
  CoordEditNumPad::setKFLogDegree( coord, true );
}

/** Sets all edit fields to the passed coordinate value in KFLog format. */
void LongEditNumPad::setKFLogDegree( const int coord )
{
  CoordEditNumPad::setKFLogDegree( coord, false );
}
