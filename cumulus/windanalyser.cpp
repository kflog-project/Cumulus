/***********************************************************************
 **
 **   windanalyser.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by Andre Somers
 **                   2009-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#include <QtGlobal>

#include "windanalyser.h"
#include "mapcalc.h"
#include "gpsnmea.h"
#include "generalconfig.h"

/*
  About Wind analysis

  Currently, the wind is analyzed by finding the minimum and the maximum
  ground speeds measured while flying a circle. The direction of the wind is taken
  to be the direction in which the speed reaches it's maximum value, the speed
  is half the difference between the maximum and the minimum speeds measured.
  A quality parameter, based on the number of circles already flown (the first
  circles are taken to be less accurate) and the angle between the headings at
  minimum and maximum speeds, is calculated in order to be able to weigh the
  resulting measurement.

  There are other options for determining the wind speed. You could for instance
  add all the vectors in a circle, and take the resulting vector as the wind speed.
  This is a more complex method, but because it is based on more heading/speed
  measurements by the GPS, it is probably more accurate. If equipped with
  instruments that pass along airspeed, the calculations can be compensated for
  changes in airspeed, resulting in better measurements. We are now assuming
  the pilot flies in perfect circles with constant airspeed, which is of course
  not a safe assumption.
  The quality indication we are calculation can also be approached differently,
  by calculating how constant the speed in the circle would be if corrected for
  the wind speed we just derived. The more constant, the better. This is again
  more CPU intensive, but may produce better results.

  Some of the errors made here will be averaged-out by the WindStore, which keeps
  a number of wind measurements and calculates a weighted average based on quality.
*/

WindAnalyser::WindAnalyser(QObject * parent) : QObject(parent)
{
  // Initialization
  active=false;
  circleLeft=false;
  circleCount=0;
  startmarker=0;
  circleDeg = 0;
  lastHeading = 0;
  pastHalfway=false;

  GeneralConfig *conf = GeneralConfig::instance();

  minSatCnt = conf->getWindMinSatCount();
  curModeOK=false;
}


WindAnalyser::~WindAnalyser()
{}


/** Called if a new sample is available in the sample list. */
void WindAnalyser::slot_newSample()
{
  if (!active)
    return; //only work if we are in active mode
  Vector curVec=calculator->samplelist.at(0).vector;
  bool fullCircle=false;
  // qDebug( "WindAnalyser" );
  //circle detection
  if( lastHeading ) {
    int diff= abs( curVec.getAngleDeg() - lastHeading );
    if( diff > 180 )
      diff = abs(diff - 360 );
    // qDebug("diff: %d",diff );
    circleDeg += diff;
  }
  lastHeading = curVec.getAngleDeg();
  // qDebug( "circling: %d",circleDeg  );

  if(circleDeg > 360 ) {
    //full circle made!
    fullCircle=true;
    circleDeg = 0;
    circleCount++;  //increase the number of circles flown (used to determine the quality)
  }

  if (fullCircle) { //we have completed a full circle!
    _calcWind();    //calculate the wind for this circle
    fullCircle=false;
    minVector=curVec.Clone();
    maxVector=curVec.Clone();
    //no need to reset fullCircle, it will automatically be reset in the next iteration.
  } else {
    // qDebug("curVec: %f/%f", (float)curVec.getKph(), (float)curVec.getAngleDeg() );
    if (curVec.getSpeed().getMps()<minVector.getSpeed().getMps())
      minVector=curVec.Clone();
    if (curVec.getSpeed().getMps()>maxVector.getSpeed().getMps())
      maxVector=curVec.Clone();
    // qDebug("minVec: %f/%d", (float)minVector.getKph(), (int)minVector.getAngleDeg() );
    // qDebug("maxVec: %f/%d", (float)maxVector.getKph(), (int)maxVector.getAngleDeg() );

  }
}


/** Called if the flight mode changes */
void WindAnalyser::slot_newFlightMode(Calculator::flightmode fm, int marker)
{
  active=false;  //we are inactive by default
  circleCount=0; //reset the circle counter for each flight mode change. The important thing to measure is the number of turns in this thermal only.
  circleDeg = 0;
  if (fm==Calculator::circlingL) {
    circleLeft=true;
  } else if (fm==Calculator::circlingR) {
    circleLeft=false;
  } else {
    curModeOK=false;
    return; //ok, so we are not circling. Exit function.
  }
  //remember that our current mode is ok.
  curModeOK=true;
  //do we have enough satellites in view?
  if (satCnt<minSatCnt)
    return;

  //initialize analyzer-parameters
  startmarker=marker;
  startheading=calculator->samplelist[0].vector.getAngleDeg();
  active=true;
  minVector=calculator->samplelist[0].vector.Clone();
  maxVector=minVector.Clone();
}


void WindAnalyser::_calcWind()
{
  int aDiff=angleDiff((int)minVector.getAngleDeg(),(int)maxVector.getAngleDeg());
  int quality;
  // qDebug(" _calcWind %d min:%d max:%d\n\n", aDiff, minVector.getAngleDeg(), maxVector.getAngleDeg());
  /*determine quality.
    Currently, we are using the question how well the min and the max vectors
    are on opposing sides of the circle to determine the quality. 140 degrees is
    the minimum separation, 180 is ideal.
    Furthermore, the first two circles are considered to be of lesser quality.
  */

  quality=5-((180-abs(aDiff))/8);
  if (circleCount<2)
    quality--;
  if (circleCount<1)
    quality--;
  // qDebug("quality %d",quality );

  if (quality<1)
    return;   //Measurement quality too low

  quality = qMax(quality,5);  //5 is maximum quality, make sure we honor that.

  //change to work with radials, that's faster because it is the internal format.
  int ang= maxVector.getAngleDeg();
  maxVector.setAngle( int(normalize(int(ang+180)) ));
  Vector a = maxVector.Clone();
  a.add( minVector );

  //take both directions for min and max vector into account
  // qDebug("maxAngle %d/%f minAngle %d/%f mid:%d/%f", maxVector.getAngleDeg(),maxVector.getKph(), minVector.getAngleDeg(),  minVector.getKph(),   a.getAngleDeg(), a.getKph() );

  //create a vector object for the resulting wind
  Vector result;
  //the direction of the wind is the direction where the greatest speed occurred
  result.setAngle(int(a.getAngleDeg()));
  //the speed of the wind is half the difference between the minimum and the maximum speeds.
  result.setSpeed(Speed((maxVector.getSpeed().getMps()-minVector.getSpeed().getMps())/2));

  //let the world know about our measurement!
  //qDebug("Wind: %d/%f\n", (int)result.getAngleDeg(),(float)result.getSpeed().getKph());
  emit newMeasurement(result,quality);
}

void WindAnalyser::slot_newConstellation( SatInfo& newConstellation )
{
  satCnt = newConstellation.satsInUse;

  if (active && (satCnt < minSatCnt))  //we are active, but the sat count drops below minimum
    {
      active=false;
      curModeOK=true;
      return;
    }

  if (!active && curModeOK && satCnt>=minSatCnt) { //we are not active because we had low sat count, but that has been rectified so we become active
    //initialize analyzer-parameters
    startmarker=calculator->samplelist[0].marker;
    startheading=calculator->samplelist[0].vector.getAngleDeg();
    active=true;
    minVector=calculator->samplelist[0].vector.Clone();
    maxVector=minVector.Clone();
  }
}
