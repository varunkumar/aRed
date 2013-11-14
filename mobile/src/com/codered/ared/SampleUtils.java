package com.codered.ared;

import android.app.Activity;
import android.util.DisplayMetrics;

/// A java utility class used by the QCAR SDK samples.

public class SampleUtils {

    // Constant for inches to consider it a tablet
    private final static float DEVICEINFO_TABLET_MINIMUM_INCHES = 6.0f;
    
    // Checks if the device is a tablet
    public static boolean isTablet( Activity activity ) {
        
        boolean isTablet = false;
        
        DisplayMetrics metrics = new DisplayMetrics();
        activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
        float physicalWidthInches = metrics.widthPixels / metrics.xdpi;
        float physicalHeightInches = metrics.heightPixels / metrics.ydpi;
        double deviceInches = Math.sqrt( physicalWidthInches * physicalWidthInches + physicalHeightInches * physicalHeightInches );
        DebugLog.LOGI("Screen inches: " + deviceInches);
        
        if ( deviceInches >= DEVICEINFO_TABLET_MINIMUM_INCHES)
            isTablet = true;
        
        return isTablet;
    }
    
}
