package com.example.nativeapp;

public class SensorBridge {
    static {
        System.loadLibrary("nativeapp");
    }

    public native String getSensorData();
}
