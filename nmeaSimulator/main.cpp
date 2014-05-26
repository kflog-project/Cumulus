/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : 23.12.2003
    copyright            : (C) 2003 by Eckhard Völlm
                               2008 Axel Pauli portage to Qt 4.3
                               2009 Axel Pauli GSA extension
                               2010 Axel Pauli Pause option
                               2010 Axel Pauli Sentence extension
                               2010 Axel Pauli Reset language environment
                                               variables to prevent wrong floating
                                               point formatting.
                               2012 Axel Pauli Play option added
                               2013 Axel Pauli ttySx enabled as additional device

    email                : kflog.cumulus@gmail.com

    $Id$

    NMEA simulator for Cumulus.

    The simulator is part of the cumulus package and can be used for
    testing purposes. The output of the simulator is written to different
    devices, configurable with the device option.

    a) device=ttyS0,4800 writes data into a serial device
    b) device=/tmp/nmeasim writes data into a named pipe, default pipe name
       is /tmp/nmeasim.

    To read in the simulator data into cumulus, open the GPS configuration dialog
    in cumulus and select the appropriate device. Note, that the
    devices must be the same on both sides. Now cumulus is ready to
    receive the data from the NMEA simulator.

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <clocale>
#include <termios.h>

#include <QtCore>

#include "vector.h"
#include "glider.h"
#include "gpgsa.h"
#include "play.h"
#include "sentence.h"

using namespace std;

static    double lat=48.50;
static    double lon=9.4510;
static    float  speed=100.0;
static    QString direction="right";
static    float  heading=230.0;
static    float  wind=25.0;
static    float  winddir=270.0;   // wind is coming from west usually
static    float  radius=120.0;    // default circle radius
static    float  altitude=1000.0; // default altitude
static    float  climb=0.0;       // default climb rate zero
static    int    Time=3600;       // lets fly one hour per default
static    int    Pause=1000;      // default pause is 1000ms
static    bool   gotAltitude = false;
static    QString playFile;   // file to be played
static    int    skip=0;      // lines to be skipped in the file
static    QString confFile;   // configuration file name

static    QString sentences[10];

// The default pipe path for the simulator
#define DEV_PIPE "/tmp/nmeasim"

// Name of used default device, normally a pipe.
static QString device = DEV_PIPE;

// The device file descriptor to be used. Can be a tty or a pipe.
static int devFd = -1;

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

uint getBaudrate(int rate);

void safeConfig();

void closeAndExit(int /* signal */ )
{
  if( devFd != -1 )
    {
      close(devFd);
    }

  // safe current used parameters to file
  safeConfig();

  // if we do not unlink, we can start this prog again and again and
  // cumulus gets back the GPS fix. If we unlink, no chance to
  // reconnect without major changes maybe we should unlink the fifo
  // from gpsClient at exit if device like *nmeasim unlink(toCumulus);
  exit(0);
}

int init_io( void )
{
  // SIGPIPE is receicved if cumulus was terminated and we try
  // to write into the pipe without other endpoint
  signal(SIGPIPE, closeAndExit);

  // SIGINT is received at key press of Ctrl+C
  signal(SIGINT, closeAndExit);

  if( device.startsWith("tty") )
    {
      // A tty is used as device
      QList<QString> ttyParams = device.split(",");

      if( ttyParams.size() < 2 )
        {
          cerr << "Missing tty arguments! Expected are device=ttySx,<Speed>" << endl;
          exit( -1 );
        }

      bool ok;
      QString tty = "/dev/" + ttyParams.at(0);
      uint speed = getBaudrate( ttyParams.at(1).toInt(&ok) );

      if( ! ok )
        {
          cerr << "Wrong tty speed passed! Must be an integer value." << endl;
          exit( -1 );
        }

      devFd = open( tty.toLatin1().data(), O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK );

      if( devFd == -1 )
        {
          perror( "Error open Simulator output device:" );
          exit( -1 );
        }

      struct termios newtio;

      // http://www.mkssoftware.com/docs/man5/struct_termios.5.asp
      //
      // Prepare serial port settings for raw mode. That is important
      // otherwise Flarm binary communication do not work!
      //
      // - no canonical input (no line oriented input)
      // - 8 data bits
      // - no parity
      // - no interpretation of special characters
      // - no hardware control

      // Port control modes
      // CS8    8 bits per byte
      // CLOCAL Ignore modem status lines
      // CREAD  Enable receiver
      newtio.c_cflag = CS8 | CLOCAL | CREAD;

      // Port input modes
      // raw input without any special handling
      newtio.c_iflag = 0;

      // Port output modes
      // raw output without any special handling
      newtio.c_oflag = 0;

      // Port local modes
      // raw input/output without any special handling
      newtio.c_lflag = 0;

      // The values of the MIN and TIME members of the c_cc array of the termios
      // structure are used to determine how to process the bytes received.
      //
      // MIN represents the minimum number of bytes that should be received when
      // the read() function returns successfully.
      //
      // TIME is a timer of 0.1 second granularity (or as close to that value as
      // can be accommodated) that is used to time out bursty and short-term data
      // transmissions.
      newtio.c_cc[VMIN]  = 1;
      newtio.c_cc[VTIME] = 0;

      // AP: Note, the setting of the speed must be done at last
      // because the manipulation of the c_iflag and c_oflag can
      // destroy the already assigned values! Needed me several hours
      // to find out that. Setting the baud rate under c_cflag seems
      // also to work.
      cfsetispeed( &newtio, speed ); // set baud rate for input
      cfsetospeed( &newtio, speed ); // set baud rate for output
      tcflush(devFd, TCIOFLUSH);
      tcsetattr(devFd, TCSANOW, &newtio);
      fcntl(devFd, F_SETFL, FNDELAY); // NON blocking io is requested
    }
  else
    {
      // A pipe is used as device
      int ret = mkfifo(device.toLatin1(), S_IRUSR | S_IWUSR);

      // no output if file exists, see comment in closeAndExit
      if(ret && errno != EEXIST) perror("mkfifo");

      devFd = open(device.toLatin1(), O_WRONLY);

      if( devFd < 0 ) perror("open pipe");
    }

  return ( devFd );
}

void usleep( int t )
{
  struct timespec ts;
  ts.tv_sec  = t/1000000;
  ts.tv_nsec = (t%1000000)*1000;
  nanosleep (&ts, NULL);
}

void scanConfig( QString cfg )
{
  if( cfg.trimmed().isEmpty() )
    {
      return;
    }

  if( cfg.left(4) == "lat=" )
    {
      int deg,min,sec;
      char NS;
      float flat;

      if(cfg.contains(':') )
        {
          sscanf(cfg.toLatin1().data()+4,"%2d:%2d:%2d%c", &deg,&min,&sec, &NS );
          lat = (double)deg+(double)min/60.0+(double)sec/3600.0;

          if (NS=='S')
            lat = -lat;
          else if(NS!='N')
            cerr << "Invalid Latitude Coordinate N|S possible not: " << NS << endl;
        }
      else
        {
          sscanf(cfg.toLatin1().data()+4,"%f", &flat );
          lat = (double)flat;
        }
    }
  else if( cfg.left(4) == "lon=" )
    {
      int deg,min,sec;
      char EW;
      float flon;

      if(cfg.contains(':') )
        {
          sscanf(cfg.toLatin1().data()+4,"%3d:%2d:%2d%c", &deg,&min,&sec, &EW );
          lon = (double)deg+(double)min/60.0+(double)sec/3600.0;

          if (EW=='W')
            lon = -lon;
          else if(EW!='E')
            cerr << "Invalid Longitude Coordinate E|S possible not: " << EW << endl;
        }
      else
        {
          sscanf(cfg.toLatin1().data()+4,"%f", &flon );
          lon = (double)flon;
        }

    }
  else if( cfg.left(4) == "alt=" )
    {
      gotAltitude = true;
      sscanf(cfg.toLatin1().data()+4,"%f", &altitude );
    }
  else if( cfg.left(6) == "speed=" )
    {
      sscanf(cfg.toLatin1().data()+6,"%f", &speed );
    }
  else if( cfg.left(5) == "head=" )
    {
      sscanf(cfg.toLatin1().data()+5,"%f", &heading );
    }
  else if( cfg.left(5) == "wind=" )
    {
      sscanf(cfg.toLatin1().data()+5,"%f", &wind );
    }
  else if( cfg.left(7) == "radius=" )
    {
      sscanf(cfg.toLatin1().data()+7,"%f", &radius );
    }
  else if( cfg.left(6) == "climb=" )
    {
      sscanf(cfg.toLatin1().data()+6,"%f", &climb );
    }
  else if( cfg.left(5) == "time=" )
    {
      sscanf(cfg.toLatin1().data()+5,"%d", &Time );
    }
  else if( cfg.left(4) == "dir=" )
    {
      direction = cfg.mid(4).trimmed();
    }
  else if( cfg.left(8) == "winddir=" )
    {
      sscanf(cfg.toLatin1().data()+8,"%f", &winddir );
    }
  else if( cfg.left(7) == "device=" )
    {
      device = cfg.mid(7).trimmed();
    }
  else if( cfg.left(6) == "pause=" )
    {
      sscanf(cfg.toLatin1().data()+6,"%d", &Pause );
    }
  else if( cfg.startsWith("sentence") )
    {
      bool ok;
      int i = cfg.mid(8,1).toInt(&ok);

      if( ok )
        {
          sentences[i] = cfg.mid(10).trimmed();
        }
    }
  else if( cfg.left(5) == "file=" )
    {
      playFile = cfg.mid(5);
    }
  else if( cfg.left(5) == "skip=" )
    {
      bool ok;
      skip = cfg.mid(5).toInt(&ok);

      if( ! ok )
        {
          skip = 0;
        }
    }
  else
    {
      cerr << "Unknown parameter: '"
           << cfg.toLatin1().data()
           << "' ignored!" << endl;
    }
}

void safeConfig()
{
  FILE* file = fopen(confFile.toLatin1().data(), "w");

  fprintf(file,"lat=%f\n", (float)lat );
  fprintf(file,"lon=%f\n", (float)lon );
  fprintf(file,"speed=%f\n", (float)speed );
  fprintf(file,"head=%f\n", (float)heading );
  fprintf(file,"wind=%f\n", (float)wind );
  fprintf(file,"winddir=%f\n", (float)winddir );
  fprintf(file,"radius=%f\n", (float)radius );
  fprintf(file,"alt=%f\n", (float)altitude );
  fprintf(file,"climb=%f\n", (float)climb );
  fprintf(file,"time=%d\n", (int)Time );
  fprintf(file,"dir=%s\n", direction.toLatin1().data() );
  fprintf(file,"device=%s\n", device.toLatin1().data() );
  fprintf(file,"pause=%d\n", (int)Pause );
  fprintf(file,"file=%s\n", playFile.toLatin1().data() );
  fprintf(file,"skip=%d\n", skip );

  for( int i = 0; i < 10; i++ )
    {
      QString key = QString("sentence%1=").arg(i);

      if( ! sentences[i].isEmpty() )
        {
          key += sentences[i];
          fprintf( file,"%s\n", key.toLatin1().data() );
        }
    }

  fclose(file);
}

void readConfig()
{
  cout << "Configuration from persistent File: "
       << confFile.toLatin1().data() << endl;

  FILE* file;
  char line[256];
  file = fopen(confFile.toLatin1().data(), "r");

  if (file )
    {
      while( fgets(line, sizeof(line), file) )
        {
          cout << line ;
          QString l=line;

          l.trimmed();

          if( ! l.isEmpty())
            {
              scanConfig(l);
            }
        }

      fclose(file);
    }

  cout << "Configuration file read" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// M A I N
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
  QString mode;

  // The wind-vector must be reversed as given *from* where the wind comes
  QStringList Argv;

  for (int i = 1; i <= argc; i++)
    {
      Argv << argv[i];
    }

  if( argc < 2 )
    {
      char *prog = basename(argv[0]);

      cout << "NMEA GPS Simulator 1.5.0 for Cumulus, 2003-2008 E. Voellm, 2009-2013 A. Pauli (GPL)" << endl << endl
           << "Usage: " << prog << " str|cir|pos|gpos [params]" << endl << endl
           << "Parameters: str:  Straight Flight "<< endl
           << "            cir:  Circling "<< endl
           << "            pos:  Fixed Position e.g. standstill in a wave (climb works)"<< endl
           << "            gpos: Fixed Position on ground "<< endl
           << "            play: Plays a recorded NMEA file. GPRMC is required to be contained!" << endl
           << "            params:"<< endl
           << "              lat=dd:mm:ss[N|S]  or lat=dd.mmmm  Initial Latitude" << endl
           << "              lon=ddd:mm:ss[E|W] or lon=dd.mmmm  Initial Longitude" << endl
           << "              speed=[km/h] head=[deg]: Glider speed and heading" << endl
           << "              wind=[km/h]  winddir=[deg]: Wind force and direction" << endl
           << "              radius=[m]:  needed for circling" << endl
           << "              dir=[right|left]: Direction of Circle" << endl
           << "              alt=[m]: Altitude of Glider" << endl
           << "              climb=[m/s]: Climbrate" << endl
           << "              sentence0...9='NMEA sentence without checksum': $ and * can be omitted" << endl
           << "              time=[s]: duration of operation" << endl
           << "              pause=[ms]: pause between to send periods" << endl
           << "              device=[path to named pipe]: write into this pipe, default is /tmp/nmeasim" << endl
           << "              device=[ttySx,<speed>]: write to device ttySx with the given speed" << endl
           << "              file=[path to file]: to be played" << endl
           << "              skip=[number]: lines to be skipped in the play file" << endl
           << "            Note: all values can also be specified as float, like 110.5 " << endl << endl
           << "Example: " << prog << " str lat=48:31:48N lon=009:24:00E speed=125 winddir=270" << endl << endl
           << "NMEA output is written into named pipe '" << device.toLatin1().data() << "'." << endl
           << "Cumulus should use the same device name to get these data." << endl;

        exit (0);
    }

  mode = Argv[0];

  // @AP: Reset the locale that is used for number formatting to "C" locale.
  setlocale( LC_ALL, "" );
  setlocale( LC_NUMERIC, "C" );
  setenv( "LC_NUMERIC", "C", 1 );
  setenv( "LANG", "C", 1 );

  // First of all read command configuration from file.
  // Determine configuration file position. It is normally stored in the home
  // directory of the user.
  const char *home = getenv("HOME");

  if( home )
    {
      confFile = QString(home) + "/nmea.cfg";
    }
  else
    {
      confFile = "./nmea.cfg";
    }

  readConfig();

  // Override by things given at command line
  cout << "\nParameters from command line:" << endl;

  for (int i = 1; i < argc; i++)
    {
      cout <<  Argv[i].toLatin1().data() << endl;
      scanConfig( Argv[i] );
    }

  // calculate Wind as it was given
  cout << "Parameters for operation:" << endl;

  float winddirTrue = winddir+180;

  if ( winddirTrue > 360.0 )
    winddirTrue -= 360.0;

  if( mode == "str" )
    cout << "Mode:      Straight Flight  " << endl;
  if( mode == "cir" )
    cout << "Mode:      Circling  " << endl;
  if( mode == "gpos" )
    {
      cout << "Mode:      Fixed Ground Position  " << endl;
      if( !gotAltitude)
        altitude = 100.0;  // lower default Altitude
    }

  if( mode == "pos" )
    {
      cout << "Mode:      Fixed Position in Flight (Standstill in a wave) " << endl;
    }

  const int fifo = init_io();

  if( fifo < 0 )
    {
      return -1; // device error
    }

  if( mode == "play" )
    {
      cout << "Mode:      Play a recorded file" << endl;
      cout << "File:      " << playFile.toLatin1().data() << endl;
      cout << "Skip:      " << skip << endl;
      cout << "Pause:     " << Pause << " ms" << endl;
      cout << "Device:    " << device.toLatin1().data() << endl;

      Play play( playFile, fifo );

      play.startPlaying(skip, Pause);

      close( fifo );

      // safe current parameters to file
      safeConfig();
      return 0;
    }

  cout << "Latitude:  " << lat << endl;
  cout << "Longitude: " << lon << endl;
  cout << "Speed:     " << speed << " km/h" << endl;
  cout << "Heading:   " << heading << " deg" << endl;
  cout << "Wind:      " << wind << " km/h" << endl;
  cout << "Winddir:   " << winddir << " deg" << endl;
  cout << "Altitude:  " << altitude << " m" << endl;
  cout << "Radius:    " << radius << " m  (if circling)" << endl;
  cout << "Direction: " << direction.toLatin1().data() << " Turn" << endl;
  cout << "Climbrate: " << climb << " m/s" << endl;
  cout << "Time:      " << Time << " sec" << endl;
  cout << "Pause:     " << Pause << " ms" << endl;
  cout << "Device:    " << device.toLatin1().data() << endl;

  for( int i = 0; i < 10; i++ )
    {
      QString key = QString("sentence%1: ").arg(i);

      if( ! sentences[i].isEmpty() )
        {
          key += sentences[i];

          cout << key.toLatin1().data() << endl;
        }
    }

  cout << endl;

  glider myGl( lat, lon, speed, heading, wind, winddirTrue, altitude, climb );
  myGl.setFd( fifo );
  myGl.setCircle( radius, direction );

  // @AP: This is used for the GSA output simulation
  uint gsa = 0;
  QStringList satIds;
  satIds << "14" << "32" << "17" << "20" << "11" << "23" << "28" << "25" << "35";
  QString pdop = "1.7";
  QString hdop = "1.1";
  QString vdop = "1.2";

  QTime start = QTime::currentTime();

  if( Pause < 100 )
    {
      // Minimum is set to 100ms
      Pause = 100;
    }

  int nextTimeReporting = 0;

  while( start.elapsed() < Time*1000 )
    {
      gsa++;

      if( nextTimeReporting < start.elapsed() )
        {
          nextTimeReporting = start.elapsed() + 1000;
          cout << "Seconds remaining: " << Time - start.elapsed()/1000 << endl;
        }

      usleep( Pause * 1000 );

      if( mode == "str" )
        myGl.Straight();
      if( mode == "cir" )
        myGl.Circle();
      if( mode == "pos" )
        myGl.FixedPos();
      if( mode == "gpos" )
        myGl.FixedPosGround();

      // GSA output simulation
      if( ! (gsa % 40) )
        {
          satIds.clear();
          satIds << "14" << "32" << "17" << "20" << "11" << "23" << "28" << "25" << "35";
          pdop = "1.7";
          hdop = "1.1";
          vdop = "1.2";
        }
      else if( ! (gsa % 30) )
        {
          satIds.clear();
          satIds << "32" << "17" << "20" << "11" << "23" << "28" << "25" << "35";
          pdop = "1.5";
          hdop = "1.3";
          vdop = "1.7";
        }
      else if( ! (gsa % 20) )
        {
          satIds.clear();
          satIds << "10" << "15" << "33" << "17" << "19" << "11" << "23" << "28" << "25" << "36";
          pdop = "1.2";
          hdop = "1.4";
          satIds.clear();
          satIds << "14" << "32" << "17" << "20" << "11" << "23" << "28" << "25" << "35";
          pdop = "1.7";
          hdop = "1.1";
          vdop = "1.2";

          vdop = "1.3";
        }
      else if( ! (gsa % 10) )
        {
          satIds.clear();
          satIds << "14" << "32" << "17" << "20" << "11" << "23" << "28";
          pdop = "1.9";
          hdop = "1.3";
          vdop = "1.5";
        }

      GPGSA myGPGSA;
      myGPGSA.send( satIds, pdop, hdop, vdop, fifo );

      for( int i = 0; i < 10; i++ )
        {
          if( ! sentences[i].isEmpty() )
            {
              Sentence mySentence;
              mySentence.send( sentences[i], fifo );
            }
        }
    }

  // safe current parameters to file
  safeConfig();
  close( fifo );
  return 0;
}

/**
 * Translates the baud rate to a terminal speed definition.
 */
uint getBaudrate(int rate)
{
  switch (rate)
    {
    case 600:
      return B600;
    case 1200:
      return B1200;
    case 2400:
      return B2400;
    case 4800:
      return B4800;
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    case 230400:
      return B230400;
    default:
      cerr << "Warning: Baud rate " << rate << " is undefined, using 4800bps!" << endl;
      return B4800;
    }
}
