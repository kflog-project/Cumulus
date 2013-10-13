/***********************************************************************
 **
 **   CumulusActivity.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c):  2010-2012 by Josua Dietze
 **                   2012-2013 by Axel Pauli <kflog.cumulus@gmail.com>
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
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.Object;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

//import android.R;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.media.AsyncPlayer;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import org.kde.necessitas.origo.QtActivity;

import org.kflog.cumulus.BluetoothService;
import org.kflog.cumulus.R;

/**
 * @class CumulusActivity
 * 
 * @author Axel Pauli
 * 
 * @email <kflog.cumulus@gmail.com>
 * 
 * @date 2012-2013
 * 
 * @version $Id$
 * 
 * @short This class handles the Cumulus activity live cycle.
 * 
 * This class handles the Cumulus activity live cycle. It communicates via JNI with
 * the Qt application part. The Qt application part is a C++ GUI based on the Qt
 * SDK.
 *
 */
public class CumulusActivity extends QtActivity
{
  static final String                        TAG       = "Java#CumulusActivity";
  static final int               DIALOG_CLOSE_ID       = 0;
  static final int               DIALOG_MENU_ID        = 1;
  static final int               DIALOG_NO_SDCARD      = 2;
  static final int               DIALOG_ZIP_ERR        = 3;
  static final int               DIALOG_GPS_MENU_ID    = 4;
  static final int               DIALOG_GPS_ID         = 5;
  static final int               DIALOG_BT_ID          = 6;
  static final int               DIALOG_NO_DATA_FOLDER = 7;
  static final int               DIALOG_TOGGELS_ID     = 8;
  static final int               DIALOG_SETUP_ID       = 9;
  static final int               DIALOG_NO_PAIRED_BTD  = 10;

  static final int               REQUEST_ENABLE_BT = 99;

  // After this time and no user activity or movement the screen is dimmed.
  static final long              DIMM1_SCREEN_TO    = 120000;
  static final long              DIMM2_SCREEN_TO    = 300000;

  // Screen dimm values. Don't define a dimm value as zero. The application is
  // destroyed then observed on Motorola Defy.
  //
  // The help describes the brightness values in the following way:
  // This can be used to override the user's preferred brightness of the screen.
  // A value of less than 0, the default, means to use the preferred screen
  // brightness. 0 to 1 adjusts the brightness from dark to full bright.
  static final float             DIMM1_SCREEN_VALUE = 0.10f;
  static final float             DIMM2_SCREEN_VALUE = 0.01f;

  // current screen brightness value
  private float                  m_curScreenBrightness = -1.0f;

  // Flags used to handle screen dimming
  private boolean                m_currentDimmState   = false;
  private boolean                m_requestedDimmState = false;

  // System time of last user action in ms
  private long                   m_lastUserAction = 0;

  // Timer to handle screen dimming
  private Timer screenDimmTimer                   = null;

  private AsyncPlayer            apl, npl;
  private LocationManager        lm               = null;
  private LocationListener       ll               = null;
  private GpsStatus.Listener     gl               = null;
  private GpsStatus.NmeaListener nl               = null;
  private boolean                gpsEnabled       = false;
  private boolean                nmeaIsReceived   = false;
  private boolean                locationUpdated  = false;
  private int                    lastGpsStatus    = -1;
  private boolean                infoBoxesVisible = true;

  static private String          appDataPath      = "";
  static private String          addDataPath      = "";
  
  // Check flag for SD-Card mounted and accessible
  private boolean m_sdCardOk                      = false;
  
  // Check flag if Cumulus data folder on the SD-Card is accessible
  private boolean m_cumulusFolderOk               = false;

  // Set to true, if the data at /sdcard/Cumulus are available
  static private boolean         m_addDataInstalled = false;
  
  static private Object          m_objectRef      = null;
  static private Object          m_ActivityMutex  = new Object();
  
  // Information about the default display
  private DisplayMetrics displayMetrics            = new DisplayMetrics();
  
  private NotificationManager notificationManager  = null;

  // Used by the Bluetooth Service Handling
  private Set<BluetoothDevice>   m_pairedBtDevices = null;
  private String                 m_btMacArray[]    = null;
  private BluetoothService       m_btService       = null;
  
  // Used to signal, that we waiting to the on state of the Bluetooth adapter 
  private boolean m_wait4BluetoothAdapterOn = false;

  /**
   *  The Handler that gets information back from the BluetoothService
   */
  private final Handler m_btHandler =
    new Handler()
     {
       @Override
       public void handleMessage( Message msg )
         {
           switch (msg.what)
             {
               case BluetoothService.MESSAGE_STATE_CHANGE:

               Log.i( TAG, "MESSAGE_STATE_CHANGE: " + msg.arg1 );

               switch (msg.arg1)
                 {
                   case BluetoothService.STATE_CONNECTED:
                     reportGpsStatus(1);
                     break;

                   case BluetoothService.STATE_CONNECTING:
                     break;

                   case BluetoothService.STATE_LISTEN:
                     break;

                   case BluetoothService.STATE_NONE:
                     reportGpsStatus(0);
                     break;
                 }
               break;

						 case BluetoothService.MESSAGE_WRITE:
							 break;

						 case BluetoothService.MESSAGE_READ:
							 break;

						 case BluetoothService.MESSAGE_DEVICE_NAME:
							 // shows the connected device's name
							 Toast.makeText( getApplicationContext(),
															 getString(R.string.connected2) + " " +
															 msg.getData().getString(BluetoothService.DEVICE_NAME),
															 Toast.LENGTH_LONG )
									 .show();
							 break;

						 case BluetoothService.MESSAGE_TOAST:
							 Toast.makeText( getApplicationContext(),
															 msg.getData().getString(BluetoothService.TOAST),
															 Toast.LENGTH_LONG )
										.show();
							 break;
						 }
				 }
		 };
		 
	// Create a BroadcastReceiver for listen to Bluetooth intents.
  // See here: http://developer.android.com/guide/topics/connectivity/bluetooth.html
	private final BroadcastReceiver bcReceiver =
	  new BroadcastReceiver()
			{
				public void onReceive(Context context, Intent intent)
					{
						String action = intent.getAction();
						
						// Bluetooth discovery is finished
						if( BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action))
							{
								Log.d( TAG, "BluetoothAdapter.ACTION_DISCOVERY_FINISHED" );
							}
						else if( BluetoothAdapter.ACTION_STATE_CHANGED.equals(action))
							{
								int newState = intent.getIntExtra( BluetoothAdapter.EXTRA_STATE, -1 );
								
								Log.d( TAG, "BluetoothAdapter.ACTION_STATE_CHANGED to " + newState );
								
								if( newState == BluetoothAdapter.STATE_ON )
									{
										// Now the BT adapter is on and the BT device dialog can be shown.
										if( m_wait4BluetoothAdapterOn == true )
											{
												createBtGpsDeviceDialog();
											}
									}
							}
					}
			};

  // Native C++ functions
  public static native void nativeGpsFix( double latitude,
                                          double longitude,
                                          double altitude,
                                          float speed,
                                          float heading,
                                          float accu,
                                          long time );

  public static native void nativeByteFromGps(byte newByte);
  public static native void nativeNmeaString(String nmea);
  public static native void nativeGpsStatus(int status);
  public static native void nativeKeypress(char code);
  public static native boolean isRootWindow();

  /**
   * Called from JNI to get the class instance.
   */
  private static Object getObjectRef()
    {
      // Log.d(TAG, "getObjectRef is called" );
      synchronized (m_ActivityMutex)
        {
          return m_objectRef;
        }
    }
  
  /**
   * Called from JNI to get the add data install state.
   */
  private static boolean addDataInstalled()
    {
      synchronized(m_ActivityMutex)
        {
          return m_addDataInstalled;
        }
    }
  
  /**
   * Called from JNI to send a SMS to the returner. The SMS text consists of a
   * mobile number, followed by a separator semicolon and the SMS text body.
   * The mobile number can be omitted.
   * 
   * [<mobile-number>];<sms-text>
   * 
   * \param smsText Text to be sent as SMS to the returner.
   * 
   */
  void callReturner( String smsText )
    {
		  String number = "";
		  String text   = "";
		  
		  StringBuffer sb = new StringBuffer( smsText );
		  
		  int idx = sb.indexOf( ";" );
		  
		  if( idx != -1 )
		  	{
		  		if( idx == 0 )
		  			{
		  				// No mobile number is contained
		  				if( sb.length() > idx + 1 )
		  					{
		  						text = sb.substring( idx + 1 );
		  					}
		  			}
		  		else
		  			{
		  				// The SMS text contains a mobile number and a text body separated
		  				// separated by a semicolon.
		  				number = sb.substring( 0, idx );
		  				
		  				if( sb.length() > idx + 1 )
		  					{
		  						text = sb.substring( idx + 1 );
		  					}		  				
		  			}
		  	}
		  
		  // The SMS must be sent in the GUI activity thread. Therefore we post
		  // it to the SMS handler.
		  final String fnumber = number;
		  final String ftext   = text;
		  
		  runOnUiThread( new Runnable()
		  	{
	        @Override
	        public void run()
        	{
        	  // http://www.androidpit.de/de/android/wiki/view/SMS_senden_und_empfangen
        	  // WE call the default SMS App to send the SMS.
      		  Intent intent = new Intent(Intent.ACTION_VIEW);
      		  intent.setType("vnd.android-dir/mms-sms");
      		  
      		  if( fnumber.length() > 0 )
      		  	{
      		  		intent.putExtra("address", fnumber);
      		  	}
      		  
      		  intent.putExtra("sms_body", ftext);
      		  
              try
                {
                  // Call default SMS App
                  startActivity(intent);
                }
              catch (ActivityNotFoundException e)
                {
                  Log.e("SMS", "SMS activity unavailable: " + e.getMessage());
                  
                  Toast.makeText( getApplicationContext(),
                		          getString(R.string.noSmsService),
                                  Toast.LENGTH_LONG )
                       .show();
                }
	          }
		  	});
    }
  
  /**
   * Called from the QtActivity.onCreate to get information if the App is startable
   * or not. If not, QtActivity.onCreate returns without starting the native part.
   * 
   * @return The result of the pre-checks. True if all is ok otherwise false
   */
  @Override
  protected boolean checkPreconditions()
    {
      return( m_sdCardOk && m_cumulusFolderOk ) ;
    }

  @Override
  public void onCreate( Bundle savedInstanceState )
  {
    Log.d(TAG, "onCreate Entry" );
    Log.d(TAG, "CPU_ABI=" + Build.CPU_ABI);
    Log.d(TAG, "BRAND=" + Build.BRAND);
    Log.d(TAG, "PRODUCT=" + Build.PRODUCT);
    Log.d(TAG, "MANUFACTURER=" + Build.MANUFACTURER);
    Log.d(TAG, "HARDWARE="+ Build.HARDWARE);
    Log.d(TAG, "MODEL=" + Build.MODEL);
    Log.d(TAG, "DEVICE=" + Build.DEVICE);
    Log.d(TAG, "DISPLAY=" + Build.DISPLAY);
    Log.d(TAG, "BOARD=" + Build.BOARD);
    Log.d(TAG, "FINGERPRINT=" + Build.FINGERPRINT);
    Log.d(TAG, "ID=" + Build.ID);
    Log.d(TAG, "SERIAL=" + Build.SERIAL);
    // Log.d(TAG, "RADIO=" + Build.getRadioVersion());
    
    getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
    Log.d(TAG, "DISPLAY_METRICS=" + displayMetrics.toString());

    synchronized (m_ActivityMutex)
      {
        m_objectRef = this;
      }
    
		// Get the internal data directory for our App.
	  final String appDataDir = getDir("Cumulus", MODE_PRIVATE ).getAbsolutePath();

    // Check, if the internal data are already installed. To check that, a special
	  // file name is used, which is created after data installation. The file name
    // includes the package version, to ensure that with every new version the data
    // are reinstalled.
	  final String pvcFileName = "pvc_" + String.valueOf(getPackageVersionCode());
	  
    File pvcAppFile = new File( appDataDir + File.separator + pvcFileName );

    Log.d(TAG, "pvcApp=" + pvcAppFile.getAbsolutePath());

    if (! pvcAppFile.exists() )
      {
      	// We do the data install in an extra thread to minimize the runtime in our activity.
      	AppDataInstallThread adit = new AppDataInstallThread( appDataDir, pvcFileName );
      	adit.start();
      }
    else
    	{
        // The app data seem to be installed, make directory info for other threads available.
        synchronized(appDataPath)
          {
            appDataPath = appDataDir;
          }
    	}

    String ess = Environment.getExternalStorageState();

    // Cumulus user application data are installed at the SD-Card in the folder
    // Cumulus. So we need to check, if the SD-Card is mounted with read/write access
    // and if the Cumulus directory exists.
    if( Environment.MEDIA_MOUNTED.equals(ess) )
      {
        m_sdCardOk = true;
        
        // SD-Card is mounted, check, if the Cumulus directory exists
        File addDataDir = new File(Environment.getExternalStorageDirectory(), "Cumulus");
        
        if( addDataDir.exists() )
          {
           m_cumulusFolderOk = true;
          }
        else
          {
            m_cumulusFolderOk = addDataDir.mkdir();
          }
        
        if( m_cumulusFolderOk == true )
        {
          // Other treads waiting for this info
          synchronized( addDataPath )
            {
              addDataPath = addDataDir.getAbsolutePath();
            }

          // Check, if the external data are already installed. To check that, a special
          // file name is used, which is created after data installation. The file name
          // includes the package version, to ensure that with every new version the data
          // are reinstalled.
          File pvcAddFile = new File( addDataDir.getAbsolutePath() + File.separator + pvcFileName );

          Log.d(TAG, "pvcAdd=" + pvcAddFile.getAbsolutePath());
          
          boolean doInstall = ! pvcAddFile.exists();
          
          if( doInstall == false )
            {
              // We additionally check the existence of different files.
              // Maybe the user has removed them in the meantime.
              File helpFile   = new File( addDataDir.getAbsolutePath() + "/help/en/cumulus.html" );
              File deFile     = new File( addDataDir.getAbsolutePath() + "/locale/de/cumulus_de.qm" );
              File notifyFile = new File( addDataDir.getAbsolutePath() + "/sounds/Notify.wav" );
              File alarmFile  = new File( addDataDir.getAbsolutePath() + "/sounds/Alarm.wav" );
             
              if( ! helpFile.exists()   || ! deFile.exists() ||
                  ! notifyFile.exists() || ! alarmFile.exists() )
                {
                  // There are missing files, require a new installation.
                  doInstall = true;
                }
            }
          
          if( doInstall == true )
            {
              // Extract zip file from asset folder. That task is done in
              // an extra thread.
              AddDataInstallThread adit = new AddDataInstallThread(
                  addDataDir.getAbsolutePath(), pvcFileName);
              adit.start();
            }
          else
            {
              // The add data seem to be installed, make directory info for other threads available.              
              synchronized(m_ActivityMutex)
                {
                  m_addDataInstalled = true;
                }
            }
         }
      }
    
    // Call super class, that will load the native C++ part of Cumulus.
    super.onCreate( savedInstanceState );

    // Check the state of the mounted SD-Card
    if( ! m_sdCardOk )
      {
        Log.e(TAG, "Exiting, SdCard is not mounted or not writeable!" );
        showDialog(DIALOG_NO_SDCARD);
        return;
      }

    // Check the state of the Cumulus folder at the SD-Card
    if( ! m_cumulusFolderOk )
      {
        Log.e(TAG, "Cannot create folder Cumulus on the SD card!" );
        showDialog(DIALOG_NO_DATA_FOLDER);
        return;
      }

    Window w = getWindow();
    w.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);

    // Switch the screen permanently on
    w.setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

    apl = new AsyncPlayer("alarm_player");
    npl = new AsyncPlayer("notification_player");

    lm = (LocationManager) getSystemService(Context.LOCATION_SERVICE);

    if( lm == null )
      {
        Log.w(TAG, "System said: LOCATION_SERVICE is null!" );
      }
    else
    	{
        nl = new GpsStatus.NmeaListener()
          {
            public void onNmeaReceived( long timestamp, String nmea )
              {
                nmeaIsReceived = true;

                if( gpsEnabled )
                  {
                    // Forward GPS data only, if user has enabled that.
                    try
                      {
                        nativeNmeaString(nmea);
                      }
                    catch(UnsatisfiedLinkError e)
                      { 
                        // Ignore exception, if native part is not yet loaded.
                      }
                  }
              }
          };
          
        if( lm.addNmeaListener(nl) == false )
          {
            Log.e( TAG, "Cannot add NMEA listener to Location Manager!" );
            
            // reset NMEA listener
            nl = null;
          }
      }
    
    /* Add an icon to the notification area while Cumulus runs, to
       remind the user that we're sucking his battery empty. */
    notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
    Notification notification = new Notification( R.drawable.icon, null,
                                                  System.currentTimeMillis() );
    Context context = getApplicationContext();
    CharSequence contentTitle = "Cumulus";
    CharSequence contentText = getString(R.string.running);
    Intent notificationIntent = new Intent(this, CumulusActivity.class);
    PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);

    notification.setLatestEventInfo(context, contentTitle, contentText, contentIntent);
    notification.flags |= Notification.FLAG_ONGOING_EVENT;
    
    if( notificationManager != null )
    {
    	notificationManager.notify(1, notification);
    }
    
    // Register a BroadcastReceiver for BT intents
    IntentFilter filter = new IntentFilter( BluetoothAdapter.ACTION_DISCOVERY_FINISHED );
    filter.addAction( BluetoothAdapter.ACTION_STATE_CHANGED );
    
    // Don't forget to unregister during onDestroy
    registerReceiver( bcReceiver, filter );
    
    Log.d(TAG, "onCreate exit" );
  }

  @Override
  protected void onStart()
    {
      Log.d(TAG, "onStart()" );
      super.onStart();
    }

  @Override
  protected void onRestart()
    {
      Log.d(TAG, "onRestart()" );
      super.onRestart();
    }

  @Override
  protected void onResume()
    {
      Log.d(TAG, "onResume()" );
      super.onResume();

      // Switch user screen brightness on
      switchOffScreenDimming();

      if( screenDimmTimer == null )
        {
          // Log.d(TAG, "onResume(): new screenDimmTimer is activated" );
          screenDimmTimer = new Timer();
          TimerTask dimmTask = new ScreenDimmerTimerTask();
          screenDimmTimer.scheduleAtFixedRate(dimmTask, 10000, 10000);
        }
     }

  @Override
  protected void onPause()
    {
      Log.d(TAG, "onPause()" );
      super.onPause();

      screenDimmTimer.cancel();
      screenDimmTimer = null;
    }

  @Override
  protected void onStop()
    {
      Log.d(TAG, "onStop()" );
      super.onStop();
    }

  @Override
  protected void onDestroy()
    {
      Log.d(TAG, "onDestroy" );

      notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
      
      if( notificationManager != null )
        {
        	notificationManager.cancelAll();
        	Log.d(TAG, "onDestroy: Cancel all Notifs now" );
        }

      if( lm != null && nl != null )
        {
          lm.removeNmeaListener(nl);
        }

      if( lm != null & ll != null )
        {         
          lm.removeUpdates(ll);
        }

      if( m_btService != null )
        {
          // terminate all BT threads
          m_btService.stop();
        }
      
      // unregister BT receiver.
      unregisterReceiver( bcReceiver );
      
      // call super class
      super.onDestroy();

      if( isFinishing() == false )
        {
	    	  // Terminate the App, if the OS has called the onDestroy method
	    	  Log.d(TAG, "onDestroy: isFinishing() is false, calling exit!" );  	  
	    	  System.exit(0);
        }
    }

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
    Log.d(TAG, "onKeyDown, key pressed: " + event.toString());
	  
	  // There was a problem report on Google Developer, that a native function
	  // was not found. Maybe it was not yet loaded.
	  // java.lang.UnsatisfiedLinkError: isRootWindow
	  try
	    {
    		if( keyCode == KeyEvent.KEYCODE_BACK )
    			{
    				// Send close key to QtApp. It can be used to close a widget or the whole
    				// application.
    				nativeKeypress((char) 28);
    				return true;
    			}
    
        if( keyCode == KeyEvent.KEYCODE_MENU )
          {
            if( isRootWindow() )
              {
                // Only the root window can show this dialog.
                showDialog(DIALOG_MENU_ID);
              }
    
            return true;
          }
	    }
	  catch(UnsatisfiedLinkError e)
	    { 
	      // ignore exception and consume event.
	      return true;
	    }

    return super.onKeyDown(keyCode, event);
  }

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		Log.d(TAG, "onKeyUp, key pressed: " + event.toString());

		if( keyCode == KeyEvent.KEYCODE_BACK )
			{
				return true;
			}

    return super.onKeyUp(keyCode, event);
  }

  @Override
  public void onUserInteraction()
  {
    // Log.v(TAG, "onUserInteraction()");
    switchOffScreenDimming();
    super.onUserInteraction();
  }

  private LocationListener createLocationListener()
  {
    ll = new LocationListener()
      {
        @Override
        public void onLocationChanged(Location loc)
        {
          if( gpsEnabled == true && nmeaIsReceived == false )
          {
            // If the GPS device do not deliver raw NMEA sentences, we
            // forward these GPS data which is provided here. Otherwise
            // our application gets no GPS data.
            try
              {
                nativeGpsFix( loc.getLatitude(), loc.getLongitude(),
                              loc.getAltitude(),
                              loc.getSpeed(), loc.getBearing(),
                              loc.getAccuracy(), loc.getTime());
              }
            catch(UnsatisfiedLinkError e)
              { 
                // Ignore exception, if native part is not yet loaded.
              }
           }
        }

        @Override
        public void onProviderDisabled(String provider)
        {
          // User disables GPS in the Android status menu.
          Log.d(TAG, "onProviderDisabled: Provider=" + provider);

          if( provider == LocationManager.GPS_PROVIDER )
          {  
            // GPS receiver is disconnected
            Log.d(TAG, "onProviderDisabled: GPS=False");
            
            reportGpsStatus(0);
            gpsEnabled = false;
            nmeaIsReceived = false;
          }
        }

        @Override
        public void onProviderEnabled(String provider)
        {
          // User enables GPS in the Android status menu
          Log.d(TAG, "onProviderEnabled: Provider=" + provider);

          if( provider.equals(LocationManager.GPS_PROVIDER) )
            {
              // GPS receiver is connected
              Log.d(TAG, "onProviderEnabled: GPS=True");

              reportGpsStatus(1);
              gpsEnabled = true;
              nmeaIsReceived = false;
            }
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras)
          {
            // If the number of satellites changes, this method is always called.
            // Therefore we report only right status changes.
            Log.d(TAG, "onStatusChanged: Provider=" + provider +
                   ", Status=" + status);

            if( provider.equals(LocationManager.GPS_PROVIDER) && gpsEnabled )
            {
              if( status == LocationProvider.AVAILABLE )
                {
                  if( lastGpsStatus != status )
                    {
                      // GPS receiver is connected
                      reportGpsStatus(1);
                    }
                }
              else
                {
                  if( lastGpsStatus != status )
                    {
                      // GPS receiver is disconnected
                      reportGpsStatus(0);
                    }
                }

              // save the last reported status
              lastGpsStatus = status;
            }
          }
      };

    return ll;
  }
  
	protected Dialog onCreateDialog(int id)
	{
		AlertDialog alert;

		AlertDialog.Builder builder = new AlertDialog.Builder(this);

		switch(id)
		{
			case DIALOG_CLOSE_ID:
				builder.setMessage(getString(R.string.reallyClose))
							 .setCancelable(false)
							 .setPositiveButton( getString(android.R.string.yes), new DialogInterface.OnClickListener() {
																		public void onClick(DialogInterface dialog, int id) {
																								CumulusActivity.this.finish();
																		}
																	})
							 .setNegativeButton( getString(android.R.string.no), new DialogInterface.OnClickListener() {
																		public void onClick(DialogInterface dialog, int id) {
																			dialog.cancel();
																		}
																	});

        alert = builder.create();
        break;

			case DIALOG_MENU_ID:
				CharSequence[] m_items = { getString(R.string.setup),
																	 getString(R.string.gps),
																	 getString(R.string.toggles),
																	 getString(R.string.quit) };

				builder.setTitle(getString(R.string.mainMenu));
				builder.setItems( m_items, new DialogInterface.OnClickListener()
					{
    					public void onClick(DialogInterface dialog, int item) 
    						{
        						switch(item)
        						{
          						case 0:
          							showDialog( DIALOG_SETUP_ID );
          							break;
          						case 1:
          							showDialog( DIALOG_GPS_MENU_ID );
          							break;
          						case 2:
          							showDialog( DIALOG_TOGGELS_ID );
          							break;
          						case 3:
          							// Qt main window will get a quit
          							nativeKeypress((char)30);
          							break;
        						}
        					}
        		} );
				
				alert = builder.create();
				break;
       
			case DIALOG_SETUP_ID:
				CharSequence[] s_items = { getString(R.string.setupGeneral),
																	 getString(R.string.setupPreFlight) };
        
				builder.setTitle(getString(R.string.setupMenu));
				builder.setItems( s_items, new DialogInterface.OnClickListener()
					{
    					public void onClick(DialogInterface dialog, int item) 
    						{
        						switch(item)
        						{
          						case 0:
          							// open setup general dialog
          							nativeKeypress((char)25);
          							break;
          						case 1:
          							// open setup preflight dialog
          							nativeKeypress((char)26);
          							break;
         						}
        					}
        		} );
				
				alert = builder.create();
				break;
				
			case DIALOG_GPS_MENU_ID:
				CharSequence[] g_items = { getString(R.string.gpsOn),
						                       getString(R.string.gpsStatus) };
        
				if( gpsEnabled )
					{
						g_items[0] = getString(R.string.gpsOff);
					}

				builder.setTitle(getString(R.string.gpsMenu));
				builder.setItems( g_items, new DialogInterface.OnClickListener()
					{
    					public void onClick(DialogInterface dialog, int item) 
    						{
        						switch(item)
        						{
          						case 0:
          							toggleGps();
          							removeDialog( DIALOG_GPS_MENU_ID );
          							break;
          						case 1:
          							nativeKeypress((char)27);
          							break;
         						}
        					}
        		} );
				
				alert = builder.create();
				break;

			case DIALOG_TOGGELS_ID:
				CharSequence[] t_items = { "" };
			
				if( infoBoxesVisible )
					{
						t_items[0] = getString(R.string.mapInfoBarHide);
					}
				else
					{
						t_items[0] = getString(R.string.mapInfoBarShow);
					}
        
				builder.setTitle(getString(R.string.togglesMenu));
				builder.setItems( t_items, new DialogInterface.OnClickListener()
					{
    					public void onClick(DialogInterface dialog, int item) 
    						{
        						switch(item)
        						{
          						case 0:
          							nativeKeypress((char) 29);
          							infoBoxesVisible = ! infoBoxesVisible;
          							removeDialog( DIALOG_TOGGELS_ID );
          							break;
         						}
        					}
        		} );
				
				alert = builder.create();
				break;

		case DIALOG_NO_SDCARD:
			builder.setMessage( getString(R.string.sdcardNeeded) )
					.setCancelable(false)
					.setPositiveButton(getString(android.R.string.ok), new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int id) {
										 CumulusActivity.this.finish();
				}
			});
			alert = builder.create();
			break;
			
		case DIALOG_NO_DATA_FOLDER:
			builder.setMessage( getString(R.string.noDataFolder) )
					.setCancelable(false)
					.setPositiveButton(getString(android.R.string.ok), new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int id) {
										 CumulusActivity.this.finish();
				}
			});
			alert = builder.create();
			break;

		case DIALOG_ZIP_ERR:
			builder.setMessage( getString(R.string.errorUnzip) )
						 .setCancelable(false)
						 .setPositiveButton(getString(android.R.string.ok), new DialogInterface.OnClickListener()
							 {
								 public void onClick(DialogInterface dialog, int id)
									 {
										 CumulusActivity.this.finish();
									}
							 } );
			alert = builder.create();
			break;

		case DIALOG_GPS_ID:
			CharSequence[] l_gitems = {getString(R.string.gpsInternal), getString(R.string.gpsBluetooth)};

			builder.setTitle(getString(R.string.gpsMenu));
			builder.setItems(l_gitems, new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int item)
						{
							switch(item)
							{
								case 0:
									enableInternalGps( true );
									break;
								case 1:
									enableBtGps( true );
									break;
							}
						}
				});
			alert = builder.create();
			break;

		case DIALOG_BT_ID:

			if( m_pairedBtDevices == null || m_pairedBtDevices.size() == 0 )
				{
					return null;
				}

			builder.setTitle(getString(R.string.gpsBtMenu));

			CharSequence[] l_bt_items = new CharSequence[m_pairedBtDevices.size()];
			m_btMacArray = new String[m_pairedBtDevices.size()];

			int idx = 0;

			// Loop through paired devices
			for( BluetoothDevice device : m_pairedBtDevices )
				{
					Log.d(TAG, "chooseBtGps(): " + device.getName() + "="
								+ device.getAddress());

					l_bt_items[idx]   = device.getName() + " (" + device.getAddress() + ")";
					m_btMacArray[idx] = device.getAddress();
					idx++;
				}

			builder.setItems(l_bt_items, new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int item)
						{
							removeDialog(DIALOG_BT_ID);

              // Fetch MAC address of clicked item
              if( m_btMacArray != null )
                {
                  connect2BtDevice( m_btMacArray[item] );
                }
            }
        });
      alert = builder.create();
      break;
      
		case DIALOG_NO_PAIRED_BTD:
			builder.setMessage( getString(R.string.noPairedBtD) )
						 .setCancelable(false)
						 .setPositiveButton( getString(android.R.string.ok),
																 new DialogInterface.OnClickListener()
																	 {
												              public void onClick(DialogInterface dialog, int id)
												              	{
																		      CumulusActivity.this.finish();
												                }
											             });
	    alert = builder.create();
			break;

		default:
			return super.onCreateDialog(id);
		}

		return alert;
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

  /**
   * Forward a GPS NMEA command to the BT connected GPS device.
   */
  synchronized boolean gpsCmd( String cmd )
  {
  	Log.v(TAG, "gpsCmd(): " + cmd );
  	
	if( m_btService != null && gpsEnabled == true )
		{
			m_btService.write( cmd.getBytes() );
			return true;
		}
		
	return false;
  }
  
  /**
   * Forward a byte to the BT connected GPS device.
   */
  synchronized boolean byte2Gps( byte newByte )
  {
  	if( m_btService != null && gpsEnabled == true )
  	{
  	  byte[] data = new byte[1];
  	  data[0] = newByte;
  		m_btService.write( data );
  		return true;
  	}
	
	  return false;
  }

  synchronized private void dimmScreen( float value )
    {
      // Log.v(TAG, "dimmScreen(" + value  + ")");

      WindowManager.LayoutParams lp = getWindow().getAttributes();

      if( lp.screenBrightness != value )
        {
          lp.screenBrightness = value;
          getWindow().setAttributes( lp );
        }

      setCurScreenBrightness( value );
    }
  
  /**
   * This method is called by the native code to handle the screen dimming.
   *
   * @param newState true means dimm the screen, false means bright the screen
   */
  void dimmScreen( boolean newState )
    {
      // Log.v(TAG, "dimmScreen(" + newState  + ")");
  	  // save requested dimm state from C++ API side.
  	  setRequestedDimmState( newState );
  	}

  /**
   * This method returns the data directory of the App. Called via jni to pass
   * this info to the native C++ side.
   * 
   * @return The data directory of the App
   */
  String getAppDataDir()
    {
      synchronized(appDataPath)
        {
          return appDataPath;
        }
    }

  /**
   * This method returns the additional data directory of the App. Called via jni to
   *  pass this info to the native C++ side.
   * 
   * @return The additional data directory of the App
   */
  String getAddDataDir()
    {
        synchronized(addDataPath)
        {
          return addDataPath;
        }
    }
  
  String getLanguage()
    {
      Configuration config = getResources().getConfiguration();
      // returns language as e.g. "en_US" or de_DE
      return config.locale.getDefault().toString();
    }

  /**
   * Gets the display metrics from the Android system and return them as key value
   * string to the caller.
   * 
   * @return display metrics as key value string
   */
  String getDisplayMetrics()
    {
      getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
      
      StringBuffer buffer = new StringBuffer();
      
      buffer.append("density=").append(displayMetrics.density).append(';')
            .append("densityDpi=").append(displayMetrics.densityDpi).append(';')
            .append("heightPixels=").append(displayMetrics.heightPixels).append(';')
            .append("scaledDensity=").append(displayMetrics.scaledDensity).append(';')
            .append("widthPixels=").append(displayMetrics.widthPixels).append(';')
            .append("xdpi=").append(displayMetrics.xdpi).append(';')
            .append("ydpi=").append(displayMetrics.ydpi).append(';');
          
      return buffer.toString();
    }
  
  /**
   * Gets the build data from the Android system as key value string devided by
   * a pipe sign.
   * 
   * @return bild data as key value string
   */
  String getBildData()
	{
	  StringBuffer buffer = new StringBuffer();
	  
      buffer.append("CPU_ABI|").append(Build.CPU_ABI).append('|')
		    .append("BRAND|").append(Build.BRAND).append('|')
		    .append("PRODUCT|").append(Build.PRODUCT).append('|')
		    .append("MANUFACTURER|").append(Build.MANUFACTURER).append('|')
		    .append("HARDWARE|").append(Build.HARDWARE).append('|')
		    .append("MODEL|").append(Build.MODEL).append('|')
		    .append("DEVICE|").append(Build.DEVICE).append('|')
		    .append("DISPLAY|").append(Build.DISPLAY).append('|')
		    .append("FINGERPRINT|").append(Build.FINGERPRINT).append('|')
		    .append("ID|").append(Build.ID).append('|')
		    .append("SERIAL|").append(Build.SERIAL).append('|');
    
      return buffer.toString();
	}
  
  /**
   * Called from the native side to signal a shutdown. In that case all connections
   * at java side should be closed to avoid a calling of the native side.
   */
  synchronized void nativeShutdown()
    {
      Log.d(TAG, "nativeShutdown" );

      notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
      
      if( notificationManager != null )
        {
          notificationManager.cancelAll();
          Log.d(TAG, "nativeShutdown: Cancel all Notifs now" );
        }

      if( lm != null && nl != null )
        {
          lm.removeNmeaListener(nl);
          nl = null;
        }

      if( lm != null & ll != null )
        {         
          lm.removeUpdates(ll);
          ll = null;
        }
      
      if( lm != null )
        {
          lm = null;
        }

      if( m_btService != null )
        {
          // terminate all BT threads
          m_btService.stop();
          m_btService = null;
        }
    }

	private void toggleGps()
	{
		removeDialog(DIALOG_MENU_ID);

		BluetoothAdapter mBtAdapter = BluetoothAdapter.getDefaultAdapter();

		// As next ask which GPS device shall be used. Possibilities are the internal GPS
		// device or connected BT devices.
		if( lm == null || mBtAdapter == null )
			{
				// No location service available. Do nothing otherwise an exception is raised.
				Toast.makeText(this, getString(R.string.gpsDeviceNo), Toast.LENGTH_SHORT).show();
				gpsEnabled = false;
				return;
			}

    if( gpsEnabled == true )
      {
        Log.i(TAG, "Disable GPS");
        
        reportGpsStatus(0);
        gpsEnabled = false;

        // Lock, if Bt Service is used. It will be terminated then.
        if( m_btService != null )
          {
            m_btService.stop();
            m_btService = null;
          }
      }
    else
      {
        Log.i(TAG, "Enable GPS");

        if( lm != null && mBtAdapter != null )
          {
            showDialog(DIALOG_GPS_ID);
          }
        else if( lm != null )
          {
            enableInternalGps( false );
          }
        else if( mBtAdapter != null )
          {
            enableBtGps( false );
          }
      }
  }

	private void enableInternalGps( boolean clearDialog )
		{
			if( clearDialog )
				{
					removeDialog(DIALOG_GPS_ID);
				}
			
			if( lm != null )
			  {
			    if( ll == null )
			      {
              try
                {
                  ll = createLocationListener();
                  lm.requestLocationUpdates( LocationManager.GPS_PROVIDER, 1000, 0, ll );
                }
              catch( IllegalArgumentException e )
                {
                  // It seems there is no GPS provider available on this device.
                  Log.e( TAG, "Device has no GPS provider: " + e.getMessage() );
                  lm = null;
                  ll = null;
                  return;
                }
			      }

          if( lm.isProviderEnabled(LocationManager.GPS_PROVIDER) == false )
            {
              showGPSDisabledAlertToUser();
            }
          else
            {
              reportGpsStatus(1);
              gpsEnabled = true;
            }
			  }
    }

	private void enableBtGps( boolean clearDialog )
		{
			if( clearDialog )
				{
					removeDialog(DIALOG_GPS_ID);
				}

			if (! BluetoothAdapter.getDefaultAdapter().isEnabled())
				{
					Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
					startActivityForResult( enableBtIntent, REQUEST_ENABLE_BT );
				  m_wait4BluetoothAdapterOn = true;
				}
			else
				{
					createBtGpsDeviceDialog();
				}
		}

	private void createBtGpsDeviceDialog()
		{
			m_wait4BluetoothAdapterOn = false;
			
			m_btMacArray = null;
			m_pairedBtDevices = BluetoothAdapter.getDefaultAdapter().getBondedDevices();

			// If there are paired devices?
			if( m_pairedBtDevices.size() > 0 )
				{
					showDialog(DIALOG_BT_ID);
				}
			else
				{
					m_pairedBtDevices = null;
					showDialog(DIALOG_NO_PAIRED_BTD);
				}
		}

	private void connect2BtDevice( String macAddress )
		{
			Log.i( TAG, "connectBtDevice " + macAddress );

			if( m_btService == null )
				{
					m_btService = new BluetoothService( this, m_btHandler );
				}

			final BluetoothDevice device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(macAddress);

      /* Opens a connection to the selected BT device. */
      m_btService.connect( device );

      // set the GPS status to enabled
      gpsEnabled = true;
      
      if( lm != null && ll != null )
        {  
          // We switch off the data delivery from the internal GPS.
          lm.removeUpdates(ll);
          ll = null;
        }
    }

	protected void onActivityResult(int requestCode, int resultCode, Intent data)
		{
			Log.d( TAG, "onActivityResult ReqCode=" + requestCode + ", ResCode=" + resultCode );
			
			if( requestCode == REQUEST_ENABLE_BT )
				{
					// On single instance the result code is always NOK! Therefore we wait
					// until the adapter on state is broadcasted.
					if( resultCode == RESULT_OK )
						{
							Log.d( TAG, "BluetoothAdapter is switched on by the user" );
						}

					return;
				}

			super.onActivityResult(requestCode, resultCode, data);
		}

  private void showGPSDisabledAlertToUser()
    {
      AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);

      alertDialogBuilder
        .setMessage(getString(R.string.gpsQueryEnable))
        .setCancelable(false)
        .setPositiveButton( getString(R.string.gpsGoto),
                            new DialogInterface.OnClickListener()
                              {
                                public void onClick( DialogInterface dialog, int id )
                                  {
                                    Intent callGPSSettingIntent =	new Intent( android.provider.Settings.ACTION_LOCATION_SOURCE_SETTINGS );
                                    startActivity( callGPSSettingIntent );
                                  }
                              }
                          );

      alertDialogBuilder.setNegativeButton( getString(android.R.string.cancel),
                                            new DialogInterface.OnClickListener()
                                              {
                                                public void onClick(DialogInterface dialog, int id)
                                                  {
                                                    dialog.cancel();
                                                  }
                                              }
                                         );

      AlertDialog alert = alertDialogBuilder.create();
      alert.show();
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
	
	/**
	 * Wrapper around native call nativeGpsStatus to prevent an exception, if native
	 * C++ is not yet loaded.
	 * 
	 * @param status new status to be reported to native part.
	 */
	private void reportGpsStatus( int status )
		{
      try
      	{
      		// Call native C++ part to forward GPS status.
      		nativeGpsStatus( status );
      	}
  	  catch(UnsatisfiedLinkError e)
  	    { 
  	      // Ignore exception, if native part is not yet loaded.
  	    }
		}

	/**
	 * Removes all pvc_... files in the passed directory.
	 * 
	 * @param directoryName Name of directory
	 */
	private void removePvcFiles( String directoryName )
		{
			File directory = new File( directoryName );
			
			if( ! directory.exists() )
				{
					return;
				}

			// Get all files in directory
			File[] files = directory.listFiles();

			for (File file : files)
				{
					if( file.isFile() && file.getName().startsWith("pvc_") )
						{
    					if( ! file.delete() )
    						{
    							// Failed to delete file
    							Log.d( TAG, "removeDirContent: Failed to delete " + file );
    						}
						}
				}
		}

	private class AppDataInstallThread extends Thread
	{
		private String m_appDataDirName;
		private String m_pvcAppFileName;
		
    private Handler m_Handler;

		/**
		 * Constructor of app data install thread.
		 * 
		 * @param appDataDirName The full path name of the app data directory
		 * 
		 * @param pvcAppFileName The simple file name of the package version control file.
		 */
		AppDataInstallThread( String appDataDirName, String pvcAppFileName )
  		{
  			super( "AppDataInstall" );
  			
  			m_appDataDirName = appDataDirName;
  			m_pvcAppFileName = pvcAppFileName;
  			
  		  // Creates a handler in the calling thread to pass later back results to it
  			// from the running thread.
  			m_Handler = new Handler();
  		}
		
		public void run()
			{
			  Log.i( TAG, "AppData are installed at: " + m_appDataDirName );
				
	    	// At first remove all package version control files.
	    	removePvcFiles( m_appDataDirName );

	      boolean res = installAppData( m_appDataDirName, getAssets() );

	      if( res == false )
	        {
            m_Handler.post( new Runnable()
                            {
                              @Override
                              public void run()
                                {
                                  showDialog(DIALOG_ZIP_ERR);
                                }
                            } );              

	          return;
	        }

	      try
	        {
	  			  File pvcAppFile = new File( m_appDataDirName + File.separator + m_pvcAppFileName );

	          // Store an install marker file
	          OutputStream out = new FileOutputStream( pvcAppFile );
	          out.close();
	        }
	      catch (Exception e)
	        {
	          Log.e(TAG, "PVC app file error: " + e.getMessage());
	        }

	      Log.i( TAG, "AppData install finished.");

			  // another thread is waiting for this info
			  synchronized(appDataPath)
			  {
			    appDataPath = m_appDataDirName;
			  }
		  }
	}
	
  private boolean installAppData( String appDir, AssetManager am )
    {
      InputStream stream = null;

      String appDataFile = getString(R.string.appDataFile);

      try
        {
          stream = am.open(appDataFile);
        }
      catch (IOException e)
        {

          Log.e(TAG, "installAppData: File " + appDataFile + " not found!");
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

      if( !res )
        {
          Log.e(TAG, "installAppData failed!");
        }

      Log.d(TAG, "installAppData succeeded!");

      return res;
    }

  private class AddDataInstallThread extends Thread
    {
      private String m_addDataDirName;
      private String m_pvcAddFileName;
      
      private Handler m_Handler;
      
      /**
       * Constructor of add data install thread.
       * 
       * @param addDataDirName The full path name of the add data directory
       * 
       * @param pvcAddFileName The simple file name of the package version control file.
       */
      AddDataInstallThread( String addDataDirName, String pvcAddFileName )
        {
          super( "AddDataInstall" );
          
          m_addDataDirName = addDataDirName;
          m_pvcAddFileName = pvcAddFileName;
          
          // Creates a handler in the calling thread to pass later back results to it
          // from the running thread.
          m_Handler = new Handler();

        }
      
      public void run()
        {
          Log.i( TAG, "AddData are installed at: " + m_addDataDirName );
          
          // At first remove all package version control files.
          removePvcFiles( m_addDataDirName );
          
          // Extract zip file from asset folder
          boolean res = installAddData( m_addDataDirName, getAssets() );
    
          if( res == false )
            {
              m_Handler.post( new Runnable()
                              {
                                @Override
                                public void run()
                                  {
                                    showDialog(DIALOG_ZIP_ERR);
                                  }
                              } );              
              return;
            }
          
          try
            {
              File pvcAddFile = new File( m_addDataDirName + File.separator + m_pvcAddFileName );
    
              // Store an install marker file
              OutputStream out = new FileOutputStream( pvcAddFile );
              out.close();
            }
          catch (Exception e)
            {
              Log.e(TAG, "PVC add file error: " + e.getMessage());
            }        
          
          Log.i( TAG, "AddData install finished.");
    
          // another thread is waiting for this info
          synchronized(m_ActivityMutex)
            {
              m_addDataInstalled = true;
            }
        }
    }
  
  private boolean installAddData( String addDir, AssetManager am )
    {
      String addDataFile = getString(R.string.addDataFile);

      InputStream stream = null;

      try
        {
          stream = am.open(addDataFile);
        }
      catch (IOException e)
        {
          Log.e( TAG, "InstallAddData error: " + e.getMessage());
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

      if( !res )
        {
          Log.e( TAG, "InstallAddData failed!");
        }

      Log.d( TAG, "InstallAddData install succeeded!");

      return res;
    }

  public boolean unzip(InputStream stream, String destPath)
  {
    try
    {
      ZipInputStream zis = new ZipInputStream(new BufferedInputStream(stream));
      ZipEntry entry;

      // Read each entry from the ZipInputStream until no more entry is found
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

  private void switchOffScreenDimming()
    {
      // Switch user screen brightness on
      dimmScreen( -1.0f );
      setCurrentDimmState( false );
      setLastUserAction( System.currentTimeMillis() );
    }

  synchronized private boolean currentDimmState()
    {
      return m_currentDimmState;
    }

  synchronized private void setCurrentDimmState(boolean currentDimmState)
    {
      this.m_currentDimmState = currentDimmState;
    }

  synchronized private boolean requestedDimmState()
    {
      return m_requestedDimmState;
    }

  synchronized private void setRequestedDimmState(boolean requestedDimmState)
    {
      this.m_requestedDimmState = requestedDimmState;
    }

  synchronized private long lastUserAction()
    {
      return m_lastUserAction;
    }

  synchronized private void setLastUserAction(long lastUserAction)
    {
      this.m_lastUserAction = lastUserAction;
    }

  private synchronized float curScreenBrightness()
    {
      return m_curScreenBrightness;
    }

  private synchronized void setCurScreenBrightness(float screenBrightness)
    {
      this.m_curScreenBrightness = screenBrightness;
    }

  private class ScreenDimmerTimerTask extends TimerTask
  {
    private Handler mHandler;

    public ScreenDimmerTimerTask()
      {
        // Creates a handler in the calling thread in which the timer action is
        // later executed. Only the Activity thread can execute GUI actions!
        mHandler = new Handler();
      }

    @Override
    public void run()
      {
        // This thread uses the Handler of the creating thread to execute the
        // necessary GUI actions there.
        boolean rds = requestedDimmState();
        boolean cds = currentDimmState();
        long    lua = lastUserAction();

        // Log.v(TAG, "ScreenDimmerTimerTask: rds=" + rds + ", cds=" + cds + ", lus=" + (System.currentTimeMillis() - lua));

        if( cds == true && curScreenBrightness() == DIMM1_SCREEN_VALUE &&
            (System.currentTimeMillis() - lua) >= DIMM2_SCREEN_TO )
          {
            // Activate second dimm state of screen.
             mHandler.post(new Runnable() {
              @Override
              public void run() {
                  dimmScreen( DIMM2_SCREEN_VALUE );
                }
              });

            return;
          }

        if( rds == cds )
          {
            // No new dimm request, do nothing more.
            return;
          }

        if( rds == false )
          {
            setCurrentDimmState( rds );
            setLastUserAction( System.currentTimeMillis() );

            // Switch user screen brightness on
            mHandler.post(new Runnable() {
              @Override
              public void run() {
                  dimmScreen( -1.0f );
                }
              });
            return;
          }

        if( rds == true && (System.currentTimeMillis() - lua) >= DIMM1_SCREEN_TO )
          {
            setCurrentDimmState( rds );

            // Activate first dimm state of screen.
            mHandler.post(new Runnable()
                            {
                              @Override
                              public void run()
                                {
                                  dimmScreen(DIMM1_SCREEN_VALUE);
                                }
                            } );
            return;
          }
      }
   } // End of inner Class ScreenDimmerTimerTask

} // End of Class
