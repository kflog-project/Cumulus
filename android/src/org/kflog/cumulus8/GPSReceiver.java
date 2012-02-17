/***********************************************************************
 **
 **   GPSReceiver.java
 **
 **   This file is part of Cumulus4Android
 **
 ************************************************************************
 **
 **   Copyright (c):  2010-2012 by Josua Dietze
 **                   2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

package org.kflog.cumulus8;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class GPSReceiver extends BroadcastReceiver
{
	private boolean fix = false;

	public boolean gpsFix()
	{
		return fix;
	}

	@Override
	public void onReceive(Context context, Intent intent)
	{
		fix = intent.getBooleanExtra("enabled", false);
		Log.i( "Java#Cumulus",
		       "GPS Receiver: fix is " + intent.getBooleanExtra("enabled", false));
	}
}
