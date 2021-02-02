/*
 * windcalculation.cpp
 *
 *  Created on: 02.02.2021
 *      Author: axel
 */

#include <cstdio>
#include <cmath>
#include <iostream>

void calWind( double tas, double gs, double th, double tc )
{
  double wca = (tc - th) * M_PI / 180.0;

  // Apply the Cosinus sentence: c^2 = a^2 + b^2 − 2 * a * b * cos( α )
  // to calculate the WS (wind speed)
  double wsq = (tas * tas) + (gs * gs ) - ( 2 * tas * gs * cos( wca ) );

  double ws = sqrt( wsq );

  // printf( "wsq=%f, ws=%f\n", wsq, ws );

  // WS / sin(WCA)
  double term = ws / sin( wca );

  // printf( "WS/sin(WCA)=%f\n", term );

  // calculate WA (wind angle) in °
  double wa = asin( tas / term ) * 180.0 / M_PI;

  // printf( "tas/term=%f, WA=%f\n", tas/term, wa );

  // Wind direction: W = TC- WA
  double wd = tc - wa;

  if( wd < 0.0 )
    {
      // correction of negative values.
      wd += 360.0;
    }

  printf( "\nTAS=%.0f, GS=%.0f, TH=%.0f, TC=%.0f --> WD=%f°, WS=%f km/h\n\n",
          tas, gs, th, tc, wd, ws );
}

int main()
{
  double tas, gs, tc, th;

  std::cout << "Enter TAS in km/h: ";
  std::cin >> tas;

  std::cout << "Enter GS in km/h: ";
  std::cin >> gs;

  std::cout << "Enter TH in °: ";
  std::cin >> th;

  std::cout << "Enter TC in °: ";
  std::cin >> tc;

  calWind( tas, gs, th, tc );
}

