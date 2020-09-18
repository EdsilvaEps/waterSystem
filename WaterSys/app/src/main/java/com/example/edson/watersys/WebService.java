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
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.MqttPersistenceException;

import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
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

    //public static final String BROKER = "ssl://maqiatto.com:3883";
    public static final String BROKER = "tcp://maqiatto.com:1883";

    public static final String USERNAME = "netosilvan78@gmail.com";
    public static final String PASSWORD = "y4NW1jgJi3b7";

    public MqttAndroidClient mqttAndroidClient;
    public MqttConnectOptions mqttConnectOptions;

    // TODO: eventually, at some point, maybe, create a settings page to change those data below
    public static final String CLIENT_ID = "waterSysApp";

    final String TOPIC_1 = Constants.level_route;
    final String TOPIC_2 = Constants.dispense_water_route;



    public WebService(Context context){

        MqttSetup(context);
        MqttConnect();

        mqttAndroidClient.setCallback(new MqttCallbackExtended() {
            @Override
            public void connectComplete(boolean reconnect, String serverURI) {

            }
            @Override
            public void connectionLost(Throwable cause) {

            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                Log.d("topic:" + topic, "message:" + message.toString());

            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {

            }
        });

    }

    public void setCallback(MqttCallbackExtended callback) {
        mqttAndroidClient.setCallback(callback);
    }



    void MqttSetup(Context context){

        mqttAndroidClient = new MqttAndroidClient(context, BROKER, CLIENT_ID);
        mqttConnectOptions = new MqttConnectOptions();

        mqttConnectOptions.setUserName(USERNAME);
        mqttConnectOptions.setPassword(PASSWORD.toCharArray());
        mqttConnectOptions.setAutomaticReconnect(true);
        mqttConnectOptions.setCleanSession(true);

        /**
         * SSL broker requires a certificate to authenticate their connection
         * Certificate can be found in resources folder /res/raw/
         */
        if (BROKER.contains("ssl")){
            SocketFactory.SocketFactoryOptions socketFactoryOptions = new SocketFactory.SocketFactoryOptions();
            try {
                socketFactoryOptions.withCaInputStream(context.getResources().openRawResource(R.raw.comodorsaaddtrustca));
                mqttConnectOptions.setSocketFactory(new SocketFactory(socketFactoryOptions));
            } catch (IOException | NoSuchAlgorithmException | KeyStoreException | CertificateException | KeyManagementException | UnrecoverableKeyException e) {
                e.printStackTrace();
            }
        }
    }


    void MqttConnect(){

        try{

            final IMqttToken token = mqttAndroidClient.connect(mqttConnectOptions);
            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    // connected
                    Log.d("mqtt:", "connected, token:" + asyncActionToken.toString());
                    subscribe(TOPIC_1, (byte) 1);
                    subscribe(TOPIC_2, (byte) 1);


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

    void subscribe(String topic, byte qos){

        Log.d("mqtt:", "Subscribing to " + topic);


        try{

            IMqttToken subToken = mqttAndroidClient.subscribe(topic, qos);
            subToken.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d("mqtt:", "subscribed" + asyncActionToken.toString());

                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.d("mqtt:", "subscribing error");

                }
            });

        } catch (MqttException e){
            e.printStackTrace();
        }
    }


    void publish(String topic, String msg){

        // qos 0
        Log.d("mqqt: ", "sending msg " + msg + " to " + topic);

        byte[] encodedPayload = new byte[0];
        try {
            encodedPayload = msg.getBytes("UTF-8");
            MqttMessage message = new MqttMessage(encodedPayload);
            message.setId(5866);
            message.setRetained(true);
            message.setQos(0);
            mqttAndroidClient.publish(topic, message);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (MqttPersistenceException e) {
            e.printStackTrace();
        } catch (MqttException e) {
            e.printStackTrace();
        }

    }

    public void publishToTopic(final String topic, Integer qos, String message){
        publish(topic, message);
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









    // - ---------- old --------------------
    /*
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


            IMqttToken subToken = mqttAndroidClient.subscribe(Constants.ping_path, 0);
            subToken.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {

                    Log.w("Mqtt","Subscribed to " + Constants.ping_path);
                    publishToTopic(Constants.ping_path, 0, "Hey! App connected!");
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
        Log.w("mqtt", "sending message " + message + " to " + Constants.change_program);

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

    } */









}
