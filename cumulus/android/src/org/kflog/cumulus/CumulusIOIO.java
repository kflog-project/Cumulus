/***********************************************************************
 **
 **   CumulusIOIO.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c):  2014-2015 by Axel Pauli <kflog.cumulus@gmail.com>
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

package org.kflog.cumulus;

import java.util.HashSet;
import java.util.Set;

import android.content.ContextWrapper;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import ioio.lib.api.IOIO;
import ioio.lib.api.IOIO.VersionType;
import ioio.lib.impl.SocketIOIOConnection;
import ioio.lib.util.IOIOLooper;
import ioio.lib.util.IOIOLooperProvider;
import ioio.lib.util.android.IOIOAndroidApplicationHelper;

/**
 * A convenience class for easy creation of IOIO-based services.
 * 
 * It is used by creating a concrete {@link Service} in your application, which
 * extends this class. This class then takes care of proper creation and
 * abortion of the IOIO connection and of a dedicated thread for IOIO
 * communication.
 * <p>
 * In the basic usage the client should extend this class and implement
 * {@link #createIOIOLooper()}, which should return an implementation of the
 * {@link IOIOLooper} interface. In this implementation, the client implements
 * the {@link IOIOLooper#setup(ioio.lib.api.IOIO)} method, which gets called as
 * soon as communication with the IOIO is established, and the
 * {@link IOIOLooper#loop()} method, which gets called repetitively as long as
 * the IOIO is connected.
 * <p>
 * In addition, the {@link IOIOLooper#disconnected()} method may be overridden
 * in order to execute logic as soon as a disconnection occurs for whichever
 * reason. The {@link IOIOLooper#incompatible()} method may be overridden in
 * order to take action in case where a IOIO whose firmware is incompatible with
 * the IOIOLib version that application is built with.
 * <p>
 * In a more advanced use case, more than one IOIO is available. In this case, a
 * thread will be created for each IOIO, whose semantics are as defined above.
 * If the client needs to be able to distinguish between them, it is possible to
 * override {@link #createIOIOLooper(String, Object)} instead of
 * {@link #createIOIOLooper()}. The first argument provided will contain the
 * connection class name, such as ioio.lib.impl.SocketIOIOConnection for a
 * connection established over a TCP socket (which is used over ADB). The second
 * argument will contain information specific to the connection type. For
 * example, in the case of {@link SocketIOIOConnection}, the second argument
 * will contain an {@link Integer} representing the local port number.
 */
public class CumulusIOIO implements IOIOLooperProvider
{
  private static final String TAG = "CumIOIO";

  /**
   * Debug flag control
   */
  private static final boolean D = true;

  /**
   * Context wrapper instance
   */
  private ContextWrapper m_wrapper = null;

  private IOIOAndroidApplicationHelper m_helper = null;
  
  private Handler m_msgHandler = null;

  private boolean m_started = false;
  
  final private Set<CumulusIOIOLooper> ioioLooperSet = new HashSet<CumulusIOIOLooper>();
  
  /**
   * Active IOIO looper instance.
   */
  private CumulusIOIOLooper m_activeIoioLooper = null;
  
  /**
   * Mutex for m_activeIoioLooper object.
   */
  private final Object m_activeIoioLooperMutex = new Object();


  public CumulusIOIO(ContextWrapper wrapper, Handler msgHandler)
  {
    m_wrapper = wrapper;
    m_msgHandler = msgHandler;
    m_helper = new IOIOAndroidApplicationHelper(wrapper, this);
  }
  
  /**
   * Returns the current state of the IOIO.
   * 
   * @return true if IOIO is running otherwise false
   */
  public boolean isStarted()
  {
    return m_started;
  }
  
  /**
   * Subclasses should call this method from their own onCreate() if overloaded.
   * It takes care of connecting with the IOIO.
   */
  public void create()
  {
    m_helper.create();
  }

  /**
   * Subclasses should call this method from their own onDestroy() if
   * overloaded. It takes care of disconnecting with the IOIO.
   */
  public void destroy()
  {
    m_helper.destroy();
  }

  /**
   * This is the old onStart() method. Override and/or call this method only if
   * you're using Android API level lower than 5. Otherwise you should call the
   * onStartCommand() method.
   */
  public void start()
  {
    if (!m_started)
      {
        m_helper.start();
        m_started = true;
      }
    else
      {
        m_helper.restart();
      }
  }

  /**
   * Subclasses should call this method if they wish to disconnect from the
   * IOIO(s) until the next onStart().
   */
  public void stop()
  {
    if (m_started)
      {
        m_helper.stop();
        m_started = false;
      }
  }

  /**
   * Subclasses must either implement this method or its other overload by
   * returning an implementation of {@link IOIOLooper}. A dedicated thread will
   * be created for each available IOIO, from which the {@link IOIOLooper}'s
   * methods will be invoked. <code>null</code> may be returned if the client is
   * not interested to create a thread for this IOIO. In multi-IOIO scenarios,
   * where you want to identify which IOIO the thread is for, consider
   * overriding {@link #createIOIOLooper(String, Object)} instead.
   * 
   * @return An implementation of {@link IOIOLooper}, or <code>null</code> to
   *         skip.
   */
  protected IOIOLooper createIOIOLooper()
  {
    synchronized (m_activeIoioLooperMutex)
      {
        CumulusIOIOLooper ioioLopper = new CumulusIOIOLooper( m_wrapper, this );
        
        ioioLooperSet.add( ioioLopper );
        
        if( D ) Log.d(TAG, "createIOIOLooper= " + ioioLopper);
        
        return ioioLopper;
      }
  }

  @Override
  public IOIOLooper createIOIOLooper(String connectionType, Object extra)
  {
    Log.d(TAG, "createIOIOLooper for: " + connectionType);
    return createIOIOLooper();
  }
  
  /**
   * Write bytes to the activated Uart of the IOIO.
   * 
   * @param buffer Bytes to write to the Uart of the IOIO
   * 
   * @return true in case of success other wise false
   */
  public boolean writeUart( byte[] buffer )
  {
    if( m_activeIoioLooper != null && isStarted() )
      {
        return m_activeIoioLooper.writeUart( buffer );
      }
    
    return false;
  }

  /**
   * Reset IOIO looper. This method is used by the CumulusIOIOLooper as
   * callback, if the IOIO was disconnected to reset its instance here.
   * 
   * @param CumulusIOIOLooper Instance to be reset
   */
  public void resetIoioLooper(CumulusIOIOLooper ioioLooper)
  {
    if( D ) Log.d(TAG, "resetIoioLooper= " + m_activeIoioLooper);
    
    synchronized (m_activeIoioLooperMutex)
      {
        if (m_activeIoioLooper != null && m_activeIoioLooper == ioioLooper)
          {
            m_activeIoioLooper = null;
          }
        
        ioioLooperSet.remove( ioioLooper );
      }
  }
  
  /**
   * Set active IOIO looper. This method is used by the CumulusIOIOLooper as
   * callback, if the IOIO has connected successfully.
   * 
   * @param CumulusIOIOLooper Active instance of IOIO looper
   */
  public void activedIoioLooper( CumulusIOIOLooper ioioLooper )
  {
    if( D ) Log.d(TAG, "activedIoioLooper= " + ioioLooper);

    m_activeIoioLooper = ioioLooper;
  }

  /**
   * This method is used by the CumulusIOIOLooper as callback. It reports that
   * an incompatible IOIO firmware is detected to tell that the user. Note! The
   * argument ioio can only be queried for version strings!
   */
  public void reportIncompatible(IOIO ioio)
  {
    synchronized (m_activeIoioLooperMutex)
      {
    	Bundle data = new Bundle();
    	data.putString("HWV", ioio.getImplVersion(VersionType.HARDWARE_VER));
    	data.putString("BLV", ioio.getImplVersion(VersionType.BOOTLOADER_VER));
    	data.putString("FWV", ioio.getImplVersion(VersionType.APP_FIRMWARE_VER));
    	data.putString("LIBV", ioio.getImplVersion(VersionType.IOIOLIB_VER));
    	
    	Message msg = m_msgHandler.obtainMessage(R.id.msg_ioio_incompatible);
    	msg.setData(data);
    	msg.sendToTarget();
      }
  }

} // End of class
