/***********************************************************************
 **
 **   BluetoothService.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c): 2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**************************************************************************
 * Note, the Android code example BluetoothChatService is partly reused by
 * Cumulus as BluetoothService.
 *
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************/

package org.kflog.cumulus;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import org.kflog.cumulus.CumulusActivity;
import org.kflog.cumulus.R;

/**
 * This class does all the work for setting up and managing Bluetooth
 * connections with other devices. It has a thread that listens for
 * incoming connections, a thread for connecting with a device, and a
 * thread for performing data transmissions when connected.
 */
public class BluetoothService
{
  // Debugging
  private static final String TAG = "BTService";
  private static final boolean D = false;
  
  // Name for the SDP record when creating server socket
  private static final String SDP_NAME = "BT_Servive_Cumulus";
  
  // Unique UUID for this application, using common Serial Port Profile Id
  private static final UUID SPP_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
  
  // Message types emitted by the BluetoothService Handler
  public static final int MESSAGE_STATE_CHANGE = 1;
  public static final int MESSAGE_READ         = 2;
  public static final int MESSAGE_WRITE        = 3;
  public static final int MESSAGE_DEVICE_NAME  = 4;
  public static final int MESSAGE_TOAST        = 5;

  // Key names sent from the BluetoothService Handler
  public static final String DEVICE_NAME = "device_name";
  public static final String TOAST = "toast";

  // Member fields
  private final Context mContext;
  private final BluetoothAdapter mAdapter;
  private final Handler mHandler;
  private AcceptThread mAcceptThread             = null;
  private ConnectThread mConnectThread           = null;
  private ConnectedThread mConnectedThread       = null;
  private BluetoothDevice mConnectedRemoteDevice = null;
  private int mState                             = STATE_NONE;
  private boolean mIsServer                      = false;
  private Timer mReconnectTimer                  = null;
  
  // Connection retry time to a remote client in milliseconds.
  private final int RETRY_TIME                   = 20000;
  
  // Constants that indicate the current connection state
  public static final int STATE_NONE = 0;       // we're doing nothing
  public static final int STATE_LISTEN = 1;     // now listening for incoming connections
  public static final int STATE_CONNECTING = 2; // now initiating an outgoing connection
  public static final int STATE_CONNECTED = 3;  // now connected to a remote device

  /**
   * Constructor. Prepares a new BluetoothService session. The session can be used as
   * server or client.
   * 
   * @param context  The UI Activity Context
   * @param handler  A Handler to send messages back to the UI Activity
   */
  public BluetoothService(Context context, Handler handler)
		{
			mContext = context;
			mAdapter = BluetoothAdapter.getDefaultAdapter();
			mState = STATE_NONE;
			mHandler = handler;
		}

	/**
	 * Set the current state of the chat connection
	 * 
	 * @param state An integer defining the current connection state
	 */
	private synchronized void setState(int state)
		{
			if(D) Log.d(TAG, "setState() " + mState + " -> " + state);

			mState = state;

			// Give the new state to the Handler so the UI Activity can update
			mHandler.obtainMessage(BluetoothService.MESSAGE_STATE_CHANGE, state, -1).sendToTarget();
		}

	/**
	 * @return The current connection state.
	 */
	public synchronized int getState()
		{
			return mState;
		}

	/**
	 * @return The connected device name.
	 */
	public String getConnectedDeviceName()
  	{
  		if( mConnectedRemoteDevice != null )
  			{
  				return mConnectedRemoteDevice.getName();
  			}
  		
  		return "";
  	}
	
	/**
	 * @return The MAC of the connected device.
	 */
	public String getConnectedMac()
    {
  		if( mConnectedRemoteDevice != null )
  			{
  				return mConnectedRemoteDevice.getAddress();
  			}
  		
    	return "";
    }

  /**
   * Setup a Bluetooth server socket for incoming connections. Specifically start
   * AcceptThread to begin a session in listening (server) mode.
   */
	public synchronized void start()
		{
			if(D) Log.d(TAG, "start");

			// Cancel any thread attempting to make a connection
			if (mConnectThread != null)
				{
					mConnectThread.cancel();
					mConnectThread = null;
				}

			// Cancel any thread currently running a connection
			if (mConnectedThread != null)
				{
					mConnectedThread.cancel();
					mConnectedThread = null;
				}

			setState( STATE_LISTEN );

			// Start the thread to listen on a BluetoothServerSocket
			if (mAcceptThread == null)
				{
					mAcceptThread = new AcceptThread();
					mAcceptThread.start();
				}
			
			mIsServer = true;
		}

	/**
	 * Start the ConnectThread to initiate a connection to a remote device.
	 * 
	 * @param device The BluetoothDevice to connect
	 */
	public synchronized void connect(BluetoothDevice device)
		{
			if(D) Log.d(TAG, "Connecting to: " + device);

			mConnectedRemoteDevice = device;
			
			// Cancel any thread attempting to make a connection
			if (mState == STATE_CONNECTING)
				{
					if (mConnectThread != null)
						{
							mConnectThread.cancel();
							mConnectThread = null;
						}
				}

			// Cancel any thread currently running a connection
			if (mConnectedThread != null)
				{
					mConnectedThread.cancel();
					mConnectedThread = null;
				}

			// Start the thread to connect with the given device
			mConnectThread = new ConnectThread(device);
			mConnectThread.start();
			setState(STATE_CONNECTING);
		}

	/**
	 * Start the ConnectedThread to begin managing a Bluetooth connection
	 * 
	 * @param socket The BluetoothSocket on which the connection was made
	 * @param device The BluetoothDevice that has been connected
	 */
	public synchronized void connected( BluetoothSocket socket,
	                                    BluetoothDevice device )
		{
			if(D) Log.d(TAG, "connected");

			// Cancel the thread that completed the connection
			if (mConnectThread != null)
				{
					mConnectThread.cancel();
					mConnectThread = null;
				}

			// Cancel any thread currently running a connection
			if (mConnectedThread != null)
				{
					mConnectedThread.cancel();
					mConnectedThread = null;
				}

			// Cancel the accept thread because we only want to connect to one device
			if (mAcceptThread != null)
				{
					mAcceptThread.cancel();
					mAcceptThread = null;
				}

			// Start the thread to manage the connection and perform transmissions
			mConnectedThread = new ConnectedThread(socket);
			mConnectedThread.start();

			// Send the name of the connected device back to the UI Activity
			Message msg = mHandler.obtainMessage(BluetoothService.MESSAGE_DEVICE_NAME);
			Bundle bundle = new Bundle();
			bundle.putString(BluetoothService.DEVICE_NAME, device.getName());
			msg.setData(bundle);
			mHandler.sendMessage(msg);

			setState(STATE_CONNECTED);
		}

	/**
	 * Stop all threads
	 */
	public synchronized void stop()
		{
			if(D) Log.d(TAG, "stop");

			if (mConnectThread != null)
				{
					mConnectThread.cancel();
					mConnectThread = null;
				}

			if (mConnectedThread != null)
				{
					mConnectedThread.cancel();
					mConnectedThread = null;
				}

			if (mAcceptThread != null)
				{
					mAcceptThread.cancel();
					mAcceptThread = null;
				}
			
			if( mReconnectTimer != null )
			  {
			    mReconnectTimer.cancel();
			    mReconnectTimer = null;
			  }
			
			setState(STATE_NONE);
			mIsServer = false;
		}

	/**
	 * Write to the ConnectedThread in an unsynchronized manner
	 * 
	 * @param out The bytes to write
	 *
	 * @see ConnectedThread#write(byte[])
	 */
	public void write(byte[] out)
		{
			// Create temporary object
			ConnectedThread r;
			
			// Synchronize a copy of the ConnectedThread
			synchronized (this)
				{
					if (mState != STATE_CONNECTED)
						{
							return;
						}
					
					r = mConnectedThread;
				}
			
			// Perform the write unsynchronized
			r.write(out);
		}

	/**
	 * Indicates that the connection attempt to the remote device failed and notifies
	 * the UI Activity.
	 */
	private void connectionFailed()
		{
			// Send a failure message back to the Activity
			Message msg = mHandler.obtainMessage(BluetoothService.MESSAGE_TOAST);
			Bundle bundle = new Bundle();
			bundle.putString(BluetoothService.TOAST, mContext.getString(R.string.btError1));
			msg.setData(bundle);
			mHandler.sendMessage(msg);

			// Setup a timer for a reconnection try.
			connectionRetry();
		}

	/**
	 * Indicates that the connection was lost and notifies the UI Activity.
	 */
	private void connectionLost()
		{
			// Send a failure message back to the Activity
			Message msg = mHandler.obtainMessage(BluetoothService.MESSAGE_TOAST);
			Bundle bundle = new Bundle();
			bundle.putString(BluetoothService.TOAST, mContext.getString(R.string.btError2));
			msg.setData(bundle);
			mHandler.sendMessage(msg);

      if( mIsServer )
        {
          // Start the service again to open a listening socket.
          BluetoothService.this.start();
        }
      else
        {
          // Initiate a connection retry to the remote device.
          connectionRetry();
        }
		}
	
	/**
	 * Setup a timer for an outgoing connection retry to a remote device after error.
	 */
	synchronized void connectionRetry()
	  {
	    Log.d(TAG, "Connection retry scheduled to " + mConnectedRemoteDevice.getAddress());
	    
      if( mReconnectTimer == null && mConnectedRemoteDevice != null )
        {
          mReconnectTimer = new Timer( "BtReconnect" );
        }
      
      mReconnectTimer.schedule( new TimerTask() { 
                        public void run() {
                          // Attempt to reconnect to the device
                          connect( mConnectedRemoteDevice );
                        } 
                      }, RETRY_TIME );	    
	  }

	/**
	 * This thread runs while listening for incoming connections. It behaves like
	 * a server-side client. It runs until a connection is accepted (or until cancelled).
	 */
	private class AcceptThread extends Thread
	{
		// The local server socket
		private final BluetoothServerSocket mmServerSocket;

		public AcceptThread()
			{
				BluetoothServerSocket tmp = null;
				
				// Create a new listening server socket
				try
					{
						tmp = mAdapter.listenUsingRfcommWithServiceRecord( SDP_NAME, SPP_UUID );
					}
				catch (IOException e)
					{
						Log.e(TAG, "AcceptThread: listen() failed", e);
					}
				
				mmServerSocket = tmp;
			}

		public void run()
			{
				if(D) Log.d(TAG, "BEGIN mAcceptThread " + this);
				
				setName("BtAcceptThread");

				BluetoothSocket socket = null;

				// Listen to the server socket if we're not connected
				while ( mState != STATE_CONNECTED )
					{
						try
							{
	               // if(D) Log.v(TAG, "entering accept()");

								// This is a blocking call and will only return on a
								// successful connection or an exception
								socket = mmServerSocket.accept();
								
                // if(D) Log.v(TAG, "left accept()");
							}
						catch (IOException e)
							{
								Log.e(TAG, "AcceptThread: accept() failed", e);
								break;
							}

						// If a connection was accepted
						if (socket != null)
							{							  
							  Log.v(TAG, "accept(): Device=" + socket.getRemoteDevice().getName() +
							             "Mac=" + socket.getRemoteDevice().getAddress() );
							  
								synchronized (BluetoothService.this)
									{
										switch (mState)
											{
											case STATE_LISTEN:
											case STATE_CONNECTING:
												// Situation normal. Start the connected thread.
												connected(socket, socket.getRemoteDevice());
												break;
											case STATE_NONE:
											case STATE_CONNECTED:
												// Either not ready or already connected. Terminate new
												// socket.
												try
													{
														socket.close();
													}
												catch (IOException e)
													{
														Log.e(TAG, "AcceptThread: Could not close unwanted socket", e);
													}
												break;
											}
									}
							}
					}
				
				if(D) Log.i(TAG, "END mAcceptThread");
			}

		public void cancel()
			{
				if(D) Log.d(TAG, "AcceptThread: cancel " + this);
				
				if( mmServerSocket == null )
  				{
  				  return;
  				}
				
				try
					{
						mmServerSocket.close();
					}
				catch (IOException e)
					{
						Log.e(TAG, "AcceptThread: close() of server socket failed", e);
					}
			}
	} // End of private class

	/**
	 * This thread runs while attempting to make an outgoing connection with a
	 * device. It runs straight through; the connection either succeeds or fails.
	 */
	private class ConnectThread extends Thread
	{
		private BluetoothSocket mmSocket    = null;
		private BluetoothDevice mmDevice    = null;

		/**
		 * 
		 * @param device The remote device to be connected.
		 */
		public ConnectThread(BluetoothDevice device)
			{
				mmDevice = device;

				BluetoothSocket tmp = null;
				
				// Get a BluetoothSocket for a connection with the given BluetoothDevice
				try
					{
						tmp = device.createRfcommSocketToServiceRecord(SPP_UUID);
					}
				catch (IOException e)
					{
						Log.e(TAG, "ConnectThread: create() failed", e);
					}
				
				mmSocket = tmp;
			}

		public void run()
			{
				if(D) Log.i(TAG, "BEGIN mConnectThread");
				
				setName("BtConnectThread");

				// Always cancel discovery because it will slow down a connection
				mAdapter.cancelDiscovery();

				// Make a connection to the BluetoothSocket
				try
					{
						// This is a blocking call and will only return on a
						// successful connection or an exception is thrown.
		        // if(D) Log.v(TAG, "ConnectThread enter connect()");
						mmSocket.connect();
						// if(D) Log.v(TAG, "ConnectThread left connect()");
					}
				catch (IOException e)
					{
            Log.e( TAG, "ConnectThread: Unable to connect() to device='" +
                   mmDevice.getName() + "' MAC=" + mmDevice.getAddress() +
                   " " + e.toString() );

						// Close the socket
						try
							{
								mmSocket.close();
							}
						catch (IOException e2)
							{
								Log.e( TAG, "ConnectThread: close failed: ", e2 );
							}
						
						connectionFailed();
						return;
					}

				// Reset the ConnectThread because we're done
				synchronized (BluetoothService.this)
					{
						mConnectThread = null;
						
		        // Reset connection Timer retry
		        if( mReconnectTimer != null )
		          {
		            mReconnectTimer.cancel();
		            mReconnectTimer = null;
		          }
					}

				// Start the connected thread
				connected(mmSocket, mmDevice);
			}

		public void cancel()
			{
	      if(D)
	         {
	           Log.d(TAG, "ConnectThread: cancel " + this);
	         }

			  if( mmSocket == null )
			    {
			      return;
			    }
			  
				try
					{
						mmSocket.close();
					}
				catch (IOException e)
					{
						Log.e(TAG, "ConnectThread: close failed: ", e);
					}
			}
	} // End of private class

	/**
	 * This thread runs during a connection with a remote device. It handles all
	 * incoming and outgoing transmissions.
	 */
	private class ConnectedThread extends Thread
	{
		private final BluetoothSocket     mmSocket;
		private final BufferedInputStream mmInReader;
		private final OutputStream        mmOutStream;
		private boolean                   mAbort;
		
		/**
		 * @param socket An opened Bluetooth socket.
		 */
		public ConnectedThread(BluetoothSocket socket)
			{
				if(D) Log.d(TAG, "create ConnectedThread");
				
				mmSocket            = socket;
				InputStream  tmpIn  = null;
				OutputStream tmpOut = null;
				mAbort              = false;

				// Get the BluetoothSocket input and output streams
				try
					{
						tmpIn  = socket.getInputStream();
						tmpOut = socket.getOutputStream();
					}
				
				catch (IOException e)
					{
						Log.e(TAG, "ConnectedThread: temp socket streams not created", e);
					}
				
				mmInReader  = new BufferedInputStream((tmpIn));
				mmOutStream = tmpOut;
			}

		public void run()
			{
				if(D) Log.i(TAG, "BEGIN mConnectedThread");
				
				setName("BtConnectedThread");
				
				// Keep listening to the InputStream while connected
				while (true)
					{
						try
							{
								/*
								// Read a line from the InputStream
								String line = mmInReader.readLine();
								
								if( line.trim().length() == 0 )
									{
										continue;
									}
								
								CumulusActivity.nativeNmeaString( line );
								*/
								
								// Read a character from the input stream and pass it via JNI
								// to the native C++ part.
								int character = mmInReader.read();
								
								if( character == -1 )
									{
										Log.w(TAG, "BtConnectedThread read -1 (EOF)");
										
										if( getAbort() == false )
										  {
										    // Only notify a connection lost, if the socket was not closed
										    // by our software.
										    connectionLost();
										  }

										break;
									}
								
								CumulusActivity.nativeByteFromGps( (byte) (character & 0xff) );
							}
						
						catch (IOException e)
							{								
								if( getAbort() == false )
								  {
								    // Only notify a connection lost, if the socket was not closed
								    // by our software.
										Log.e(TAG, "ConnectedThread: read failed", e);
								    connectionLost();
								  }
								
								break;
							}
					}
			}

		/**
		 * Write to the connected OutStream.
		 * 
		 * @param buffer The bytes to write
		 */
		public void write(byte[] buffer)
			{
				try
					{
						mmOutStream.write(buffer);

						// Share the sent message back to the UI Activity
						// mHandler.obtainMessage(BluetoothService.MESSAGE_WRITE, -1, -1, buffer).sendToTarget();
					}
				catch (IOException e)
					{
						Log.e(TAG, "ConnectedThread: write failed", e);
					}
			}

		/**
		 * @param abortFlag Sets the abort flag to the passed value.
		 */
		synchronized private void setAbort( boolean abortFlag )
		  {
		    mAbort = abortFlag;
		  }
		
		/*
		 * @return The content of the abort flag.
		 */
    synchronized private boolean getAbort()
      {
        return mAbort;
      }

    public void cancel()
			{
        if(D) Log.d(TAG, "ConnectedThread: cancel " + this);

				try
					{
					  setAbort( true );
						mmSocket.close();
						
						if( mmInReader != null )
							{
								mmInReader.close();
							}
						
						if( mmOutStream != null )
              {
                mmOutStream.close();
              }
					}
				catch (IOException e)
					{
						Log.e(TAG, "ConnectedThread: close failed", e);
					}
			}
	} // End of private class
	
} // End of class
