package com.mogujie.io;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class NetworkBroadcastReceiver extends BroadcastReceiver{
	
	public void onReceive (Context context, Intent intent) {  
        //从Intent中获取action 
		String strAction = intent.getAction();
		//Log.i("LibCoreBroadcastReceiver","onReceive--->action:" + strAction);
		// 当监听到 启动，解锁，锁屏的广播的时候，启动Service
		if(strAction.equals(Intent.ACTION_BOOT_COMPLETED) ||
			strAction.equals(Intent.ACTION_SCREEN_ON) ||
			strAction.equals(Intent.ACTION_SCREEN_OFF)) {
			Intent serviceIntent = new Intent(context,NetworkService.class);
			context.startService(serviceIntent);
		}
    }
}
