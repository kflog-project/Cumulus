/***********************************************************************
**
**   vario.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cstdlib>

#include "vario.h"
#include "altitude.h"
#include "calculator.h"
#include "generalconfig.h"

Vario::Vario(QObject* parent) :
  QObject(parent),
  m_intTime(3000),
  m_TEKOn(false),
  m_energyAlt(0.0),
  m_TekAdjust(0.0),
  m_sampleList( LimitedList<AltSample>( 61 ) )
{
  GeneralConfig *conf = GeneralConfig::instance();

  m_intTime = conf->getVarioIntegrationTime() * 1000;
  m_TEKOn = conf->getVarioTekCompensation();
  m_TekAdjust = (100.0 + conf->getVarioTekAdjust()) / 100.0;

  // Timeout supervision of delivery of new altitude values
  connect( &m_timeOut, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
}

Vario::~Vario()
{
  m_timeOut.stop();
}

void Vario::newAltitude()
{
  // Start or restart the timer to supervise the calling of this
  // method. If the timer expires the variometer is set to zero.
  m_timeOut.setSingleShot( true );
  m_timeOut.start( m_intTime + 2500 );

  if( calculator->samplelist.count() < 10 )
    {
      // to less samples in the list
      return;
    }

  int i = 1; // index for list access
  int max = calculator->samplelist.count();
  double sum = 0.0;

  bool resultAvailable = false;

  // Step through the list. Note, the list is inverse ordered, last sample at
  // first position.
  QTime startTime = calculator->samplelist.at( 0 ).time;

  while( i < max )
    {
      double energyAlt1 = 0.0;
      double energyAlt2 = 0.0;
      const FlightSample *sample1 = &calculator->samplelist.at( i - 1 );
      const FlightSample *sample2 = &calculator->samplelist.at( i );

      // calculate energy altitude for both samples
      if( m_TEKOn )
        {
          double speed1 = sample1->airspeed.getMps();
          double speed2 = sample2->airspeed.getMps();

          if( (calculator->currentFlightMode() != Calculator::circlingL &&
               calculator->currentFlightMode() != Calculator::circlingR) ||
               speed1 == 0.0 || speed2 == 0.0 )
            {
              // If we do not circling or the calculated airspeed is zero
              // we do take the ground speed as basis.
              Vector v1 = sample1->vector;
              Vector v2 = sample1->vector;
              speed1 = v1.getSpeed().getMps();
              speed2 = v2.getSpeed().getMps();
            }

          energyAlt1  = (speed1 * speed1) / (2 * 9.81);
          energyAlt2  = (speed2 * speed2) / (2 * 9.81);
        }

      // if( i == 2 )
      // qDebug("Airspeed %f, EnergyAltitude %f, TekAdj %f",sample1->airspeed.getKph(), energyAlt1, _TekAdjust );

      int timeDist = sample2->time.msecsTo( startTime );

      if( timeDist > m_intTime )
        {
          // time difference to big
          break;
        }

      i++;

      double diff = (sample1->altitude.getMeters() + energyAlt1 * m_TekAdjust) -
                    (sample2->altitude.getMeters() + energyAlt2 * m_TekAdjust);

      int elapsed = sample2->time.msecsTo( sample1->time );

      sum += (1000.0 * diff / (double) elapsed);

      resultAvailable = true;

      // qDebug("Vario: max=%d, i=%d, diff=%f, elapsed=%dms, sum=%f",
      //	max, i, diff, elapsed, sum );
    }

  Speed lift;

  if( resultAvailable )
    {
      lift.setMps( sum / (i - 1) );
    }

  // qDebug ("New vario=%s, samples=%d", lift.getTextVertical(true, 3).latin1(), i );
  emit newVario( lift );
}

void Vario::newPressureAltitude( const Altitude& altitude, const Speed& tas )
{
  // static QTime zeit = QTime::currentTime();

  // qDebug() << "Alt=" << altitude.getMeters() << "ZeitSpanne=" << zeit.restart();

  // Start or restart the timer to supervise the calling of this
  // method. If the timer expires the variometer is set to zero.
  m_timeOut.setSingleShot( true );
  m_timeOut.start( 5000 );

  // That is the minimum altitude difference, which must be reached to
  // say the difference is acceptable. The unit is m/s.
  const double limit = 0.35;

  AltSample sample;

  sample.altitude  = altitude.getMeters();
  sample.tas       = tas.getMps();
  sample.timeStamp = QDateTime::currentMSecsSinceEpoch();

  // Add the new sample to the list.
  m_sampleList.add( sample );

  // qDebug() << "m_sampleList.count()=" << m_sampleList.count();

  if( m_sampleList.count() < 5 )
    {
      // Too less samples in the list.
      return;
    }

  int i = 1; // index for list access
  int max = m_sampleList.count();
  double lift = 0.0;

  bool resultAvailable = false;

  // Step through the list. Note, the list is inverse ordered, last sample at
  // first position.
  qint64 elapsedTime = 0;

  while( i < max )
    {
      double energyAlt1 = 0.0;
      double energyAlt2 = 0.0;

      const AltSample& sample1 = m_sampleList.at( i - 1 );
      const AltSample& sample2 = m_sampleList.at( i );

      double tas1 = sample1.tas;
      double tas2 = sample2.tas;

      qint64 timeDist = sample1.timeStamp - sample2.timeStamp;

      elapsedTime += timeDist;

      if( elapsedTime > m_intTime )
        {
          // The defined time period is reached, leave loop.
          break;
        }

      i++;
      double altDiff;

      if( m_TEKOn && tas1 > 0.0 && tas2 > 0.0 )
        {
          energyAlt1  = (tas1 * tas1) / (2 * 9.81) * m_TekAdjust;
          energyAlt2  = (tas2 * tas2) / (2 * 9.81) * m_TekAdjust;

          altDiff = (((sample1.altitude + energyAlt1) -
                      (sample2.altitude + energyAlt2)) / (double) timeDist) * 1000.0;
        }
      else
        {
          altDiff = ((sample1.altitude - sample2.altitude) / (double) timeDist) * 1000.0;
#if 0
          qDebug() << "i=" << (i-2)
                   << "timeDist" << timeDist << " ms"
                   << "A1=" << sample1.altitude
                   << "A2=" << sample2.altitude
                   << "AltDiff=" << altDiff;
#endif
        }

      if( fabs( altDiff ) > limit )
        {
          // If the altitude difference to low, we ignore that value in the
          // calculation. That is done to filter out noise values.
          lift += altDiff;
        }

      // qDebug() << "lastI=" << (i-1) << "lift=" << lift;

      resultAvailable = true;

      // qDebug( "Vario: max=%d, i=%d, diff=%f, elapsed=%lldms, lift=%f",
      //         max, i, (sample1.altitude - sample2.altitude), elapsedTime, lift );
    }

  Speed lifting;

  if( resultAvailable )
    {
      lifting.setMps( lift / (double) (i - 1) );
    }

  // qDebug ("New vario=%f, samples=%d", lifting.getMps(), i );
  emit newVario( lifting );
}

/** This slot is called by the internal timer, to signal a
    timeout. It resets the vario to initial. */
void Vario::slotTimeout()
{
  // Reset all to defaults, due to no new data have arrived over the
  // whole integration period and the measurement is senseless now.
  m_sampleList.clear();
  Speed lift;
  emit newVario( lift );
}

/** This slot is called, if the integration time has been changed.
    The new value is passed in seconds. */
void Vario::slotNewVarioTime( int newTime )
{
  // qDebug("Vario::slotNewTime=%d", newTime );
  m_intTime = newTime * 1000;
}

void Vario::slotNewTEKMode( bool newMode )
{
  // qDebug("Vario::slotNewTEKMode=%d", newMode );
  m_TEKOn = newMode;
}

void Vario::slotNewTEKAdjust(int adjust)
{
  // qDebug("Vario::slotNewTEKAdjust");
  m_TekAdjust = (double)((100.0 + adjust) / 100.0);
}
