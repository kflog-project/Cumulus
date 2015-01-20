/***********************************************************************
 **
 **   CumulusIOIOLooper.java
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

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.OutputStream;

import android.content.ContextWrapper;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;
import ioio.lib.api.Uart;
import ioio.lib.api.exception.ConnectionLostException;
import ioio.lib.api.exception.OutOfResourceException;
import ioio.lib.util.BaseIOIOLooper;

import org.kflog.cumulus.CumulusActivity;
import org.kflog.cumulus.CumulusIOIO;

/**
 * An implementation of {@link BaseIOIOLooper}.
 * 
 * This base class provides implementations for all methods and provides the
 * {@link #ioio_} field for subclasses.
 * 
 */
public class CumulusIOIOLooper extends BaseIOIOLooper
{
  private static final String TAG = "CumIOIOLooper";

  /**
   * Debug flag control
   */
  private static final boolean D = true;
  
  /**
   * Context wrapper instance.
   */
  private ContextWrapper m_context = null;
  /**
   * Callback to the creator of this instance
   */
  private CumulusIOIO m_callback = null;

  /**
   * Array of possible Uarts.
   */
  private Uart m_uarts[] = new Uart[4];

  /**
   * Array of possible Uart threads.
   */
  private UartThread m_uartThreads[] = new UartThread[4];

  /**
   * The index of the active uart. 0...3 or -1 if undefined.
   */
  private int m_activeUart = -1;
  
  /**
   * Tx and Rx pins of IOIO uarts 0...3.
   * 
   * 4 uarts maybe on a IOIO board. According to xcsoar they use the
   * following pins:
   * 
   * Uart 0: TX=3, RX=4
   * Uart 1: TX=5, RX=6
   * Uart 2: TX=10, RX=11
   * Uart 3: TX=12, RX=13
   */
  static final private int txRxPins[][] = { {3,4}, {5,6}, {10,11}, {12,13} };

  /**
   * Default constructor
   */
  public CumulusIOIOLooper( ContextWrapper contextWrapper )
  {
    m_context = contextWrapper;
  }

  /**
   * Constructor with callback object
   * 
   * @param callback
   *          Object to creator of this class.
   */
  public CumulusIOIOLooper(ContextWrapper contextWrapper, CumulusIOIO callback)
  {
    m_context = contextWrapper;
    m_callback = callback;
  }

  /**
   * Write to the active Uart thread
   * 
   * @param out The bytes to write.
   * 
   * @return True in case of success otherwise false
   */
  public boolean writeUart(byte[] buffer)
  {
    if (m_activeUart >= 0 && m_activeUart <= 3)
      {
        synchronized (this)
          {
            if( m_uartThreads[m_activeUart] != null )
              {
                m_uartThreads[m_activeUart].write(buffer);
                return true;
              }
          }
      }

    return false;
  }

  /**
   * Convert a parity string to an enumeration value.
   * 
   * @param parity A string as 0 (none), 1 (even), 2 (odd)
   * 
   * @return Uart parity enumeration
   */
  private Uart.Parity getUartParity( String parity )
  {
    if( parity.equals("0"))
      {
        return Uart.Parity.NONE;
      }
    
    if( parity.equals("1"))
      {
        return Uart.Parity.EVEN;
      }
    
    if( parity.equals("2"))
      {
        return Uart.Parity.ODD;
      }
    
    return Uart.Parity.NONE; 
  }

  /**
   * This method will be called as soon as connection to the IOIO has been
   * established. Typically, this will include opening pins and modules using
   * the openXXX() methods of the {@link #ioio_} field.
   * 
   * @throws ConnectionLostException
   *           The connection to the IOIO has been lost.
   * @throws InterruptedException
   *           The thread has been interrupted.
   */
  protected void setup() throws ConnectionLostException, InterruptedException
  {
    if (D) Log.d(TAG, "IOIO setup is called!");

    // Retrieves the IOIO Uarts settings
    SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(m_context);

    int uartSpeed[] = { 4800, 4800, 4800, 4800 };
    
    try
    {    
      uartSpeed[0] = Integer.parseInt( settings.getString(m_context.getString(R.string.pref_setup_uart_0_speed),
                                                          m_context.getString(R.string.pref_setup_speed_default)));
      uartSpeed[1] = Integer.parseInt( settings.getString(m_context.getString(R.string.pref_setup_uart_1_speed),
                                                          m_context.getString(R.string.pref_setup_speed_default)));
      uartSpeed[2] = Integer.parseInt( settings.getString(m_context.getString(R.string.pref_setup_uart_2_speed),
                                                          m_context.getString(R.string.pref_setup_speed_default)));
      uartSpeed[3] = Integer.parseInt( settings.getString(m_context.getString(R.string.pref_setup_uart_3_speed),
                                                          m_context.getString(R.string.pref_setup_speed_default)));
    }
    catch( NumberFormatException e )
    {
      Log.e(TAG, e.toString());
    }
    
    String uartParity[] = new String[4];
    
    uartParity[0] = settings.getString(m_context.getString(R.string.pref_setup_uart_0_parity),
                                       m_context.getString(R.string.pref_setup_parity_default));
    uartParity[1] = settings.getString(m_context.getString(R.string.pref_setup_uart_1_parity),
                                       m_context.getString(R.string.pref_setup_parity_default));
    uartParity[2] = settings.getString(m_context.getString(R.string.pref_setup_uart_2_parity),
                                       m_context.getString(R.string.pref_setup_parity_default));
    uartParity[3] = settings.getString(m_context.getString(R.string.pref_setup_uart_3_parity),
                                       m_context.getString(R.string.pref_setup_parity_default));  
    
    int uart2Activated = 0;
    
    try
    {
      uart2Activated = Integer.parseInt( settings.getString(m_context.getString(R.string.pref_active_uart),
                                                            m_context.getString(R.string.pref_active_uart_default)));
    }
    catch( NumberFormatException e )
    {
      Log.e(TAG, e.toString());
      uart2Activated = 0;
    }
        
    // Activate the uart, which the user has selected.
    if ( uart2Activated == 0 )
      {
        createUart( 0, uartSpeed[0], getUartParity(uartParity[0]) );
      }
    else if (uart2Activated == 1 )
      {
        createUart( 1, uartSpeed[1], getUartParity(uartParity[1]) );
      }
    else if (uart2Activated == 2 )
      {
        createUart( 2, uartSpeed[2], getUartParity(uartParity[2]) );
      }
    else if (uart2Activated == 3 )
      {
        createUart( 3, uartSpeed[3], getUartParity(uartParity[3]) );
      }
  }
  
  private void createUart( int index, int speed, Uart.Parity parity )
    throws ConnectionLostException, InterruptedException
  {
    if (D)
    {
      Log.d( TAG, "Creating Uart_" + index +
             ", TxPin=" + txRxPins[index][0] +
             ", RxPin=" + txRxPins[index][1] +
             ", Speed=" + speed );
    }
    
    try
      {
        m_uarts[index] = ioio_.openUart( txRxPins[index][1],
                                         txRxPins[index][0],
                                         speed,
                                         parity,
                                         Uart.StopBits.ONE );

        // Setup a reader loop in an extra thread for uart 0.
        m_uartThreads[index] = new UartThread(m_uarts[index], index);
        m_uartThreads[index].start();
        m_activeUart = index;
      }
    catch (OutOfResourceException e)
      {
        m_uarts[index] = null;
        Log.e(TAG, "setup(): Uart_" + index +
                   ", Tx=" + txRxPins[index][0] +
                   ", Rx=" + txRxPins[index][1] +
                   ", Speed=" + speed +
                   " not existing!");
      }
    
    if (m_callback != null)
      {
        // Report to caller, that this instance is connected to to IOIO.
        m_callback.activedIoioLooper(this);
      }
    
    Log.d( TAG, "Created Uart_" + index + ", m_activeUart=" + m_activeUart + ", OBJ=" + this);
  }

  @Override
  public void loop() throws ConnectionLostException, InterruptedException
  {
    // In this loop different checks could be added to control the work of the
    // uarts.
    Thread.sleep(600000);
  }

  @Override
  /**
   * IOIO communication is lost or closed.
   */
  public void disconnected()
  {
    if (D) Log.d(TAG, "IOIO disconnected is called!");

    synchronized( this )
    {
      for (int i = 0; i < 4; i++)
        {
          if (m_uarts[i] != null)
            {
              if (m_uartThreads[i] != null &&
                  m_uartThreads[i].getState() != Thread.State.TERMINATED)
                {
                  m_uartThreads[i].setAbort(true);
                  m_uartThreads[i] = null;
                }
  
              m_uarts[i] = null;
            }
        }
  
      m_activeUart = -1;
    }

    if (m_callback != null)
      {
        m_callback.resetIoioLooper(this);
      }
  }

  @Override
  /**
   * Called, if an incompatible IOIO firmware is detected.
   */
  public void incompatible()
  {
    Log.e(TAG, "IOIO incompatible is called!");

    if (m_callback != null)
      {
        m_callback.reportIncompatible(ioio_);
        m_callback.resetIoioLooper(this);
      }
  }

  /**
   * This thread runs during a connection with an uart. It handles all
   * incoming/outgoing data.
   */
  private class UartThread extends Thread
  {
    private String UTAG;
    private Uart m_uart = null;
    private int m_uartIndex = -1;
    private BufferedInputStream m_uartReader = null;
    private OutputStream m_uartWriter = null;
    private boolean m_abort = false;

    /**
     * Creation of an uart thread. The thread opens an input reader stream and
     * reads single characters until abort is set. That must be done, because
     * Flarm can operate in binary mode.
     * 
     * @param uart Uart object
     * 
     * @param index Uart index to be used as selector
     */
    public UartThread(Uart uart, int index)
    {
      UTAG = "UartThread_" + index;

      if (D) Log.d(TAG, "creating " + UTAG);

      m_uart = uart;
      m_uartIndex = index;

      // Gets the Uart input stream
      m_uartReader = new BufferedInputStream( uart.getInputStream(), 8192 );

      // Gets the Uart output stream
      m_uartWriter = uart.getOutputStream();
    }

    /**
     * This method runs a loop and tries to read bytes from the uart input
     * stream, which are forwarded to the Cumulus activity instance.
     */
    public void run()
    {
      if (D) Log.d(TAG, "Run " + UTAG + " is started");

      setName(UTAG);

      // Read from InputStream until abort is set or an exception was catched.
      while (getAbort() == false)
        {
          try
            {
              /*
               * // Read a line from the InputStream String line =
               * mmInReader.readLine();
               * 
               * if( line.trim().length() == 0 ) { continue; }
               * 
               * CumulusActivity.nativeNmeaString( line );
               */

              // Read a character from the input stream and pass it via JNI
              // to the native C++ part.
              int character = m_uartReader.read();

              if (character == -1)
                {
                  Log.w(TAG, UTAG + ": read -1 (EOF)");
                  break;
                }
              
              // Forward byte to CumulusActivity
              CumulusActivity.byteFromGps((byte) (character & 0xff));
            }

          catch (IOException e)
            {
              if (!getAbort())
                {
                  Log.e(TAG, UTAG + ": read failed, closing port!", e);
                }
              
              break;
            }
        }
      
      cancel();
      
      if (D) Log.i(TAG, "Run " + UTAG + " is finished");
    }

    /**
     * Write to the connected OutStream.
     * 
     * @param buffer The bytes to write
     */
    public void write(byte[] buffer)
    {
      if (m_uartWriter == null || getAbort() == true )
        {
          Log.e(TAG, UTAG + "write failed due to writer is null or abort is true");
          return;
        }

      try
        {
          m_uartWriter.write(buffer);
        }
      catch (IOException e)
        {
          Log.e(TAG, UTAG + ": write failed", e);
        }
    }

    /**
     * @param abortFlag
     *          Sets the abort flag to the passed value.
     */
    synchronized private void setAbort(boolean abortFlag)
    {
      m_abort = abortFlag;
    }

    /*
     * @return The content of the abort flag.
     */
    synchronized private boolean getAbort()
    {
      return m_abort;
    }

    synchronized public void cancel()
    {
      if (D) Log.d(TAG, UTAG + ": cancel is called by " + this);

      try
        {
          setAbort(true);

          if (m_uartReader != null)
            {
              m_uartReader.close();
              m_uartReader = null;
            }

          if (m_uartWriter != null)
            {
              m_uartWriter.close();
              m_uartWriter = null;
            }
          
          if( m_uart != null )
            {
              m_uart.close();
              m_uart = null;
            }
          
          m_uarts[m_uartIndex] = null;
          m_uartThreads[m_uartIndex] = null;
          m_activeUart = -1;
        }
      catch (IOException e)
        {
          Log.e(TAG, UTAG + ": close failed", e);
        }
    }
  } // End of private class

} // End of class
