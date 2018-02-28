package com.example.edson.watersys;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.provider.SyncStateContract;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresApi;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Toolbar;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

public class MainC extends AppCompatActivity {

    Toolbar mainToolbar;
    FloatingActionButton connectBtn;
    Button summer, winter, nomode, waterPlants;
    TextView modetxt, modetextaux, status, winterTimeout, summerTimeout;

    BluetoothAdapter mBluetoothAdapter = null;
    Set<BluetoothDevice> pairedDevices;
    String targetDev = "HC-06";
    String lastMsg = "";
    ArrayList<String> msgBuffer = new ArrayList<>();


    public static final String TAG = "MainActivity";
    private final static int REQUEST_ENABLE_BT = 1;
    //private Handler mHandler; // handler that gets info from Bluetooth service
    private BluetoothService mBluetoothService = null;
    private StringBuffer mOutStringBuffer;






    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //mainToolbar = (Toolbar)findViewById(R.id.toolbar);


        connectBtn = findViewById(R.id.fab);
        summer = findViewById(R.id.button2);
        winter = findViewById(R.id.button3);
        nomode = findViewById(R.id.button4);
        waterPlants = findViewById(R.id.button1);

        modetxt = findViewById(R.id.textView2);
        modetextaux = findViewById(R.id.textView3);
        status = findViewById(R.id.textView5);

        summerTimeout = findViewById(R.id.textView10);
        winterTimeout = findViewById(R.id.textView9);

        summerTimeout.setVisibility(View.INVISIBLE);
        winterTimeout.setVisibility(View.INVISIBLE);

        requestBt();
        connectBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                if(mBluetoothAdapter == null){
                    requestBt();
                }

                if(mBluetoothService == null){
                    setupBTService();

                } else if(mBluetoothService.getState() == BluetoothService.STATE_CONNECTED){
                    mBluetoothService.stop();
                    status.setText("Disconnected");
                    blockButtons(false);
                }
                if(!mBluetoothService.start()) mBluetoothAdapter.startDiscovery();


            }
        });

        waterPlants.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendMessage("1");

            }
        });

        summer.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                selectMode(Constants.COMMAND_SUMMER_MODE);

            }
        });

        winter.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                selectMode(Constants.COMMAND_WINTER_MODE);
            }
        });

        nomode.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                selectMode(Constants.COMMAND_NOMODE);
            }
        });

        IntentFilter filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        registerReceiver(mReceiver, filter);


    }

    private void requestBt() {
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            // Device doesn't support Bluetooth
            finish();
        }

        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

    }


    // Create a BroadcastReceiver for ACTION_FOUND.
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                // Discovery has found a device. Get the BluetoothDevice
                // object and its info from the Intent.
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                String deviceName = device.getName();
                String deviceHardwareAddress = device.getAddress(); // MAC address
                if(deviceName.equals(targetDev)){
                    mssg("Device discovered, trying to connect...");
                    mBluetoothService.connect(device);
                }
            }
        }
    };


    private void mssg(String tx){
        Toast.makeText(getApplicationContext(), tx, Toast.LENGTH_SHORT).show();
    }

    /**
     * setup operations with bluetooth
     */
    private void setupBTService(){

        mBluetoothService = new BluetoothService(getApplicationContext(), mHandler);
        // Initialize the buffer for outgoing messages

        mOutStringBuffer = new StringBuffer("");

        if(mBluetoothService.getState() == BluetoothService.STATE_NONE) status.setText("Disconnected");





    }



    /**
     * Sends a message.
     *
     * @param message A string of text to send.
     */
    private void sendMessage(String message) {
        // Check that we're actually connected before trying anything
        if (mBluetoothService == null) {
            mssg("Not connected");
            System.out.println("not connected");
            return;
        }

        if (mBluetoothService.getState() != BluetoothService.STATE_CONNECTED) {
            mssg("Not connected");
            System.out.println("not connected");
            return;
        }

        if(message.equals(Constants.COMMAND_WATER)) lastMsg = message;
        else if(message.equals(Constants.COMMAND_GETDATA)) lastMsg = message;

        // Check that there's actually something to send
        if (message.length() > 0) {
            // Get the message bytes and tell the BluetoothService to write
            byte[] send = message.getBytes();
            mBluetoothService.write(send);

            // Reset out string buffer to zero
            mOutStringBuffer.setLength(0);
        }
    }





    @Override
    public void onResume() {
        super.onResume();

        // Performing this check in onResume() covers the case in which BT was
        // not enabled during onStart(), so we were paused to enable it...
        // onResume() will be called when ACTION_REQUEST_ENABLE activity returns.
        if (mBluetoothService != null) {
            // Only if the state is STATE_NONE, do we know that we haven't started already
            if (mBluetoothService.getState() == BluetoothService.STATE_NONE) {
                // Start the Bluetooth chat services
                mBluetoothService.start();
            }
        }
    }


    /**
     * The Handler that gets information back from the BluetoothChatService
     */
    @SuppressLint("HandlerLeak")
    private final Handler mHandler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            switch (msg.what){
                case Constants.MESSAGE_STATE_CHANGE:

                    int mState = mBluetoothService.getState();
                    if(mState == BluetoothService.STATE_NONE) status.setText("Disconnected");
                    if(mState == BluetoothService.STATE_CONNECTED){
                        status.setText("Connected");
                        delayMili(500);
                        MainC.this.sendMessage(Constants.COMMAND_GETDATA);

                    }
                    if(mState == BluetoothService.STATE_CONNECTING) status.setText("Connecting...");

                    break;

                case Constants.MESSAGE_WRITE:
                    byte[] writeBuf = (byte[]) msg.obj;
                    // construct a string from the buffer
                    //lastMsg = new String(writeBuf);
                    //System.out.println(lastMsg);
                    break;
                case Constants.MESSAGE_READ:
                    //byte[] readBuf = (byte[]) msg.obj;
                    // construct a string from the valid bytes in the buffer

                    //String readMsg = new String (readBuf, 0, msg.arg1);
                    //System.out.println(msg.obj);

                    processResponse(msg.getData().getString(Constants.MESSAGE));
                    break;
                case Constants.MESSAGE_TOAST:
                    mssg(msg.getData().toString());
            }



        }
    };

    /**
     * for easy delaying of thread
     * @param timeout timeout of delay
     */
    private void delayMili(long timeout){
        try {
            TimeUnit.MILLISECONDS.sleep(timeout);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    /**
     * Select mode of operation of the system
     */
    private void selectMode(String mode){
        if (mBluetoothService == null) {
            mssg("Not connected");
            System.out.println("not connected");
            return;
        }

        if (mBluetoothService.getState() != BluetoothService.STATE_CONNECTED){
            mssg("not connected");
            return;
        }

        if(mode.contains(Constants.COMMAND_SUMMER_MODE)){
            sendMessage(Constants.COMMAND_SUMMER_MODE);
            summer.setBackgroundResource(R.drawable.buttonshape);
            winter.setBackgroundResource(R.drawable.buttonshapedeact);
            nomode.setBackgroundResource(R.drawable.buttonshapedeact);
            modetxt.setText("SUMMER MODE");
            modetextaux.setText("Waters every two days");
            delayMili(30);
            sendMessage(Constants.COMMAND_GETDATA);
        }

        else if(mode.contains(Constants.COMMAND_WINTER_MODE)){
            sendMessage(Constants.COMMAND_WINTER_MODE);
            summer.setBackgroundResource(R.drawable.buttonshapedeact);
            winter.setBackgroundResource(R.drawable.buttonshape);
            nomode.setBackgroundResource(R.drawable.buttonshapedeact);
            modetxt.setText("WINTER MODE");
            modetextaux.setText("Waters weekly");
            delayMili(30);
            sendMessage(Constants.COMMAND_GETDATA);
        }

        else if(mode.contains(Constants.COMMAND_NOMODE) || mode.contains("0")){
            sendMessage(Constants.COMMAND_NOMODE);
            summer.setBackgroundResource(R.drawable.buttonshapedeact);
            winter.setBackgroundResource(R.drawable.buttonshapedeact);
            nomode.setBackgroundResource(R.drawable.buttonshape);
            modetxt.setText("MANUAL MODE");
            modetextaux.setText("Waters when you click");
            delayMili(30);
            sendMessage(Constants.COMMAND_GETDATA);
        }


    }

    /**
     * handle answers from the bt system based on
     * previous commands sent
     */
    private void processResponse(String msg){
        if (lastMsg.equals(Constants.COMMAND_GETDATA)){

            Log.d(TAG,"GETTING DATA");

            if (msg.contains("n")){

                System.out.println("end");

                System.out.println(msgBuffer);

                if (msgBuffer.get(0).contains("0")){
                    if(msgBuffer.size() > 2){
                        selectMode(msgBuffer.get(1));
                        String text = msgBuffer.get(2);
                        text = text.replace("\n", "").replace("\r", "");
                        calculateDeadline(Integer.valueOf(text), msgBuffer.get(1));
                    } else{
                        selectMode(msgBuffer.get(0));
                    }


                }
                else{
                    selectMode(msgBuffer.get(0));
                    String text = msgBuffer.get(1);
                    text = text.replace("\n", "").replace("\r", "");
                    calculateDeadline(Integer.valueOf(text), msgBuffer.get(0));
                }


                lastMsg = "";

                msgBuffer.clear();

            }

            else{

                msgBuffer.add(msg);

            }
        }

        if(lastMsg.equals(Constants.COMMAND_WATER)){
            Log.d(TAG,"WATERING...");

            blockButtons(true);

            if (msg.contains("n")){

                System.out.println("end");

                lastMsg = "";

                blockButtons(false);

            }

            else{

                System.out.println(msg);

                String m = "Watering..." +  msg;

                status.setText(m);

            }
        } else{
            Log.d(TAG,"RANDOM DATA");
            blockButtons(true);

            if(!msg.contains("n")){

                String m = "Occupied..." + msg;

                status.setText(m);

            } else blockButtons(false);

            System.out.println(msg);
        }


    }

    /**
     * prevent watering button from being clicked while watering operation is
     * happening
     * @param sw block and unblock
     */
    private void blockButtons(Boolean sw){
        if(sw){
            waterPlants.setBackgroundResource(R.drawable.buttonblocked);
            waterPlants.setEnabled(false);
            //status.setText("Watering...");
        } else{
            if(mBluetoothService.getState() == BluetoothService.STATE_CONNECTED) status.setText("Connected");
            if(mBluetoothService.getState() == BluetoothService.STATE_NONE) status.setText("Disconnected");
            if(mBluetoothService.getState() == BluetoothService.STATE_CONNECTING) status.setText("Connecting...");
            waterPlants.setBackgroundResource(R.drawable.buttonshapedoit);
            waterPlants.setEnabled(true);

        }
    }

    /**
     * calculates and displays the time remaining until the next watering of plants
     * @param deadline deadline in seconds
     * @param mode winter or summer mode
     */
    private void calculateDeadline(Integer deadline, String mode){
        int days = 0, hours = 0, minutes = 0;

        days = deadline / 60 / 60 / 24;
        hours = (deadline / 60 / 60) % 24;
        minutes = (deadline / 60) % 60;

        String text = days + "d " + hours + "h " + minutes + "m";

        if(mode.contains(Constants.COMMAND_WINTER_MODE)){
            summerTimeout.setVisibility(View.INVISIBLE);
            winterTimeout.setText(text);
            winterTimeout.setVisibility(View.VISIBLE);
        }
        else if(mode.contains(Constants.COMMAND_SUMMER_MODE)){
            winterTimeout.setVisibility(View.INVISIBLE);
            summerTimeout.setText(text);
            summerTimeout.setVisibility(View.VISIBLE);
        } else {
            winterTimeout.setVisibility(View.INVISIBLE);
            summerTimeout.setVisibility(View.INVISIBLE);
        }

    }





    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
        if(mBluetoothService != null){
            mBluetoothService.stop();
        }
    }
}
