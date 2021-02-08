package com.example.edson.watersys.adapters;

import com.example.edson.watersys.R;
import com.example.edson.watersys.database.DBHadler;
import com.example.edson.watersys.objs.WateringPlan;

import android.animation.ValueAnimator;
import android.content.Context;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.support.v7.widget.CardView;
import android.support.v7.widget.RecyclerView;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.List;

public class PlansAdapter extends RecyclerView.Adapter<PlansAdapter.PlansViewHolder> {

    private List<WateringPlan> plans;
    private Boolean detailsVisible = false;
    private Context context;
    private DBHadler db; // we'll need the dbHandler to manage the items
    // Provide a reference to the views for each data item
    // Complex data items may need more than one view per item, and
    // you provide access to all the views for a data item in a view holder
    public static class PlansViewHolder extends RecyclerView.ViewHolder{

        private TextView title; // id: plan_text
        private TextView time; // id: card1_timetxt
        private TextView days; // id: card1_daystxt
        private TextView amount; // id: card1_amounttxt
        private TextView auxTxt1;
        private TextView auxTxt2;
        private TextView auxTxt3;
        private TextView auxTxt4;
        private TextView auto_txt;
        private Button delete; // id: deletebtn
        private Button update; // id: updatebtn

        private ImageView details; // id: moredetails
        private CardView itemCard; // id: view_plans_cardview

        private PlansViewHolder(View itemView) {
            super(itemView);
            title = itemView.findViewById(R.id.plan_text);
            time = itemView.findViewById(R.id.card1_timetxt);
            days = itemView.findViewById(R.id.card1_daystxt);
            amount = itemView.findViewById(R.id.card1_amounttxt);
            details = itemView.findViewById(R.id.moredetails);
            itemCard = itemView.findViewById(R.id.view_plans_cardview);
            auxTxt1 = itemView.findViewById(R.id.card1_txt2);
            auxTxt2 = itemView.findViewById(R.id.card1_txt3);
            auxTxt3 = itemView.findViewById(R.id.card1_txt4);
            auxTxt4 = itemView.findViewById(R.id.card1_txt5);
            auto_txt = itemView.findViewById(R.id.list_item_auto);
            delete = itemView.findViewById(R.id.deletebtn);
            update = itemView.findViewById(R.id.updatebtn);



        }


        private void bind(WateringPlan plan ,Boolean visible){

            if(plan.isAutomatic()){
                auto_txt.setVisibility(visible ? View.VISIBLE : View.GONE);
                auxTxt3.setVisibility(visible ? View.INVISIBLE : View.GONE); // we need this to anchor the buttons

            } else{
                time.setVisibility(visible ? View.VISIBLE : View.GONE);
                days.setVisibility(visible ? View.VISIBLE : View.GONE);
                auxTxt2.setVisibility(visible ? View.VISIBLE : View.GONE);
                auxTxt3.setVisibility(visible ? View.VISIBLE : View.GONE);

                // concatenating the days
                String sdays = "";
                for (String day : plan.getDays()){
                    if(day == null){
                        break;
                    } else sdays += day + " ";
                }

                time.setText(plan.getTime().toString());
                days.setText(sdays);
            }

            amount.setVisibility(visible ? View.VISIBLE : View.GONE);

            // auxiliary text
            auxTxt1.setVisibility(visible ? View.VISIBLE : View.GONE);
            auxTxt4.setVisibility(visible ? View.VISIBLE : View.GONE);
            // btns
            delete.setVisibility(visible ? View.VISIBLE : View.GONE);
            update.setVisibility(visible ? View.VISIBLE : View.GONE);

            if(visible){
                details.setImageResource(R.mipmap.arrow_down_fg);
            } else{
                details.setImageResource(R.mipmap.arrow_right_fg);
            }

            title.setText(plan.getTitle());
            String sAmount = String.valueOf(plan.getAmount()) + " ml";
            amount.setText(sAmount);

        }
    }

    // Provide a suitable constructor (depends on the kind of dataset)
    public PlansAdapter(List<WateringPlan> plans, Context context, DBHadler db){
        this.plans = plans;
        this.context = context;
        this.db = db;
    }


    // Create new views (invoked by the layout manager)
    @Override
    public PlansViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {

        View viewholder = LayoutInflater.from(parent.getContext()).
                inflate(R.layout.my_plans_list_item, parent, false);

        PlansViewHolder plansViewHolder = new PlansViewHolder(viewholder);
        return plansViewHolder;
    }

    // Replace the contents of a view (invoked by the layout manager)
    @Override
    public void onBindViewHolder(final PlansViewHolder holder, final int position) {


        final WateringPlan plan = plans.get(position);


        holder.bind(plan, detailsVisible);
        holder.itemCard.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                detailsVisible = !detailsVisible;
                notifyItemChanged(position);

            }
        });

        holder.details.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                detailsVisible = !detailsVisible;
                notifyItemChanged(position);

            }
        });

        holder.delete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO: add some dialog box before deleting
                db.deletePlan(plan);
                plans.remove(plan);
                notifyItemRemoved(position);
                notifyItemRangeChanged(position, plans.size());
                notifyDataSetChanged();
                Toast.makeText(context, "Plan deleted!", Toast.LENGTH_SHORT).show();
            }
        });

        holder.update.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO: handle update page
                Toast.makeText(context, "not implemented yet", Toast.LENGTH_SHORT).show();
            }
        });


    }

    // Return the size of your dataset (invoked by the layout manager)
    @Override
    public int getItemCount() {
        return plans.size();
    }

    // calculate height to be inflated
    private int calcHeight(Context context){

        try{

            WindowManager windowManager = (WindowManager)context.getSystemService(Context.WINDOW_SERVICE);
            DisplayMetrics dimension = new DisplayMetrics();
            windowManager.getDefaultDisplay().getMetrics(dimension);
            return dimension.heightPixels;

        } catch (NullPointerException e){
            e.printStackTrace();
        }

        return -1;

    }




}
