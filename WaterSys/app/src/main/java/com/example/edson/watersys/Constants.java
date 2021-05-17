package com.example.edson.watersys;

import java.util.ArrayList;
import java.util.List;

/**
 * Defines several constants used in the app
 */
public interface Constants {

    // MQTT route constants
    public static final String dispense_water_route = "netosilvan78@gmail.com/system/control/dispenseWater";
    //public static final String schedule_route = "system/app/timing";
    //public static final String amount_route = "system/app/amount";
    //public static final String days_route = "system/app/days";
    String change_program = "netosilvan78@gmail.com/system/hardware/setupProgram";
    String ping_path = "netosilvan78@gmail.com/system/ping";
    String report = "system/report";
    String level_route = "netosilvan78@gmail.com/system/hardware/level";
    public static final String DATABASE_NAME = "plansinfo";



    // Message types sent fro  m the BluetoothChatService Handler
    public static final int MESSAGE_STATE_CHANGE = 1;
    public static final int MESSAGE_READ = 2;
    public static final int MESSAGE_WRITE = 3;
    public static final int MESSAGE_DEVICE_NAME = 4;
    public static final int MESSAGE_TOAST = 5;



    // Key names received from the BluetoothChatService Handler
    public static final String DEVICE_NAME = "device_name";
    public static final String TOAST = "toast";
    public static final String MESSAGE = "message";

    public static final String COMMAND_WATER = "1";
    String COMMAND_SUMMER_MODE = "3";
    String COMMAND_WINTER_MODE = "4";
    String COMMAND_NOMODE = "8";
    String COMMAND_GETDATA = "9";

    String WCONNECTED = "Connection status: Wi-fi Connected";
    String WDISCONNECTED = "Connection status: Wi-fi Disconnected";
    String WUNKNOWN = "Connection status: Unknown";

    public static final String API_BASE_URL = "https://watering-system-api.herokuapp.com";
    public static final String INFOS_URI = "/infos";
    public static final String SINGLE_INFO_URI = "/info";
    public static final String ORDERS_URI = "/orders";
    public static final String SINGLE_ORDER_URI = "/order";

    public static final double AMOUNT_TINY_BIT = 1000;
    public static final double AMOUNT_SMALL_AMOUNT = 2000;
    public static final double AMOUNT_JUST_ENOUGH = 4000;
    public static final double AMOUNT_A_LOT = 8000;
    public static final double AMOUNT_BIG_AMOUNT = 16000;

    // TODO: set those intervals right (space them evenly)
    public static final float[] INTERVAL_TWICE_DAY = {12,0, 24,0 , 0,0 , 0,0 , 0,0};
    public static final float[] INTERVAL_DAILY = {24,0 , 0,0 , 0,0 , 0,0 , 0,0};
    public static final float[] INTERVAL_FIVE_T_WEEK = {24,0 , 24,0 , 24,0 , 24,0 , 48,0};
    public static final float[] INTERVAL_FOUR_T_WEEK = {24,0 , 24,0 , 24,0, 24,0, 72,0};
    public static final float[] INTERVAL_THREE_T_WEEK = {24,0 , 24,0 , 24,0, 96,0, 0,0};
    public static final float[] INTERVAL_TWICE_WEEK = {24,0, 24,0, 120,0, 0,0 , 0,0};
    public static final float[] INTEVAL_WEEKLY = {168,0, 0,0 , 0,0 , 0,0 , 0,0};


}
