package com.example.edson.watersys;

/**
 * Defines several constants used between {@link BluetoothService} and the UI.
 */
public interface Constants {

    // Message types sent from the BluetoothChatService Handler
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

}
