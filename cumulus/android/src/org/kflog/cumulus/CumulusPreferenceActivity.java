/***********************************************************************
 **
 **   CumulusPreferenceActivity.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c):  2014 by Axel Pauli <kflog.cumulus@gmail.com>
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * @class CumulusPreferenceActivity
 * 
 * @author Axel Pauli
 * 
 * @email <kflog.cumulus@gmail.com>
 * 
 * @date 2014
 * 
 * @version $Id$
 * 
 * @short This class handles the Cumulus preference settings of IOIO uarts
 * 
 * This class handles the Cumulus preference settings of IOIO uarts. Four
 * different uarts can be setup in speed and parity here.
 * 
 */

package org.kflog.cumulus;

import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.KeyEvent;

public class CumulusPreferenceActivity
  extends PreferenceActivity
  implements OnSharedPreferenceChangeListener
{
  static private final String TAG = "CumPrefActivity";
  
  // Debug enable flag
  static private final boolean D = true;
  
  // Flag to store configuration changes.
  private boolean m_configChanged = false;
  
  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    if( D ) Log.d(TAG, "onCreate()");
    
    super.onCreate(savedInstanceState);
  
    // Read preference screens from the xml resource file.
    addPreferencesFromResource(R.xml.preferences);
  
    // PreferenceManager.setDefaultValues(this, R.xml.preferences, false);
    updatePrefItems( false );
  }
  
  protected void onStart()
    {
      if( D ) Log.d(TAG, "onStart()");
      super.onStart();
    }
  
  protected void onRestart()
    {
      if( D ) Log.d(TAG, "onRestart()");
      super.onRestart();
    }
  
  protected void onResume()
    {
      if( D ) Log.d(TAG, "onResume()");
      super.onResume();
  
      // Update the preference screens
      updatePrefItems( false );
      
      // Setup callback handler again
      getPreferenceScreen().getSharedPreferences()
                           .registerOnSharedPreferenceChangeListener(this);
  
    }
  
  protected void onPause()
    {
      if( D ) Log.d(TAG, "onPause()");
      super.onPause();
    }
  
  protected void onStop()
    {
      if( D ) Log.d(TAG, "onStop()");
      super.onStop();
      
      // Remove callback handler
      getPreferenceScreen().getSharedPreferences()
                           .unregisterOnSharedPreferenceChangeListener(this);
    }
  
  protected void onDestroy()
    {
      if( D ) Log.d(TAG, "onDestroy()");
      super.onDestroy();
    }
  
  @Override
  public boolean onKeyDown(int keycode, KeyEvent e)
    {
      if( D ) Log.d(TAG, "onKeyDown(): " + keycode);
      
      switch (keycode)
        {
        case KeyEvent.KEYCODE_BACK:
          
          if( isConfigChanged() )
            {
              // Tells the calling activity, that configuration was changed.
              Intent i = new Intent();
              i.putExtra( "Result", "IOIO Uart Configuration changed!" );
              setResult(RESULT_OK, i);
            }
          
          // finish activity
          finish();
          return true;
          
         default:
           break;
        }
  
      return super.onKeyDown(keycode, e);
    }
  
  /**
   * This method updates the preference objects with the configured values.
   * 
   * @param refreshContent If set to true, the content of all preference screens
   *                       is updated otherwise not.
   */
  private void updatePrefItems( boolean refreshContent )
    {
      if( D ) Log.d(TAG, "updatePrefItems()");
      
      SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(this);
      
      Preference pref;
      String value, value1, value2;
      String key;
      
      String uarts[] = new String[4];
      uarts[0] = getString(R.string.pref_setup_uart_0);
      uarts[1] = getString(R.string.pref_setup_uart_1);
      uarts[2] = getString(R.string.pref_setup_uart_2);
      uarts[3] = getString(R.string.pref_setup_uart_3);
      
      String uartSummaries[] = new String[4];
      uartSummaries[0] = getString(R.string.pref_setup_uart_0_summary);
      uartSummaries[1] = getString(R.string.pref_setup_uart_1_summary);
      uartSummaries[2] = getString(R.string.pref_setup_uart_2_summary);
      uartSummaries[3] = getString(R.string.pref_setup_uart_3_summary);
    
      String speedKeys[] = new String[4];
      speedKeys[0] = getString(R.string.pref_setup_uart_0_speed);
      speedKeys[1] = getString(R.string.pref_setup_uart_1_speed);
      speedKeys[2] = getString(R.string.pref_setup_uart_2_speed);
      speedKeys[3] = getString(R.string.pref_setup_uart_3_speed);
      
      String parityKeys[] = new String[4];
      parityKeys[0] = getString(R.string.pref_setup_uart_0_parity);
      parityKeys[1] = getString(R.string.pref_setup_uart_1_parity);
      parityKeys[2] = getString(R.string.pref_setup_uart_2_parity);
      parityKeys[3] = getString(R.string.pref_setup_uart_3_parity);
      
      // Array with translated parity key strings.
      String[] parities = getResources().getStringArray(R.array.parityKeys);
      
      for( int i = 0; i < 4; i++ )
        {
          // Update summaries with the currently set data items
          key = speedKeys[i];
          pref = findPreference(key);
          value1 = sharedPref.getString(key, getString(R.string.pref_setup_speed_default));
          pref.setSummary( value1 );
          
          key = parityKeys[i];
          pref = findPreference(key);
          value2 = sharedPref.getString(key, getString(R.string.pref_setup_parity_default));
          value2 = parities[Integer.valueOf(value2)];
          pref.setSummary( value2 );
          
          key = uarts[i];
          pref = findPreference(key);
          pref.setSummary( uartSummaries[i] + ", " + value1 + ", " + value2 );
        }
      
      // Show the active IOIO uart in the summary
      pref = findPreference(getString(R.string.pref_active_uart));
      value = sharedPref.getString(getString(R.string.pref_active_uart),
                                   getString(R.string.pref_active_uart_default));
      
      if( value.equals("-1") )
        {
          // If no IOIO uart is selected, the key -1 is returned.
          value = getString(R.string.pref_none);
        }
      
      pref.setSummary( getString(R.string.pref_active_uart_summary) + " " + value );
      
      if( refreshContent )
        {
          // That will update the content of all preference screens
          onContentChanged();
        }
    }
  
  /**
   * Callback which handles configuration changes done by the user. The
   * configuration screens will be updated.
   */
  public void onSharedPreferenceChanged( SharedPreferences sharedPreferences,
                                         String key)
    {
      if( D ) Log.d(TAG, "onSharedPreferenceChanged(): Key=" + key);
      
      if ( key.equals(getString(R.string.pref_active_uart)) )
        {
          setConfigChanged();
          updatePrefItems(false);
          return;
        }
      
      if ( key.startsWith("pref_setup_uart_") )
        {
          setConfigChanged();
          updatePrefItems(true);
        }
    }
  
  /**
   * Query for configuration changes.
   * 
   * @return true if configuration changes have done otherwise false
   */
  public boolean isConfigChanged()
    {
      return m_configChanged;
    }
  
  /**
   * Set the flag that configuration changes have occurred.
   */
  private void setConfigChanged()
    {
      this.m_configChanged = true;
    }
}
