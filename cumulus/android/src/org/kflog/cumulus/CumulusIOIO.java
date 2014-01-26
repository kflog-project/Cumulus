/***********************************************************************
 **
 **   CumulusIOIO.java
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

package org.kflog.cumulus;

import android.content.ContextWrapper;
import android.util.Log;
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
  private static final String TAG = "Java#CumulusIOIO";

  private IOIOAndroidApplicationHelper m_helper = null;

  private boolean m_started = false;
  
  /**
   * IOIO looper instance.
   */
  private CumulusIOIOLooper m_ioioLooper = null;
  
  /**
   * Mutex for m_ioioLooper object.
   */
  private final Object m_ioioLooperMutex = new Object();


  public CumulusIOIO(ContextWrapper wrapper)
  {
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
   * you're using Android API level lower than 5. Otherwise you shoud call the
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
    synchronized (m_ioioLooperMutex)
      {
        m_ioioLooper = new CumulusIOIOLooper(this);
        return m_ioioLooper;
      }
  }

  @Override
  public IOIOLooper createIOIOLooper(String connectionType, Object extra)
  {
    Log.d(TAG, "createIOIOLooper for: " + connectionType);
    return createIOIOLooper();
  }

  /**
   * 
   * @return Instance of CumulusIOIOLooper looper or null, if no IOIO is
   *         running.
   */
  public CumulusIOIOLooper getIoioLooper()
  {
    synchronized (m_ioioLooperMutex)
      {
        return m_ioioLooper;
      }
  }

  /**
   * Reset IOIO looper. This method is used by the CumulusIOIOLooper as
   * callback, if the IOIO was disconnected to reset its instance here.
   * 
   * @param CumulusIOIOLooper Instance to be reset
   */
  public void resetIoioLooper(CumulusIOIOLooper ioioLooper)
  {
    synchronized (m_ioioLooperMutex)
      {
        if (m_ioioLooper != null && m_ioioLooper == ioioLooper)
          {
            m_ioioLooper = null;
          }
      }
  }

} // End of class
