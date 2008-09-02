/***********************************************************************
 **
 **   hwinfo.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004 by Eckhard Völlm, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef CLASS_HWINFO
#define CLASS_HWINFO
#define PATH_PROC_CPUINFO "/proc/cpuinfo"
#define PATH_PROC_MEMINFO "/proc/meminfo"

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

/**
 * @short Returns information on the underlying hardware
 * @author Eckhard Vï¿½llm
 *
 * This class is used for all hardware depending-functions. It can
 * return things like the type of device, and other usefull hardware
 * information.
 */
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
      n810
    };

    /**
     * Destructor
     */
    ~HwInfo();

    /**
     * @returns the instance of the class, and creates an instance if there was none.
     */
    inline static HwInfo* instance( void )
    {
      if(!theInstance)
        theInstance = new HwInfo;
      return( theInstance );
    };

    /**
     * @returns the type of hardware device Cumulus is running on.
     */
    inline hwType getType( void )
    {
      return _hwType;
    };
    inline hwSubType getSubType( void )
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
     * @returns the rotation of the framebuffer:
     * 0    no rotation
     * 1    90 degrees rotation
     * 2    180 degrees rotation
     * 3    270 degrees rotation
     */
    const int getFBRotation()
    {
      return _fbRot;
    };

    /**
     * @returns the depth of the framebuffer in bits
     */
    const int getFBDepth()
    {
      return _fbDepth;
    };

  private:
    /**
     * Because this is a singleton, the constructor is private!
     */
    HwInfo();

    static HwInfo *theInstance;
    enum hwType _hwType;
    enum hwSubType _hwSubType;
    QString _hwString;
    int _fbRot;
    int _fbDepth;
  };

#define HWINFO HwInfo::instance()

#endif
