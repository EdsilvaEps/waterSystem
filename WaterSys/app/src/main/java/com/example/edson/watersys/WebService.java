package com.example.edson.watersys;

import android.content.Context;
import android.util.Log;


import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.MqttPersistenceException;
import org.eclipse.paho.client.mqttv3.internal.wire.MqttConnect;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.security.Key;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.Security;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManagerFactory;

public class WebService {

    // source for TLS implementation:
    // https://github.com/eurbon/Paho-MQTT-Android-TCP-TLS-WSS-Example/

    public static final String SECURE_BROKER = "ssl://maqiatto.com:3883";
    public static final String BROKER = "tcp://maqiatto.com:1883";

    public static final String USERNAME = "netosilvan78@gmail.com";
    public static final String PASSWORD = "y4NW1jgJi3b7";

    public MqttAndroidClient mqttAndroidClient;
    public MqttConnectOptions mqttConnectOptions;

    // TODO: eventually, at some point, maybe, create a settings page to change those data below
    public static final String CLIENT_ID = "waterSysApp";

    final String TOPIC_1 = Constants.level_route;
    final String TOPIC_2 = Constants.dispense_water_route;

    boolean secureConnection = true;



    public WebService(Context context){

        MqttSetup(context);
        MqttConnect(context);

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

        String broker = BROKER;
        if(secureConnection) broker = SECURE_BROKER;

        mqttAndroidClient = new MqttAndroidClient(context, broker, CLIENT_ID);
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
            Log.d("mqtt", "connecting to " + BROKER);

            //SSLSocketFactory sslSocketFactory =

            SocketFactory.SocketFactoryOptions socketFactoryOptions = new SocketFactory.SocketFactoryOptions();

            try{
                InputStream caCrtFileI = context.getResources().openRawResource(R.raw.ca);
                mqttConnectOptions.setSocketFactory(getSingleSocketFactory(caCrtFileI));
            } catch (Exception e){
                e.printStackTrace();
            }


        }
    }


    void MqttConnect(final Context context){

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
                    Log.d("mqtt:", "not connected, token:" + asyncActionToken.toString() + " msg: " + exception.getMessage() + " " + exception.getCause().toString());
                    if(secureConnection){
                        Log.e("mqtt", "Secure connection failed, trying regular connection");
                        secureConnection = false;
                        MqttSetup(context);
                        MqttConnect(context);
                    }

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
            message.setRetained(false);
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

    //One-way authentication means that the server-side authenticates the client. The core code is as follows:
    public static SSLSocketFactory getSingleSocketFactory(InputStream caCrtFileInputStream) throws Exception{
        Security.addProvider(new BouncyCastleProvider());
        X509Certificate caCert = null;

        BufferedInputStream bis = new BufferedInputStream(caCrtFileInputStream);
        CertificateFactory cf = CertificateFactory.getInstance("X.509");

        while (bis.available() > 0){
            caCert = (X509Certificate) cf.generateCertificate(bis);
        }

        Log.d("mqtt", "ca = " + caCert.getSubjectDN());

        KeyStore caKs = KeyStore.getInstance(KeyStore.getDefaultType());
        caKs.load(null, null);
        caKs.setCertificateEntry("cert-certificate", caCert);
        TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        tmf.init(caKs);
        SSLContext sslContext = SSLContext.getInstance("TLSv1.2");
        sslContext.init(null, tmf.getTrustManagers(), null);
        return sslContext.getSocketFactory();

    }











}
