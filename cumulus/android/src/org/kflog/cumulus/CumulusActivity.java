/***********************************************************************
 **
 **   CumulusActivity.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c):  2010-2012 by Josua Dietze
 **                   2012-2022 by Axel Pauli <kflog.cumulus@gmail.com>
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
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
import java.net.URL;
import java.net.URLConnection;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import org.kde.necessitas.origo.QtActivity;

import android.annotation.SuppressLint;
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
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
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

/**
 * @class CumulusActivity
 * 
 * @author Axel Pauli
 * 
 * @email <kflog.cumulus@gmail.com>
 * 
 * @date 2012-2024
 * 
 * @version 1.12
 * 
 * @short This class handles the Cumulus activity live cycle.
 * 
 * This class handles the Cumulus activity live cycle. It communicates
 * via JNI with the Qt application part. The Qt application part is a C++
 * GUI based on the Qt SDK.
 * 
 */
public class CumulusActivity extends QtActivity
{
  static final String TAG = "CumActivity";
  
  // Flag to store a restart of the App after a kill by the OS due to low on memory.
  private boolean m_isRestarted = false;
  
  // After this time and no user activity or movement the screen is dimmed.
  static final long DIMM1_SCREEN_TO = 120000;
  static final long DIMM2_SCREEN_TO = 300000;

  // Screen dimm values. Don't define a dimm value as zero. The application is
  // destroyed then observed on Motorola Defy.
  //
  // The help describes the brightness values in the following way:
  // This can be used to override the user's preferred brightness of the screen.
  // A value of less than 0, the default, means to use the preferred screen
  // brightness. 0 to 1 adjusts the brightness from dark to full bright.
  static final float DIMM1_SCREEN_VALUE = 0.10f;
  static final float DIMM2_SCREEN_VALUE = 0.01f;

  // current screen brightness value
  private float m_curScreenBrightness = -1.0f;

  // Flags used to handle screen dimming
  private boolean m_currentDimmState = false;
  private boolean m_requestedDimmState = false;
  
  // Flag to store state of native side.
  private boolean m_nativeIsUp = false;
  
  // This list stores the GPS menu items
  private List<String> gpsMenuItems = new ArrayList<String>();

  // System time of last user action in ms
  private long m_lastUserAction = 0;

  // Timer to handle screen dimming
  private Timer screenDimmTimer = null;

  private AsyncPlayer apl, npl;
  private LocationManager lm = null;
  private LocationListener ll = null;
  private GpsStatus.Listener gl = null;
  private GpsStatus.NmeaListener nl = null;
  static private boolean gpsEnabled = false;
  private boolean nmeaIsReceived = false;
  private int lastGpsStatus = -1;
  private boolean infoBoxesVisible = true;

  static private String appDataPath = "";
  static private String addDataPath = "";

  // Check flag for SD-Card mounted and accessible
  private boolean m_sdCardOk = false;

  // Check flag if Cumulus data folder on the SD-Card is accessible
  private boolean m_cumulusFolderOk = false;

  // Set to true, if the data at /sdcard/Cumulus are available
  static private boolean m_addDataInstalled = false;

  static private Object m_objectRef = null;
  static final private Object m_ActivityMutex = new Object();

  // Information about the default display
  private DisplayMetrics displayMetrics = new DisplayMetrics();

  private NotificationManager notificationManager = null;

  // Barometric sensor variables
  static private SensorManager m_SensorManager = null;
  static private Sensor m_BaroSensor = null;
  static private BaroSensorListener m_BaroSensorListener = null;

  // Used by IOIO service handling
  static private CumulusIOIO m_ioio = null;

  // Used by the Bluetooth Service Handling
  private Set<BluetoothDevice> m_pairedBtDevices = null;
  private String m_btMacArray[] = null;
  private BluetoothService m_btService = null;
  private String m_btMacAddress = null;

  // Used to signal, that we waiting to the on state of the Bluetooth adapter
  private boolean m_wait4BluetoothAdapterOn = false;
  
  @SuppressWarnings("rawtypes")
  protected static Class m_cumulusServiceClass = null;
  
  /**
   * GPS states used for reconnection after recreation.
   */
  static final byte GPS_DEVICE_NONE = 1;
  static final byte GPS_DEVICE_INTERNAL = 2;
  static final byte GPS_DEVICE_BT = 3;
  static final byte GPS_DEVICE_IOIO = 4;

  /**
   * That bundle contains a restored instance state or is null, if nothing
   * is restored.
   */
  private Bundle m_restoreInstanceState = null;
  
  /**
   * The Handler that gets information back from the BluetoothService
   */
  private final Handler m_btHandler = new Handler()
  {
    @Override
    public void handleMessage(Message msg)
    {
      switch (msg.what)
        {
          case BluetoothService.MESSAGE_STATE_CHANGE:

            Log.i(TAG, "MESSAGE_STATE_CHANGE: " + msg.arg1);

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
            Toast.makeText(
                getApplicationContext(),
                getString(R.string.connected2) + " "
                    + msg.getData().getString(BluetoothService.DEVICE_NAME),
                Toast.LENGTH_LONG).show();
            break;

          case BluetoothService.MESSAGE_TOAST:
            Toast.makeText(getApplicationContext(),
                msg.getData().getString(BluetoothService.TOAST),
                Toast.LENGTH_LONG).show();
            break;
        }
    }
  };

  // Create a BroadcastReceiver for listen to Bluetooth intents.
  // See here:
  // http://developer.android.com/guide/topics/connectivity/bluetooth.html
  private final BroadcastReceiver bcReceiver = new BroadcastReceiver()
  {
    @Override
    public void onReceive(Context context, Intent intent)
    {
      String action = intent.getAction();

      // Bluetooth discovery is finished
      if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action))
        {
          Log.d(TAG, "BluetoothAdapter.ACTION_DISCOVERY_FINISHED");
        }
      else if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action))
        {
          int newState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1);

          Log.d(TAG, "BluetoothAdapter.ACTION_STATE_CHANGED to " + newState);

          if (newState == BluetoothAdapter.STATE_ON)
            {
              // Now the BT adapter is on and the BT device dialog can be shown.
              if (m_wait4BluetoothAdapterOn == true)
                {
                  createBtGpsDeviceDialog();
                }
            }
        }
    }
  };

  /**
   * A Handler that gets information back from other objects.
   */
  @SuppressLint("HandlerLeak")
  private final Handler m_commHandler = new Handler()
    {
      @Override
      public void handleMessage(Message msg)
      {
        switch (msg.what)
          {
            case R.id.msg_ioio_incompatible:
              // Message from CumulusIOIO, the IOIO is incompatible.
              showDialog(R.id.dialog_ioio_incompatible, msg.getData());
              break;
              
            default:
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

  public static native void nativeByteFromGps(byte newByte);

  public static native void nativeNmeaString(String nmea);

  public static native void nativeBaroPressure(double pressure);

  public static native void nativeGpsStatus(int status);

  public static native void nativeKeypress(char code);

  public static native boolean isRootWindow();
  
  public static native void nativeHttpsResponse(int errorCode, String response, long cb);

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
    synchronized (m_ActivityMutex)
      {
        return m_addDataInstalled;
      }
  }

  /**
   * Called from JNI to open the hardware menu.
   */
  private void openHardwareMenu()
  {
    runOnUiThread(new Runnable()
    {
      @Override
      public void run()
      {
        if (m_BaroSensor != null)
          {
            showDialog(R.id.dialog_hw_id2);
          }
        else
          {
            showDialog(R.id.dialog_hw_id1);
          }
      }
    });
  }

  /**
   * Called from JNI to send a SMS to the retriever. The SMS text consists of a
   * mobile number, followed by a separator semicolon and the SMS text body. The
   * mobile number can be omitted.
   * 
   * [<mobile-number>];<sms-text>
   * 
   * \param smsText Text to be sent as SMS to the retriever.
   * 
   */
  void callRetriever(String smsText)
  {
    String number = "";
    String text = "";

    StringBuffer sb = new StringBuffer(smsText);

    int idx = sb.indexOf(";");

    if (idx != -1)
      {
        if (idx == 0)
          {
            // No mobile number is contained
            if (sb.length() > idx + 1)
              {
                text = sb.substring(idx + 1);
              }
          }
        else
          {
            // The SMS text contains a mobile number and a text body separated
            // separated by a semicolon.
            number = sb.substring(0, idx);

            if (sb.length() > idx + 1)
              {
                text = sb.substring(idx + 1);
              }
          }
      }

    // The SMS must be sent in the GUI activity thread. Therefore we send
    // it to the SMS handler.
    final String fnumber = number;
    final String ftext = text;

    runOnUiThread(new Runnable()
    {
      @Override
      public void run()
      {
        // http://www.androidpit.de/de/android/wiki/view/SMS_senden_und_empfangen
        // WE call the default SMS App to send the SMS.
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setType("vnd.android-dir/mms-sms");

        if (fnumber.length() > 0)
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

            Toast.makeText(getApplicationContext(),
                getString(R.string.noSmsService), Toast.LENGTH_LONG).show();
          }
      }
    });
  }
 
  /**
   * Called from JNI to request a HTTPS file download from the native side.
   * 
   * @param urlIn Url pointing to the source to be downloaded.
   * 
   * @param destinationIn Full file path, where the downloaded file shall be stored.
   * 
   * @param cbIn Callback from native side.
   */
  synchronized boolean downloadFile( final String urlIn,
                                     final String destinationIn,
                                     final long cbIn )
  {
    Log.i( TAG, "DownloadFile: Entry URL: " + urlIn + ", Dest: " + destinationIn );
    
    Runnable r = new Runnable()
      {
        public void run()
          {
        	  executeDownlaod( urlIn, destinationIn, cbIn );
          }
      };

    new Thread(r).start();
    return true;
  }
  
  void executeDownlaod( String urlIn, String destinationIn, long cbIn )
  {
    Log.i( TAG, "executeDownlaod: Entry URL: " + urlIn + ", Dest: " + destinationIn );
    
    // sleep 100ms to guarantee the return of the calling method.
    try
    {
      Thread.sleep( 100 );
    }
    
    catch (InterruptedException e1)
    {
      e1.printStackTrace();
      nativeHttpsResponse( -1,
                           e1.getMessage() + ", Cause: " + e1.getCause(),
                           cbIn );
      return;
    }
    
    BufferedInputStream in = null;
    FileOutputStream out = null;
    
    // Create a trust manager that does not validate certificate chains
    TrustManager[] trustAllCerts = new TrustManager[] { new X509TrustManager() {
        public java.security.cert.X509Certificate[] getAcceptedIssuers() {
            return null;
        }
        @Override
        public void checkClientTrusted(X509Certificate[] chain, String authType)
            throws CertificateException
        {
        }
        @Override
        public void checkServerTrusted(X509Certificate[] chain, String authType)
            throws CertificateException
        {
        }
      }
    };
    
    // Install the all-trusting trust manager
    SSLContext sc = null;
    
    try
    {
      sc = SSLContext.getInstance("SSL");
    }
    
    catch (NoSuchAlgorithmException e2)
    {
      e2.printStackTrace();
      Log.e( TAG, e2.getMessage() );
      nativeHttpsResponse( -1,
                           e2.getMessage() + ", Cause: " + e2.getCause(),
                           cbIn );
      return;
    }
    
    try
    {
      sc.init(null, trustAllCerts, new java.security.SecureRandom());
    } 
    
    catch (KeyManagementException e2)
    {
      e2.printStackTrace();
      Log.e( TAG, e2.getMessage() );
      nativeHttpsResponse( -1,
                           e2.getMessage() + ", Cause: " + e2.getCause(),
                           cbIn );
                           return;
    }
    
    HttpsURLConnection.setDefaultSSLSocketFactory( sc.getSocketFactory() );

    // Create all-trusting host name verifier
    @SuppressWarnings("unused")
    HostnameVerifier allHostsValid = new HostnameVerifier() {
        @Override
        public boolean verify(String hostname, SSLSession session)
        {
          return true;
        }
    };

    // Install the all-trusting host verifier
    // HttpsURLConnection.setDefaultHostnameVerifier(allHostsValid);
    
    try
    {
      URL url = new URL(urlIn);
      URLConnection con = url.openConnection();
      
      in = new BufferedInputStream( con.getInputStream() );
      out = new FileOutputStream( destinationIn );
      
      byte dataBuffer[] = new byte[1024];
      int bytesRead;
      
      while( (bytesRead = in.read(dataBuffer, 0, 1024)) != -1 )
        {
          out.write(dataBuffer, 0, bytesRead);
        }
      }
    
    catch (IOException e)
      {
        // handle exception
        Log.e( TAG, "Download error for " + urlIn +": \"" + e.getMessage() +
               "\", Code: \"" + e.getCause() + "\"" );
        
        if( e.getCause() == null )
        {
          // create an empty output file in case of null cause. It is received,
          // when the download file does not exist.
          try
            {
              out = new FileOutputStream( destinationIn );
              out.close();
              Log.e( TAG, "Creating empty download file for null cause." );
            }
          
          catch (IOException e1) {};
          nativeHttpsResponse( 0, urlIn, cbIn );
          return;
        }
        
        // That should be a common error.
        nativeHttpsResponse( -1,
        		                 e.getMessage() + ", Cause: " + e.getCause(),
        		                 cbIn );
        return;
      }
    
    finally
    {
      if( in != null )
        {
          try { in.close(); } catch (IOException e) {};
        }
      
      if( out != null )
        {
          try { out.close(); } catch (IOException e) {};
        }
    }
    
    nativeHttpsResponse( 0, urlIn, cbIn );
    Log.i( TAG, "Download Ok for " + urlIn );
  }

  /**
   * Setup a barometer sensor listener, if a barometer sensor is build in.
   * 
   * @return true in case of success otherwise false
   */
  private boolean activateBarometerSensor()
  {
    if (m_BaroSensor == null)
      {
        return false;
      }

    if (m_BaroSensorListener == null)
      {
        m_BaroSensorListener = new BaroSensorListener();
      }

    m_SensorManager.registerListener(m_BaroSensorListener, m_BaroSensor,
                                     SensorManager.SENSOR_DELAY_NORMAL);
    return true;
  }

  /**
   * Remove a barometer sensor listener, if it is active.
   * 
   * @return true in case of success otherwise false
   */
  private boolean deactivateBarometerSensor()
  {
    if (m_BaroSensor == null || m_BaroSensorListener == null)
      {
        return false;
      }

    m_SensorManager.unregisterListener(m_BaroSensorListener);
    m_BaroSensorListener = null;

    return true;
  }

  private class BaroSensorListener implements SensorEventListener
  {
    private int elements = 0;
    private long start = 0;
    private float sum = 0f;

    public void reset()
    {
      elements = 0;
      start = 0;
      sum = 0f;
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy)
    {
    }

    @Override
    synchronized public void onSensorChanged(SensorEvent event)
    {
      if (start == 0)
        {
          // Set start time for average building.
          start = event.timestamp;
          sum = 0f;
          return;
        }

      elements++;

      sum += event.values[0];

      // The timestamp uses as unit nano seconds.
      if (event.timestamp - start < 995000000)
        {
          return;
        }

      // About every 1000 ms a average pressure value is calculated.
      double averagePressure = sum / elements;

      try
      {
        // Send pressure value to native application part.
        nativeBaroPressure( averagePressure );
      }
      catch( UnsatisfiedLinkError e )
      {
        // Ignore exception, if native part is not yet loaded.
      }

      reset();

      // Set new start time.
      start = event.timestamp;
    }
  }

  /**
   * Called from the QtActivity.onCreate to get information if the App is
   * startable or not. If not, QtActivity.onCreate returns without starting the
   * native part.
   * 
   * @return The result of the pre-checks. True if all is ok otherwise false
   */
  protected boolean checkPreconditions()
  {
    return (m_sdCardOk && m_cumulusFolderOk);
  }

  /**
   * Send a byte from the GPS stream to the native Cumulus part, if GPS is
   * enabled. If a shutdown is requested from the native part, the GPS enable
   * status will be reset.
   * 
   * @param newByte Byte of GPS data stream
   */
  synchronized public static void byteFromGps(byte newByte)
  {
    if ( gpsEnabled )
      {
        nativeByteFromGps(newByte);
      }
  }

  public static String bundle2String(Bundle bundle) {
		String string = "Bundle{";
		for (String key : bundle.keySet()) {
			string += " " + key + " => " + bundle.get(key) + ";";
		}
		string += " }Bundle";
		return string;
	}
  
  /**
   * Returns the restarted flag.
   * 
   * @return True if system was restarted otherwise false.
   */
  synchronized public boolean isRestarted()
  {
      return m_isRestarted;
  }
  
  /**
   * Sets the restart flag to the passed value.
   * 
   * @param flag New value for the restart flag.
   */
  synchronized public void setRestarted(boolean flag)
  {
      m_isRestarted = flag;
  }
  
  /**
   * Returns the current Android API level
   * 
   * @return The current Android API level as integer
   */
  synchronized public int getApiLevel()
  {
      return android.os.Build.VERSION.SDK_INT;
  }

  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    if( savedInstanceState != null )
      {
        Log.d(TAG, "onCreate: " + bundle2String(savedInstanceState));
    
        // @AP: That is a workaround to prevent the black screen on Qt side
        // after a kill of the running Cumulus instance because of low memory.
        // That can happen, if another app is opened in the foreground and the
        // Dalvid VM decided to kill the Cumulus process. The Qt part saved
        // some information for reinstall but that is a bug.
        savedInstanceState.putBoolean( "Started", false );
    
        // Set restart flag to true, to restore some settings in the native part.
        setRestarted( true );
      }
	
    try
      {
        Log.d(TAG, "onCreate Entry");
        Log.d(TAG, "API Level=" + Build.VERSION.SDK_INT);
        Log.d(TAG, "CPU_ABI=" + Build.CPU_ABI);
        Log.d(TAG, "BRAND=" + Build.BRAND);
        Log.d(TAG, "PRODUCT=" + Build.PRODUCT);
        Log.d(TAG, "MANUFACTURER=" + Build.MANUFACTURER);
        Log.d(TAG, "HARDWARE=" + Build.HARDWARE);
        Log.d(TAG, "MODEL=" + Build.MODEL);
        Log.d(TAG, "DEVICE=" + Build.DEVICE);
        Log.d(TAG, "DISPLAY=" + Build.DISPLAY);
        Log.d(TAG, "BOARD=" + Build.BOARD);
        Log.d(TAG, "FINGERPRINT=" + Build.FINGERPRINT);
        Log.d(TAG, "ID=" + Build.ID);

        if (Build.VERSION.SDK_INT >= 9)
          {
            // Log.d(TAG, "SERIAL=" + Build.SERIAL); // API 9 and higher
          }
      }
    catch (java.lang.NoSuchFieldError e)
      {
        // Ignore this exception
      }

    synchronized (m_ActivityMutex)
      {
        // Set object reference
        m_objectRef = this;
      }
    
    if( m_cumulusServiceClass == null )
      {
        m_cumulusServiceClass = CumulusService.class;
      }

    // Start a service, what shall prevent killing of our App instance.
    startService( new Intent(this, CumulusService.class) );

    m_SensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
    m_BaroSensor = m_SensorManager.getDefaultSensor(Sensor.TYPE_PRESSURE);

    if( m_BaroSensor != null )
      {
        String text = "BaroSensor: Name=" + m_BaroSensor.getName()
            + ", Vendor=" + m_BaroSensor.getVendor() + ", Resolution= "
            + m_BaroSensor.getResolution() + ", MaxRange="
            + m_BaroSensor.getMaximumRange() + ", Power="
            + m_BaroSensor.getPower();

        Log.d(TAG, text);
      }
    else
      {
        Log.d(TAG, "No Barometric sensor available!");
      }

    // Object to IOIO services
    m_ioio = new CumulusIOIO( this, m_commHandler );
    m_ioio.create();

    getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
    Log.d(TAG, "DISPLAY_METRICS=" + displayMetrics.toString());

    // Get the internal data directory for our App.
    final String appDataDir = getDir("Cumulus", MODE_PRIVATE).getAbsolutePath();

    // Check, if the internal data are already installed. To check that, a
    // special file name is used, which is created after data installation.
    // The file name includes the package version, to ensure that with every
    // new version the data are reinstalled.
    final String pvcFileName = "pvc_" + String.valueOf(getPackageVersionCode());

    File pvcAppFile = new File(appDataDir + File.separator + pvcFileName);

    Log.d(TAG, "pvcApp=" + pvcAppFile.getAbsolutePath());

    if (!pvcAppFile.exists())
      {
        // We do the data install in an extra thread to minimize the runtime in
        // our activity.
        AppDataInstallThread adit = new AppDataInstallThread(appDataDir, pvcFileName);
        adit.start();
      }
    else
      {
        // The app data seem to be installed, make directory info for other
        // threads available.
        synchronized (appDataPath)
          {
            appDataPath = appDataDir;
          }
      }

    String ess = Environment.getExternalStorageState();

    // Cumulus user application data are installed at the SD-Card in the folder
    // Cumulus. So we need to check, if the SD-Card is mounted with read/write
    // access
    // and if the Cumulus directory exists.
    if (Environment.MEDIA_MOUNTED.equals(ess))
      {
        m_sdCardOk = true;

        // SD-Card is mounted, check, if the Cumulus directory exists
        File addDataDir = new File(Environment.getExternalStorageDirectory(),
            "Cumulus");

        if (addDataDir.exists())
          {
            m_cumulusFolderOk = true;
          }
        else
          {
            m_cumulusFolderOk = addDataDir.mkdir();
          }

        if (m_cumulusFolderOk == true)
          {
            // Other treads waiting for this info
            synchronized (addDataPath)
              {
                addDataPath = addDataDir.getAbsolutePath();
                m_addDataInstalled = true;
              }
          }
      }

    // Call super class, that will load the native C++ part of Cumulus.
    super.onCreate(savedInstanceState);

    // Check the state of the mounted SD-Card
    if (!m_sdCardOk)
      {
        Log.e(TAG, "Exiting, SdCard is not mounted or not writeable!");
        showDialog(R.id.dialog_no_sdcard);
        return;
      }

    // Check the state of the Cumulus folder at the SD-Card
    if (!m_cumulusFolderOk)
      {
        Log.e(TAG, "Cannot create folder Cumulus on the SD card!");
        showDialog(R.id.dialog_no_data_folder);
        return;
      }

    Window w = getWindow();
    w.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);

    // Switch the screen permanently on
    w.setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
        WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

    apl = new AsyncPlayer("alarm_player");
    npl = new AsyncPlayer("notification_player");

    lm = (LocationManager) getSystemService(Context.LOCATION_SERVICE);

    if (lm == null)
      {
        Log.w(TAG, "System said: LOCATION_SERVICE is null!");
      }
    else
      {
        nl = new GpsStatus.NmeaListener()
        {
          @Override
          public void onNmeaReceived(long timestamp, String nmea)
          {
            nmeaIsReceived = true;

            if (gpsEnabled)
              {
                // Forward GPS data only, if user has enabled that.
                try
                  {
                    nativeNmeaString(nmea);
                  }
                catch (UnsatisfiedLinkError e)
                  {
                    // Ignore exception, if native part is not yet loaded.
                  }
              }
          }
        };

        if (lm.addNmeaListener(nl) == false)
          {
            Log.e(TAG, "Cannot add NMEA listener to Location Manager!");

            // reset NMEA listener
            nl = null;
          }
      }

    /*
     * Add an icon to the notification area while Cumulus runs, to remind the
     * user that we're sucking his battery empty.
     */
    notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
    Notification notification = new Notification(R.drawable.icon, null,
        System.currentTimeMillis());
    Context context = getApplicationContext();
    CharSequence contentTitle = "Cumulus";
    CharSequence contentText = getString(R.string.running);
    Intent notificationIntent = new Intent(this, CumulusActivity.class);
    PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
        notificationIntent, 0);

    notification.setLatestEventInfo(context, contentTitle, contentText,
        contentIntent);
    notification.flags |= Notification.FLAG_ONGOING_EVENT;

    if (notificationManager != null)
      {
        notificationManager.notify(1, notification);
      }

    // Register a BroadcastReceiver for BT intents, if Bluetooth is supported by
    // this device.
    if( BluetoothAdapter.getDefaultAdapter() != null )
    {
	  IntentFilter filter = new IntentFilter(
	                              BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
	  filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
	
	  // Don't forget to unregister during onDestroy
	  registerReceiver(bcReceiver, filter);
    }
    
    if( m_BaroSensor != null )
    {
      // Switch on again the baro sensor, if it was on at the last run.
      SharedPreferences sp = getPreferences( MODE_PRIVATE );
      
      boolean baroState = sp.getBoolean( "BaroSensorState", false );
      
      if( baroState == true )
        {
           activateBarometerSensor();
        }
    }

    Log.d(TAG, "onCreate exit");
  }

  @Override
  protected void onStart()
  {
    Log.d(TAG, "onStart()");
    super.onStart();

    // Start IOIO services will be done on user request only
    // m_ioio.start();
  }

  @Override
  protected void onRestart()
  {
    Log.d(TAG, "onRestart()");
    super.onRestart();
  }

  @Override
  protected void onResume()
  {
    Log.d(TAG, "onResume()");
    super.onResume();

    // Switch user screen brightness on
    switchOffScreenDimming();

    if (screenDimmTimer == null)
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
    Log.d(TAG, "onPause()");
    super.onPause();

    screenDimmTimer.cancel();
    screenDimmTimer = null;
  }

  @Override
  protected void onStop()
  {
    Log.d(TAG, "onStop()");
    super.onStop();
  }

  @Override
  protected void onDestroy()
  {
    Log.d(TAG, "onDestroy");

    if(m_cumulusServiceClass != null )
    {
      stopService( new Intent(this, CumulusService.class) );
      m_cumulusServiceClass = null;
    }
    
    notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

    if (notificationManager != null)
      {
        notificationManager.cancelAll();
        Log.d(TAG, "onDestroy: Cancel all Notifs now");
      }

    // Stop barometer sensor callback.
    if (m_BaroSensorListener != null)
      {
        m_SensorManager.unregisterListener(m_BaroSensorListener);
        m_BaroSensorListener.reset();
      }

    if (lm != null && nl != null)
      {
        lm.removeNmeaListener(nl);
      }

    if (lm != null & ll != null)
      {
        lm.removeUpdates(ll);
      }

    if (m_btService != null)
      {
        // terminate all BT threads
        m_btService.stop();
      }

    if( BluetoothAdapter.getDefaultAdapter() != null )
    {
      // The device supports Bluetooth, unregister BT receiver.
      unregisterReceiver(bcReceiver);
    }

    // Stop IOIO services
    m_ioio.stop();

    // destroy IOIO services
    m_ioio.destroy();

    // call super class
    super.onDestroy();

    if (isFinishing() == false)
      {
        Log.i(TAG, "onDestroy: isFinishing() is called by the OS!");
        // System.exit(0);
      }
  }

  protected void onSaveInstanceState(Bundle outState)
  {	
    if( lm != null && ll != null )
    {
       // Internal GPS is activated
       outState.putByte("GpsDevice", GPS_DEVICE_INTERNAL );
    }
    else if (m_btService != null)
    {
      // BT GPS is activated
      outState.putByte("GpsDevice", GPS_DEVICE_BT );
      outState.putString("BtMacAddress", m_btMacAddress );
    }
    else if ( m_ioio.isStarted() == true )
    {
       // IOIO GPS is activated
       outState.putByte("GpsDevice", GPS_DEVICE_IOIO);
    }
    else
    {
       // No GPS is activated
       outState.putByte("GpsDevice", GPS_DEVICE_NONE );
    }

    Log.d(TAG, "onSaveInstanceState: " + bundle2String(outState));
    super.onSaveInstanceState(outState);
  }
  
  protected void onRestoreInstanceState(Bundle savedInstanceState)
  {
      Log.d(TAG, "onRestoreInstanceState: " + bundle2String(savedInstanceState));

      // Save instance state for later restore.
      m_restoreInstanceState = savedInstanceState;

      super.onRestoreInstanceState(savedInstanceState);
  }
  
  /**
   * Called to restore a previous saved instance state, when the native part
   * has startup finished reported in method nativeShutdown(false).
   */
  private void restoreInstanceState()
  {	
	Log.d(TAG, "restoreInstanceState: " + bundle2String(m_restoreInstanceState));

	if( m_restoreInstanceState == null )
	{
	  return;
	}

	if( m_restoreInstanceState.getByte("GpsDevice") == GPS_DEVICE_INTERNAL )
	{
	  enableInternalGps(false);
	}
	else if( m_restoreInstanceState.getByte("GpsDevice") == GPS_DEVICE_BT )
	{
	  String mac = m_restoreInstanceState.getString("BtMacAddress");
	  
	  if( mac != null && mac.length() != 0 )
	  {
	    connect2BtDevice( mac );
	  }
	}
	else if( m_restoreInstanceState.getByte("GpsDevice") == GPS_DEVICE_IOIO )
	{
	  // We have to wait a while, to ensure that the IOIO service is running
	  //  before we can restart the ioio connection. It's now done after 5s.
	  m_commHandler.postDelayed( new Runnable() {
	                             public void run() {
	    	                        enableIoioGps( false );
	                             }
                               }, 5000 );
	}
	
	// All is done, reset saved instance state.
	m_restoreInstanceState = null;
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event)
  {
    Log.d(TAG, "onKeyDown, key pressed: " + event.toString());
    
    if( ! m_nativeIsUp )
      {
    	// ignore key events, if native is not up
        return true;
      }

    // There was a problem report on Google Developer, that a native function
    // was not found. Maybe it was not yet loaded.
    // java.lang.UnsatisfiedLinkError: isRootWindow
    try
      {
        if (keyCode == KeyEvent.KEYCODE_BACK)
          {
            // Send close key to QtApp. It can be used to close a widget or the
            // whole
            // application.
            nativeKeypress((char) 28);
            return true;
          }
        
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP)
          {
            // Key volume up was pressed. It is sent as plus key to the native part.
            // If it is not catch here, a core dump is raised at native side.
            nativeKeypress((char) 43);
            return true;
          }

        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN)
          {
            // Key volume down was pressed. It is sent as minus key to the native part.
            // If it is not catch here, a core dump is raised at native side.
            nativeKeypress((char) 45);
            return true;
          }

        if (keyCode == KeyEvent.KEYCODE_MENU)
          {
            if (isRootWindow())
              {
                // Only the root window can show this dialog.
                showDialog(R.id.dialog_main_menu);
              }

            return true;
          }
      }
    catch (UnsatisfiedLinkError e)
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

    if( ! m_nativeIsUp )
      {
        // ignore key events, if native is not up
        return true;
      }

    if (keyCode == KeyEvent.KEYCODE_BACK ||
        keyCode == KeyEvent.KEYCODE_VOLUME_UP ||
        keyCode == KeyEvent.KEYCODE_VOLUME_DOWN )
      {
        return true;
      }

    try
      {
    	return super.onKeyUp(keyCode, event);
      }
    catch (UnsatisfiedLinkError e)
      {
        // ignore exception and consume event.
        return true;    	
      }
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
        if (gpsEnabled == true && nmeaIsReceived == false)
          {
            // If the GPS device do not deliver raw NMEA sentences, we
            // forward these GPS data which is provided here. Otherwise
            // our application gets no GPS data.
            try
              {
                nativeGpsFix(loc.getLatitude(), loc.getLongitude(),
                    loc.getAltitude(), loc.getSpeed(), loc.getBearing(),
                    loc.getAccuracy(), loc.getTime());
              }
            catch (UnsatisfiedLinkError e)
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

        if (provider == LocationManager.GPS_PROVIDER)
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

        if (provider.equals(LocationManager.GPS_PROVIDER))
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
        // Log.d(TAG, "onStatusChanged: Provider=" + provider + ", Status=" + status);

        if (provider.equals(LocationManager.GPS_PROVIDER) && gpsEnabled)
          {
            if (status == LocationProvider.AVAILABLE)
              {
                if (lastGpsStatus != status)
                  {
                    // GPS receiver is connected
                    reportGpsStatus(1);
                  }
              }
            else
              {
                if (lastGpsStatus != status)
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

  @Override
  protected Dialog onCreateDialog(int id)
  {
    AlertDialog alert;

    AlertDialog.Builder builder = new AlertDialog.Builder(this);

    switch (id)
      {
        case R.id.dialog_close:
          builder
              .setMessage(getString(R.string.reallyClose))
              .setCancelable(false)
              .setPositiveButton(getString(android.R.string.yes),
                  new DialogInterface.OnClickListener()
                  {
                    @Override
                    public void onClick(DialogInterface dialog, int id)
                    {
                      CumulusActivity.this.finish();
                    }
                  })
              .setNegativeButton(getString(android.R.string.no),
                  new DialogInterface.OnClickListener()
                  {
                    @Override
                    public void onClick(DialogInterface dialog, int id)
                    {
                      dialog.cancel();
                    }
                  });

          alert = builder.create();
          break;

        case R.id.dialog_main_menu:
          builder.setTitle(getString(R.string.mainMenu));

          CharSequence[] m_items;

          if (m_BaroSensor == null)
            {
              m_items = new CharSequence[4];
              m_items[0] = getString(R.string.setup);
              m_items[1] = getString(R.string.gps);
              m_items[2] = getString(R.string.toggles);
              m_items[3] = getString(R.string.quit);

              builder.setItems(m_items, new DialogInterface.OnClickListener()
              {
                @Override
                public void onClick(DialogInterface dialog, int item)
                {
                  switch (item)
                    {
                      case 0:
                        showDialog(R.id.dialog_setup);
                        break;
                      case 1:
                        showDialog(R.id.dialog_gps_menu);
                        break;
                      case 2:
                        showDialog(R.id.dialog_toggles);
                        break;
                      case 3:
                        // Qt main window will get a quit
                        nativeKeypress((char) 30);
                        break;
                    }
                }
              });
            }
          else
            {
              m_items = new CharSequence[5];
              m_items[0] = getString(R.string.setup);
              m_items[1] = getString(R.string.gps);
              m_items[2] = getString(R.string.baroSensor);
              m_items[3] = getString(R.string.toggles);
              m_items[4] = getString(R.string.quit);

              builder.setItems(m_items, new DialogInterface.OnClickListener()
              {
                @Override
                public void onClick(DialogInterface dialog, int item)
                {
                  switch (item)
                    {
                      case 0:
                        showDialog(R.id.dialog_setup);
                        break;
                      case 1:
                        showDialog(R.id.dialog_gps_menu);
                        break;
                      case 2:
                        showDialog(R.id.dialog_baro_sensor);
                        break;
                      case 3:
                        showDialog(R.id.dialog_toggles);
                        break;
                      case 4:
                        // Qt main window will get a quit
                        nativeKeypress((char) 30);
                        break;
                    }
                }
              });
            }

          alert = builder.create();
          break;

        case R.id.dialog_hw_id1:
          CharSequence[] hw_items1 = { "IOIO",
                                       getString(R.string.gps) };

          builder.setTitle(getString(R.string.hardwareMenu));
          builder.setItems(hw_items1, new DialogInterface.OnClickListener()
          {
            @Override
            public void onClick(DialogInterface dialog, int item)
            {
              switch (item)
                {
                case 0:
                  // start IOIO uart preference activity
                  startPreferenceActivity();
                  break;
                case 1:
                  showDialog(R.id.dialog_gps_menu);
                  break;
                }
            }
          });

          alert = builder.create();
          break;

        case R.id.dialog_hw_id2:
          CharSequence[] hw_items2 = { "IOIO",
                                       getString(R.string.gps),
                                       getString(R.string.baroSensor) };

          builder.setTitle(getString(R.string.hardwareMenu));
          builder.setItems(hw_items2, new DialogInterface.OnClickListener()
          {
            @Override
            public void onClick(DialogInterface dialog, int item)
            {
              switch (item)
                {
                  case 0:
                  // start IOIO uart preference activity
                  startPreferenceActivity();
                  break;
                  case 1:
                    showDialog(R.id.dialog_gps_menu);
                    break;
                  case 2:
                    showDialog(R.id.dialog_baro_sensor);
                    break;
                }
            }
          });

          alert = builder.create();
          break;

        case R.id.dialog_setup:
          CharSequence[] s_items = {
              getString(R.string.gpsIoio),
              getString(R.string.setupGeneral),
              getString(R.string.setupPreFlight) };

          builder.setTitle(getString(R.string.setupMenu));
          builder.setItems(s_items, new DialogInterface.OnClickListener()
          {
            @Override
            public void onClick(DialogInterface dialog, int item)
            {
              switch (item)
                {
                  case 0:
                    // start IOIO uart preference activity
                    startPreferenceActivity();
                    break;                  
                  case 1:
                    // open setup general dialog
                    nativeKeypress((char) 25);
                    break;
                  case 2:
                    // open setup preflight dialog
                    nativeKeypress((char) 26);
                    break;
                }
            }
          });

          alert = builder.create();
          break;

        case R.id.dialog_gps_menu:
          CharSequence[] g_items = { getString(R.string.gpsOn),
              getString(R.string.gpsStatus) };

          if (gpsEnabled)
            {
              g_items[0] = getString(R.string.gpsOff);
            }

          builder.setTitle(getString(R.string.gpsMenu));
          builder.setItems(g_items, new DialogInterface.OnClickListener()
          {
            @Override
            public void onClick(DialogInterface dialog, int item)
            {
              switch (item)
                {
                  case 0:
                    toggleGps();
                    removeDialog(R.id.dialog_gps_menu);
                    break;
                  case 1:
                    nativeKeypress((char) 27);
                    break;
                }
            }
          });

          alert = builder.create();
          break;

        case R.id.dialog_toggles:
          CharSequence[] t_items = { "" };

          if (infoBoxesVisible)
            {
              t_items[0] = getString(R.string.mapInfoBarHide);
            }
          else
            {
              t_items[0] = getString(R.string.mapInfoBarShow);
            }

          builder.setTitle(getString(R.string.togglesMenu));
          builder.setItems(t_items, new DialogInterface.OnClickListener()
          {
            @Override
            public void onClick(DialogInterface dialog, int item)
            {
              switch (item)
                {
                  case 0:
                    nativeKeypress((char) 29);
                    infoBoxesVisible = !infoBoxesVisible;
                    removeDialog(R.id.dialog_toggles);
                    break;
                }
            }
          });

          alert = builder.create();
          break;

        case R.id.dialog_baro_sensor:

          CharSequence[] a_items = { "" };

          if (m_BaroSensorListener != null)
            {
              a_items[0] = getString(R.string.baroSensorOff);
            }
          else
            {
              a_items[0] = getString(R.string.baroSensorOn);
            }

          builder.setTitle(getString(R.string.baroSensorMenu));
          builder.setItems(a_items, new DialogInterface.OnClickListener()
          {
            @Override
            public void onClick(DialogInterface dialog, int item)
            {
              switch (item)
                {
                  case 0:

                    if (m_BaroSensorListener != null)
                      {
                        deactivateBarometerSensor();
                        
                        Editor e = getPreferences( MODE_PRIVATE ).edit();
                        e.putBoolean( "BaroSensorState", false );
                        e.commit();
                      }
                    else
                      {
                        activateBarometerSensor();
                        
                        Editor e = getPreferences( MODE_PRIVATE ).edit();
                        e.putBoolean( "BaroSensorState", true );
                        e.commit();
                      }

                    removeDialog(R.id.dialog_baro_sensor);
                    break;
                }
            }
          });

          alert = builder.create();
          break;

        case R.id.dialog_no_sdcard:
          builder
              .setMessage(getString(R.string.sdcardNeeded))
              .setCancelable(false)
              .setPositiveButton(getString(android.R.string.ok),
                  new DialogInterface.OnClickListener()
                  {
                    @Override
                    public void onClick(DialogInterface dialog, int id)
                    {
                      CumulusActivity.this.finish();
                    }
                  });
          alert = builder.create();
          break;

        case R.id.dialog_no_data_folder:
          builder
              .setMessage(getString(R.string.noDataFolder))
              .setCancelable(false)
              .setPositiveButton(getString(android.R.string.ok),
                  new DialogInterface.OnClickListener()
                  {
                    @Override
                    public void onClick(DialogInterface dialog, int id)
                    {
                      CumulusActivity.this.finish();
                    }
                  });
          alert = builder.create();
          break;

        case R.id.dialog_zip_error:
          builder
              .setMessage(getString(R.string.errorUnzip))
              .setCancelable(false)
              .setPositiveButton(getString(android.R.string.ok),
                  new DialogInterface.OnClickListener()
                  {
                    @Override
                    public void onClick(DialogInterface dialog, int id)
                    {
                      CumulusActivity.this.finish();
                    }
                  });
          alert = builder.create();
          break;

        case R.id.dialog_gps:
          
          gpsMenuItems.clear();
          
          if ( lm != null )
            {
              gpsMenuItems.add(getString(R.string.gpsInternal));
            }
          
          if ( BluetoothAdapter.getDefaultAdapter() != null )
            {
              gpsMenuItems.add(getString(R.string.gpsBluetooth));
            }
          
          if ( m_ioio.isStarted() == false )
            {
              gpsMenuItems.add(getString(R.string.gpsIoio));
            }

          CharSequence[] l_gitems = gpsMenuItems.toArray(new CharSequence[gpsMenuItems.size()]);

          builder.setTitle(getString(R.string.gpsMenu));
          builder.setItems(l_gitems, new DialogInterface.OnClickListener()
          {
            @Override
            public void onClick(DialogInterface dialog, int item)
            {
              if( gpsMenuItems.get(item).equals(getString(R.string.gpsInternal)))
                {
                  enableInternalGps(true);
                }             
              else if( gpsMenuItems.get(item).equals(getString(R.string.gpsBluetooth)))
                {
                  enableBtGps(true);
                }
              else if( gpsMenuItems.get(item).equals(getString(R.string.gpsIoio)))
                {
                  enableIoioGps(true);
                }
              
              return;
            }
          });
          alert = builder.create();
          break;

        case R.id.dialog_bt:

          if (m_pairedBtDevices == null || m_pairedBtDevices.size() == 0)
            {
              return null;
            }

          builder.setTitle(getString(R.string.gpsBtMenu));

          CharSequence[] l_bt_items = new CharSequence[m_pairedBtDevices.size()];
          m_btMacArray = new String[m_pairedBtDevices.size()];

          int idx = 0;

          // Loop through paired devices
          for (BluetoothDevice device : m_pairedBtDevices)
            {
              Log.d( TAG, "chooseBtGps(): " + device.getName() + "=" +
                     device.getAddress());

              l_bt_items[idx] = device.getName() + " (" + device.getAddress()
                  + ")";
              m_btMacArray[idx] = device.getAddress();
              idx++;
            }

          builder.setItems(l_bt_items, new DialogInterface.OnClickListener()
          {
            @Override
            public void onClick(DialogInterface dialog, int item)
            {
              removeDialog(R.id.dialog_bt);

              // Fetch MAC address of clicked item
              if (m_btMacArray != null)
                {
                  connect2BtDevice(m_btMacArray[item]);
                }
            }
          });
          alert = builder.create();
          break;

        case R.id.dialog_no_paired_bt_devices:
          builder
              .setMessage(getString(R.string.noPairedBtD))
              .setCancelable(false)
              .setPositiveButton(getString(android.R.string.ok),
                  new DialogInterface.OnClickListener()
                  {
                    @Override
                    public void onClick(DialogInterface dialog, int id)
                    {
                      dialog.cancel();
                      reportGpsStatus(0);
                      gpsEnabled = false;
                    }
                  });
          alert = builder.create();
          break;

        default:
          return super.onCreateDialog(id);
      }

    return alert;
  }
  
  @Override
  protected Dialog onCreateDialog(int id, Bundle args)
  {
    AlertDialog alert;

    AlertDialog.Builder builder = new AlertDialog.Builder(this);

    switch (id)
      {
      case R.id.dialog_ioio_incompatible:
    	  // Retrieve additional data from the Bundle.
    	  String msg = getString(R.string.ioioIncompatible) + "\n\n" +
    	               "IOIO LibVersion " + args.getString("LIBV") +
    	               " != IOIO FirmwareVersion " + args.getString("FWV");
          builder
              .setMessage(msg)
              .setCancelable(false)
              .setPositiveButton(getString(android.R.string.ok),
                  new DialogInterface.OnClickListener()
                  {
                    @Override
                    public void onClick(DialogInterface dialog, int id)
                    {
                      dialog.cancel();
                      reportGpsStatus(0);
                      gpsEnabled = false;
                      m_ioio.stop();
                    }
                  });
          alert = builder.create();
          break;

        default:
          return super.onCreateDialog(id, args);
      }
    
    return alert;
  }
  
  void playSound( int stream )
  {
    if (stream == 0)
      {
        Uri sf = Uri.parse( "android.resource://" + getPackageName() + "/" + R.raw.notify );
        stream = AudioManager.STREAM_NOTIFICATION;
        npl.play(this.getApplicationContext(), sf, false, stream);
      }
    else if (stream == 1)
      {
        Uri sf = Uri.parse( "android.resource://" + getPackageName() + "/" + R.raw.alarm );
        stream = AudioManager.STREAM_ALARM;
        apl.play(this.getApplicationContext(), sf, false, stream);
      }
  }
  
  /**
   * Forward a GPS NMEA command to the connected GPS device. Can be BT or USB IOIO.
   */
  synchronized boolean gpsCmd(String cmd)
  {
    if (!gpsEnabled)
      {
        return false;
      }

    if (m_btService != null)
      {
        // BT Service is activated
        m_btService.write(cmd.getBytes());
        return true;
      }

    if( m_ioio != null && m_ioio.isStarted() )
      {
        // IOIO service is activated, write data to Uart.
        return m_ioio.writeUart(cmd.getBytes());
      }

    return false;
  }

  /**
   * Forward a byte to the connected GPS device. Can be BT or USB IOIO.
   */
  synchronized boolean byte2Gps(byte newByte)
  {
    if (!gpsEnabled)
      {
        return false;
      }

    if (m_btService != null)
      {
        // BT Service is activated
        byte[] data = new byte[1];
        data[0] = newByte;
        m_btService.write(data);
        return true;
      }

    if( m_ioio != null && m_ioio.isStarted() )
      {
        // IOIO service is activated, write data to Uart.
        byte[] data = new byte[1];
        data[0] = newByte;
        return m_ioio.writeUart( data );
      }

    return false;
  }

  synchronized private void dimmScreen(float value)
  {
    // Log.v(TAG, "dimmScreen(" + value + ")");

    WindowManager.LayoutParams lp = getWindow().getAttributes();

    if (lp.screenBrightness != value)
      {
        lp.screenBrightness = value;
        getWindow().setAttributes(lp);
      }

    setCurScreenBrightness(value);
  }

  /**
   * This method is called by the native code to handle the screen dimming.
   * 
   * @param newState
   *          true means dimm the screen, false means bright the screen
   */
  void dimmScreen(boolean newState)
  {
    // Log.v(TAG, "dimmScreen(" + newState + ")");
    // save requested dimm state from C++ API side.
    setRequestedDimmState(newState);
  }

  /**
   * This method returns the data directory of the App. Called via jni to pass
   * this info to the native C++ side.
   * 
   * @return The data directory of the App
   */
  String getAppDataDir()
  {
    synchronized (appDataPath)
      {
        return appDataPath;
      }
  }

  /**
   * This method returns the additional data directory of the App. Called via
   * jni to pass this info to the native C++ side.
   * 
   * @return The additional data directory of the App
   */
  String getAddDataDir()
  {
    synchronized (addDataPath)
      {
        Log.d( TAG, "addDataPath=" + addDataPath );
        return addDataPath;
      }
  }

  String getLanguage()
  {
    // returns language as e.g. "en_US" or de_DE
    return Locale.getDefault().toString();
  }

  /**
   * Gets the display metrics from the Android system and return them as key
   * value string to the caller.
   * 
   * @return display metrics as key value string
   */
  String getDisplayMetrics()
  {
    getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

    StringBuffer buffer = new StringBuffer();

    buffer.append("density=").append(displayMetrics.density).append(';')
          .append("densityDpi=").append(displayMetrics.densityDpi).append(';')
          .append("heightPixels=").append(displayMetrics.heightPixels)
          .append(';').append("scaledDensity=")
          .append(displayMetrics.scaledDensity).append(';')
          .append("widthPixels=").append(displayMetrics.widthPixels).append(';')
          .append("xdpi=").append(displayMetrics.xdpi).append(';')
          .append("ydpi=").append(displayMetrics.ydpi).append(';');

    return buffer.toString();
  }

  /**
   * Gets the build data from the Android system as key value string. The single
   * elements are separated by a newline sign.
   * 
   * @return Build data as key value string
   */
  String getBuildData()
  {
    StringBuffer buffer = new StringBuffer();

    buffer.append("CPU_ABI\n").append(Build.CPU_ABI).append('\n')
        .append("BRAND\n").append(Build.BRAND).append('\n').append("PRODUCT\n")
        .append(Build.PRODUCT).append('\n').append("MANUFACTURER\n")
        .append(Build.MANUFACTURER).append('\n').append("HARDWARE\n")
        .append(Build.HARDWARE).append('\n').append("MODEL\n")
        .append(Build.MODEL).append('\n').append("DEVICE\n")
        .append(Build.DEVICE).append('\n').append("DISPLAY\n")
        .append(Build.DISPLAY).append('\n').append("FINGERPRINT\n")
        .append(Build.FINGERPRINT).append('\n').append("ID\n").append(Build.ID)
        .append('\n');

    if (Build.VERSION.SDK_INT >= 9)
      {
        // buffer.append("SERIAL\n").append(Build.SERIAL).append('\n');
      }

    return buffer.toString();
  }

  /**
   * Called from the native side to signal a complete startup or that a shutdown
   * is requested. In shutdown case all connections at java side will be closed
   * to avoid a calling of the native side.
   * 
   * @param state false means the native part is up after startup,
   *              true means the native part is going to shutdown
   */
  synchronized void nativeShutdown( boolean state)
  {
    Log.d( TAG, "nativeShutdown=" + state );
    
    m_nativeIsUp = ! state;
    
    if( state == false )
      {
        // If state is reported as false the native part is up and operational
    	// after startup.
    	if( m_restoreInstanceState != null )
    	{
    	    runOnUiThread(new Runnable()
    	    {
    	      @Override
    	      public void run()
    	      {
    	    	restoreInstanceState();
    	      }
    	     });
       	}
    	
        return;
      }

    gpsEnabled = false;

    notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

    if (notificationManager != null)
      {
        notificationManager.cancelAll();
        Log.d(TAG, "nativeShutdown: Cancel all Notifs now");
      }

    if (lm != null && nl != null)
      {
        lm.removeNmeaListener(nl);
        nl = null;
      }

    if (lm != null & ll != null)
      {
        lm.removeUpdates(ll);
        ll = null;
      }

    if (lm != null)
      {
        lm = null;
      }

    if (m_btService != null)
      {
        // terminate all BT threads
        m_btService.stop();
        m_btService = null;
      }

    deactivateBarometerSensor();
  }

  private void toggleGps()
  {
    removeDialog(R.id.dialog_main_menu);

    BluetoothAdapter mBtAdapter = BluetoothAdapter.getDefaultAdapter();

    if (gpsEnabled == true)
      {
        Log.i(TAG, "Disable GPS");
        
        if (lm != null && ll != null)
        {
          // We switch off the data delivery from the internal GPS.
          lm.removeUpdates(ll);
          ll = null;
        }

        reportGpsStatus(0);
        gpsEnabled = false;

        // Lock, if Bt Service is used. It will be terminated then.
        if (mBtAdapter != null && m_btService != null)
          {
            m_btService.stop();
            m_btService = null;
          }
        
        // Lock, if IOIO Service is used. It will be stopped then.
        if( m_ioio.isStarted() )
          {
            m_ioio.stop();
          }
      }
    else
      {
        Log.i(TAG, "Enable GPS");

        if ((lm != null || mBtAdapter != null) && m_ioio.isStarted() == false)
          {
            showDialog(R.id.dialog_gps);
          }
        else if (lm != null)
          {
            enableInternalGps(false);
          }
        else if (mBtAdapter != null)
          {
            enableBtGps(false);
          }
        else if ( m_ioio.isStarted() == false )
          {
            enableIoioGps(false);
          }
      }
  }

  private void enableInternalGps(boolean clearDialog)
  {
    if (clearDialog)
      {
        removeDialog(R.id.dialog_gps);
      }

    if (lm != null)
      {
        if (ll == null)
          {
            try
              {
                ll = createLocationListener();
                lm.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1000, 0, ll);
              }
            catch (IllegalArgumentException e)
              {
                // It seems there is no GPS provider available on this device.
                // No BT service available
                Toast.makeText(getApplicationContext(),
                               getString(R.string.noGpsService), Toast.LENGTH_LONG).show();

                Log.e(TAG, "Device has no GPS provider: " + e.getMessage());
                lm = null;
                ll = null;
                return;
              }
          }

        if (lm.isProviderEnabled(LocationManager.GPS_PROVIDER) == false)
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

  private void enableIoioGps(boolean clearDialog)
  {
    if (clearDialog)
      {
        removeDialog(R.id.dialog_gps);
      }
    
    if (lm != null && ll != null)
    {
      // We switch off the data delivery from the internal GPS.
      lm.removeUpdates(ll);
      ll = null;
    }
    
    // Starts the ioio service
    m_ioio.start();

    reportGpsStatus(1);
    gpsEnabled = true;
  }

  private void enableBtGps(boolean clearDialog)
  {
    if (clearDialog)
      {
        removeDialog(R.id.dialog_gps);
      }
    
    if( BluetoothAdapter.getDefaultAdapter() == null )
      {
        // No BT service available
        Toast.makeText(getApplicationContext(),
                       getString(R.string.noBtService), Toast.LENGTH_LONG).show();
        return;
      }

    if (!BluetoothAdapter.getDefaultAdapter().isEnabled())
      {
        Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        startActivityForResult(enableBtIntent, R.id.request_enable_bluetooth);
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
    if (m_pairedBtDevices.size() > 0)
      {
        showDialog(R.id.dialog_bt);
      }
    else
      {
        m_pairedBtDevices = null;
        showDialog(R.id.dialog_no_paired_bt_devices);
      }
  }

  private void connect2BtDevice(String macAddress)
  {
    Log.i(TAG, "connectBtDevice " + macAddress);
    
    m_btMacAddress = macAddress;

    if (m_btService == null)
      {
        m_btService = new BluetoothService(this, m_btHandler);
      }

    final BluetoothDevice device = BluetoothAdapter.getDefaultAdapter()
        .getRemoteDevice(macAddress);

    /* Opens a connection to the selected BT device. */
    m_btService.connect(device);

    // set the GPS status to enabled
    gpsEnabled = true;

    if (lm != null && ll != null)
      {
        // We switch off the data delivery from the internal GPS.
        lm.removeUpdates(ll);
        ll = null;
      }
  }

  /**
   * This starts the CumulusPreferenceActivity.
   */
  protected void startPreferenceActivity()
  {
    Intent i = new Intent(this, CumulusPreferenceActivity.class);
    startActivityForResult(i, R.id.request_ioio_preferences);
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data)
  {
    Log.d(TAG, "onActivityResult ReqCode=" + requestCode +
               ", ResCode=" + resultCode );

    if( requestCode == R.id.request_enable_bluetooth )
      {
        // On single instance the result code is always NOK! Therefore we wait
        // until the adapter on state is broadcasted.
        if (resultCode == RESULT_OK)
          {
            Log.d(TAG, "BluetoothAdapter is switched on by the user");
          }

        return;
      }
    else if( requestCode == R.id.request_ioio_preferences )
      {
        // IOIO Uart preferences have been changed
        if (resultCode == RESULT_OK)
          {
            // Restart IOIO
            if( m_ioio.isStarted() )
              {
                Log.d(TAG, "IOIO Uart config has been changed, restarting IOIO board");
                m_ioio.stop();
                m_ioio.start();
              }
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
        .setPositiveButton(getString(R.string.gpsGoto),
            new DialogInterface.OnClickListener()
            {
              @Override
              public void onClick(DialogInterface dialog, int id)
              {
                Intent callGPSSettingIntent = new Intent(
                    android.provider.Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                startActivity(callGPSSettingIntent);
              }
            });

    alertDialogBuilder.setNegativeButton(getString(android.R.string.cancel),
        new DialogInterface.OnClickListener()
        {
          @Override
          public void onClick(DialogInterface dialog, int id)
          {
            dialog.cancel();
          }
        });

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
   * Wrapper around native call nativeGpsStatus to prevent an exception, if
   * native C++ is not yet loaded.
   * 
   * @param status new status to be reported to native part.
   */
  private void reportGpsStatus(int status)
  {
    try
      {
        // Call native C++ part to forward GPS status.
        nativeGpsStatus(status);
      }
    catch (UnsatisfiedLinkError e)
      {
        // Ignore exception, if native part is not yet loaded.
      }
  }

  /**
   * Removes all pvc_... files in the passed directory.
   * 
   * @param directoryName
   *          Name of directory
   */
  private void removePvcFiles(String directoryName)
  {
    File directory = new File(directoryName);

    if (!directory.exists())
      {
        return;
      }

    // Get all files in directory
    File[] files = directory.listFiles();

    for (File file : files)
      {
        if (file.isFile() && file.getName().startsWith("pvc_"))
          {
            if (!file.delete())
              {
                // Failed to delete file
                Log.d(TAG, "removeDirContent: Failed to delete " + file);
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
     * @param appDataDirName
     *          The full path name of the app data directory
     * 
     * @param pvcAppFileName
     *          The simple file name of the package version control file.
     */
    AppDataInstallThread(String appDataDirName, String pvcAppFileName)
    {
      super("AppDataInstall");

      m_appDataDirName = appDataDirName;
      m_pvcAppFileName = pvcAppFileName;

      // Creates a handler in the calling thread to pass later back results to
      // it from the running thread.
      m_Handler = new Handler();
    }

    @Override
    public void run()
    {
      Log.i(TAG, "AppData are installed at: " + m_appDataDirName);

      // At first remove all package version control files.
      removePvcFiles(m_appDataDirName);

      boolean res = installAppData(m_appDataDirName, getAssets());

      if (res == false)
        {
          m_Handler.post(new Runnable()
          {
            @Override
            public void run()
            {
              showDialog(R.id.dialog_zip_error);
            }
          });

          return;
        }

      try
        {
          File pvcAppFile = new File(m_appDataDirName + File.separator
              + m_pvcAppFileName);

          // Store an install marker file
          OutputStream out = new FileOutputStream(pvcAppFile);
          out.close();
        }
      catch (Exception e)
        {
          Log.e(TAG, "PVC app file error: " + e.getMessage());
        }

      Log.i(TAG, "AppData install finished.");

      // another thread is waiting for this info
      synchronized (appDataPath)
        {
          appDataPath = m_appDataDirName;
        }
    }
  }

  private boolean installAppData(String appDir, AssetManager am)
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

    if (!res)
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
     * @param addDataDirName
     *          The full path name of the add data directory
     * 
     * @param pvcAddFileName
     *          The simple file name of the package version control file.
     */
    AddDataInstallThread(String addDataDirName, String pvcAddFileName)
    {
      super("AddDataInstall");

      m_addDataDirName = addDataDirName;
      m_pvcAddFileName = pvcAddFileName;

      // Creates a handler in the calling thread to pass later back results to
      // it from the running thread.
      m_Handler = new Handler();
    }

    @Override
    public void run()
    {
      Log.i(TAG, "AddData are installed at: " + m_addDataDirName);

      // At first remove all package version control files.
      removePvcFiles(m_addDataDirName);

      // Extract zip file from asset folder
      boolean res = installAddData(m_addDataDirName, getAssets());

      if (res == false)
        {
          m_Handler.post(new Runnable()
          {
            @Override
            public void run()
            {
              showDialog(R.id.dialog_zip_error);
            }
          });
          return;
        }

      try
        {
          File pvcAddFile = new File(m_addDataDirName + File.separator
              + m_pvcAddFileName);

          // Store an install marker file
          OutputStream out = new FileOutputStream(pvcAddFile);
          out.close();
        }
      catch (Exception e)
        {
          Log.e(TAG, "PVC add file error: " + e.getMessage());
        }

      Log.i(TAG, "AddData install finished.");

      // another thread is waiting for this info
      synchronized (m_ActivityMutex)
        {
          m_addDataInstalled = true;
        }
    }
  }

  private boolean installAddData(String addDir, AssetManager am)
  {
    String addDataFile = getString(R.string.addDataFile);

    InputStream stream = null;

    try
      {
        stream = am.open(addDataFile);
      }
    catch (IOException e)
      {
        Log.e(TAG, "InstallAddData error: " + e.getMessage());
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
        Log.e(TAG, "InstallAddData failed!");
      }

    Log.d(TAG, "InstallAddData install succeeded!");

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
            // System.out.println("Unzipping: " + entry.getName() +
            // ", Directory: "
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
                Log.e("Unzip", "Error: " + e.getMessage());
                return false;
              }

            BufferedOutputStream bos = new BufferedOutputStream(fos,
                buffer.length);

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
    dimmScreen(-1.0f);
    setCurrentDimmState(false);
    setLastUserAction(System.currentTimeMillis());
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
      long lua = lastUserAction();

      // Log.v(TAG, "ScreenDimmerTimerTask: rds=" + rds + ", cds=" + cds +
      // ", lus=" + (System.currentTimeMillis() - lua));

      if (cds == true && curScreenBrightness() == DIMM1_SCREEN_VALUE
          && (System.currentTimeMillis() - lua) >= DIMM2_SCREEN_TO)
        {
          // Activate second dimm state of screen.
          mHandler.post(new Runnable()
          {
            @Override
            public void run()
            {
              dimmScreen(DIMM2_SCREEN_VALUE);
            }
          });

          return;
        }

      if (rds == cds)
        {
          // No new dimm request, do nothing more.
          return;
        }

      if (rds == false)
        {
          setCurrentDimmState(rds);
          setLastUserAction(System.currentTimeMillis());

          // Switch user screen brightness on
          mHandler.post(new Runnable()
          {
            @Override
            public void run()
            {
              dimmScreen(-1.0f);
            }
          });
          return;
        }

      if (rds == true && (System.currentTimeMillis() - lua) >= DIMM1_SCREEN_TO)
        {
          setCurrentDimmState(rds);

          // Activate first dimm state of screen.
          mHandler.post(new Runnable()
          {
            @Override
            public void run()
            {
              dimmScreen(DIMM1_SCREEN_VALUE);
            }
          });
          return;
        }
    }
  } // End of inner Class ScreenDimmerTimerTask

} // End of Class
