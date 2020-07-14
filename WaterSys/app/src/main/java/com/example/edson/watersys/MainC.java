package com.example.edson.watersys;

import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.provider.SyncStateContract;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresApi;
import android.support.constraint.ConstraintLayout;
import android.support.design.widget.BottomNavigationView;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.NavigationView;
import android.support.v4.content.ContextCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewTreeObserver;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.support.v7.widget.Toolbar;

import com.example.edson.watersys.database.DBHadler;
import com.example.edson.watersys.objs.WateringPlan;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

import static android.graphics.Color.BLACK;
import static android.graphics.Color.GRAY;
import static android.graphics.Color.GREEN;
import static android.graphics.Color.RED;

public class MainC extends AppCompatActivity {
    private static final String ROOT_FRAG_TAG = "root_fragment";

    Toolbar mainToolbar;
    FloatingActionButton waterBtn;
    TextView statsText;
    ConstraintLayout mainLayout;
    BottomNavigationView bottomNavigationView;

    TextView connectionStatusTxt;
    ImageView connectionStautsIcon;
    ImageView tank_img;

    // plan details
    TextView level_txt;
    TextView plan_txt; // id plan_text
    TextView time_txt; // id card1_timetxt
    TextView days_txt; // id card1_daystxt
    TextView amount_txt; //id card1_amounttxt
    DBHadler db;
    List<WateringPlan> plans;
    private SharedPreferences sharedPreferences;
    private SharedPreferences.Editor editor;

    /*BluetoothAdapter mBluetoothAdapter = null;
    Set<BluetoothDevice> pairedDevices;
    String targetDev = "HC-06";
    String lastMsg = "";
    ArrayList<String> msgBuffer = new ArrayList<>();*/


    public static final String TAG = "MainActivity";


    ///private final static int REQUEST_ENABLE_BT = 1;
    //private Handler mHandler; // handler that gets info from Bluetooth service
    //private BluetoothService mBluetoothService = null;
    //private StringBuffer mOutStringBuffer;

    private DrawerLayout mDrawerLayout;

    private WebService webService;
    private String pendingMsg = "";





    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);



        waterBtn = findViewById(R.id.fab);
        connectionStatusTxt = findViewById(R.id.cardconnection_connectiontxt);
        connectionStautsIcon = findViewById(R.id.connectionicon);
        statsText = findViewById(R.id.textView11);
        mainToolbar =  findViewById(R.id.toolbar2);
        mDrawerLayout = findViewById(R.id.drawer_layout);
        bottomNavigationView = findViewById(R.id.navigationbar);
        plan_txt = findViewById(R.id.plan_text);
        time_txt = findViewById(R.id.card1_timetxt);
        days_txt = findViewById(R.id.card1_daystxt);
        amount_txt = findViewById(R.id.card1_amounttxt);
        tank_img = findViewById(R.id.tank_img);
        level_txt = findViewById(R.id.card2_leveltxt);

        db = new DBHadler(this);
        plans = new ArrayList<>();


        startMqtt();
        setConnectionStatus(webService.isConnected());
        setupCurrentPlanUI();
        /* we need the measurement of the screen to set the size of the status
        * toolbar. When we get it, we set mainToolbar's minimum height to 10% of that size*/
        mainLayout = findViewById(R.id.myconstLayout);
        Log.e(TAG, "main layout is: " + mainLayout);


        bottomNavigationView.setOnNavigationItemSelectedListener(new BottomNavigationView.OnNavigationItemSelectedListener() {
            @Override
            public boolean onNavigationItemSelected(@NonNull MenuItem item) {
                selectFragment(item);
                return true;
            }
        });

        //bottomNavigationView.setSelectedItemId(R.id.action_home);
        //bottomNavigationView.getMenu().getItem(R.id.action_home).setChecked(true);





        NavigationView navigationView = findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(
                new NavigationView.OnNavigationItemSelectedListener() {
                    @Override
                    public boolean onNavigationItemSelected(@NonNull MenuItem item) {


                        System.out.println(item.getTitle());
                        // set item as selected to persist highlight
                        item.setChecked(true);
                        // close drawer when item is tapped
                        mDrawerLayout.closeDrawers();


                        int containerViewid = -1;
                        // add code here to update the UI based on the item
                        // e.g. swap UI fragments here
                        switch ((String)item.getTitle()){
                            case "Mode Selection":
                                containerViewid = R.id.content_frame;
                                break;
                            case "Create New Mode":
                                //ft.replace(R.id.content_frame, new ModeSelectionFragment());
                                containerViewid = R.id.content_frame;
                                break;
                            case "Reports":
                                System.out.println("item 0");
                                //ft.replace(R.id.content_frame, new ModeSelectionFragment());
                                break;
                            case "Configurations":
                                System.out.println("item 0");
                                //ft.replace(R.id.content_frame, new ModeSelectionFragment());
                                break;
                            case "Log Out":
                                System.out.println("item 0");
                                //ft.replace(R.id.content_frame, new ModeSelectionFragment());
                                break;


                        }

                        if(containerViewid != -1){
                            android.support.v4.app.FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
                            ft.replace(R.id.content_frame, new ModeSelectionFragment())
                                    .addToBackStack(ROOT_FRAG_TAG)
                                    .commit();
                        }


                        return true;
                    }
                }
        );

        setSupportActionBar(mainToolbar);
        android.support.v7.app.ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
       // actionBar.setHomeAsUpIndicator(R.drawable.ic_menu);

        waterBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // send message to broker if connected
                if(webService.isConnected()){
                    webService.publishToTopic(Constants.dispense_water_route, 0, "1");
                } else{
                    pendingMsg = Constants.dispense_water_route;
                    startMqtt();
                }
            }
        });



        sharedPreferences = getApplicationContext().getSharedPreferences(getString(R.string.plan_prefs), Context.MODE_PRIVATE);
        String lvl = sharedPreferences.getString(getString(R.string.level_amount), "5");
        setupLvl(lvl);





    }

    // sets up the level card
    public void setupLvl(String lvl){


        switch (lvl){
            case "5":
                tank_img.setImageResource(R.mipmap.tank_5_percent_fg);
                level_txt.setText("under 5%");
                break;
            case "40":
                tank_img.setImageResource(R.mipmap.tank20pcent_fg);
                level_txt.setText("under 20%");
                break;

            case "50":
                tank_img.setImageResource(R.mipmap.tank50pcent_fg);
                level_txt.setText("under 50%");
                break;

            case "70":
                tank_img.setImageResource(R.mipmap.tank70pcent_fg);
                level_txt.setText("under 70%");
                break;

            case "90":
                tank_img.setImageResource(R.mipmap.tank90pcent_fg);
                level_txt.setText("under 90%");
                break;

            default: System.out.println("could not read message");

        }




    }

    // fragment selection for the bottow nav menu
    private void selectFragment(MenuItem item) {
        android.support.v4.app.Fragment frag = null;
        // init corresponding fragment
        switch (item.getItemId()){
            case R.id.action_home:
                // do nothing, cuz we already here
                break;
            case R.id.action_plans:
                // plans view fragment
                frag = PlanViewFragment.newInstance();
                break;
            case R.id.action_settings:
                // add frag here;
                frag = PlanSelectionFragment.newInstance();
                break;


        }

        if(frag != null){
            android.support.v4.app.FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
            transaction.replace(R.id.content_frame, frag);
            transaction.commit();
        } else Log.w(TAG, "Home menu btn pressed");




    }

    /**
     * Initializes the mqtt service on the main activity context
     */
    private void startMqtt() {
        // initializing our web service mqtt and setting callbacks
        webService = new WebService(getApplicationContext());
        webService.setCallback(new MqttCallbackExtended() {
            @Override
            public void connectComplete(boolean reconnect, String serverURI) {
                setConnectionStatus(true);
                webService.publishToTopic(Constants.report, 0, "1");
                pendingMsgHander(); // check if there are pending messages to be sent
            }

            @Override
            public void connectionLost(Throwable cause) {
                // do something
                setConnectionStatus(false);
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                setConnectionStatus(true);

                // decodes and interprets received messages
                if(topic.equals(Constants.level_route)){
                    String msg = new String(message.getPayload(), "UTF-8");
                    setupLvl(msg);
                    saveLevelData(msg, getApplicationContext());
                }

            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                Toast.makeText(getApplicationContext(), "control message sent to server", Toast.LENGTH_SHORT).show();

            }
        });
    }

    /**
     * change the UI according to the connection status
     * @param isConnected
     */
    public void setConnectionStatus(Boolean isConnected){
        if (isConnected){
            connectionStatusTxt.setText(getText(R.string.status_connected_txt));
            connectionStautsIcon.setColorFilter(ContextCompat.getColor(getApplicationContext(), R.color.connected));

        }

        else {
            connectionStatusTxt.setText(getText(R.string.status_notconnected_txt));
            connectionStautsIcon.setColorFilter(ContextCompat.getColor(getApplicationContext(),R.color.reddish));

        }


    }


    // basically saves level data
    static void saveLevelData(String lvl, Context context){

        SharedPreferences sharedPreferences;

        sharedPreferences = context.getSharedPreferences(context.getString(R.string.level_prefs), Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(context.getString(R.string.level_amount), lvl);
        editor.apply();


    }

    /** Changes the user interface to show the currently selected plan
     *
     */
    public void setupCurrentPlanUI(){
        plans = db.getAllPlans();

        sharedPreferences = getApplicationContext().getSharedPreferences(getString(R.string.plan_prefs), Context.MODE_PRIVATE);
        Integer id = Integer.parseInt(sharedPreferences.getString(getString(R.string.pref_id), "-1"));

        if(id > -1){
            for(WateringPlan plan : plans){
                if(plan.getId() == id){
                    plan_txt.setText(plan.getTitle());

                    // concatenating the days
                    String sdays = "";
                    for (String day : plan.getDays()){
                        if(day == null){
                            break;
                        } else sdays += day + " ";
                    }

                    days_txt.setText(sdays);
                    time_txt.setText(plan.getTime().toString());
                    String sAmount = String.valueOf(plan.getAmount()) + " ml";
                    amount_txt.setText(sAmount);

                    // update broker about the plan data everytime we are on the main activity
                    // and hope we are using some code to compare things at the hardware level
                    // TODO: check if this method cause problems in the future
                    Log.w(TAG, "attempting to send schedule msg to router");
                    if(webService.isConnected()) {

                        webService.publishToTopic(Constants.change_program, 0, plan.getJsonString());

                        //webService.publishToTopic(Constants.schedule_route,0,plan.getTime().toString());
                        //webService.publishToTopic(Constants.amount_route, 0, plan.getAmount().toString());
                        //webService.publishToTopic(Constants.days_route, 0, sdays);

                    } else {
                        pendingMsg = Constants.change_program;
                        Log.e(TAG, "msg not sent, pending status is: " + pendingMsg);
                        startMqtt();


                    }


                }
            }

        }




    }

    /**
     * function to be called upon connection to broker
     * for resolving pending messages
     */
    public void pendingMsgHander(){
        Log.e(TAG, "checking pending messages: " + pendingMsg);
        if(pendingMsg.equals(Constants.dispense_water_route)){
            webService.publishToTopic(Constants.dispense_water_route, 0, "1");

        } else if(pendingMsg.equals(Constants.change_program)){
            setupCurrentPlanUI();

        }

        pendingMsg = "";



    }



    @Override
    public void onResume() {
        super.onResume();


    }





    @Override
    protected void onDestroy() {
        super.onDestroy();
        webService.disconnect();
        
    }
}
