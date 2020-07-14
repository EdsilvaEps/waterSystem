package com.example.edson.watersys;

import android.app.DialogFragment;
import android.content.Context;
import android.content.Intent;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.design.widget.BottomNavigationView;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.example.edson.watersys.database.DBHadler;
import com.example.edson.watersys.objs.WateringPlan;

import java.sql.Time;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Locale;

public class CreateNewPlanFragment extends Fragment {

    private DBHadler db; // handler for the database
    private EditText title; // id: plan_name
    private SeekBar amount; // id: amount_bar
    private TextView amountxt; // id: amountquanttext
    private Button timebtn; // id: timebtn
    private RadioButton mon;
    private RadioButton tue;
    private RadioButton wed;
    private RadioButton thu;
    private RadioButton fri;
    private RadioButton sat;
    private RadioButton sun;

    private BottomNavigationView bottomNavigationView;
    private FloatingActionButton fin; // id: fab

    private static String TAG = "createnew";

    public static CreateNewPlanFragment newInstance() {

        Bundle args = new Bundle();

        CreateNewPlanFragment fragment = new CreateNewPlanFragment();
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

    }

    // The onCreateView method is called when Fragment should create its View object hierarchy,
    // either dynamically or via XML layout inflation.
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, Bundle savedInstanceState) {
        //return super.onCreateView(inflater, container, savedInstanceState);
        return inflater.inflate(R.layout.content_create_new, container, false);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        title = view.findViewById(R.id.plan_name);
        amount = view.findViewById(R.id.amount_bar);
        amountxt = view.findViewById(R.id.amountquanttext);
        timebtn = view.findViewById(R.id.timebtn);
        bottomNavigationView = view.findViewById(R.id.navigationbar);
        fin = view.findViewById(R.id.fab);
        mon = view.findViewById(R.id.mon);
        tue = view.findViewById(R.id.tue);
        wed = view.findViewById(R.id.wed);
        thu = view.findViewById(R.id.thu);
        fri = view.findViewById(R.id.fri);
        sat = view.findViewById(R.id.sat);
        sun = view.findViewById(R.id.sun);


        amountxt.setText("0");

        timebtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showTimePickerDialog(v);
            }
        });

        fin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                saveNew();
            }
        });

        amount.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                amountxt.setText(String.valueOf(progress));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

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
                        frag = PlanViewFragment.newInstance();

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

    private void saveNew() {
        String titleVar = "";
        String timeVar;
        Integer amnt = 0;

        titleVar = title.getText().toString();
        amnt = Integer.valueOf(amountxt.getText().toString());
        timeVar = timebtn.getText().toString();

        ArrayList<String> days = new ArrayList<>();
        if(mon.isChecked()) days.add("Mon");
        if(tue.isChecked()) days.add("Tue");
        if(wed.isChecked()) days.add("Wed");
        if(thu.isChecked()) days.add("Thu");
        if(fri.isChecked()) days.add("Fri");
        if(sat.isChecked()) days.add("Sat");
        if(sun.isChecked()) days.add("Sun");



        // if no title is given, set a generic title
        if(titleVar.trim().length() == 0){
            titleVar = "Plan #" + String.valueOf(db.getPlansCount()+1);
        }

        if(days.isEmpty()){
            Toast.makeText(getContext(), "Please, choose at least one day for watering", Toast.LENGTH_SHORT).show();
        }
        else if(!(amnt > 0)){
            Toast.makeText(getContext(), "Please, choose an amount of water", Toast.LENGTH_SHORT).show();
        }
        else {

            // filling the other day spaces with 'null'
            if(days.size() < 7){
                int fillIndexes = 7 - days.size();
                for(int i = 0; i < fillIndexes; i++){
                    days.add(null);
                }
            }

            Log.w(TAG, "title: " + titleVar);
            Log.w(TAG, "time: " + timeVar);
            Log.w(TAG, "days: " + days);
            Log.w(TAG, "amount: " + amnt);


            DateFormat format = new SimpleDateFormat("HH:mm");
            try {
                Time timevalue = new Time(format.parse(timeVar).getTime());
                int newId = db.getPlansCount() + 1;
                WateringPlan plan = new WateringPlan(newId,titleVar, timevalue,days,amnt);
                db.addPlan(plan);

                // adding a new plan to the database
                title.setText("");
                timebtn.setText("00:00");
                amount.setProgress(0);
                mon.setChecked(false);
                tue.setChecked(false);
                wed.setChecked(false);
                thu.setChecked(false);
                fri.setChecked(false);
                sat.setChecked(false);
                sun.setChecked(false);

                Toast.makeText(getContext(), "New plan added", Toast.LENGTH_SHORT).show();

            } catch (ParseException e) {
                e.printStackTrace();
                Toast.makeText(getContext(), "Could not add plan", Toast.LENGTH_SHORT).show();

            }

            //getContext().deleteDatabase(Constants.DATABASE_NAME);



        }



    }

    @Override
    public void onDetach() {
        super.onDetach();
    }

    public void showTimePickerDialog(View v){
        android.support.v4.app.DialogFragment newFragment = new TimePickerFragment();
        newFragment.show(getActivity().getSupportFragmentManager(), "timePicker");
    }


}
