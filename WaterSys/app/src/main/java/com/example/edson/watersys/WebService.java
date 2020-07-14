package com.example.edson.watersys;

import android.content.Context;
import android.net.wifi.WifiConfiguration;
import android.util.Log;
import android.widget.Toast;


import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.DisconnectedBufferOptions;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.io.IOException;
import java.io.InputStream;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.util.Properties;

import javax.net.ssl.SSLContext;

public class WebService {

    // source for TLS implementation:
    // https://github.com/eurbon/Paho-MQTT-Android-TCP-TLS-WSS-Example/

    public MqttAndroidClient mqttAndroidClient;
    public MqttConnectOptions mqttConnectOptions;

    // TODO: eventually, at some point, maybe, create a settings page to change those data below
    final String serverURI = "maqiatto.com:1883";
    //final String serverURI = "maqiatto.com:3883";
    final String clientId = "waterSys-application";
    final String user = "netosilvan78@gmail.com";
    final String password = "y4NW1jgJi3b7";

    final String subscribeTopic = Constants.level_route;
    //final Integer port = 18488;



    public WebService(final Context context){
        mqttAndroidClient = new MqttAndroidClient(context, serverURI ,clientId);
        mqttConnectOptions = new MqttConnectOptions();

        // setting user and pwd
        mqttConnectOptions.setUserName(user);
        mqttConnectOptions.setPassword(password.toCharArray());
        mqttConnectOptions.setAutomaticReconnect(true);
        mqttConnectOptions.setCleanSession(false);

        /**
         * SSL broker requires a certificate to authenticate their connection
         * Certificate can be found in resources folder /res/raw/
         */
        if(serverURI.contains("3883")){
            SocketFactory.SocketFactoryOptions socketFactoryOptions = new SocketFactory.SocketFactoryOptions();
            try{
                socketFactoryOptions.withCaInputStream(context.getResources().openRawResource(R.raw.comodorsaaddtrustca));
                mqttConnectOptions.setSocketFactory(new SocketFactory(socketFactoryOptions));
            } catch (IOException | NoSuchAlgorithmException | KeyStoreException | CertificateException | KeyManagementException | UnrecoverableKeyException e){
                e.printStackTrace();
            }
        }

        mqttAndroidClient.setCallback(new MqttCallbackExtended() {
            @Override
            public void connectComplete(boolean reconnect, String serverURI) {
                Toast.makeText(context, "Connected!", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void connectionLost(Throwable cause) {
                Toast.makeText(context, "Connection lost", Toast.LENGTH_SHORT).show();

            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                // do something with the arriving data
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {

            }
        });

        connect();

    }


    public void setCallback(MqttCallbackExtended callback){
        mqttAndroidClient.setCallback(callback);
    }

    public void connect() {

        try {
            final IMqttToken token = mqttAndroidClient.connect(mqttConnectOptions);

            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {

                    // We are connected
                    Log.d("mqtt:", "connected, token:" + asyncActionToken.toString());
                    DisconnectedBufferOptions disconnectedBufferOptions = new DisconnectedBufferOptions();
                    disconnectedBufferOptions.setBufferEnabled(true);
                    disconnectedBufferOptions.setBufferSize(100);
                    disconnectedBufferOptions.setPersistBuffer(false);
                    disconnectedBufferOptions.setDeleteOldestMessages(false);
                    mqttAndroidClient.setBufferOpts(disconnectedBufferOptions);
                    subscribeToTopic();
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    // Something went wrong e.g. connection timeout or firewall problems
                    Log.d("mqtt:", "not connected" + asyncActionToken.toString());
                }
            });

        } catch (MqttException e){
            e.printStackTrace();
        }

    }

    private void subscribeToTopic() {



        try {


            IMqttToken subToken = mqttAndroidClient.subscribe(subscribeTopic, 0);
            subToken.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.w("Mqtt","Subscribed!");
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.w("Mqtt", "Subscribed fail!");
                }
            });


        } catch (MqttException ex){
            System.err.println("Exception subscribing");
            ex.printStackTrace();
        }
    }

    public void publishToTopic(final String topic, Integer qos, String message){

        byte[] msg = message.getBytes();

        mqttConnectOptions.setWill(topic, msg, qos, false);

        try {

            IMqttToken token = mqttAndroidClient.connect(mqttConnectOptions);
            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d("mqtt:", "send done " + asyncActionToken.toString());
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.d("mqtt:", "publish error" + asyncActionToken.toString());
                }
            });

        } catch (MqttException ex){
            System.err.println("Exception publishing");
            ex.printStackTrace();
        }


    }

    public boolean isConnected(){

        return mqttAndroidClient.isConnected();
    }

    void disconnect() {
        try {
            IMqttToken disconToken = mqttAndroidClient.disconnect();
            disconToken.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d("mqtt:", "disconnected");
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken,
                                      Throwable exception) {


                    Log.d("mqtt:", "couldnt disconnect");
                }
            });
        } catch (MqttException e) {
            e.printStackTrace();
        }

    }









}
