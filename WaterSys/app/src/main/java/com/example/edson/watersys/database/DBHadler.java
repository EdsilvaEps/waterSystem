package com.example.edson.watersys.database;
import com.example.edson.watersys.objs.WateringPlan;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import java.sql.Time;
import java.util.ArrayList;
import java.util.List;

/**
 * Class that handles the database CRUD using SQLite
 *
 * UPDATE: the designation for 'days' has changed from the first version,
 * instead of big string array of days, we are now using 7 different fields
 *
 */
public class DBHadler extends SQLiteOpenHelper {

    private static final int DATABASE_VERSION = 1;
    private static final String DATABASE_NAME = "plansinfo";
    private static final String TAG = "DBHandler";
    // table features
    private static final String TABLE_PLANS = "plans"; // table name
    private static final String KEY_ID = "id";
    private static final String KEY_TITLE = "title";
    private static final String KEY_TIME = "time";
    //private static final String KEY_DAYS = "days";
    private static final String KEY_AMOUNT = "amount";

    private static final String KEY_DAY_1 = "day1";
    private static final String KEY_DAY_2 = "day2";
    private static final String KEY_DAY_3 = "day3";
    private static final String KEY_DAY_4 = "day4";
    private static final String KEY_DAY_5 = "day5";
    private static final String KEY_DAY_6 = "day6";
    private static final String KEY_DAY_7 = "day7";





    public DBHadler(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    // method called first time when database is created
    // We usually create tables and the initialize here.
    @Override
    public void onCreate(SQLiteDatabase db) {
        // creating our table
        // PS: time will store in STRING format like "HH:MM:SS"
        String CREATE_PLANS_TABLE = "CREATE TABLE " + TABLE_PLANS + "(" +
                KEY_ID + " INTEGER PRIMARY KEY," + KEY_TITLE + " TEXT," +
                KEY_TIME + " TEXT,"  + KEY_AMOUNT + " TEXT," + KEY_DAY_1 + " TEXT," +
                KEY_DAY_2 + " TEXT," + KEY_DAY_3 + " TEXT," + KEY_DAY_4 + " TEXT," +
                KEY_DAY_5 + " TEXT," + KEY_DAY_6 + " TEXT," + KEY_DAY_7 + " TEXT" + ")";

        Log.w(TAG, CREATE_PLANS_TABLE);
        db.execSQL(CREATE_PLANS_TABLE);

    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // drop older table if existed
        db.execSQL(("DROP TABLE IF EXISTS " + TABLE_PLANS));
        // creating tables again
        onCreate(db);

    }

    // CREATE new record
    public  void addPlan(WateringPlan plan){
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(KEY_TITLE, plan.getTitle());
        values.put(KEY_TIME, plan.getTime().toString());
        values.put(KEY_AMOUNT, plan.getAmount().toString());

        ArrayList<String> days = plan.getDays();
        values.put(KEY_DAY_1, days.get(0));
        values.put(KEY_DAY_2, days.get(1));
        values.put(KEY_DAY_3, days.get(2));
        values.put(KEY_DAY_4, days.get(3));
        values.put(KEY_DAY_5, days.get(4));
        values.put(KEY_DAY_6, days.get(5));
        values.put(KEY_DAY_7, days.get(6));


        // putting the days' values into the database
        /*for(Integer i = 1; i <= 7; i++){
            String key = "day" + i.toString();
            ArrayList<String> days = plan.getDays();
            values.put(key, days.get(i-1));

        }*/

        // insert a new row
        db.insert(TABLE_PLANS, null, values);
        db.close(); // closing database connection

        Log.w(TAG, "plan added");
    }

    // READ an existing record
    public WateringPlan getPlan(int id){
       SQLiteDatabase db = this.getReadableDatabase();
       Cursor cursor = db.query(TABLE_PLANS, new String[] { KEY_ID, KEY_TITLE,
       KEY_TIME, KEY_AMOUNT, KEY_DAY_1, KEY_DAY_2, KEY_DAY_3, KEY_DAY_4, KEY_DAY_5, KEY_DAY_6, KEY_DAY_7}, KEY_ID + "=?",
       new String[] { String.valueOf(id) }, null, null, null, null);

       // get the fields from the database, parse them if necessary
        // and add to a new watering plan object
       if(cursor != null) {
           cursor.moveToFirst();
           return getParsedPlan(cursor);
       }

       return null;



    }

    // READ all plans
    public List<WateringPlan> getAllPlans(){
        List<WateringPlan> plansList = new ArrayList<WateringPlan>();

        // Select all query
        String selectQuery = "SELECT * FROM " + TABLE_PLANS;
        SQLiteDatabase db = this.getWritableDatabase();
        Cursor cursor = db.rawQuery(selectQuery, null);

        // looping through all rows and adding to list
        if(cursor.moveToFirst()){

            do {

                plansList.add(getParsedPlan(cursor));


            } while (cursor.moveToNext());
        }

        return plansList;

    }

    // COUNT all records in database
    public int getPlansCount(){
        String countQuery = "SELECT * FROM " + TABLE_PLANS;
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery(countQuery, null);
        int count = cursor.getCount();
        cursor.close();
        // return count
        return count;
    }

    //UPDATE a record
    public int updatePlan(WateringPlan plan) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(KEY_TITLE, plan.getTitle());
        values.put(KEY_TIME, plan.getTime().toString());
        values.put(KEY_AMOUNT, plan.getAmount().toString());

        // putting the days' values into the database
        for(Integer i = 1; i <= 7; i++){
            String key = "day" + i.toString();
            ArrayList<String> days = plan.getDays();
            values.put(key, days.get(i-1));

        }

        return db.update(TABLE_PLANS, values, KEY_ID + " = ?",
                new String[]{String.valueOf(plan.getId())});
    }

    // DELETE a record
    public void deletePlan(WateringPlan plan) {
        SQLiteDatabase db = this.getWritableDatabase();
        db.delete(TABLE_PLANS, KEY_ID + " = ?",
                new String[] { String.valueOf(plan.getId()) });
        db.close();
    }

    public void deleteDB(SQLiteDatabase db){
        db.execSQL(("DROP TABLE IF EXISTS " + TABLE_PLANS));

    }

    /**
     * Takes the database cursor, extracts the proper fields,
     * parse them and returns a WateringPlan object
     * @param cursor
     * @return plan
     */
    private WateringPlan getParsedPlan(Cursor cursor){
        Integer plan_id = Integer.parseInt(cursor.getString(0));
        String title = cursor.getString(1);
        Time time = Time.valueOf(cursor.getString(2));
        //String days = cursor.getString(3); 4
        Integer amount = Integer.parseInt(cursor.getString(3));

        ArrayList<String> days = new ArrayList<>();
        for(int i = 0; i <= 6; i++){
            // days cursor starts at index 4
            days.add(cursor.getString(i+4));

        }


        return new WateringPlan(plan_id, title, time, days, amount);

    }








}
