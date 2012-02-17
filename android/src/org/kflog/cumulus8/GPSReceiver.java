package org.kflog.cumulus8;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class GPSReceiver extends BroadcastReceiver {
	private boolean fix = false;
	public boolean gpsFix() {
		return fix;
	}
	@Override
	public void onReceive(Context context, Intent intent) {
		fix = intent.getBooleanExtra("enabled", false);
		Log.i("Cumulus#Java", "GPS Receiver: fix is " + intent.getBooleanExtra("enabled", false));
	}
}
