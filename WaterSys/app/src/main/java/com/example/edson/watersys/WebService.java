package com.example.edson.watersys;

import android.content.Context;
import android.os.Build;
import android.support.annotation.RequiresApi;
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

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.security.KeyStore;
import java.security.Security;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManagerFactory;

import com.hivemq.client.mqtt.MqttClient;
import com.hivemq.client.mqtt.datatypes.MqttQos;
import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient;

import static com.hivemq.client.mqtt.MqttGlobalPublishFilter.ALL;
import static java.nio.charset.StandardCharsets.UTF_8;



public class WebService  {

    // source for TLS implementation:
    // https://github.com/eurbon/Paho-MQTT-Android-TCP-TLS-WSS-Example/

    public static final String SECURE_BROKER = "ssl://maqiatto.com:3883";
    public static final String BROKER = "tcp://maqiatto.com:1883";

    public static final String USERNAME = "netosilvan78@gmail.com";
    public static final String PASSWORD = "y4NW1jgJi3b7";

    public MqttAndroidClient mqttAndroidClient;
    public MqttConnectOptions mqttConnectOptions;
    private Integer qos;
    private boolean retain; // retain message on the broker or not

    //--------------------- HIVE MQ CREDENTIALS -------------------------

    final String host = "59082674fbc44d0b95579ca81e3201a2.s1.eu.hivemq.cloud";
    final String username = "avalon";
    final String password = "WZ71o80U6PzG";
    Mqtt5BlockingClient client;

    // TODO: eventually, at some point, maybe, create a settings page to change those data below
    public static final String CLIENT_ID = "waterSysApp";

    final String TOPIC_1 = Constants.ping_path;
    final String TOPIC_2 = Constants.dispense_water_route;
    final String TOPIC_3 = Constants.power_route;
    final String TOPIC_4 = Constants.level_route;

    private boolean secureConnection = false;

    public WebService() throws Exception{

        this.client = MqttClient.builder()
                .useMqttVersion5()
                .serverHost(host)
                .serverPort(8883)
                .sslWithDefaultConfig()
                .buildBlocking();


    }


    // Hive MQTT connection
    public WebService(Context context){

        this.qos = 0;
        this.retain = false;
        MqttSetup(context);
        MqttConnect(context);


        mqttAndroidClient.setCallback(new MqttCallbackExtended() {
            @Override
            public void connectComplete(boolean reconnect, String serverURI) {
                Log.d("mqtt", "mqtt connected!");

            }
            @Override
            public void connectionLost(Throwable cause) {
                Log.d("mqtt", "connection lost! " + cause.toString());

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

    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    public void connect(){

        this.client.connectWith()
                .simpleAuth()
                .username(this.username)
                .password(UTF_8.encode(password))
                .applySimpleAuth()
                .send();

        Log.d("mqtt", "Connected successfully: " + this.client.getState().toString());

        //TODO: check if i can do that
        this.client.subscribeWith()
                .topicFilter(TOPIC_1)
                .topicFilter(TOPIC_2)
                .topicFilter(TOPIC_3)
                .topicFilter(TOPIC_4)
                .send();

    }

    public void publish(String topic, String msg, int qos, Boolean retain){
        Log.d("mqqt: ", "sending msg " + msg + " with qos: " + qos  + " to " + topic);

        client.publishWith()
                .topic(topic)
                .payload(UTF_8.encode(msg))
                .qos(MqttQos.fromCode(qos))
                .retain(retain)
                .send();

    }

    public boolean isConnected(){

        return this.client.getState().isConnected();
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
                    subscribe(TOPIC_3, (byte) 1);
                    subscribe(TOPIC_4, (byte) 1);




                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    // Something went wrong e.g. connection timeout or firewall problems
                    Log.d("mqtt:", "not connected, token:" + asyncActionToken.toString() + " msg: " + exception.getMessage() + " " + exception.getCause().toString());
                    if(secureConnection){
                        Log.e("mqtt", "Secure connection failed, trying regular connection");
                        secureConnection = false; // TODO: make this reconnection outside
                        //MqttSetup(context);
                        //MqttConnect(context);
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


    /*void publish(String topic, String msg){

        // qos 0
        Log.d("mqqt: ", "sending msg " + msg + " with qos: " + this.qos  + " to " + topic);

        byte[] encodedPayload = new byte[0];
        try {
            encodedPayload = msg.getBytes("UTF-8");
            MqttMessage message = new MqttMessage(encodedPayload);
            message.setId(5866);
            message.setRetained(this.retain);
            message.setQos(this.qos);
            mqttAndroidClient.publish(topic, message);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (MqttPersistenceException e) {
            e.printStackTrace();
        } catch (MqttException e) {
            e.printStackTrace();
        }

    }*/

    public void publishToTopic(final String topic, Integer qos, String message){
        this.qos = qos;
        if (message.equals(Constants.dispense_water_route)) this.retain = false;
        //publish(topic, message);
        publish(topic, message, qos, false);
    }

    public void publishToTopic(final String topic, Integer qos, String message, Boolean retain){
        this.qos = qos;
        this.retain = retain;
        //publish(topic, message);
        publish(topic, message, qos, retain);
    }



    void disconnect() {

        this.client.disconnect();

        /*try {
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
        }*/

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
