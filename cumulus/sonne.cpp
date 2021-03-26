/************************************************************************
 **
 **   Copyright (c): 2007-2021 by Axel Pauli, kflog.cumulus@gmail.com
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 *************************************************************************
 **
 **   sonne.cpp
 **
 **   This class is part of Cumulus. It provides calculation of sun
 **   rise and sun set times. Algorithms was taken from a Swiss
 **   german webpage. Thanks to the author Roland Brodbeck for his
 **   publication. Explanations are all in german.
 **
 ***********************************************************************/
//
// Sonnenaufgangs- und untergangsberechnung nach
// http://lexikon.astronomie.info/zeitgleichung/neu.html
// Version Januar 2005
// von Dr. sc. nat. Roland Brodbeck, Diplom Physiker ETH Zürich
//
// Ergebnisse wurden hier geprüft: http://www.generalaviation.de/sunrise/index.shtml
//
// Achtung! Der Algorithmus weisst nur eine bedingte Genauigkeit im
// Minutenbereich auf. Besonders am Polarkreis kann es zu größeren
// Differenzen gegenüber einer besseren Berechnungsmethode
// kommen. Diese Zeiten sind nur Anhaltswerte und keine amtlich
// anerkannten! Für die Richtigkeit wird keinerlei Haftung übernommen.
//
//***********************************************************************

#include <cmath>

#include "sonne.h"
#include "time_cu.h"

// einige benötigte Konstanten
static const double pi2=6.283185307179586476925286766559;
static const double pi=3.1415926535897932384626433832795;
static const double RAD = 0.017453292519943295769236907684886;

// Gregorianischer Kalender
double Sonne::JulianischesDatum ( int Jahr, int Monat, int Tag,
                                  int Stunde, int Minuten, double Sekunden )
{
  if( Monat <= 2 )
    {
      Monat=Monat + 12;
      Jahr = Jahr -1;
    }

  int Gregor = (Jahr/400)-(Jahr/100)+(Jahr/4); // Gregorianischer Kalender

  return 2400000.5+365L*Jahr - 679004L + Gregor
    + int(30.6001*(Monat+1)) + Tag + Stunde/24.0
    + Minuten/1440.0 + Sekunden/86400.0;
}

double Sonne::InPi( double x )
{
  int n = (int)(x/pi2);
  x = x - n*pi2;

  if (x<0) x +=pi2;

  return x;
}

// Neigung der Erdachse
double Sonne::eps( const double T )
{
  return RAD*(23.43929111 + (-46.8150*T - 0.00059*T*T + 0.001813*T*T*T)/3600.0);
}

double Sonne::BerechneZeitgleichung( double &DK,double T )
{
  double RA_Mittel = 18.71506921 + 2400.0513369*T +(2.5862e-5 - 1.72e-9*T)*T*T;

  double M  = InPi(pi2 * (0.993133 + 99.997361*T));
  double L  = InPi(pi2 * (  0.7859453 + M/pi2 + (6893.0*sin(M)+72.0*sin(2.0*M)+6191.2*T) / 1296.0e3));
  double e = eps(T);
  double RA = atan(tan(L)*cos(e));

  if (RA<0) RA+=pi;

  if (L>pi) RA+=pi;

  RA = 24.0*RA/pi2;
  DK = asin(sin(e)*sin(L));

  // Damit 0 <= RA_Mittel < 24
  RA_Mittel = 24.0*InPi(pi2*RA_Mittel/24.0)/pi2;

  double dRA = RA_Mittel - RA;

  if (dRA < -12) dRA+=24;

  if (dRA > 12) dRA-=24;

  dRA = dRA* 1.0027379;

  return dRA ;
}

// Auf:	        Aufgangszeit in hh:mm
// Unter:	Untergangszeit in hh:mm
// Datum:       Tagesdatum
// Jahr:	4 stellig, z.B. 2007
// Monat:	1-12
// Tag:	        1-31
// Position:    x=Breite, y=Länge WGS84 Koordinaten im KFlog Format
// Zeitzone:	0=Weltzeit (UTC)
//              1=Winterzeit (MEZ)
//              2=Sommerzeit (MESZ)
//
//  Rückgabe true=OK, false=Fehler, im Fehlerfall sind Auf und Unter
//  Variablen nicht gesetzt
bool Sonne::sonneAufUnter( QString& Auf, QString& Unter,
                           QDate& Datum,
                           QPoint& Position,
                           QString& Zeitzone )
{
  static const double JD2000 = 2451545;

  if( Datum.isNull() || ! Datum.isValid() )
    {
      Datum = Datum.currentDate();
    }

  double JD = JulianischesDatum( Datum.year(), Datum.month(), Datum.day() ); // Stunde=12

  double T = (JD - JD2000)/36525.0;
  double DK;
  double h = -50.0/60.0*RAD;

  // Koordinaten muessen durch 600000 dividiert werden um sie wieder
  // in normale Gradzahlen zu konvertieren
  double B = Position.x() / 600000.0 * RAD; // geographische Breite
  double Laenge = Position.y() / 600000.0;  // geographische Laenge
  double Zeitgleichung = BerechneZeitgleichung(DK,T);
  double Zeitdifferenz = acos((sin(h) - sin(B)*sin(DK)) / (cos(B)*cos(DK)))/pi;

#ifdef ISNAN_WO_STD
  if( isnan(Zeitdifferenz) )
#else
  if( std::isnan(Zeitdifferenz) )
#endif
    {
      // qDebug("Sonne::sonneAufUnter(): ist keine Zahl NaN (not a number)");
      return false;
    }

  Zeitdifferenz *= 12.;

  double AufgangOrtszeit = 12 - Zeitdifferenz - Zeitgleichung;
  double UntergangOrtszeit = 12 + Zeitdifferenz - Zeitgleichung;

  double AufgangWeltzeit = AufgangOrtszeit - Laenge /15;
  double UntergangWeltzeit = UntergangOrtszeit - Laenge /15;

  /**
  qDebug( "Sonne::sonneAufUnter(): AufgangOrtszeit=%f, UntergangOrtszeit=%f",
           AufgangOrtszeit, UntergangOrtszeit );

  qDebug( "Sonne::sonneAufUnter(): AufgangWeltzeit=%f, UntergangWeltzeit=%f",
           AufgangWeltzeit, UntergangWeltzeit );
  **/

  if( AufgangWeltzeit < 0.0 )
    {
      // Richtung Osten kann die Weltzeit negativ werden. Das muss man korregieren.
      AufgangWeltzeit += 24.0;
    }

  // Berechne Auf- und Untergang in Sekunden
  int Aufgang = (int) rint(AufgangWeltzeit * 3600);
  int Untergang = (int) rint(UntergangWeltzeit * 3600);

  if( Aufgang % 60 > 30 ) Aufgang += 30;
  if( Untergang % 60 > 30 ) Untergang += 30;

  int hAuf = (Aufgang / 3600) % 24;
  int mAuf = (Aufgang % 3600) / 60;

  int hUnter = (Untergang / 3600) % 24;
  int mUnter = (Untergang % 3600) / 60;

  Auf = QString( "%1:%2" ).arg( hAuf, 2, 10, QChar('0') )
                          .arg( mAuf, 2, 10, QChar('0') );

  Unter = QString( "%1:%2" ).arg( hUnter, 2, 10, QChar('0') )
                            .arg( mUnter, 2, 10, QChar('0') );

  // Die Auf- und Untergangszeiten sind auf UTC bezogen
  if( Time::getTimeUnit() == Time::local )
    {
      // convert UTC times to local times
      QDateTime localSR( Datum, QTime::fromString( Auf, "hh:mm"), Qt::UTC );
      QDateTime localSS( Datum, QTime::fromString( Unter, "hh:mm"), Qt::UTC );

      // convert UTC to local time
      localSR = localSR.toLocalTime ();
      localSS = localSS.toLocalTime ();

      // convert time to string
      Auf = localSR.time().toString("hh:mm");
      Unter = localSS.time().toString("hh:mm");

      Zeitzone = ""; // die lokale Zeitzone ist ein Leertext
    }
  else
    {
      Zeitzone = "UTC";
    }

  return true;
}
