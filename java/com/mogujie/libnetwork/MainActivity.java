package com.mogujie.libnetwork;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.mogujie.io.NetCheckResult;
import com.mogujie.io.Network;
import com.mogujie.io.NetworkService;

public class MainActivity extends Activity implements OnClickListener {

    private Button btnConnect = null;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        startServices();
        setContentView(R.layout.activity_main);
        btnConnect = (Button) findViewById(R.id.btnConnect);
        btnConnect.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.btnConnect) {
            btnConnect.setClickable(false);
//            String LoginIp1 = "221.228.199.41";
            String LoginIp1 = "10.11.2.222";
            int LoginPort = 8001;

            int nHandle = Network.getInstance().connect(LoginIp1, LoginPort);

            int status = Network.getInstance().getStatus(nHandle);
            Log.e("MainActivity", "socket status:" + String.valueOf(status));
            String s = "GET /index.html HTTP/1.1 \r\n";
            byte[] testMsg = s.getBytes();

            byte[] result =  Network.getInstance().netCheck(LoginIp1, LoginPort, testMsg, testMsg.length);


            Log.e("MainActivity", new String(result));
        }
    }

    private void startServices() {
        Intent intentService = new Intent(getApplicationContext(), NetworkService.class);
        startService(intentService);
    }
}
