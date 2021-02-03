/*
 * headingculation.cpp
 *
 *  Created on: 02.02.2021
 *
 *  Author: axel
 *
 *  https://www.mikrocontroller.net/topic/396914
 */

#include <cstdio>
#include <cmath>
#include <iostream>

#define COMPASS_VALUES 5
#define TEST_DIRECTION 359.9

int main()
{
  float compass[COMPASS_VALUES]= { 270, 271, 272, 269, 268.5 };
  int i;
  float d;
  float d_sum;
  float course;

  // calculate average course derivation
  course = compass[0];
  d_sum = 0;

  for (i=1; i<COMPASS_VALUES; i++)
  {
    d = compass[i] - course;
    if (d >  180.0) d -= 360.0;
    if (d < -180.0) d += 360.0;
    d_sum += d;
    printf( "Loop[%d]: %f-%f=%f d_sum=%f\n", i, compass[i], course, d, d_sum );
  }

  printf( "d_sum=%f\n", d_sum );

  d_sum  /= (COMPASS_VALUES);

  printf( "d_sum Average=%f\n", d_sum );

  course += d_sum;

  // normalize
  if (course >= 360.0) course -= 360.0;
  if (course <     0) course += 360.0;
  // output

  printf("Course: %f°\n", course);
}

