/***********************************************************************
 **
 **   CumulusActivity.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c):  2010-2012 by Josua Dietze
 **                   2012      by Axel Pauli
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
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.media.AsyncPlayer;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import org.kde.necessitas.origo.QtActivity;

import org.kflog.cumulus.BluetoothService;
import org.kflog.cumulus.R;

public class CumulusActivity extends QtActivity
{
  static final String                        TAG  = "Java#CumulusActivity";
  static final int               DIALOG_CLOSE_ID  = 0;
  static final int               DIALOG_MENU_ID   = 1;
  static final int               DIALOG_NO_SDCARD = 2;
  static final int               DIALOG_ZIP_ERR   = 3;
  static final int               DIALOG_GPS_ID    = 4;
  static final int               DIALOG_BT_ID     = 5;

  static final int               MENU_SETUP       = 0;
  static final int               MENU_PREFLIGHT   = 1;
  static final int               MENU_QUIT        = 2;

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
  private String                 bestProvider;
  private LocationListener       ll               = null;
  private GpsStatus.Listener     gl               = null;
  private GpsStatus.NmeaListener nl               = null;
  private boolean                gpsEnabled       = false;
  private boolean                nmeaIsReceived   = false;
  private boolean                locationUpdated  = false;
  private int                    lastGpsStatus    = -1;

  static private String          appDataPath      = "";
  static private String          addDataPath      = "";
  static private Object          m_objectRef      = null;
  static private Object          m_ActivityMutex  = new Object();

  // Used by the Bluetooth Service Handling
  private Set<BluetoothDevice>   m_pairedBtDevices = null;
  private String                 m_btMacArray[]    = null;
  private BluetoothService       m_btService       = null;

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
                         nativeGpsStatus(1);
                         break;

                       case BluetoothService.STATE_CONNECTING:
                         break;

                       case BluetoothService.STATE_LISTEN:
                         break;

                       case BluetoothService.STATE_NONE:
                         nativeGpsStatus(0);
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
															 getString(R.string.connected2) +
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

  // Native C++ functions
  public static native void nativeGpsFix( double latitude,
                                          double longitude,
                                          double altitude,
                                          float speed,
                                          float heading,
                                          float accu,
                                          long time );

  public static native void nativeNmeaString(String nmea);
  public static native void nativeGpsStatus(int status);
  public static native void keyboardAction(int action);
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

  @Override
  public void onCreate( Bundle savedInstanceState )
  {
    Log.d(TAG, "onCreate Entry" );

    synchronized (m_ActivityMutex)
      {
        m_objectRef = this;
      }

    super.onCreate( savedInstanceState );

    boolean dataFolderAvailable = false;

    String state = Environment.getExternalStorageState();

    String tmpString = "";

    if (! Environment.MEDIA_MOUNTED.equals(state))
      {
        Log.d(TAG, "Exiting, SdCard is not mounted!" );
        showDialog(DIALOG_NO_SDCARD);
        return;
      }

    // We can read and write the media
    dataFolderAvailable = true;

    File addDataDir = new File(Environment.getExternalStorageDirectory(), "Cumulus");

    tmpString = addDataDir.getAbsolutePath();

    if( ! addDataDir.exists() )
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

            if( res == false )
              {
                showDialog(DIALOG_ZIP_ERR);
                return;
              }
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
            showDialog(DIALOG_ZIP_ERR);
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

    Window w = getWindow();
    w.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);

    // Switch the screen permanently on
    w.setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

    apl = new AsyncPlayer("alarm_player");
    npl = new AsyncPlayer("notification_player");

    lm = (LocationManager) getSystemService(Context.LOCATION_SERVICE);

    if( lm == null )
      {
        Log.d(TAG, "System said: LOCATION_SERVICE is null!" );
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
                    nativeNmeaString(nmea);
                  }
              }
          };

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
                 nativeGpsFix( loc.getLatitude(), loc.getLongitude(),
                               loc.getAltitude(),
                               loc.getSpeed(), loc.getBearing(),
                               loc.getAccuracy(), loc.getTime());
               }
            }

            @Override
            public void onProviderDisabled(String provider)
            {
              Log.d(TAG, "onProviderDisabled: Provider=" + provider);

              if( provider == LocationManager.GPS_PROVIDER )
              {
                  if( lm != null )
                  {
                      lm.removeNmeaListener(nl);
                    }

                // GPS receiver is disconnected
                Log.d(TAG, "onProviderDisabled: GPS=False");
                nativeGpsStatus(0);
                gpsEnabled = false;
                nmeaIsReceived = false;
              }
            }

            @Override
            public void onProviderEnabled(String provider)
            {
              Log.d(TAG, "onProviderEnabled: Provider=" + provider);

              if( provider.equals(LocationManager.GPS_PROVIDER) )
                {
                  // GPS receiver is connected
                  Log.d(TAG, "onProviderEnabled: GPS=True");

                  nativeGpsStatus(1);
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
                          nativeGpsStatus(1);
                        }
                    }
                  else
                    {
                      if( lastGpsStatus != status )
                        {
                          // GPS receiver is disconnected
                          nativeGpsStatus(0);
                        }
                    }

                  // save the last reported status
                  lastGpsStatus = status;
                }
              }
          };

        lm.addNmeaListener(nl);
        lm.requestLocationUpdates( LocationManager.GPS_PROVIDER, 1000, 0, ll );
      }

    Log.d(TAG, "onCreate exit" );
  }

  @Override
  protected void onStart()
    {
      Log.d(TAG, "onStart()" );
      super.onStart();
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
  protected void onDestroy()
    {
      Log.d(TAG, "onDestroy" );

      if( lm != null )
        {
          lm.removeNmeaListener(nl);
          lm.removeUpdates(ll);
        }

      if( m_btService != null )
        {
          // terminate all BT threads
          m_btService.stop();
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
				// Send close key to QtApp. It can be used to close a widget or the whole
				// application.
				nativeKeypress((char) 28);
				return true;
			}

    if( keyCode == KeyEvent.KEYCODE_MENU )
      {
        if(isRootWindow() )
          {
            // Only the root window can show this dialog.
            showDialog(DIALOG_MENU_ID);
          }

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

  @Override
  public void onUserInteraction()
  {
    // Log.v(TAG, "onUserInteraction()");

    switchOffScreenDimming();
    super.onUserInteraction();
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
			CharSequence[] m_items = { getString(R.string.setupGeneral),
																 getString(R.string.setupPreFlight),
																 getString(R.string.gpsOn),
																 getString(R.string.gpsStatus),
																 getString(R.string.quit) };

			if (gpsEnabled)
			{
				m_items[2] = getString(R.string.gpsOff);
			}

			builder.setTitle(getString(R.string.mainMenu));
			builder.setItems(m_items, new DialogInterface.OnClickListener() {
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
						// Current window will get a quit
						nativeKeypress((char)28);
						break;
					}
				}
			});
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
					Log.v(TAG, "chooseBtGps(): " + device.getName() + "="
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
        nativeGpsStatus(0);
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

      if( lm.isProviderEnabled(LocationManager.GPS_PROVIDER) == false )
        {
          showGPSDisabledAlertToUser();
        }
      else
        {
          nativeGpsStatus(1);
          gpsEnabled = true;
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
				}
			else
				{
					createBtGpsDeviceDialog();
				}
		}

	private void createBtGpsDeviceDialog()
		{
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
    }

	protected void onActivityResult(int requestCode, int resultCode, Intent data)
		{
			if( requestCode == REQUEST_ENABLE_BT )
				{
					if( resultCode == RESULT_OK )
						{
							createBtGpsDeviceDialog();
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
            mHandler.post(new Runnable() {
              @Override
              public void run() {
                  dimmScreen( DIMM1_SCREEN_VALUE );
                }
              });
            return;
          }
      }
   } // End of inner Class ScreenDimmerTimerTask

} // End of Class
