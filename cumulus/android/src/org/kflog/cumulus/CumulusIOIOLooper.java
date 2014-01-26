/***********************************************************************
 **
 **   CumulusIOIOLooper.java
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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.SharedPreferences;
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
  private static final String TAG = "Java#CumulusIOIOLooper";

  /**
   * Debug flag control
   */
  private static final boolean D = true;

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
  public CumulusIOIOLooper()
  {
  }

  /**
   * Constructor with callback object
   * 
   * @param callback
   *          Object to creator of this class.
   */
  public CumulusIOIOLooper(CumulusIOIO callback)
  {
    m_callback = callback;
  }

  /**
   * Returns the uart object according to the selection index.
   * 
   * @param index
   *          The uart object to be selected. 1...3
   * 
   * @return Selected uart object if it exists or null in error case.
   */
  public Uart getUart(int index)
  {
    switch (index)
      {
        case 0:
        case 1:
        case 2:
        case 3:
          return m_uarts[index];
        default:
          return null;
      }
  }

  /**
   * Write to the active Uart thread in an unsynchronized manner
   * 
   * @param out
   *          The bytes to write.
   * 
   * @return True in case of success otherwise false
   */
  public boolean writeUart(byte[] buffer)
  {
    if (m_activeUart >= 0 && m_activeUart <= 3)
      {
        // Create temporary object
        UartThread u;

        // Synchronize a copy of the uart thread
        synchronized (this)
          {
            u = m_uartThreads[m_activeUart];
          }

        if (u != null)
          {
            // Perform the write unsynchronized
            u.write(buffer);
            return true;
          }
      }

    return false;
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
    if (D)
      Log.d(TAG, "IOIO setup is called!");

    SharedPreferences settings = CumulusActivity.cumulusSettings;

    int uartSpeed[] = new int[4];
    uartSpeed[0] = settings.getInt(CumulusActivity.Uart0Speed, 57600);
    uartSpeed[1] = settings.getInt(CumulusActivity.Uart1Speed, 57600);
    uartSpeed[2] = settings.getInt(CumulusActivity.Uart2Speed, 57600);
    uartSpeed[3] = settings.getInt(CumulusActivity.Uart3Speed, 57600);

    boolean uart0Enabled = settings.getBoolean(CumulusActivity.Uart0Enabled, true);
    boolean uart1Enabled = settings.getBoolean(CumulusActivity.Uart1Enabled, false);
    boolean uart2Enabled = settings.getBoolean(CumulusActivity.Uart2Enabled, false);
    boolean uart3Enabled = settings.getBoolean(CumulusActivity.Uart3Enabled, false);

    // Activate the uart, which has the user selected.
    if ( uart0Enabled )
      {
        createUart( 0, uartSpeed[0] );
      }
    else if (uart1Enabled)
      {
        createUart( 1, uartSpeed[1] );
      }
    else if (uart2Enabled)
      {
        createUart( 2, uartSpeed[2] );
      }
    else if (uart3Enabled)
      {
        createUart( 3, uartSpeed[3] );
      }
  }
  
  private void createUart( int index, int speed )
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
                                         Uart.Parity.NONE,
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
  }

  @Override
  public void loop() throws ConnectionLostException, InterruptedException
  {
    // In this loop different checks can be added to control the work of the
    // uarts.
    Thread.sleep(60000);
  }

  @Override
  /**
   * IOIO communication is lost or closed.
   */
  public void disconnected()
  {
    if (D)
      Log.d(TAG, "IOIO disconnected is called!");

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
    private InputStream m_uartStreamReader = null;
    // private BufferedInputStream m_uartBufferReader = null;
    private OutputStream m_uartStreamWriter = null;
    private boolean m_abort = false;

    /**
     * Creation of an uart thread. The thread opens an input reader stream and
     * reads single characters until abort is set. That must be done, because
     * Flarm can operate in binary mode.
     * 
     * @param uart
     *          Uart object
     * 
     * @param index
     *          index of uart
     */
    public UartThread(Uart uart, int index)
    {
      UTAG = "UartThread_" + index;

      if (D)
        Log.d(TAG, "creating " + UTAG);

      m_uart = uart;
      m_uartIndex = index;

      // Gets the Uart input stream
      m_uartStreamReader = uart.getInputStream();
      // m_uartReader = new BufferedInputStream((tmpIn));

      // Gets the Uart output stream
      m_uartStreamWriter = uart.getOutputStream();
    }

    /**
     * This method runs a loop and tries to read bytes from the uart input
     * stream, which are forwarded to the Cumulus activity instance.
     */
    public void run()
    {
      if (D)
        Log.i(TAG, "Run " + UTAG + " is started");

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
              int character = m_uartStreamReader.read();

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
      
      if (D)
        Log.i(TAG, "Run " + UTAG + " is finished");
    }

    /**
     * Write to the connected OutStream.
     * 
     * @param buffer The bytes to write
     */
    public void write(byte[] buffer)
    {
      if (m_uartStreamWriter == null || getAbort() == true )
        {
          return;
        }

      try
        {
          m_uartStreamWriter.write(buffer);
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
      if (D)
        Log.d(TAG, UTAG + ": cancel is called by " + this);

      try
        {
          setAbort(true);

          if (m_uartStreamReader != null)
            {
              m_uartStreamReader.close();
              m_uartStreamReader = null;
            }

          if (m_uartStreamWriter != null)
            {
              m_uartStreamWriter.close();
              m_uartStreamWriter = null;
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
