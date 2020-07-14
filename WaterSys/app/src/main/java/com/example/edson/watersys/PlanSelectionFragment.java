package com.example.edson.watersys;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.design.widget.BottomNavigationView;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.example.edson.watersys.adapters.PlanSelectionAdapter;
import com.example.edson.watersys.database.DBHadler;
import com.example.edson.watersys.objs.WateringPlan;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.util.List;

public class PlanSelectionFragment extends Fragment {

    private static final String TAG = "selectfragment";
    private FloatingActionButton mFloatingButton;
    private BottomNavigationView bottomNavigationView;
    private TextView noplansTxtView;

    DBHadler db;
    private List<WateringPlan> plans; // watering plans to be displayed

    // recyclerview components for displaying the list of plans
    private RecyclerView mRecyclerView;
    private RecyclerView.Adapter mAdapter;
    private RecyclerView.LayoutManager layoutManager;

    private SharedPreferences sharedPreferences;
    private SharedPreferences.Editor editor;
    private Integer defaultPlanId;
    private IntentFilter intentFilter;

    private BroadcastReceiver broadcastReceiver;
    private LocalBroadcastManager manager;

    private WebService webService;

    String pendingMsg = "";



    public static PlanSelectionFragment newInstance() {

        Bundle args = new Bundle();

        PlanSelectionFragment fragment = new PlanSelectionFragment();
        fragment.setArguments(args);
        return fragment;

    }


    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
    }

    // This event fires 2nd, before views are created for the fragment
    // The onCreate method is called when the Fragment instance is being created, or re-created.
    // Use onCreate for any standard setup that does not require the activity to be fully created
    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        db = new DBHadler(getContext());
        plans = db.getAllPlans();
        webService = new WebService(getContext());


    }

    // The onCreateView method is called when Fragment should create its View object hierarchy,
    // either dynamically or via XML layout inflation.
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, Bundle savedInstanceState) {
        //return super.onCreateView(inflater, container, savedInstanceState);
        return inflater.inflate(R.layout.content_select_plan, container, false);
    }

    // This event is triggered soon after onCreateView().
    // onViewCreated() is only called if the view returned from onCreateView() is non-null.
    // Any view setup should occur here.  E.g., view lookups and attaching view listeners.
    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        bottomNavigationView = view.findViewById(R.id.navigationbar);
        mRecyclerView = view.findViewById(R.id.plansrecyclerview);
        mFloatingButton = view.findViewById(R.id.fab);
        noplansTxtView = view.findViewById(R.id.noplanstxt);

        bottomNavigationView.setSelectedItemId(R.id.action_plans);

        // use this setting to improve performance if you know that changes
        // in content do not change the layout size of the RecyclerView
        mRecyclerView.setHasFixedSize(false);

        // use a linear layout manager
        layoutManager = new LinearLayoutManager(getContext());
        mRecyclerView.setLayoutManager(layoutManager);

        activeChanged(); // checks the currently active watering plan
        startMqtt();


        try{

            // specify an adapter
            mAdapter = new PlanSelectionAdapter(plans, getContext());

            // if no plans are found, there should not be the recyclerview there
            if ((plans.isEmpty())) {
                mRecyclerView.setVisibility(View.GONE);
            } else {
                noplansTxtView.setVisibility(View.GONE);
            }

            // setting adapter
            mRecyclerView.setAdapter(mAdapter);


        } catch (Exception e){
            e.printStackTrace();
        }


        /**
         * this activity listens for changes in the activation status
         * of the watering plans, when the user switches on the flip on one
         * of the plans, any other is switched off
         * TODO: block the user from flipping any other switch while this operation is being made
         */
        broadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Log.w(TAG, "default plan changed");
                activeChanged();
                mAdapter.notifyDataSetChanged();
                Toast.makeText(context, "new plan selected", Toast.LENGTH_SHORT).show();

            }
        };

        manager= LocalBroadcastManager.getInstance(getContext());
        manager.registerReceiver(broadcastReceiver, new IntentFilter(getString(R.string.change_default_plan_intent)));

        //intentFilter = new IntentFilter(getString(R.string.change_default_plan_intent));
        //intentFilter.addAction();


        mFloatingButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // go to creation of plans
                Fragment frag = CreateNewPlanFragment.newInstance();
                FragmentTransaction transaction = getActivity().getSupportFragmentManager().beginTransaction();
                transaction.replace(R.id.content_frame, frag);
                transaction.commit();
            }
        });


        bottomNavigationView.setOnNavigationItemSelectedListener(new BottomNavigationView.OnNavigationItemSelectedListener() {
            @Override
            public boolean onNavigationItemSelected(@NonNull MenuItem item) {
                android.support.v4.app.Fragment frag = null;
                // init corresponding fragment
                switch (item.getItemId()){
                    case R.id.action_home:
                        // goto main
                        //getActivity().onBackPressed();
                        Intent intent = new Intent(getContext(), MainC.class);
                        getActivity().startActivity(intent);
                        break;
                    case R.id.action_plans:
                        frag = PlanViewFragment.newInstance();
                        // do nothin cuz we already here
                        break;
                    case R.id.action_settings:

                        // do nothin cuz we already here
                        break;


                }
                if(frag != null){
                    android.support.v4.app.FragmentTransaction transaction = getActivity().getSupportFragmentManager().beginTransaction();
                    transaction.replace(R.id.content_frame, frag);
                    transaction.commit();
                } else Log.w(TAG, "Home menu btn pressed");
                return true;
            }
        });


    }

    // This method is called when the fragment is no longer connected to the Activity
    // Any references saved in onAttach should be nulled out here to prevent memory leaks.
    @Override
    public void onDetach() {
        super.onDetach();
        manager.unregisterReceiver(broadcastReceiver);

    }

    // This method is called after the parent Activity's onCreate() method has completed.
    // Accessing the view hierarchy of the parent activity must be done in the onActivityCreated.
    // At this point, it is safe to search for activity View objects by their ID, for example.
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);


    }

    /**
     * changes the active status of the plans
     * ensuring that only one plan is active at a time
     */
    private void activeChanged(){

        try{

            sharedPreferences = getContext().getSharedPreferences(getString(R.string.plan_prefs), Context.MODE_PRIVATE);

            String id = sharedPreferences.getString(getString(R.string.pref_id), "");

            if(!id.equals("")){ // if there's plans

                defaultPlanId = Integer.valueOf(id);
                Log.w(TAG, "current active: " + defaultPlanId);


                String title = sharedPreferences.getString(getString(R.string.pref_title), "");
                Log.w(TAG, "current active title: " + title);

                //defaultPlanId = Integer.getInteger(sharedPreferences.getString(getString(R.string.pref_id), "-1"));

                for(WateringPlan plan : plans){
                    if (plan.getId() == defaultPlanId){
                        plan.setActive(true);

                        // send data about change to broker
                        if(webService.isConnected()){

                            webService.publishToTopic(Constants.change_program, 0, plan.getJsonString());

                            //webService.publishToTopic(Constants.schedule_route, 0, plan.getTime().toString());
                            //webService.publishToTopic(Constants.amount_route, 0, plan.getAmount().toString());
                            //webService.publishToTopic(Constants.days_route, 0, sdays);

                        } else{

                            pendingMsg = Constants.change_program;
                            startMqtt();


                        }
                    } else plan.setActive(false);
                }

            }




        } catch (NullPointerException e){
            e.printStackTrace();
        }


    }

    /**
     * Initializes the mqtt service on the main activity context
     */
    private void startMqtt() {
        // initializing our web service mqtt and setting callbacks
        webService = new WebService(getContext());
        webService.setCallback(new MqttCallbackExtended() {
            @Override
            public void connectComplete(boolean reconnect, String serverURI) {
                pendingMsgHander(); // check if there are pending messages to be sent
            }

            @Override
            public void connectionLost(Throwable cause) {
                // do something
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {


            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                Toast.makeText(getContext(), "control message sent to server", Toast.LENGTH_SHORT).show();

            }
        });
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
            activeChanged();

        }

        pendingMsg = "";



    }
}
