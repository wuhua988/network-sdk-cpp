package com.mogujie.io;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

public class NetworkService extends Service{
	
	private NetworkServiceBinder binder = new NetworkServiceBinder();
	
	public class NetworkServiceBinder extends Binder {
		public NetworkService getService() {
			return NetworkService.this;
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return binder;
	}
	
	@SuppressLint("NewApi") @Override
	public void onCreate() {
		super.onCreate();
		Log.i("NetworkService", "onCreate");
		loadNDK();
		//make the service foreground, so stop "360 yi jian qingli"(a clean tool) to stop our app
		//todo eric study wechat's mechanism, use a better solution
		startForeground((int)System.currentTimeMillis(),new Notification());
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Log.i("NetworkService", "onStartCommand");
		loadNDK();
		return START_STICKY;
	}
	
	private void loadNDK()
	{
		Network.getInstance().initEnv();
		Network.getInstance().runEvent();
	}
}
