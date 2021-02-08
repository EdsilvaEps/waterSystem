package com.example.edson.watersys.objs;

import org.json.JSONException;
import org.json.JSONObject;

import java.sql.Time;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Object class that represents a single watering plan with
 * wth their features
 */
public class WateringPlan {
    private int id;
    private String title;
    private Time   time;
    private ArrayList<String> days;
    private Integer amount;
    private Boolean active;
    private Boolean autoWatering;
    private Integer timezone;


    public WateringPlan(){

    }

    public WateringPlan(int id, String title, Time time, ArrayList<String> days, int amount, boolean auto){
        this.setId(id);
        this.setTitle(title);
        this.setTime(time);
        this.setDays(days);
        this.setAmount(amount);
        this.active = false;
        this.timezone = -14400; // Manaus tz (gmt -4)
        this.autoWatering = auto;

    }

    public WateringPlan(int id, String title, Time time, String days, int amount, boolean auto){
        this.setId(id);
        this.setTitle(title);
        this.setTime(time);
        this.setDaysFromString(days);
        this.setAmount(amount);
        this.active = false;
        this.autoWatering = auto;
    }

    // automatic watering
    public WateringPlan(int id){
        this.setId(id);
        this.setTitle("auto watering");
        this.setAmount(1000);
        this.active = false;
        this.timezone = -14400; // Manaus tz (gmt -4)
        this.autoWatering = true;

    }


    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public Time getTime() {
        return time;
    }

    public Boolean isActive() {
        return active;
    }

    public void setActive(Boolean active) {
        this.active = active;
    }

    public void setTime(Time time) {
        this.time = time;
    }

    public ArrayList<String> getDays() {
        return days;
    }

    public void setDays(ArrayList<String> days) {
        this.days = days;
    }

    public Integer getAmount() {
        return amount;
    }

    public void setAmount(Integer amount) {
        this.amount = amount;
    }

    public  Boolean isAutomatic(){ return autoWatering; }

    public void setAutoWatering(Boolean auto){ this.autoWatering = auto; }

    public String getStringOfDays() {
        String daysString = "";
        for(String day : this.days){
            daysString += day;
            daysString += ",";
        }

        return daysString;
    }

    public void setDaysFromString(String daysString){
        Pattern pattern = Pattern.compile("(\\w+),");
        Matcher matcher = pattern.matcher(daysString);
        while (matcher.find()){
            System.out.println(matcher.group());
        }

        // temporary until we figure the regex out
        ArrayList<String> weekdays = new ArrayList<String>();
        weekdays.add("Mon");
        weekdays.add("Tue");
        weekdays.add("Wed");
        weekdays.add("Thu");
        weekdays.add("Fri");
        this.days = weekdays;

    }

    // get json string for automatic watering
    public String getAutoJsonString(){

        // codificando o array de dias
        int[] weekDays = new int[]{-1,-1,-1,-1,-1,-1,-1};

        JSONObject obj = new JSONObject();
        String regex = "(?<=\"deadlineDays\":)\"|\"(?=\\}\\])";

        try {
            obj.put("amountWater", this.amount);
            obj.put("gmtTimezone", this.timezone);
            obj.put("deadlineHour", 0);
            obj.put("deadlineMinute", 0);
            obj.put("deadlineSecond", 0);
            obj.put("deadlineDays", Arrays.toString(weekDays));
            obj.put("automaticWatering", true);

        } catch (JSONException e) {
            e.printStackTrace();
        }
        String msg = obj.toString().replaceAll(regex, "");

        return msg;

    }


    public String getJsonString(){

        // codificando o array de dias
        int[] weekDays = new int[]{-1,-1,-1,-1,-1,-1,-1};
        int i = 0;

        for(String day : this.days){
            if(day == null) break;
            if(day.equals("Sun")) weekDays[i] = 0;
            if(day.equals("Mon")) weekDays[i] = 1;
            if(day.equals("Tue")) weekDays[i] = 2;
            if(day.equals("Wed")) weekDays[i] = 3;
            if(day.equals("Thu")) weekDays[i] = 4;
            if(day.equals("Fri")) weekDays[i] = 5;
            if(day.equals("Sat")) weekDays[i] = 6;
            i ++;
        }

        JSONObject obj = new JSONObject();
        String regex = "(?<=\"deadlineDays\":)\"|\"(?=\\}\\])";


        try {
            obj.put("amountWater", this.amount);
            obj.put("gmtTimezone", this.timezone);
            obj.put("deadlineHour", this.time.getHours());
            obj.put("deadlineMinute", this.time.getMinutes());
            obj.put("deadlineSecond", this.time.getSeconds());
            obj.put("deadlineDays", Arrays.toString(weekDays));
            obj.put("automaticWatering", false);

        } catch (JSONException e) {
            e.printStackTrace();
        }
        String msg = obj.toString().replaceAll(regex, "");

        return msg;


    }

}
