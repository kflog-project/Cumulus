/***********************************************************************
 **
 **   hwinfo.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004      by Eckhard Völlm
 **                   2008-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * @author Eckhard Völlm, Axel Pauli
 *
 * @short Returns information on the underlying hardware
 *
 * This class is used for all hardware depending functions. It can
 * return things like the type of a device and other useful hardware
 * information.
 *
 */

#ifndef CLASS_HWINFO
#define CLASS_HWINFO

#include <QString>

#define PATH_PROC_CPUINFO   "/proc/cpuinfo"
#define PATH_PROC_MEMINFO   "/proc/meminfo"
#define PATH_PROC_MOUNTINFO "/proc/mounts"

/* This number (0,5 MB in KB) is subtracted from the amount of free space
   on the heap, to allow for memory fragmentation. It may need to be
   adjusted, but first experiments show that this would be a reasonable
   factor. The free space on the heap can most often not be fully used,
   because one can't put large memory blocks in small holes. Continuous
   creating and deleting of objects leads to memory fragmentation, resulting
   in some free memory on the heap that in effect can't be put to use. */
#define HEAP_FRAGMENTATION_FACTOR 256

#define PATH_CF_INFO    "/var/lib/pcmcia/stab"

// Path under OpenZaurus
#define PATH_CF_INFO1   "/var/run/stab"

class HwInfo
  {
  public:
    enum hwType
    {
      unknown,
      desktop,
      nokia
    };

    enum hwSubType
    {
      other,
      n800,
      n810,
      n900
    };

    /**
     * Destructor
     */
    ~HwInfo();

    /**
     * @returns the instance of the class, and creates an instance if there was none.
     */
    static HwInfo* instance( void )
    {
      if(! theInstance)
        {
          theInstance = new HwInfo;
        }

      return( theInstance );
    };

    /**
     * @returns the type or vendor of hardware device Cumulus is running on.
     */
    hwType getType( void )
    {
      return _hwType;
    };

    /**
     * @returns the subtype of hardware device Cumulus is running on.
     */
    hwSubType getSubType( void )
    {
      return _hwSubType;
    };

    /**
     * @returns the hardware device string Cumulus is running on.
     */
    const QString getTypeString( void )
    {
      return _hwString;
    };

    /**
     * Reads the still usable memory (free + cache + buffers) from /proc/meminfo
     * @returns the usable memory in kB.
     */
    int getFreeMemory();

    /**
     * Reads /var/lib/pcmcia/stab to find out the device for the CF GPS
     */
    const QString getCfDevice( void );

    /**
     * @returns the rotation of the frame buffer:
     * 0    no rotation
     * 1    90 degrees rotation
     * 2    180 degrees rotation
     * 3    270 degrees rotation
     */
    int getFBRotation()
    {
      return _fbRot;
    };

    /**
     * @returns the depth of the frame buffer in bits
     */
    int getFBDepth()
    {
      return _fbDepth;
    };

    /**
     * Checks if an active mount does exist.
     *
     * @param mountPoint Path to be check for mounting.
     *
     * @returns true in case of success otherwise false.
     */
    static bool isMounted( const QString& mountPoint );

  private:
    /**
     * Because this is a singleton, the constructor is private!
     */
    HwInfo();

    /**
     * Because this is a singleton, don't allow copies and assignments.
     */
    HwInfo(const HwInfo& ){};
    HwInfo& operator=(const HwInfo& ){return *this;};

    static HwInfo *theInstance;
    enum hwType _hwType;
    enum hwSubType _hwSubType;
    QString _hwString;
    int _fbRot;
    int _fbDepth;
  };

#endif
