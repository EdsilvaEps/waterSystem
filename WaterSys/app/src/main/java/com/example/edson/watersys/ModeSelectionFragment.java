package com.example.edson.watersys;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.FragmentActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

public class ModeSelectionFragment extends android.support.v4.app.Fragment {
    EditText title, description;
    Spinner intervals, amount;
    Button save_use, save, cancel;
    FragmentActivity listener;

    ArrayAdapter<CharSequence> amountAdapter;
    ArrayAdapter<CharSequence> intervalAdapter;

    double amountquant = 0;
    float[] intervalslist = {};

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        if(context instanceof Activity){
            this.listener = (FragmentActivity) context;
        }
    }

    // This event fires 2nd, before views are created for the fragment
    // The onCreate method is called when the Fragment instance is being created, or re-created.
    // Use onCreate for any standard setup that does not require the activity to be fully created
    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


    }

    // The onCreateView method is called when Fragment should create its View object hierarchy,
    // either dynamically or via XML layout inflation.
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, Bundle savedInstanceState) {
        //return super.onCreateView(inflater, container, savedInstanceState);
        return inflater.inflate(R.layout.mode_selection_fragment, container, false);
    }

    // This event is triggered soon after onCreateView().
    // onViewCreated() is only called if the view returned from onCreateView() is non-null.
    // Any view setup should occur here.  E.g., view lookups and attaching view listeners.
    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        title = (EditText) view.findViewById(R.id.title_edittext);
        description = (EditText) view.findViewById(R.id.description_edittext);
        intervals = (Spinner) view.findViewById(R.id.quantity_choice);
        amount = (Spinner) view.findViewById(R.id.amount_choice);
        save_use = (Button) view.findViewById(R.id.save_and_use);
        save = (Button) view.findViewById(R.id.save);
        cancel = (Button) view.findViewById(R.id.cancel);

        amountAdapter = ArrayAdapter.createFromResource(getActivity(), R.array.quantity_choice_array,
                android.R.layout.simple_spinner_item);
        intervalAdapter = ArrayAdapter.createFromResource(getActivity(), R.array.interval_array_list,
                android.R.layout.simple_spinner_item);

        intervals.setAdapter(intervalAdapter);
        amount.setAdapter(amountAdapter);


        intervals.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                // set intervalslist variable with the chosen intervals
                switch (position){
                    case 0:
                        intervalslist = Constants.INTERVAL_TWICE_DAY;
                        break;
                    case 1:
                        intervalslist = Constants.INTERVAL_DAILY;
                        break;
                    case 2:
                        intervalslist = Constants.INTERVAL_FIVE_T_WEEK;
                        break;
                    case 3:
                        intervalslist = Constants.INTERVAL_FOUR_T_WEEK;
                        break;
                    case 4:
                        intervalslist = Constants.INTERVAL_THREE_T_WEEK;
                        break;
                    case 5:
                        intervalslist = Constants.INTERVAL_TWICE_WEEK;
                        break;
                    case 6:
                        intervalslist = Constants.INTEVAL_WEEKLY;
                        break;
                    default:
                        intervalslist = new float[]{};
                        break;
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                intervalslist = new float[]{};

            }
        });

        amount.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                // assign amount quantities to amountquant variable depending on user's choice
                switch (position){
                    case 0:
                        amountquant = Constants.AMOUNT_TINY_BIT;
                        break;
                    case 1:
                        amountquant = Constants.AMOUNT_SMALL_AMOUNT;
                        break;
                    case 2:
                        amountquant = Constants.AMOUNT_JUST_ENOUGH;
                        break;
                    case 3:
                        amountquant = Constants.AMOUNT_A_LOT;
                        break;
                    case 4:
                        amountquant = Constants.AMOUNT_BIG_AMOUNT;
                        break;
                    default:
                        amountquant = 0;
                        break;
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                amountquant = 0;

            }
        });

        save_use.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // add here method to save and activate the created mode

            }
        });

        save.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // add here method to save method
            }
        });

        cancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {


            }
        });



    }

    // This method is called when the fragment is no longer connected to the Activity
    // Any references saved in onAttach should be nulled out here to prevent memory leaks.
    @Override
    public void onDetach() {
        super.onDetach();
        this.listener = null;
    }

    // This method is called after the parent Activity's onCreate() method has completed.
    // Accessing the view hierarchy of the parent activity must be done in the onActivityCreated.
    // At this point, it is safe to search for activity View objects by their ID, for example.
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);


    }

}
