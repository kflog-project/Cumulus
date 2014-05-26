/***************************************************************************
                          polar.cpp  -  description
                             -------------------
    begin                : Okt 18 2002
    copyright            : (C) 2002      by Eggert Ehmke
                               2008-2013 by Axel Pauli

    email                : eggert.ehmke@berlin.de, kflog.cumulus@gmail.com

    $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cmath>

#include <QtGui>

#include "polar.h"

Polar::Polar() :
  _name(""),
  _v1(0),
  _w1(0),
  _v2(0),
  _w2(0),
  _v3(0),
  _w3(0),
  _a(0),
  _aa(0),
  _b(0),
  _bb(0),
  _c(0),
  _cc(0),
  _water(0),
  _bugs(0),
  _emptyWeight(0),
  _grossWeight(0),
  _wingArea(0),
  _seats(0),
  _maxWater(0)
{
}

Polar::Polar(const QString& name,const Speed& v1, const Speed& w1,
             const Speed& v2, const Speed& w2,
             const Speed& v3, const Speed& w3,
             double /*wingLoad*/, double wingArea,
             double emptyWeight, double grossWeight) :
    _name(name),
    _v1(v1),
    _w1(w1),
    _v2(v2),
    _w2(w2),
    _v3(v3),
    _w3(w3),
    _water(0),
    _bugs(0),
    _emptyWeight(emptyWeight),
    _grossWeight(grossWeight),
    _wingArea(wingArea),
    _seats(1),
    _maxWater(0)
{
  double V1 = v1.getMps();
  double V2 = v2.getMps();
  double V3 = v3.getMps();
  double W1 = w1.getMps();
  double W2 = w2.getMps();
  double W3 = w3.getMps();

  double d = V1*V1*(V2-V3)+V2*V2*(V3-V1)+V3*V3*(V1-V2);

  if (d == 0.0)
    {
      _a = _aa = 0.0;
    }
  else
    {
      _a = _aa = ((V2-V3)*(W1-W3)+(V3-V1)*(W2-W3))/d;
    }

  d = V2-V3;

  if (d == 0.0)
    {
      _b = _bb=0.0;
    }
  else
    {
      _b =_bb = (W2-W3-_aa*(V2*V2-V3*V3))/d;
    }

  _c = _cc = W3 - _aa*V3*V3 - _bb*V3;
}

Polar::Polar (const Polar& polar) :
  _name (polar._name),
  _v1 (polar._v1),
  _w1 (polar._w1),
  _v2 (polar._v2),
  _w2 (polar._w2),
  _v3 (polar._v3),
  _w3 (polar._w3),
  _a  (polar._a),
  _aa  (polar._aa),
  _b  (polar._b),
  _bb  (polar._bb),
  _c  (polar._c),
  _cc  (polar._cc),
  _water (polar._water),
  _bugs (polar._bugs),
  _emptyWeight (polar._emptyWeight),
  _grossWeight (polar._grossWeight),
  _wingArea(polar._wingArea),
  _seats (polar._seats),
  _maxWater (polar._maxWater)
{}

Polar::~Polar()
{}

void Polar::setWater (int water, int bugs)
{
  _water = water;
  _bugs = bugs;

  // If empty weight equal gross weight we assume a added load of 90 Kg.

  double addedLoad = 0.0;
  double weight    = _emptyWeight;
  double A;

  if( _emptyWeight >= _grossWeight )
    {
      addedLoad = 0.0;
    }
  else
    {
      addedLoad = _grossWeight - _emptyWeight;
    }

  // qDebug( "Polar::setWater: water=%d, bugs=%d, addedLoad=%f", water, bugs, addedLoad );
  if( weight != 0.0 )
    {
      A = sqrt( (weight + addedLoad + _water) / weight );
    }
  else
    {
      A = 1.0;
    }

  double B = sqrt(1.0 + bugs/100.0);

  // qDebug ("a:%f, b:%f, c:%f", _aa, _bb, _cc);
  // Reichmann page 182
  _a = _aa / A;      // negative
  _b = _bb / B;      // positive
  _c = _cc * A * B;  // negative
  // we just increase the #sinking rate; this is not quite correct but gives reasonable results
}

/**
 * calculate sinking rate for given airspeed
 * we return a positive value for sinking !
 */
Speed Polar::getSink (const Speed& speed) const
{
  // this is the basic polar equation III, Reichmann page 181
  return -(speed*speed*_a + speed*_b + _c);
}

/**
 * calculate best airspeed for given wind, lift and Mc
 */
Speed Polar::bestSpeed (const Speed& wind, const Speed& lift, const Speed& mc) const
{
  // headwind counts negative
  // qDebug ("Polar::bestSpeed(%s)", _name.latin1());
  // qDebug ("bestSpeed: wind = %s, lift = %s", wind.getTextHorizontal().latin1(), lift.getTextVertical().latin1());

  // this is the polar equation transformed into a speed/lift coordinate system
  // the Reichmann equation V on Page 183 does not include wind. Mistake ?
  Speed speed;
  double temp = (wind * wind * _a - wind * _b + _c + lift - mc) / _a;

  if( temp >= 0.0 )
    {
      // now we go back into the original coordinate system
      speed = sqrt( temp ) - wind;
    }
  else
    {
      // qDebug ("inside polar, no valid result");
      // is this a reasonable approach ?
      speed = -wind;
    }
  // qDebug ("best speed: %s", speed.getTextHorizontal().latin1());
  return speed;
}

/**
  * calculate best glide ratio for given wind and lift;
  */
double Polar::bestLD (const Speed& airspeed,
                      const Speed& groundspeed,
                      const Speed& lift) const
{
  // calculate l/d over ground; here the mc value is not included
  // we calculate a positive value for l/d; this is not quite correct, but makes life easy
  double ld = groundspeed / (getSink(airspeed) - lift);
  // qDebug ("best ld: %f", ld);
  return ld;
}

/** draw a graphical polar on the given widget;
  * draw glide path according to lift, wind and McCready value
  */
void Polar::drawPolar (QWidget* view, const Speed& wind,
                       const Speed& lift, const Speed& mc) const
{
    QPainter p(view);
    p.translate (15, 30);

    QPen pen;
    pen.setWidth(3);

    // some initializations for the give units.
    // all drawing is done with meters/second units in both directions.
    // we use the Speed class to do the conversions.
    int minspeed, maxspeed, stepspeed;

    switch (Speed::getHorizontalUnit())
    {
      case Speed::metersPerSecond:

        if( (_v3.getKph() + 10) < 50 )
          {
            minspeed = 3;
            maxspeed = 21;
            stepspeed = 3;
            break;
          }

        if( (_v3.getKph() + 10)  < 100 )
          {
            minspeed = 5;
            maxspeed = 40;
            stepspeed = 5;
            break;
          }

          minspeed = 10;
          maxspeed = 70;
          stepspeed = 10;
          break;


      case Speed::kilometersPerHour:

        if( (_v3.getKph() + 10) < 50 )
          {
            minspeed = 10;
            maxspeed = 70;
            stepspeed = 10;
            break;
          }

        if( (_v3.getKph() + 10)  < 100 )
          {
            minspeed = 20;
            maxspeed = 140;
            stepspeed = 20;
            break;
          }

          minspeed = 20;
          maxspeed = 230;
          stepspeed = 30;
          break;

      case Speed::knots:

        if( (_v3.getKph() + 10) < 50 )
          {
            minspeed = 5;
            maxspeed = 35;
            stepspeed = 5;
            break;
          }

        if( (_v3.getKph() + 10)  < 100 )
          {
            minspeed = 10;
            maxspeed = 70;
            stepspeed = 10;
            break;
          }

          minspeed = 10;
          maxspeed = 130;
          stepspeed = 20;
          break;

      case Speed::milesPerHour:

        if( (_v3.getKph() + 10) < 50 )
          {
            minspeed = 5;
            maxspeed = 35;
            stepspeed = 5;
            break;
          }

        if( (_v3.getKph() + 10)  < 100 )
          {
            minspeed = 10;
            maxspeed = 70;
            stepspeed = 10;
            break;
          }

          minspeed = 10;
          maxspeed = 130;
          stepspeed = 20;
          break;

      default:

          minspeed = 0;
          maxspeed = 0;
          stepspeed = 0;
          break;
    }

    Speed sink;
    int minsink, maxsink, stepsink;

    switch (Speed::getVerticalUnit())
    {
      case Speed::metersPerSecond:
          minsink = 1;
          maxsink = 5;
          stepsink = 1;
          sink.setMps(maxsink);
          break;
      case Speed::kilometersPerHour:
          minsink = 5;
          maxsink = 20;
          stepsink = 5;
          sink.setKph(maxsink);
          break;
      case Speed::knots:
          minsink = 2;
          maxsink = 10;
          stepsink = 2;
          sink.setKnot(maxsink);
          break;
      case Speed::feetPerMinute:
          minsink = 200;
          maxsink = 1000;
          stepsink = 200;
          sink.setFpm(maxsink);
          break;
      default:
          minsink = 0;
          maxsink = 0;
          stepsink = 0;
          qWarning ("invalid vertical speed: %d", Speed::getVerticalUnit());
          break;
    }

    Speed speed;
    speed.setHorizontalValue (maxspeed);

    double X = double(view->width() - 30) / double(speed.getMps());
    double Y = double(view->height() - 30 - 80) / double(sink.getMps());

    QFont font = p.font();
    font.setPixelSize(14);
    p.setFont (font);
    // draw speed values and vertical grid lines
    pen.setColor(Qt::black);
    p.setPen( pen );
    p.drawLine (0, -30, 0, (int)(sink.getMps()*Y)); // Y axis

    for (int spd = minspeed; spd <= maxspeed; spd += stepspeed)
    {
      speed.setHorizontalValue (spd);

      pen.setColor(Qt::black);
      p.setPen (pen);
      p.drawLine ((int)(speed.getMps()*X), -5, (int)(speed.getMps()*X), 0);

      QString txt;

          if( (spd + stepspeed) <= maxspeed )
            {
              txt.sprintf("%3d %s", spd, Speed::getHorizontalUnitText().toLatin1().data() );
            }
          else
            {
              txt.sprintf("%3d", spd );
            }

      pen.setColor(Qt::blue);
      p.setPen (pen);
      p.drawText ((int)(speed.getMps()*X)-25, -7, txt);
      pen.setColor(Qt::darkGray);
      p.setPen (pen);
      p.drawLine ((int)(speed.getMps()*X), 1, (int)(speed.getMps()*X), (int)(sink.getMps()*Y));
    }

    // draw sink values and horizontal grid lines
    pen.setColor(Qt::black);
    p.setPen (pen);
    p.drawLine (-15, 0, (int)(speed.getMps()*X), 0);

    for (int snk = minsink; snk <= maxsink; snk += stepsink)
      {
        sink.setVerticalValue (snk);

        QString txt;
        txt.sprintf("%d %s", snk, Speed::getVerticalUnitText().toLatin1().data() );
        pen.setColor(Qt::black);
        p.setPen (pen);
        p.drawText ( 7, (int)(sink.getMps()*Y)-2, txt);
        pen.setColor(Qt::black);
        p.setPen (pen);
        p.drawLine (-5, (int)(sink.getMps()*Y), 0, (int)(sink.getMps()*Y));
        pen.setColor(Qt::darkGray);
        p.setPen (pen);
        p.drawLine (1, (int)(sink.getMps()*Y), (int)(speed.getMps()*X), (int)(sink.getMps()*Y));
    }

    // draw the actual polar; this is the simplest part :-))
    pen.setColor(Qt::red);
    p.setPen (pen);

    // Start and stop airspeed in m/s for polar drawing
    double start = 3.0;
    double stop  = 70.0;

    int lastX = (int)rint(start*X);
    int lastY = (int)rint(getSink (start)*Y);

    bool sinkCorrected = false;

    for (int spd = start; spd <= stop; spd++)
      {
        Speed sinkSpd (getSink(spd));
        int x = (int)rint(spd*X);
        int y = (int)rint(sinkSpd*Y);

        // Do not draw under the last horizontal grid line
        if ( sinkSpd.getMps() >= sink.getMps() )
          {
            sinkCorrected = true;
            lastX = x;
            lastY = y;
            continue;
          }

        if( sinkCorrected == true )
          {
            sinkCorrected = false;
            lastX = x;
            lastY = y;
            continue;
          }

        // stop drawing at the last vertical grid line
        if (spd > speed.getMps())
          {
            int dx = lastX - x;

            if( dx == 0 )
              {
                break;
              }

            int dy = lastY - y;
            int x0 = (int) (speed * X);
            int y0 = (int) rint((x0*dy - lastX*dy + lastY*dx) / dx);
            p.drawLine (lastX, lastY, x0, y0);
            break;
        }

        p.drawLine (lastX, lastY, x, y);
        lastX = x;
        lastY = y;
    }

    Speed bestspeed = bestSpeed (wind, lift, mc);
    double bestld = bestLD (bestspeed, bestspeed+wind, lift);

    font.setPixelSize(16);
    p.setFont(font);
    pen.setColor(Qt::blue);
    p.setPen (pen);
    // head wind counts negative;

    // draw cross at wind/lift origin
    p.drawLine ((int)(-wind*X)-3, (int)((lift-mc)*Y), (int)(-wind*X)+3, (int)((lift-mc)*Y));
    p.drawLine ((int)(-wind*X), (int)((lift-mc)*Y)-3, (int)(-wind*X), (int)((lift-mc)*Y)+3);
    // draw tangent of polar; this includes the mc value
    p.drawLine ((int)(-wind*X), (int)((lift-mc)*Y), (int)(bestspeed*X), (int)(getSink(bestspeed)*Y));
    // draw line of best l/d over ground; this does not include mc value !
    pen.setColor(Qt::green);
    p.setPen (pen);
    p.drawLine ((int)(-wind*X)-3, (int)(lift*Y), (int)(-wind*X)+3, (int)(lift*Y));
    p.drawLine ((int)(-wind*X), (int)(lift*Y)-3, (int)(-wind*X), (int)(lift*Y)+3);
    p.drawLine ((int)(-wind*X), (int)(lift*Y), (int)(bestspeed*X), (int)(getSink(bestspeed)*Y));
    // draw little circle at best speed point
    p.setBrush (Qt::red);
    pen.setColor(Qt::blue);
    p.setPen (pen);
    p.drawEllipse ((int)(bestspeed*X)-2, (int)(getSink(bestspeed)*Y)-2, 5, 5);

    int y = (int)(sink*Y)+5;
    QString msg;

#if 0
    if (Speed::getVerticalUnit() != Speed::metersPerSecond) {
        msg = QString(" = %1 m/s").arg( sink.getMps(), 0, 'f', 1 );
        p.drawText(0, y+=font.pixelSize()+2, sink.getVerticalText() + msg);
    }
#endif

    if( fabs( wind.getMps() ) > 0.01 && fabs( lift.getMps() ) > 0.01 )
      {
        p.drawText( 0, y += font.pixelSize() + 2, QObject::tr( "Wind: " )
            + wind.getHorizontalText() + ", " + QObject::tr( "Lift: " )
            + lift.getVerticalText() );
      }
    else if( fabs( wind.getMps() ) > 0.01 )
      {
        p.drawText( 0, y += font.pixelSize() + 2, QObject::tr( "Wind: " )
            + wind.getHorizontalText() );
      }
    else if( fabs( lift.getMps() ) > 0.01 )
      {
        p.drawText( 0, y += font.pixelSize() + 2, QObject::tr( "Lift: " )
            + lift.getVerticalText() );
      }

    msg = "";

    if ( _emptyWeight < _grossWeight )
      {
        msg += QString( QObject::tr("Added load: %1 Kg") ).arg((int)(_grossWeight-_emptyWeight));
      }

    if ( _water != 0 )
      {
        if( msg.size() > 0 )
          {
            msg += ", ";
          }

        msg += QString( QObject::tr("Water ballast: %1 l").arg(_water) );
      }

    if ( _bugs != 0 )
      {
        if( msg.size() > 0 )
          {
            msg += ", ";
          }

      msg += QString( QObject::tr("Bugs: %1 %").arg(_bugs) );

      }

    p.drawText (0, y+=font.pixelSize()+2, msg );

    msg = QString( QObject::tr("Best speed: %1, Sinking: %2") )
                  .arg( bestspeed.getHorizontalText() )
                  .arg( getSink(bestspeed).getVerticalText(true, 2) );

    p.drawText(0, y+=font.pixelSize()+2, msg);

    msg = QString(QObject::tr("Best L/D: %1")).arg( bestld, 0, 'f', 1 );

    if( _wingArea )
      {
        double wload = 0.0;

        if( _grossWeight > 0.0 )
          {
            wload = (_grossWeight + _water) / _wingArea;
          }
        else if( _emptyWeight )
          {
            wload = (_emptyWeight + _water) / _wingArea;
          }

        if( wload )
          {
            msg += ", " + QString(QObject::tr("Wing load:")) +
                   QString(" %1 Kg/m").arg( wload, 0, 'f', 1 ) +
                   QChar(Qt::Key_twosuperior);
          }
      }

    p.drawText(0, y+=font.pixelSize()+2, msg);

    y = (int)(sink*Y)+5;
    int x = view->width()/2;

    pen.setColor(Qt::black);
    p.setPen (pen);
    msg = QObject::tr("Use cursor right/left to simulate wind");
    p.drawText(x, y+=font.pixelSize()+2, msg);
    msg = QObject::tr("Use cursor up/down to set lift");
    p.drawText(x, y+=font.pixelSize()+2, msg);
    msg = QObject::tr("Use <Shift> up/down to adjust sinking");
    p.drawText(x, y+=font.pixelSize()+2, msg);
}
