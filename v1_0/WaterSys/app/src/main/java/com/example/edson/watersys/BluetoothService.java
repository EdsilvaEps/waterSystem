package com.example.edson.watersys;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.SyncStateContract;
import android.util.Log;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;

public class BluetoothService {
    private static final String TAG = "BluetoothChatService";
    // Member fields
    private final BluetoothAdapter mAdapter;
    private final Handler mHandler;
    private ConnectThread mConnectThread;
    private ConnectedThread mConnectedThread;
    Context mContext;
    public UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private int mState;
    private int mNewState;
    private static final String targetDev = "HC-06";
    private Set<BluetoothDevice> pairedDevices;
    private BluetoothSocket mmSocket = null;

    // Constants that indicate the current connection state
    public static final int STATE_NONE = 6;       // we're doing nothing
    public static final int STATE_LISTEN = 7;     // now listening for incoming connections
    public static final int STATE_CONNECTING = 8; // now initiating an outgoing connection
    public static final int STATE_CONNECTED = 9;  // now connected to a remote device

    /**
     * Constructor. Prepares a new BluetoothChat session.
     *
     * @param context The UI Activity Context
     * @param handler A Handler to send messages back to the UI Activity
     */
    public BluetoothService(Context context, Handler handler){
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mHandler = handler;
        mContext = context;
        mState = STATE_NONE;
        mNewState = mState;

    }


    /**
     * to make toast messages more easily
     * @param tx string message
     */
    private void mssg(String tx){
        Toast.makeText(mContext, tx, Toast.LENGTH_LONG).show();
    }

    /**
     * Return the current connection state.
     */
    public synchronized int getState() {
        return mState;
    }

    /**
     * Start the bt service
     *
     */
    public boolean start(){
        // Cancel any thread attempting to make a connection
        if (mConnectThread != null) {
            mConnectThread.cancel();
            mConnectThread = null;
        }

        // Cancel any thread currently running a connection
        if (mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        return getDevicePaired();

    }

    /**
     * searches for our target device from the list of paired
     * devices
     * @return whether was the device there or not
     */
    private boolean getDevicePaired() {
        //mssg("Searching device...");
        pairedDevices = mAdapter.getBondedDevices();

        if (pairedDevices.size() > 0) {
            // There are paired devices. Get the name and address of each paired device.
            for (BluetoothDevice device : pairedDevices) {
                String deviceName = device.getName();
                String deviceHardwareAddress = device.getAddress(); // MAC address

                if(deviceName.equals(targetDev)){
                    //connect to device
                    //mssg("Device found, trying to connect...");
                    connect(device);
                    return true;

                }
            }
        }
        //mssg("Device not found, discovering...");
        return false;

    }


    /**
     * Start the ConnectThread to initiate a connection to a remote device.
     *
     * @param device The BluetoothDevice to connect
     */
    public void connect(BluetoothDevice device) {
        Log.d(TAG, "connect to: " + device.getName());

        // Cancel any thread attempting to make a connection
        if (mState == STATE_CONNECTING) {
            if (mConnectThread != null) {
                mConnectThread.cancel();
                mConnectThread = null;
            }
        }


        // Cancel any thread currently running a connection
        if (mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        // Start the thread to connect with the given device
        mConnectThread = new ConnectThread(device);
        mConnectThread.start();
    }



    /**
     * Start the ConnectedThread to begin managing a Bluetooth connection
     *
     * @param socket The BluetoothSocket on which the connection was made
     * @param device The BluetoothDevice that has been connected
     */
    public void connected(BluetoothSocket socket, BluetoothDevice device){
        // Cancel the thread that completed the connection
        //mssg("Connected");
        Log.d(TAG, "Connected");
        /*if(mConnectThread != null){
            mConnectThread.cancel();
            mConnectThread = null;
        }*/

        // Cancel any thread currently running a connection
        if (mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        mConnectedThread = new ConnectedThread(socket);
        mConnectedThread.start();
    }

    /**
     * Stop all threads
     */
    public void stop() {
        Log.d(TAG, "stop");
        mState = STATE_NONE;
        Message readMsg = mHandler.obtainMessage(Constants.MESSAGE_STATE_CHANGE, BluetoothService.STATE_NONE);
        readMsg.sendToTarget();

        if (mConnectThread != null) {
            mConnectThread.cancel();
            mConnectThread = null;
        }

        if (mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }




    }

    /**
     * Write to the ConnectedThread in an unsynchronized manner
     *
     * @param out The bytes to write
     *
     */
    public void write(byte[] out) {
        // Create temporary object
        ConnectedThread r;
        // Synchronize a copy of the ConnectedThread
        synchronized (this) {
            if (mState != STATE_CONNECTED) return;
            r = mConnectedThread;
        }
        // Perform the write unsynchronized
        r.write(out);
    }



    /**
     * This thread runs while attempting to make an outgoing connection
     * with a device. It runs straight through; the connection either
     * succeeds or fails.
     */
    private class ConnectThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final BluetoothDevice mmDevice;

        public ConnectThread(BluetoothDevice device) {
            // Use a temporary object that is later assigned to mmSocket
            // because mmSocket is final.
            BluetoothSocket tmp = null;
            mmDevice = device;
            Log.e(TAG, "Attempting to connect");

            try {
                // Get a BluetoothSocket to connect with the given BluetoothDevice.
                // MY_UUID is the app's UUID string, also used in the server code.
                tmp = device.createInsecureRfcommSocketToServiceRecord(MY_UUID);
            } catch (IOException e) {
                Log.e(TAG, "Socket's create() method failed", e);
            }
            mmSocket = tmp;
            mState = STATE_CONNECTING;
            Message readMsg = mHandler.obtainMessage(Constants.MESSAGE_STATE_CHANGE, mState);
            readMsg.sendToTarget();
            Log.e(TAG, "state connecting");

        }

        public void run() {
            // Cancel discovery because it otherwise slows down the connection.
            mAdapter.cancelDiscovery();
            Log.e(TAG, "cancel discovery");

            try {
                // Connect to the remote device through the socket. This call blocks
                // until it succeeds or throws an exception.
                mmSocket.connect();
            } catch (IOException connectException) {
                // Unable to connect; close the socket and return.
                Log.e(TAG, "connect failed", connectException);
                mState = STATE_NONE;
                Message readMsg = mHandler.obtainMessage(Constants.MESSAGE_STATE_CHANGE, mState);
                readMsg.sendToTarget();
                Log.e(TAG, "state connecting");
                try {
                    mmSocket.close();
                } catch (IOException closeException) {
                    Log.e(TAG, "Could not close the client socket", closeException);
                }
                return;
            }


            // The connection attempt succeeded. Perform work associated with
            // the connection in a separate thread.
            //mssg("Device connected");
            connected(mmSocket,mmDevice);
        }

        // Closes the client socket and causes the thread to finish.
        public void cancel() {
            try {
                mState = STATE_NONE;
                Message readMsg = mHandler.obtainMessage(Constants.MESSAGE_STATE_CHANGE, BluetoothService.STATE_NONE);
                readMsg.sendToTarget();
                mmSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "Could not close the client socket", e);
            }
        }
    }

    /**
     * This thread runs during a connection with a remote device.
     * It handles all incoming and outgoing transmissions.
     */
    private class ConnectedThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;
        private byte[] mmBuffer; // mmBuffer store for the stream

        public ConnectedThread(BluetoothSocket socket) {
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the input and output streams; using temp objects because
            // member streams are final.
            try {
                tmpIn = socket.getInputStream();
            } catch (IOException e) {
                Log.e(TAG, "Error occurred when creating input stream", e);
            }
            try {
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                Log.e(TAG, "Error occurred when creating output stream", e);
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
            mState = STATE_CONNECTED;
            Message readMsg = mHandler.obtainMessage(Constants.MESSAGE_STATE_CHANGE, mState);
            readMsg.sendToTarget();
            Log.e(TAG, "state connected");
        }

        public void run() {
            mmBuffer = new byte[1024];
            int numBytes; // bytes returned from read()

            final byte delimiter = 10; //This is the ASCII code for a newline character
            int readBufferPosition = 0;

            // Keep listening to the InputStream until an exception occurs.
            while (true) {
                try {

                    int bytesAvailable = mmInStream.available();
                    if (bytesAvailable > 0){

                        byte[] packetBytes = new byte[bytesAvailable];

                        mmInStream.read(packetBytes);

                        for (int i=0; i<bytesAvailable; i++){

                            byte b = packetBytes[i];

                            if (b == delimiter){

                                byte[] encodedBytes = new byte[readBufferPosition];
                                System.arraycopy(mmBuffer, 0, encodedBytes, 0, encodedBytes.length);
                                final String data = new String(encodedBytes, "US-ASCII");
                                readBufferPosition = 0;

                                //Message readMsg = mHandler.obtainMessage(Constants.MESSAGE_READ, -1, -1, data);
                                //readMsg.sendToTarget();

                                Message msg = mHandler.obtainMessage(Constants.MESSAGE_READ);
                                Bundle bundle = new Bundle();
                                bundle.putString(Constants.MESSAGE, data);
                                msg.setData(bundle);
                                mHandler.sendMessage(msg);

                            }
                            else mmBuffer[readBufferPosition++] = b;
                        }
                    }


                    /*// Read from the InputStream.
                    numBytes = mmInStream.read(mmBuffer);
                    // Send the obtained bytes to the UI activity.
                    Message readMsg = mHandler.obtainMessage(
                            Constants.MESSAGE_READ, numBytes, -1,
                            mmBuffer);
                    readMsg.sendToTarget();*/
                } catch (IOException e) {
                    Log.d(TAG, "Input stream was disconnected", e);
                    break;
                }
            }
        }

        // Call this from the main activity to send data to the remote device.
        public void write(byte[] bytes) {
            try {
                mmOutStream.write(bytes);

                // Share the sent message with the UI activity.
                Message writtenMsg = mHandler.obtainMessage(
                        Constants.MESSAGE_WRITE, -1, -1, mmBuffer);
                writtenMsg.sendToTarget();
            } catch (IOException e) {
                Log.e(TAG, "Error occurred when sending data", e);

                // Send a failure message back to the activity.
                Message writeErrorMsg =
                        mHandler.obtainMessage(Constants.MESSAGE_TOAST);
                Bundle bundle = new Bundle();
                bundle.putString("toast",
                        "Couldn't send data to the other device");
                writeErrorMsg.setData(bundle);
                mHandler.sendMessage(writeErrorMsg);
            }


        }

        // Call this method from the main activity to shut down the connection.
        public void cancel() {
            try {

                mmSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "Could not close the connect socket", e);
            }
        }
    }




}
