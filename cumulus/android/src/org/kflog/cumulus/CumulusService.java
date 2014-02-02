/***********************************************************************
 **
 **   CumulusService.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c): 2014 by Axel Pauli <kflog.cumulus@gmail.com>
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

package org.kflog.cumulus;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

/**
 * @class CumulusService
 * 
 * @author Axel Pauli
 * 
 * @email <kflog.cumulus@gmail.com>
 * 
 * @date 2014
 * 
 * @version $Id$
 * 
 * @short A small server to prevent App killing
 * 
 * This class provides a small server to prevent the killing of the
 * CumulusActivity on low resources by the Android OS. The App records
 * different things, which should not be interrupted.
 * 
 */

public class CumulusService extends Service
{
  private static final String TAG = "CumService";
  
  // Debug enable flag
  static private final boolean D = true;
  
  @Override
  public int onStartCommand(Intent intent, int flags, int startId)
    {
      if( D ) Log.d(TAG, "onStartCommand()");
      
//      if( intent != null )
//        {
//          int request = intent.getExtras().getInt(REQUEST);
//          
//          if( request == R.id.request_install_add_data )
//            {
//              installAddData( intent.getExtras() );
//            }
//        }
      
      return Service.START_STICKY;
    }

  @Override
  public IBinder onBind(Intent intent)
    {
      return null;
    }
  
  @Override
  public void onCreate()
    {
      if( D ) Log.d(TAG, "onCreate()");
    }
  
  @Override
  public void onDestroy()
    {
      if( D ) Log.d(TAG, "onDestroy()");
    }
}
