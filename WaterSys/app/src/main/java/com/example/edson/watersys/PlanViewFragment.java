package com.example.edson.watersys;

import com.example.edson.watersys.database.DBHadler;
import com.example.edson.watersys.objs.WateringPlan;
import com.example.edson.watersys.adapters.PlansAdapter;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.design.widget.BottomNavigationView;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import java.text.ParseException;
import java.util.List;

/**
 * PlanViewFragment handles the view of the details
 * of each plan previously added to the memory,
 * also enables deleting and editting those plans.
 */
public class PlanViewFragment extends Fragment {

    private static final String TAG = "viewfragment";
    private FloatingActionButton mFloatingButton;
    private BottomNavigationView bottomNavigationView;
    private TextView noplansTxtView;

    private DBHadler db; // handler for the database
    private List<WateringPlan> plans; // watering plans to be displayed

    // recyclerview components for displaying the list of plans
    private RecyclerView mRecyclerView;
    private RecyclerView.Adapter mAdapter;
    private RecyclerView.LayoutManager layoutManager;

    public static PlanViewFragment newInstance() {

        Bundle args = new Bundle();

        PlanViewFragment fragment = new PlanViewFragment();
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

    }

    // The onCreateView method is called when Fragment should create its View object hierarchy,
    // either dynamically or via XML layout inflation.
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, Bundle savedInstanceState) {
        //return super.onCreateView(inflater, container, savedInstanceState);
        return inflater.inflate(R.layout.content_plans, container, false);
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

        Log.w(TAG,  "No of plans:  " + plans.size());

        try{

            // specify an adapter
            mAdapter = new PlansAdapter(plans, getContext(), db);

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
                        // do nothin cuz we already here
                        break;
                    case R.id.action_settings:
                        // add frag here;
                        frag = PlanSelectionFragment.newInstance();
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
    }

    // This method is called after the parent Activity's onCreate() method has completed.
    // Accessing the view hierarchy of the parent activity must be done in the onActivityCreated.
    // At this point, it is safe to search for activity View objects by their ID, for example.
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);


    }


}
