/***********************************************************************
 **
 **   CumulusActivity.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c):  2010-2012 by Josua Dietze
 **                   2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

package org.kflog.cumulus;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.Object;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.media.AsyncPlayer;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.PowerManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.WindowManager;

import org.kde.necessitas.origo.QtActivity;
import org.kde.necessitas.origo.QtApplication;

public class CumulusActivity extends QtActivity
{
  static final int               DIALOG_CLOSE_ID  = 0;
  static final int               DIALOG_MENU_ID   = 1;
  static final int               DIALOG_CANCEL_ID = 2;
  static final int               MENU_SETUP       = 0;
  static final int               MENU_PREFLIGHT   = 1;
  static final int               MENU_QUIT        = 2;
  static final int               SOFTKEY_OPENED   = 1;
  static final int               SOFTKEY_CLOSED   = 0;

  private PowerManager.WakeLock  wl               = null;
  private AsyncPlayer            apl, npl;
  private LocationManager        lm;
  private String                 bestProvider;
  private LocationListener       ll               = null;
  private GpsStatus.Listener     gl               = null;
  private GpsStatus.NmeaListener nl               = null;
  private boolean                gpsEnabled       = false;
  private boolean                locationUpdated  = false;
  GPSReceiver                    receiver         = null;

  static private String          appDataPath      = "";
  static private String          addDataPath      = "";
  static private Object          objectRef        = null;

  public static native void nativeGpsFix( double latitude,
                                          double longitude,
                                          double altitude,
                                          float speed,
                                          float heading,
                                          float accu,
                                          long time );

  public static native void nativeNmeaString(String nmea);
  public static native void nativeGpsStatus(int status, int numsats);
  public static native void keyboardAction(int action);

  public static native void nativeKeypress(char code);
  public static native boolean isRootWindow();

  /**
   * Called via jni to get the class instance.
   */
  private static Object getObjectRef()
    {
      Log.d("Java#CumulusActivity", "getObjectRef is called" );
      return objectRef;
    }

  @Override
  public void onCreate(Bundle savedInstanceState)
	{
       Log.d("Java#CumulusActivity", "onCreate Entry" );

        objectRef = this;

        super.onCreate(savedInstanceState);

		boolean dataFolderAvailable = false;
		
		String state = Environment.getExternalStorageState();
		
		String tmpString = "";

        if (! Environment.MEDIA_MOUNTED.equals(state))
          {
            Log.d("Java#CumulusActivity", "Exiting, SdCard is not mounted!" );

            // TODO add an error text for the user
            showDialog(DIALOG_CANCEL_ID);
            return;
          }

        // We can read and write the media
        dataFolderAvailable = true;

        File addDataDir = new File(Environment.getExternalStorageDirectory(), "Cumulus");

        tmpString = addDataDir.getAbsolutePath();

        if ( ! addDataDir.exists() )
        {
          dataFolderAvailable = addDataDir.mkdir();
        }

        if( dataFolderAvailable )
        {
          // check existence of help files
          File helpFile   = new File( addDataDir.getAbsolutePath() + "/Cumulus/help/en/cumulus.html" );
          File notifyFile = new File( addDataDir.getAbsolutePath() + "/Cumulus/sounds/Notity.wav" );
          File alarmFile  = new File( addDataDir.getAbsolutePath() + "/Cumulus/sounds/Alarm.wav" );

          if( ! helpFile.exists() || ! notifyFile.exists() || ! alarmFile.exists() )
          {
            // It seems the some needed files are not installed, do it again.
            boolean res = installAddData( addDataDir.getAbsolutePath(), getAssets() );
          }
        }
    
    // another thread is waiting for this info
    synchronized(addDataPath)
    {
      addDataPath = tmpString;
    }

    // Get the internal data directory for our App.
	String appDataDir = getDir("Cumulus", MODE_PRIVATE ).getAbsolutePath();
		
	// Check, if the internal data are already installed
    int pvc =  getPackageVersionCode();
    
    File pvcFile = new File( appDataDir + "/pvc_" + String.valueOf(pvc) );
    
    Log.d("PVC", pvcFile.getAbsolutePath());
    
    if (! pvcFile.exists() )
    {
      // It seems that our App data are not installed. Do that job now.
      boolean res = installAppData( appDataDir, getAssets() );
      
      if( res == false )
      {
        // TODO add user error message
        showDialog(DIALOG_CANCEL_ID);      
        return;
      }
      
      try
      {
        // Set an install marker file
        OutputStream out = new FileOutputStream( pvcFile );
        out.close();
      }
      catch (Exception e)
      {
        Log.e("AssetNotice", e.getMessage()); 
      }
    }
	
    // another thread is waiting for this info
    synchronized(appDataPath)
    {
      appDataPath = appDataDir;
    }

    getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);

    PowerManager pm = (PowerManager)getSystemService(Context.POWER_SERVICE);
    wl = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK, "CumulusScreenAlwaysOn");
    wl.acquire();

    apl = new AsyncPlayer("alarm_player");
    npl = new AsyncPlayer("notification_player");

    receiver = new GPSReceiver();
    IntentFilter filter = new IntentFilter("android.location.GPS_ENABLED_CHANGE");
    filter.addAction("android.location.GPS_FIX_CHANGE");
    registerReceiver(receiver, filter);

//	Criteria criteria = new Criteria();
//	criteria.setAccuracy(Criteria.ACCURACY_FINE);
    lm = (LocationManager)getSystemService(Context.LOCATION_SERVICE);
		
    nl = new GpsStatus.NmeaListener()
    {
      public void onNmeaReceived(long timestamp, String nmea)
      {
        nativeNmeaString(nmea);
      }
    };

		lm.addNmeaListener(nl);

        Log.d("Java#CumulusActivity", "onCreate exit" );
  }


    @Override
    protected void onDestroy()
	{
      if( wl != null )
        {
          wl.release();
        }

      if( receiver != null )
        {
          unregisterReceiver(receiver);
        }

      // call super class
      super.onDestroy();
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
    // System.out.println("QtMain.onKeyDown, key pressed: "+event.toString());

    if( keyCode == KeyEvent.KEYCODE_BACK )
      {
         if(isRootWindow() )
           {
            // If the visible Qt window is the main view, BACK key may close the app.
            // Send a quit call to the QtApplication, to have a sure shutdown.
            // Otherwise ignore this key to prevent an unwanted shutdown.
            nativeKeypress((char) 28);
           }

          return true;
       }

    if( keyCode == KeyEvent.KEYCODE_MENU )
      {
        showDialog(DIALOG_MENU_ID);
        return true;
      }

    return super.onKeyDown(keyCode, event);
  }
	
  @Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
    // System.out.println("QtMain.onKeyUp, key released: "+event.toString());

    if( keyCode == KeyEvent.KEYCODE_BACK )
      {
        return true;
      }

    return super.onKeyUp(keyCode, event);
  }
	
  void playSound(int stream, String soundName)
    {
      Uri sf = Uri.parse("file://" + getAddDataDir() + File.separatorChar + "sounds" + File.separatorChar + soundName);
		
      if (stream == 0)
        {
          stream = AudioManager.STREAM_NOTIFICATION;
          npl.play(this.getApplicationContext(), sf, false, stream);
        }
      else if (stream == 1)
        {
          stream = AudioManager.STREAM_ALARM;
          apl.play(this.getApplicationContext(), sf, false, stream);
        }
     }

  String getAppDataDir()
	{
	  synchronized(appDataPath)
	    {
		    return appDataPath;
	    }
	}

  String getAddDataDir()
	{
	    synchronized(addDataPath)
	    {	  
	      return addDataPath;
	    }
	}

	protected AlertDialog onCreateDialog(int id) {
		AlertDialog alert;
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		switch(id) {
		case DIALOG_CLOSE_ID:
			builder.setMessage("Do you really want to close Cumulus?")
			.setCancelable(false)
			.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int id) {
                    CumulusActivity.this.finish();
				}
			})
			.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int id) {
					dialog.cancel();
				}
			});
			alert = builder.create();
			break;
			
		case DIALOG_MENU_ID:
			CharSequence[] items = {"General Setup", "Preflight Setup", "Enable GPS", "GPS Status", "Quit"};
			if (gpsEnabled)
				items[2] = "Disable GPS";
			
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Main Menu");
			builder.setItems(items, new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int item) {
					switch(item) {
					case 0:
						nativeKeypress((char)25);
						break;
					case 1:
						nativeKeypress((char)26);
						break;
					case 2:
						toggleGps();
						break;
					case 3:
						nativeKeypress((char)27);
						break;
					case 4:
                        // Send a quit call to the QtApplication, to make a sure shutdown.
                        if( isRootWindow() )
                          {
                            // Only root window will get a quit
                            nativeKeypress((char)28);
                          }
						break;
					}
				}
			});
			alert = builder.create();
			break;
			
		case DIALOG_CANCEL_ID:
			builder.setMessage("The Cumulus data directory on the storage card " +
					"seems to be missing.\nCheck if the storage is mounted or if you " +
					"have the minimum data package installed.\n\nFor more information " +
					"visit\ndraisberghof.de/android/cumulus\n\nThe program will now close.")
					.setCancelable(false)
					.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int id) {
                     CumulusActivity.this.finish();
				}
			});
			alert = builder.create();
			break;
			
		default:
			alert = null;
			break;
		}
		return alert;
	}
	
	private void toggleGps() {
		removeDialog(DIALOG_MENU_ID);
		if (gpsEnabled) {
			Log.i("Cumulus#Java", "disable LocationListener, set GPS status to OFF");
			lm.removeUpdates(ll);
			nativeGpsStatus(0, 0);
		} else {
			Log.i("Cumulus#Java", "activating LocationListener, set GPS status to NO_FIX");
			lm.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1, 1, ll);
			nativeGpsStatus(1, 0);
		}
		gpsEnabled = ! gpsEnabled;
	}
	
	/**
	 * Retrieves the package version code from the manifest.
	 * 
	 * @return The package version code
	 */
  private int getPackageVersionCode()
  {
    PackageInfo packageInfo;
    
    int version = -1;

    try
    {
      packageInfo = getPackageManager().getPackageInfo(getPackageName(), 0);
      
      version = packageInfo.versionCode;
    }
    catch (NameNotFoundException e)
    {
      Log.e("PVC", "Package not found: " + e.getMessage());
    }

    return version;
  }
  
  private boolean installAppData( String appDir, AssetManager am )
  {
    InputStream stream = null;

    String appDataFile = getString(R.string.appDataFile);

    try
    {
      stream = am.open( appDataFile );
    }
    catch (IOException e)
    {

      Log.e("AppDataDir", "File " + appDataFile + " not found!");
      return false;
    }

    boolean res = unzip(stream, appDir);

    try
    {
      stream.close();
    }
    catch (IOException e)
    {
    }

    if (!res)
    {
      Log.e("AppDataInstall", "Data install failed!");
    }
    
    Log.d("AppDataInstall", "AppData install succeeded!");

    return res;
  }
  
  private boolean installAddData( String addDir, AssetManager am )
  {
    String addDataFile = getString(R.string.addDataFile);

    InputStream stream = null;

    try
    {
      stream = am.open( addDataFile );
    }
    catch (IOException e)
    {
      Log.e("AddDataInstall", e.getMessage() );
      return false;
    }

    boolean res = unzip(stream, addDir);

    try
    {
      stream.close();
    }
    catch (IOException e)
    {
    }

    if (!res)
    {
      Log.e("AddDataInstall", "Data install failed!");
    }
    
    Log.d("AddDataInstall", "AddData install succeeded!");
    
    return res;
  }

  public boolean unzip(InputStream stream, String destPath)
  {
    try
    {
      ZipInputStream zis = new ZipInputStream(new BufferedInputStream(stream));
      ZipEntry entry;

      // Read each entry from the ZipInputStream until no more entry found
      // indicated by a null return value of the getNextEntry() method.
      while ((entry = zis.getNextEntry()) != null)
      {
        // System.out.println("Unzipping: " + entry.getName() + ", Directory: "
        // + entry.isDirectory() );
        String file = destPath + File.separator + entry.getName();

        if (entry.isDirectory())
        {
          File dir = new File(file);

          if (!dir.exists())
          {
            // System.out.println("Unzipping: Create Dir: " + destPath +
            // File.separator + entry.getName() );

            if (!dir.mkdir())
            {
              return false;
            }
          }

          continue;
        }

        int size;
        byte[] buffer = new byte[2048];

        FileOutputStream fos = null;

        try
        {
          fos = new FileOutputStream(file);
        }
        catch (FileNotFoundException e)
        {
          Log.e( "Unzip", "Error: " + e.getMessage() );
          return false;
        }

        BufferedOutputStream bos = new BufferedOutputStream(fos, buffer.length);

        while ((size = zis.read(buffer, 0, buffer.length)) != -1)
        {
          bos.write(buffer, 0, size);
        }

        bos.flush();
        bos.close();
      }

      zis.close();

    }
    catch (IOException e)
    {
      e.printStackTrace();
    }

    return true;
  }
	
} // End of Class
