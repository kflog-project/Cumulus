/***********************************************************************
**
**   Athmosphere.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/*
 * File was partly taken over from:
 *
 * https://github.com/iltis42/OpenIVario/blob/master/main/Atmosphere.h
 *
 * Thanks to Eckhard Völlm ;-)
 *
 * Methods for Athmosphere Model used in Aviation
 *
 *
 */
#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include <cmath>

// With density of water from: http://www.csgnetwork.com/waterinformation.html
// @ 22.8 degree: 0.997585
// earth gravity: 9.0807 m/s^2
// and standard ICAO air density with 1.225 kg/m3 there is:
// V(km/h) = sqrt(2*( <mmH2O> * 0.997585 * 9.807  )/1.225) * 3.6

//   Speed
// 1 mmH2O = 9,80665 Pa
// mmH2O m/s	km/h
// 100	 40,0	143,9
// 110	 41,9	150,9
// 116	 43,0	155,0
// 120	 43,8	157,6
// 130	 45,6	164,0
// 140	 47,3	170,2

class Atmosphere {

	Atmosphere() {};
	~Atmosphere() {};

public:

	/**
	 * Calculate TAS from IAS in km/h, barometric pressure in hPa and temperature
	 * in degree Celsius.
	 */
	static double tas( double ias, double baro, double temp )
	{
		return( ias * sqrt( 1.225 / ( baro * 100.0 / (287.058 * (273.15 + temp)))));
	}

  /**
   * IAS in m/s from dynamic pressure in Pa.
   */
	static double pascal2mps( double pascal )
	{
		return sqrt( 2 * pascal / 1.225 );
	}

	/**
	 * Calculate altitude in meters from static and msl pressure.
	 *
	 * You can find the used formula 12 in that report here:
	 *
	 * https://wolkenschnueffler.de/media//DIR_62701/8850f06277167a7effff8044ffffffef.pdf
	 *
	 * Title Flugmeteorologie
	 *
	 * @param pressure Static presseure in hPa.
	 *
	 * @param mslPressure MSL pressure
	 *
	 * @return Altitude in meters.
	 */
	static double calcAltitude( double pressure, double mslPressure=1013.25 )
	{
	  if( pressure <= 0.0 || mslPressure <= 0.0 )
	    {
	      return -9999.0;
	    }

	  const double power = 287.05 * 0.0065 / 9.80665;

	  const double k1 = 288.15 / 0.0065;

	  return ( k1 * (1.0 - pow(pressure / mslPressure, power) ) );
	}

};

#endif
