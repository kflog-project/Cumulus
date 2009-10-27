/***************************************************************************
    coordedit.cpp  -  This file is part of Cumulus
                             -------------------
    begin                : Mon Dec 3 2001
    copyright            : (C) 2001      by Harald Maier
                               2008-2009 by Axel Pauli

    email                : axel@kflog.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "coordedit.h"
#include "wgspoint.h"

CoordEdit::CoordEdit(QWidget *parent) : QLineEdit(parent)
{
  setObjectName("CoordEdit");
  firstSet = true;
}

/** Returns true, if initial input text has been changed */
bool CoordEdit::isInputChanged()
{
  bool res = QLineEdit::text() == initText;

  // qDebug( "CoordEdit::isInputChanged(): %d", !res );
  return !res;
}

void CoordEdit::focusInEvent (QFocusEvent *e)
{
  // overwrite default behavior of QLineEdit
  // fake a mouse event to prevent text highlight
  // e->setReason(QFocusEvent::Mouse);
  QLineEdit::focusInEvent(e);
  // set cursor to start of line
  home(false);
}


// this function handle all the keyboard input
void CoordEdit::keyPressEvent (QKeyEvent *e)
{
  QString s; // pressed Key will be assigned
  QString b = QLineEdit::text(); // current buffer
  int col;
  bool isNumber;

  if (e->text() != "")
    {
      s = e->text().toUpper();
      col = cursorPosition();

      if (hasSelectedText())
        {
          deselect();
        }

      switch (e->key())
        {
        case Qt::Key_Backspace:
          col -= 1;
          // fall through

        case Qt::Key_Delete:
          setText(text().replace(col, 1, mask.mid(col, 1)));
          setCursorPosition(col);
          break;

        default:
          if (col == int(text().length() - 1))
            {
              if (validDirection.contains(s))
                {
                  setText(text().replace(text().length() - 1, 1, s));
                  setCursorPosition(text().length());
                }
            }
          else
            {
              s.toInt(&isNumber);

              if (isNumber && mask[col] == '0')
                {
                  // @AP: here we should check, if the input makes
                  // sense. Otherwise we accept wrong entries, which will be set
                  // to zero by KFLogDegree() conversion method.

                  // qDebug("pos=%d, mask=%s, buffer=%s, input=%s",
                  //     col, mask.latin1(), b.latin1(), s.latin1() );

                  if ( validDirection == "NS" ) // latitude input
                    {
                      if ( col == 0 && s[0] == '9' )
                        {
                          // set the maximum
                          if ( WGSPoint::getFormat() == WGSPoint::DMS )
                            {
                              setText(text().replace(0, 10, "90\260 00' 00"));
                            }
                          else if ( WGSPoint::getFormat() == WGSPoint::DDM )
                            {
                              setText(text().replace(0, 10, "90\260 00.000"));
                            }
                          else
                            {
                              setText(text().replace(0, 9, "90.00000\260"));
                            }

                          setCursorPosition(col+1);
                          break;
                        }
                      else if ( col > 0 && b[0] == '9' && s[0] != '0' )
                        {
                          // set the maximum
                        if ( WGSPoint::getFormat() == WGSPoint::DMS )
                          {
                            setText(text().replace(0, 10, "90\260 00' 00"));
                          }
                        else if ( WGSPoint::getFormat() == WGSPoint::DDM )
                          {
                            setText(text().replace(0, 10, "90\260 00.000"));
                          }
                        else
                          {
                            setText(text().replace(0, 9, "90.00000\260"));
                          }

                          setCursorPosition(col);
                          break;
                        }
                      else if ( (WGSPoint::getFormat() == WGSPoint::DMS &&
                                 (col == 4 || col == 8 ) && s[0] > '5' ) ||
                                (WGSPoint::getFormat() == WGSPoint::DDM &&
                                 col == 4  && s[0] > '5' ) )
                        {
                          // reset b[col] to 5
                          setText(text().replace(col, 1, "5"));
                          setCursorPosition(col);
                          break;
                        }
                    }
                  else // longitude input
                    {
                      if ( (col == 0 && (s[0] > '1' || (s[0] == '1' && b[1] >= '8') )) )
                        {
                          // set the maximum
                          if ( WGSPoint::getFormat() == WGSPoint::DMS )
                            {
                              setText(text().replace(0, 11, "180\260 00' 00"));
                            }
                          else if ( WGSPoint::getFormat() == WGSPoint::DDM )
                            {
                              setText(text().replace(0, 11, "180\260 00.000"));
                            }
                          else
                            {
                              setText(text().replace(0, 10, "180.00000\260"));
                            }

                          setCursorPosition( s[0] == '1' ? col+1 : col );
                          break;
                        }
                      else if ( col == 1 && b[0] == '1' && s[0] >= '8' )
                        {
                          // set the maximum
                          if ( WGSPoint::getFormat() == WGSPoint::DMS )
                            {
                              setText(text().replace(0, 11, "180\260 00' 00"));
                            }
                          else if ( WGSPoint::getFormat() == WGSPoint::DDM )
                            {
                              setText(text().replace(0, 11, "180\260 00.000"));
                            }
                          else
                            {
                              setText(text().replace(0, 10, "180.00000\260"));
                            }

                          setCursorPosition( s[0] == '8' ? col+1 : col );
                          break;
                        }
                      else if ( col > 1 && b[0] == '1' && b[1] == '8' && s[0] != '0' )
                        {
                          // set the maximum
                        if ( WGSPoint::getFormat() == WGSPoint::DMS )
                          {
                            setText(text().replace(0, 11, "180\260 00' 00"));
                          }
                        else if ( WGSPoint::getFormat() == WGSPoint::DDM )
                          {
                            setText(text().replace(0, 11, "180\260 00.000"));
                          }
                        else
                          {
                            setText(text().replace(0, 10, "180.00000\260"));
                          }

                          setCursorPosition(col);
                          break;
                        }
                      else if ( (WGSPoint::getFormat() == WGSPoint::DMS &&
                                 (col == 5 || col == 9 ) && s[0] > '5' ) ||
                                (WGSPoint::getFormat() == WGSPoint::DDM &&
                                 col == 5  && s[0] > '5' ) )
                        {
                          // reset b[col] to 5
                          setText(text().replace(col, 1, "5"));
                          setCursorPosition(col);
                          break;
                        }
                    }

                  // General default handling for all other input
                  setText(text().replace(col, 1, s));
                  setCursorPosition(++col);

                  // jump to next number field
                  setCursor2NextNo(col);
                  //qDebug("ENDE: pos=%d, mask=%s, buffer=%s, input=%s",
                  // col, mask.latin1(), QLineEdit::text().latin1(), s.latin1() );
                }
            }
        } // switch
    }
  else
    {
      // route all other to default handler
      QLineEdit::keyPressEvent(e);
    }
}

/** Set cursor in dependency of position and input mask to next number field */
void CoordEdit::setCursor2NextNo( int pos )
{
  if (pos < text().length() && mask[pos] == '.')
    {
      setCursorPosition(pos + 1);
    }
  else if (pos < text().length() && mask[pos] != '0')
    {
      setCursorPosition(pos + 2);
    }
}

/** Race condition, showEvent can be called earlier as the related slot_load
    method. In this case the mask is always set */
void CoordEdit::showEvent(QShowEvent *)
{
  if (text().isEmpty())
    {
      setText(mask);
    }
}

LatEdit::LatEdit(QWidget *parent, const int base) : CoordEdit(parent)
{
  setObjectName("LatEdit");

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      mask = "00\260 00' 00\"";
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      mask = "00\260 00.000'";
    }
  else
    {
      mask = "00.00000\260";
    }

  if (base >= 0)
    {
      mask += " N";
    }
  else
    {
      mask += " S";
    }

  validDirection = "NS";
}

LongEdit::LongEdit(QWidget *parent, const int base) : CoordEdit(parent)
{
  setObjectName("LongEdit");

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      mask = "000\260 00' 00\"";
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      mask = "000\260 00.000'";
    }
  else
    {
      mask = "000.00000\260";
    }

  if (base >= 0)
    {
      mask += " E";
    }
  else
    {
      mask += " W";
    }

  validDirection = "WE";
}

/** No descriptions */
void CoordEdit::clear()
{
  setText(mask);
}

/** Returns the value of the edit box in the KFLog internal format for degrees  */
int CoordEdit::KFLogDegree()
{
  return WGSPoint::degreeToNum(QLineEdit::text());
}


/** Sets the edit box to reflect the given value. */
void CoordEdit::setKFLogDegree(int value, bool isLat)
{
  setText(WGSPoint::printPos(value, isLat));

  if ( firstSet )
    {
      firstSet = false;
      initText = QLineEdit::text(); // save initial text
    }
}

/** overloaded function */
void LatEdit::setKFLogDegree(int value)
{
  CoordEdit::setKFLogDegree(value, true);
}

/** Overloaded function */
void LongEdit::setKFLogDegree(int value)
{
  CoordEdit::setKFLogDegree(value, false);
}
