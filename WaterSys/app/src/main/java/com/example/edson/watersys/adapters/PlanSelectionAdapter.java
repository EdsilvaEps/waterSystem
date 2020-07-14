package com.example.edson.watersys.adapters;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import com.example.edson.watersys.R;
import com.example.edson.watersys.objs.WateringPlan;

import java.util.List;

public class PlanSelectionAdapter extends RecyclerView.Adapter<PlanSelectionAdapter.SelectionViewHolder> {

    private List<WateringPlan> plans;
    private Context context;
    private static final String TAG = "selectadapter";

    private SharedPreferences sharedPreferences;
    private SharedPreferences.Editor editor;




    public static class SelectionViewHolder extends RecyclerView.ViewHolder{

        private TextView title;
        private Switch aSwitch;

        public SelectionViewHolder(View itemView) {
            super(itemView);

            title = itemView.findViewById(R.id.plan_text);
            aSwitch = itemView.findViewById(R.id.simple_switch);
        }

        private void bind(WateringPlan plan){

            title.setText(plan.getTitle());

            if (plan.isActive()){
                aSwitch.setChecked(true);
            } else aSwitch.setChecked(false);

        }
    }

    // Provide a suitable constructor (depends on the kind of dataset)
    public PlanSelectionAdapter(List<WateringPlan> plans, Context context){
        this.plans = plans;
        this.context = context;
    }



    @Override
    public SelectionViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {

        View viewholder = LayoutInflater.from(parent.getContext()).
                inflate(R.layout.select_plan_list_item, parent, false);

        PlanSelectionAdapter.SelectionViewHolder selectionViewHolder = new PlanSelectionAdapter.SelectionViewHolder(viewholder);
        return selectionViewHolder;
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public void onBindViewHolder(final SelectionViewHolder holder, int position) {

        final WateringPlan plan = plans.get(position);
        holder.bind(plan);

        holder.aSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                // if selected, saving this plan as the default
                sharedPreferences = context.getSharedPreferences(context.getString(R.string.plan_prefs), Context.MODE_PRIVATE);
                editor = sharedPreferences.edit();
                if(isChecked){

                    editor.putString(context.getString(R.string.pref_id),String.valueOf(plan.getId()));
                    editor.putString(context.getString(R.string.pref_title), plan.getTitle());
                    editor.apply();
                    Log.w(TAG, "default changed");
                    broadcastIntent(context);

                }
           }
        });


    }

    @Override
    public int getItemCount() {
        return plans.size();
    }

    /**
     * method for sending a broadcast for other items of this adapter
     * used when a new plan is set as default.
     * @param context
     */
    public void broadcastIntent(Context context){
        Intent intent = new Intent();
        intent.setAction(context.getString(R.string.change_default_plan_intent));
        LocalBroadcastManager mng = LocalBroadcastManager.getInstance(context);
        mng.sendBroadcast(intent);
        //context.sendBroadcast(intent);
        Log.w(TAG, "broadcast sent");


    }


}
