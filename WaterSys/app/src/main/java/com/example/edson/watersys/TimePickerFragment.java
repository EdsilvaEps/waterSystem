package com.example.edson.watersys;

import android.app.Dialog;
import android.support.v4.app.DialogFragment;
import android.app.TimePickerDialog;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import android.widget.TimePicker;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Locale;

public class TimePickerFragment extends DialogFragment implements TimePickerDialog.OnTimeSetListener {

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {

        // Use the current time as the default values for the picker
        final Calendar c = Calendar.getInstance();
        int currhour = c.get(Calendar.HOUR_OF_DAY);
        int currminute = c.get(Calendar.MINUTE);
        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm", Locale.ENGLISH);

        return new TimePickerDialog(getActivity(), this, currhour, currminute, android.text.format.DateFormat.is24HourFormat(getActivity()));


    }

    @Override
    public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
        // Do something with the time chosen by the user
        Button tv1= getActivity().findViewById(R.id.timebtn);


        int hour = view.getCurrentHour();
        String sHour = "00";
        if(hour < 10){
            sHour = "0"+hour;
        } else {
            sHour = String.valueOf(hour);
        }

        int mminute = view.getCurrentMinute();
        String sMinute = "00";
        if(mminute < 10){
            sMinute = "0"+minute;
        } else {
            sMinute = String.valueOf(minute);
        }

        String timestr = sHour+":"+sMinute;
        tv1.setText(timestr);






    }
}
