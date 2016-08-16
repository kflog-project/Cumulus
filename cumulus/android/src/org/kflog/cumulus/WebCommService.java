/***********************************************************************
 **
 **   WebCommService.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c): 2016 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

package org.kflog.cumulus;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.text.SimpleDateFormat;
import java.util.Date;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import android.annotation.SuppressLint;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;

/**
 * This class does all the work for web communication and file download. It
 * uses http and https services.
 */
public class WebCommService
{
  // Logger tag
  private static final String TAG = "WebCommService";

  /**
   * Definition of Qt Network errors.
   */
  static final int NoError = 0;
    
  // network layer errors [relating to the destination server] (1-99):
  static final int ConnectionRefusedError = 1;
  static final int RemoteHostClosedError = 2;
  static final int HostNotFoundError = 3;
  static final int TimeoutError = 4;
  static final int OperationCanceledError = 5;
  static final int SslHandshakeFailedError = 6;
  static final int TemporaryNetworkFailureError = 7;
  static final int UnknownNetworkError = 99;
    
  // content errors (201-299):
  static final int ContentAccessDenied = 201;
  static final int ContentOperationNotPermittedError = 202;
  static final int ContentNotFoundError = 203;
  static final int AuthenticationRequiredError = 204;
  static final int ContentReSendError = 205;
  static final int UnknownContentError = 299;
    
  // protocol errors
  static final int ProtocolUnknownError = 301;
  static final int ProtocolInvalidOperationError = 302;
  static final int ProtocolInvalidUrlError = 303;
  static final int ProtocolFailure = 399;

  // Member fields
  private final Context mContext;
  
  // Return Class for webComm result
  public class WebCommResult
  {
    int    errorCode;
    String response;
  }
  
  /**
   * Constructor. Prepares a new WebCommService session.
   * 
   * @param context
   *          The UI Activity Context
   */
  public WebCommService(Context context)
  {
    mContext = context;
  }

  /**
   * Checks, if a network connectivity is available.
   * 
   * @return true in case of success otherwise false
   */
  public boolean isNetworkAvailable()
  {
  	ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
  	
  	NetworkInfo networkInfo = cm.getActiveNetworkInfo();
  	
  	// if no network is available networkInfo will be null
  	// otherwise check if we are connected
  	if (networkInfo != null &&  networkInfo.isConnected() )
  	{
  	  Log.i(TAG, "Network available and connected.");
  	  return true;
  	}
  
  	return false;
  }

  /**
   * Called to execute a HTTP file download.
   * 
   * @param url Url pointing to the source to be downloaded.
   * 
   * @param destination Full file path, where the downloaded file shall be stored.
   * 
   * @return Result code of download action.
   * 
   */
  @SuppressLint("SimpleDateFormat")
  int downloadFile( String urlIn, String destinationIn )
  {
    Log.i("DownloadFile", "Entry URL: " + urlIn + ", Dest: " + destinationIn);
      
    if( isNetworkAvailable() == false )
    {
      Log.e("DownloadFile", "No network available!");
      return TemporaryNetworkFailureError;
    }
    
      String strDate = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
  
      // Create a temporary filename by appending a date-time string
    String outputFileName = destinationIn + "_" + strDate;
    
    BufferedOutputStream outBuffer = null;
    
    try
    {
      outBuffer = new BufferedOutputStream(new FileOutputStream(outputFileName));
    } 
    catch (FileNotFoundException e)
    {
      Log.e("DownloadFile", "Cannot open output file: " + e.getMessage());
      return UnknownNetworkError;
    }
    
    URL url = null;
    
    try
    {
      url = new URL(urlIn);
    }
    catch( MalformedURLException e)
    {
      Log.e("DownloadFile", "Malformed URL:" + e.getMessage());
      try
      { 
        outBuffer.close();
      }
      catch (IOException e1) {}
      
      new File(outputFileName).delete();
      return UnknownNetworkError;
    }
    
    URLConnection urlConnection = null;
    
    try
    {
      urlConnection = url.openConnection();
    } 
    
    catch (IOException e)
    {
      Log.e("DownloadFile", "Open URL error:" + e.getMessage());
      
      try
      { 
        outBuffer.close();
      }
      
      catch (IOException e1) {}
      
      new File(outputFileName).delete();
      return UnknownNetworkError;
    }
    
    urlConnection.setReadTimeout(10000);
    urlConnection.setConnectTimeout(15000);
    
    try
    {
      urlConnection.setRequestProperty("User-Agent", "Cumulus/5.X (Qt/Android)");
    }
    catch (Exception e)
    {
      Log.e("DownloadFile", "setRequestProperty error" + e.getMessage());
      try
      { 
        outBuffer.close();
      }
      catch (IOException e1) {}
      
      new File(outputFileName).delete();
      return UnknownNetworkError;   
    }
  
    long total = 0;
    InputStream input = null;
      
    try
    {
      // open URL connection
      urlConnection.connect();
      
      // this will be useful so that you can show a typical 0-100% progress bar
      // int fileLength = urlConnection.getContentLength();
  
      // download the file
      input = new BufferedInputStream(urlConnection.getInputStream());
  
      byte data[] = new byte[8192];
      int count;
      
      while ((count = input.read(data)) != -1)
      {
        total += count;
        outBuffer.write(data, 0, count);
      }
  
      outBuffer.flush();
      outBuffer.close();
      input.close();
    }
    catch (IOException e)
    {
      Log.e("DownloadFile", "Read input stream failed:" + e.getMessage());
      try
      {
        input.close();
        outBuffer.close();
      }
      catch (IOException e1) {}
      
      new File(outputFileName).delete();
      return UnknownNetworkError;   
    }
  
    // Rename temporary download file
    new File(outputFileName).renameTo(new File(destinationIn));
    
    if( total == 0 )
    {
      return ContentNotFoundError;
    }
      
    Log.i("DownloadFile", "OK: Entry URL: " + urlIn + ", Dest: " + destinationIn);
  
    return NoError;
  }
  
  /*
   * Sends a https post request to the passed url and its parameters. The
   * response from the web server is returned.
   * 
   * @param urlIn Url pointing to the server to be called.
   * 
   * @param urlParamsIn URL parameters for the post call.
   * 
   * @return Result code and response text from the web server.
   */
  @SuppressLint("TrulyRandom")
  public WebCommResult sendHttpsRequest( String urlIn, String urlParamsIn )
  {
    WebCommResult result = new WebCommResult();
    URL url = null;
    HttpsURLConnection connection = null;
    
    try
    {
      url = new URL(urlIn);
    }
    catch( MalformedURLException e)
    {
      Log.e( TAG, "sendHttpRequest: " + e.getMessage());
      result.errorCode = ProtocolInvalidUrlError;
      result.response = e.getMessage();
      return result;
    }
    
    try
    {
      connection = (HttpsURLConnection) url.openConnection();
    }
    catch( IOException e )
    {
      Log.e( TAG, "sendHttpRequest: " + e.getMessage());
      result.errorCode = UnknownNetworkError;
      result.response = e.getMessage();
      return result;
    }
    
    try
    {
      // Create the SSL connection
      SSLContext sc;
      sc = SSLContext.getInstance("TLS");
      sc.init(null, null, new java.security.SecureRandom());
      connection.setSSLSocketFactory(sc.getSocketFactory());
      
      connection.setRequestMethod("POST");
      connection.setDoOutput(true);
      connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
      connection.setConnectTimeout(10000);
      connection.setReadTimeout(10000);
    }
    catch( Exception e )
    {
      Log.e( TAG, "sendHttpRequest: " + e.getMessage());
      result.errorCode = UnknownNetworkError;
      result.response = e.getMessage();
      return result;      
    }
      
    BufferedReader in = null;
    
    try
    {
      PrintWriter out = new PrintWriter(connection.getOutputStream());
      out.println(urlParamsIn);
      out.flush();
      out.close();
      
      in = new BufferedReader(new InputStreamReader(connection.getInputStream()), 8192);
      String inputLine;
      StringBuffer response = new StringBuffer();
      
      while ((inputLine = in.readLine()) != null)
      {
        response.append(inputLine);
      }
      
      result.errorCode = NoError;
      result.response = response.toString();
    }
      
    catch (Exception e)
    {
      result.errorCode = UnknownNetworkError;
      result.response = e.toString();
    }

    finally
    {
      if( in != null )
        {
          try
          {
            in.close();
          }
          
          catch (IOException e)
          {
            Log.e( TAG, "sendHttpRequest: " + e.getMessage() );
          }
        }
    }    
    
    return result;
  }

  /**
   * Disables the SSL certificate checking for new instances of {@link HttpsURLConnection}.
   * This has been created to aid testing on a local box, not for use on production.
   */
  public static void disableSSLCertificateChecking() {
      TrustManager[] trustAllCerts = new TrustManager[] { new X509TrustManager() {
          public X509Certificate[] getAcceptedIssuers() {
              return new X509Certificate[0];
          }

          @Override
          public void checkClientTrusted(X509Certificate[] arg0, String arg1) throws CertificateException {
              // Not implemented
          }

          @Override
          public void checkServerTrusted(X509Certificate[] arg0, String arg1) throws CertificateException {
              // Not implemented
          }
      } };

      HostnameVerifier hostnameVerifier = new HostnameVerifier() {
        @Override
        public boolean verify(String hostname, SSLSession session) {
            return true;
        }
      };

      try {
          SSLContext sc = SSLContext.getInstance("TLS");

          sc.init(null, trustAllCerts, new java.security.SecureRandom());

          HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());
          
          HttpsURLConnection.setDefaultHostnameVerifier(hostnameVerifier);
          
      } catch (KeyManagementException e) {
          e.printStackTrace();
      } catch (NoSuchAlgorithmException e) {
          e.printStackTrace();
      }
  }
  
}