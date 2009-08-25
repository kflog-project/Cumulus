/************************************************************************
 **
 **   Copyright (c): 2007-2009 by Axel Pauli, axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 *************************************************************************
 **
 **   sonne.h
 **
 **   This class is part of Cumulus. It provides calculation of sun
 **   rise and sun set times. Algorithmus was taken from a Swiss
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

#ifndef _sonne_h
#define _sonne_h

#include <QString>
#include <QPoint>
#include <QDateTime>

class Sonne
{
 public:

  // Gregorianischer Kalender
  static double JulianischesDatum ( int Jahr, int Monat, int Tag, 
                                    int Stunde=12, int Minuten=0, double Sekunden=0.0 );

  static double InPi( double x );

  // Neigung der Erdachse
  static double eps( const double T );

  static double BerechneZeitgleichung( double &DK, double T );

  // Auf:	Aufgangszeit in hh:mm
  // Unter:	Untergangszeit in hh:mm
  // Datum:     Tagesdatum
  // Position:  x=Breite, y=Länge WGS84 Koordinaten im KFlog Format
  // Zeitzone:	Zeitzone als Text, wenn nicht die lokale Zeit benutzt wird
  //
  //  Rückgabe true=OK, false=Fehler, im Fehlerfall sind Auf und Unter
  //  Variablen nicht gesetzt
  static bool sonneAufUnter( QString& Auf, QString& Unter,
                             QDate& Datum,
                             QPoint& Position,
                             QString& Zeitzone );

};

#endif
