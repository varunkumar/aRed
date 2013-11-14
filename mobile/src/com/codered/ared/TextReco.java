package com.codered.ared;

import java.lang.ref.WeakReference;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.text.Layout.Alignment;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.util.DisplayMetrics;
import android.view.GestureDetector;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.qualcomm.QCAR.QCAR;
import com.codered.ared.R;

/** The main activity for the TextReco sample. */
public class TextReco extends Activity {
    private final static int COLOR_TRANSPARENT = Color.argb(0, 255, 255, 255);
    private final static int COLOR_OPAQUE = Color.argb(178, 0, 0, 0);
    private final static int WORDLIST_MARGIN = 10;

    // Focus mode constants:
    private static final int FOCUS_MODE_NORMAL = 0;
    private static final int FOCUS_MODE_CONTINUOUS_AUTO = 1;

    // Application status constants:
    private static final int APPSTATUS_UNINITED = -1;
    private static final int APPSTATUS_INIT_APP = 0;
    private static final int APPSTATUS_INIT_QCAR = 1;
    private static final int APPSTATUS_INIT_TRACKER = 2;
    private static final int APPSTATUS_INIT_APP_AR = 3;
    private static final int APPSTATUS_LOAD_TRACKER = 4;
    private static final int APPSTATUS_INITED = 5;
    private static final int APPSTATUS_CAMERA_STOPPED = 6;
    private static final int APPSTATUS_CAMERA_RUNNING = 7;

    // Name of the native dynamic libraries to load:
    private static final String NATIVE_LIB_SAMPLE = "TextReco";
    private static final String NATIVE_LIB_QCAR = "QCAR";

    // Constants for Hiding/Showing Loading dialog
    static final int HIDE_LOADING_DIALOG = 0;
    static final int SHOW_LOADING_DIALOG = 1;

    // Our OpenGL view:
    private QCARSampleGLView mGlView;

    // Our renderer:
    private TextRecoRenderer mRenderer;

    // Display size of the device:
    private int mScreenWidth = 0;
    private int mScreenHeight = 0;

    // Constant representing invalid screen orientation to trigger a query:
    private static final int INVALID_SCREEN_ROTATION = -1;

    // Last detected screen rotation:
    private int mLastScreenRotation = INVALID_SCREEN_ROTATION;

    // The current application status:
    private int mAppStatus = APPSTATUS_UNINITED;

    // The async tasks to initialize the QCAR SDK:
    private InitQCARTask mInitQCARTask;
    private LoadTrackerTask mLoadTrackerTask;

    // An object used for synchronizing QCAR initialization, dataset loading and
    // the Android onDestroy() life cycle event. If the application is destroyed
    // while a data set is still being loaded, then we wait for the loading
    // operation to finish before shutting down QCAR:
    private Object mShutdownLock = new Object();

    // QCAR initialization flags:
    private int mQCARFlags = 0;

    // Detects the double tap gesture for launching the Camera menu
    private GestureDetector mGestureDetector;

    // Contextual Menu Options for Autofocus
    private boolean mContAutofocus = false;

    private LinearLayout mUILayout;

    private native boolean autofocus();

    private native boolean setFocusMode(int mode);

    private boolean mIsTablet;

    /** Static initializer block to load native libraries on start-up. */
    static {
        loadLibrary(NATIVE_LIB_QCAR);
        loadLibrary(NATIVE_LIB_SAMPLE);
    }

    /**
     * Creates a handler to update the status of the Loading Dialog from an UI
     * Thread
     */
    static class LoadingDialogHandler extends Handler {
        private final WeakReference<TextReco> mTextReco;

        LoadingDialogHandler(TextReco textReco) {
            mTextReco = new WeakReference<TextReco>(textReco);
        }

        public void handleMessage(Message msg) {
            TextReco textReco = mTextReco.get();
            if (textReco == null) {
                return;
            }

            if (msg.what == SHOW_LOADING_DIALOG) {
                textReco.showLoupe(false);
            } else if (msg.what == HIDE_LOADING_DIALOG) {
                textReco.showLoupe(true);
            }
        }
    }

    private Handler loadingDialogHandler = new LoadingDialogHandler(this);

    /** An async task to initialize QCAR asynchronously. */
    private class InitQCARTask extends AsyncTask<Void, Integer, Boolean> {
        // Initialize with invalid value:
        private int mProgressValue = -1;

        protected Boolean doInBackground(Void... params) {
            // Prevent the onDestroy() method to overlap with initialization:
            synchronized (mShutdownLock) {
                QCAR.setInitParameters(TextReco.this, mQCARFlags);

                do {
                    // QCAR.init() blocks until an initialization step is
                    // complete, then it proceeds to the next step and reports
                    // progress in percents (0 ... 100%).
                    // If QCAR.init() returns -1, it indicates an error.
                    // Initialization is done when progress has reached 100%.
                    mProgressValue = QCAR.init();

                    // Publish the progress value:
                    publishProgress(mProgressValue);

                    // We check whether the task has been canceled in the
                    // meantime (by calling AsyncTask.cancel(true)).
                    // and bail out if it has, thus stopping this thread.
                    // This is necessary as the AsyncTask will run to completion
                    // regardless of the status of the component that
                    // started is.
                } while (!isCancelled() && mProgressValue >= 0
                        && mProgressValue < 100);

                return (mProgressValue > 0);
            }
        }

        protected void onProgressUpdate(Integer... values) {
            // Do something with the progress value "values[0]", e.g. update
            // splash screen, progress bar, etc.
        }

        protected void onPostExecute(Boolean result) {
            // Done initializing QCAR, proceed to next application
            // initialization status:
            if (result) {
                DebugLog.LOGD("InitQCARTask::onPostExecute: QCAR "
                        + "initialization successful");

                updateApplicationStatus(APPSTATUS_INIT_TRACKER);
            } else {
                // Create dialog box for display error:
                AlertDialog dialogError = new AlertDialog.Builder(TextReco.this)
                        .create();

                dialogError.setButton(DialogInterface.BUTTON_POSITIVE, "Close",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                // Exiting application:
                                System.exit(1);
                            }
                        });

                String logMessage;

                // NOTE: Check if initialization failed because the device is
                // not supported. At this point the user should be informed
                // with a message.
                if (mProgressValue == QCAR.INIT_DEVICE_NOT_SUPPORTED) {
                    logMessage = "Failed to initialize QCAR because this "
                            + "device is not supported.";
                } else {
                    logMessage = "Failed to initialize QCAR.";
                }

                // Log error:
                DebugLog.LOGE("InitQCARTask::onPostExecute: " + logMessage
                        + " Exiting.");

                // Show dialog box with error message:
                dialogError.setMessage(logMessage);
                dialogError.show();
            }
        }
    }

    /** An async task to load the tracker data asynchronously. */
    private class LoadTrackerTask extends AsyncTask<Void, Integer, Boolean> {
        protected Boolean doInBackground(Void... params) {
            // Prevent the onDestroy() method to overlap:
            synchronized (mShutdownLock) {
                // Load the tracker data set:
                return (loadTrackerData() > 0);
            }
        }

        protected void onPostExecute(Boolean result) {
            DebugLog.LOGD("LoadTrackerTask::onPostExecute: execution "
                    + (result ? "successful" : "failed"));

            if (result) {
                // Done loading the tracker, update application status:
                updateApplicationStatus(APPSTATUS_INITED);
            } else {
                // Create dialog box for display error:
                AlertDialog dialogError = new AlertDialog.Builder(TextReco.this)
                        .create();

                dialogError.setButton(DialogInterface.BUTTON_POSITIVE, "Close",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                // Exiting application:
                                System.exit(1);
                            }
                        });

                // Show dialog box with error message:
                dialogError.setMessage("Failed to load tracker data.");
                dialogError.show();
            }
        }
    }

    /** Stores screen dimensions */
    private void storeScreenDimensions() {
        // Query display dimensions:
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        mScreenWidth = metrics.widthPixels;
        mScreenHeight = metrics.heightPixels;
    }

    /**
     * Called when the activity first starts or the user navigates back to an
     * activity.
     */
    protected void onCreate(Bundle savedInstanceState) {
        DebugLog.LOGD("TextReco::onCreate");
        super.onCreate(savedInstanceState);

        // Query the QCAR initialization flags:
        mQCARFlags = getInitializationFlags();

        // Creates the GestureDetector listener for processing double tap
        mGestureDetector = new GestureDetector(this, new GestureListener());

        // Update the application status to start initializing application:
        updateApplicationStatus(APPSTATUS_INIT_APP);

        // detect if we are a table or not
        mIsTablet = SampleUtils.isTablet(this);
    }

    /** Configure QCAR with the desired version of OpenGL ES. */
    private int getInitializationFlags() {
        int flags = 0;

        // Query the native code:
        if (getOpenGlEsVersionNative() == 1) {
            flags = QCAR.GL_11;
        } else {
            flags = QCAR.GL_20;
        }

        return flags;
    }

    /**
     * Native method for querying the OpenGL ES version. Returns 1 for OpenGl ES
     * 1.1, returns 2 for OpenGl ES 2.0.
     */
    public native int getOpenGlEsVersionNative();

    /** Native tracker initialization and deinitialization. */
    public native int initTracker();

    public native void deinitTracker();

    /** Native functions to load and destroy tracking data. */
    public native int loadTrackerData();

    public native void destroyTrackerData();

    /** Native sample initialization. */
    public native void onQCARInitializedNative();

    /** Native methods for starting and stopping the camera. */
    private native void startCamera();

    private native void stopCamera();

    /**
     * Native method for setting / updating the projection matrix for AR content
     * rendering
     */
    private native void setProjectionMatrix();

    /** Set the region of interest for the text recognition */
    private native void setROI(float centerX, float centerY, float width,
            float height);

    /** Called when the activity will start interacting with the user. */
    protected void onResume() {
        DebugLog.LOGD("TextReco::onResume");
        super.onResume();

        // QCAR-specific resume operation
        QCAR.onResume();

        // We may start the camera only if the QCAR SDK has already been
        // initialized
        if (mAppStatus == APPSTATUS_CAMERA_STOPPED) {
            updateApplicationStatus(APPSTATUS_CAMERA_RUNNING);
        }

        // Resume the GL view:
        if (mGlView != null) {
            mGlView.setVisibility(View.VISIBLE);
            mGlView.onResume();
        }
    }

    private void updateActivityOrientation() {
        Configuration config = getResources().getConfiguration();

        boolean isPortrait = false;

        switch (config.orientation) {
        case Configuration.ORIENTATION_PORTRAIT:
            isPortrait = true;
            break;
        case Configuration.ORIENTATION_LANDSCAPE:
            isPortrait = false;
            break;
        case Configuration.ORIENTATION_UNDEFINED:
        default:
            break;
        }

        DebugLog.LOGI("Activity is in "
                + (isPortrait ? "PORTRAIT" : "LANDSCAPE"));
        setActivityPortraitMode(isPortrait);
    }

    /**
     * Updates projection matrix and viewport after a screen rotation change was
     * detected.
     */
    public void updateRenderView() {
        int currentScreenRotation = getWindowManager().getDefaultDisplay()
                .getRotation();
        if (currentScreenRotation != mLastScreenRotation) {
            // Set projection matrix if there is already a valid one:
            if (QCAR.isInitialized()
                    && (mAppStatus == APPSTATUS_CAMERA_RUNNING)) {
                DebugLog.LOGD("TextReco::updateRenderView");

                // Query display dimensions:
                storeScreenDimensions();

                // Update viewport via renderer:
                mRenderer.updateRendering(mScreenWidth, mScreenHeight);

                // Update projection matrix:
                setProjectionMatrix();

                // Cache last rotation used for setting projection matrix:
                mLastScreenRotation = currentScreenRotation;
            }
        }
    }

    /** Callback for configuration changes the activity handles itself */
    public void onConfigurationChanged(Configuration config) {
        DebugLog.LOGD("TextReco::onConfigurationChanged");
        super.onConfigurationChanged(config);

        updateActivityOrientation();

        storeScreenDimensions();

        // Invalidate screen rotation to trigger query upon next render call:
        mLastScreenRotation = INVALID_SCREEN_ROTATION;
    }

    /** Called when the system is about to start resuming a previous activity. */
    protected void onPause() {
        DebugLog.LOGD("TextReco::onPause");
        super.onPause();

        if (mGlView != null) {
            mGlView.setVisibility(View.INVISIBLE);
            mGlView.onPause();
        }

        if (mAppStatus == APPSTATUS_CAMERA_RUNNING) {
            updateApplicationStatus(APPSTATUS_CAMERA_STOPPED);
        }

        // QCAR-specific pause operation
        QCAR.onPause();
    }

    /** Native function to deinitialize the application. */
    private native void deinitApplicationNative();

    /** The final call you receive before your activity is destroyed. */
    protected void onDestroy() {
        DebugLog.LOGD("TextReco::onDestroy");
        super.onDestroy();

        // Cancel potentially running tasks
        if (mInitQCARTask != null
                && mInitQCARTask.getStatus() != InitQCARTask.Status.FINISHED) {
            mInitQCARTask.cancel(true);
            mInitQCARTask = null;
        }

        if (mLoadTrackerTask != null
                && mLoadTrackerTask.getStatus() != LoadTrackerTask.Status.FINISHED) {
            mLoadTrackerTask.cancel(true);
            mLoadTrackerTask = null;
        }

        // Ensure that all asynchronous operations to initialize QCAR
        // and loading the tracker datasets do not overlap:
        synchronized (mShutdownLock) {

            // Do application deinitialization in native code:
            deinitApplicationNative();

            // Destroy the tracking data set:
            destroyTrackerData();

            // Deinit the tracker:
            deinitTracker();

            // Deinitialize QCAR SDK:
            QCAR.deinit();
        }

        System.gc();
    }

    /**
     * NOTE: this method is synchronized because of a potential concurrent
     * access by TextReco::onResume() and InitQCARTask::onPostExecute().
     */
    private synchronized void updateApplicationStatus(int appStatus) {
        // Exit if there is no change in status:
        if (mAppStatus == appStatus)
            return;

        // Store new status value:
        mAppStatus = appStatus;

        // Execute application state-specific actions:
        switch (mAppStatus) {
        case APPSTATUS_INIT_APP:
            // Initialize application elements that do not rely on QCAR
            // initialization:
            initApplication();

            // Proceed to next application initialization status:
            updateApplicationStatus(APPSTATUS_INIT_QCAR);
            break;

        case APPSTATUS_INIT_QCAR:
            // Initialize QCAR SDK asynchronously to avoid blocking the
            // main (UI) thread.
            //
            // NOTE: This task instance must be created and invoked on the
            // UI thread and it can be executed only once!
            try {
                mInitQCARTask = new InitQCARTask();
                mInitQCARTask.execute();
            } catch (Exception e) {
                DebugLog.LOGE("Initializing QCAR SDK failed");
            }
            break;

        case APPSTATUS_INIT_TRACKER:
            // Initialize the ImageTracker:
            if (initTracker() > 0) {
                // Proceed to next application initialization status:
                updateApplicationStatus(APPSTATUS_INIT_APP_AR);
            }
            break;

        case APPSTATUS_INIT_APP_AR:
            // Initialize Augmented Reality-specific application elements
            // that may rely on the fact that the QCAR SDK has been
            // already initialized:
            initApplicationAR();

            // Proceed to next application initialization status:
            updateApplicationStatus(APPSTATUS_LOAD_TRACKER);
            break;

        case APPSTATUS_LOAD_TRACKER:
            // Load the tracking data set:
            //
            // NOTE: This task instance must be created and invoked on the
            // UI thread and it can be executed only once!
            try {
                mLoadTrackerTask = new LoadTrackerTask();
                mLoadTrackerTask.execute();
            } catch (Exception e) {
                DebugLog.LOGE("Loading tracking data set failed");
            }
            break;

        case APPSTATUS_INITED:
            // Hint to the virtual machine that it would be a good time to
            // run the garbage collector:
            //
            // NOTE: This is only a hint. There is no guarantee that the
            // garbage collector will actually be run.
            System.gc();

            // Native post initialization:
            onQCARInitializedNative();

            // Activate the renderer:
            mRenderer.mIsActive = true;

            // Now add the GL surface view. It is important
            // that the OpenGL ES surface view gets added
            // BEFORE the camera is started and video
            // background is configured.
            addContentView(mGlView, new LayoutParams(LayoutParams.MATCH_PARENT,
                    LayoutParams.MATCH_PARENT));

            // Sets the UILayout to be drawn in front of the camera
            mUILayout.bringToFront();

            // Start the camera:
            updateApplicationStatus(APPSTATUS_CAMERA_RUNNING);

            break;

        case APPSTATUS_CAMERA_STOPPED:
            // Call the native function to stop the camera:
            stopCamera();
            break;

        case APPSTATUS_CAMERA_RUNNING:
            // Call the native function to start the camera:
            startCamera();

            // Hides the Loading Dialog
            loadingDialogHandler.sendEmptyMessage(HIDE_LOADING_DIALOG);

            // Set continuous auto-focus if supported by the device,
            // otherwise default back to regular auto-focus mode.
            // This will be activated by a tap to the screen in this
            // application.
            if (!setFocusMode(FOCUS_MODE_CONTINUOUS_AUTO)) {
                mContAutofocus = false;
                setFocusMode(FOCUS_MODE_NORMAL);
            } else {
                mContAutofocus = true;
            }
            break;

        default:
            throw new RuntimeException("Invalid application state");
        }
    }

    /** Tells native code whether we are in portait or landscape mode */
    private native void setActivityPortraitMode(boolean isPortrait);

    /** Initialize application GUI elements that are not related to AR. */
    private void initApplication() {
        // Set the screen orientation:
        // NOTE: Use SCREEN_ORIENTATION_LANDSCAPE or SCREEN_ORIENTATION_PORTRAIT
        // to lock the screen orientation for this activity.
        int screenOrientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;

        // Apply screen orientation
        setRequestedOrientation(screenOrientation);

        updateActivityOrientation();

        // Query display dimensions:
        storeScreenDimensions();

        // As long as this window is visible to the user, keep the device's
        // screen turned on and bright:
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    /** Native function to initialize the application. */
    private native void initApplicationNative(int width, int height);

    /** Initializes AR application components. */
    private void initApplicationAR() {
        // Do application initialization in native code (e.g. registering
        // callbacks, etc.):
        initApplicationNative(mScreenWidth, mScreenHeight);

        // Create OpenGL ES view:
        int depthSize = 16;
        int stencilSize = 0;
        boolean translucent = QCAR.requiresAlpha();

        mGlView = new QCARSampleGLView(this);
        mGlView.init(mQCARFlags, translucent, depthSize, stencilSize);

        mRenderer = new TextRecoRenderer();
        mRenderer.mActivity = this;
        mGlView.setRenderer(mRenderer);

        LayoutInflater inflater = LayoutInflater.from(this);
        mUILayout = (LinearLayout) inflater.inflate(R.layout.loupe_overlay,
                null, false);

        mUILayout.setVisibility(View.VISIBLE);
        mUILayout.setBackgroundColor(COLOR_TRANSPARENT);

        // Shows the loading indicator at start
        loadingDialogHandler.sendEmptyMessage(SHOW_LOADING_DIALOG);

        // Adds the inflated layout to the view
        addContentView(mUILayout, new LayoutParams(LayoutParams.MATCH_PARENT,
                LayoutParams.MATCH_PARENT));

    }

    void updateWordListUI(final List<String> words) {
        runOnUiThread(new Runnable() {

            public void run() {
                LinearLayout wordListLayout = (LinearLayout) mUILayout
                        .findViewById(R.id.wordList);
                wordListLayout.removeAllViews();

                if (words.size() > 0) {
                    LayoutParams params = wordListLayout.getLayoutParams();
                    // Changes the height and width to the specified *pixels*
                    int maxTextHeight = params.height - (2 * WORDLIST_MARGIN);

                    int[] textInfo = fontSizeForTextHeight(maxTextHeight,
                            words.size(), params.width, 32, 12);

                    int count = -1;
                    int nbWords = textInfo[2]; // number of words we can display
                    for (String word : words) {
                        count++;
                        if (count == nbWords) {
                            break;
                        }
                        TextView tv = new TextView(TextReco.this);
                        tv.setText(word);
                        LinearLayout.LayoutParams txtParams = new LinearLayout.LayoutParams(
                                LayoutParams.MATCH_PARENT,
                                LayoutParams.WRAP_CONTENT);
                        txtParams.setMargins(0, (count == 0) ? WORDLIST_MARGIN
                                : 0, 0,
                                (count == (nbWords - 1)) ? WORDLIST_MARGIN : 0);
                        tv.setLayoutParams(txtParams);
                        tv.setGravity(Gravity.CENTER_VERTICAL
                                | Gravity.CENTER_HORIZONTAL);
                        tv.setTextSize(textInfo[0]);
                        tv.setHeight(textInfo[1]);

                        //wordListLayout.addView(tv);
                    }
                }
            }
        });
    }

    /** A helper for loading native libraries stored in "libs/armeabi*". */
    public static boolean loadLibrary(String nLibName) {
        try {
            System.loadLibrary(nLibName);
            DebugLog.LOGI("Native library lib" + nLibName + ".so loaded");
            return true;
        } catch (UnsatisfiedLinkError ulee) {
            DebugLog.LOGE("The library lib" + nLibName
                    + ".so could not be loaded");
        } catch (SecurityException se) {
            DebugLog.LOGE("The library lib" + nLibName
                    + ".so was not allowed to be loaded");
        }

        return false;
    }

    public boolean onTouchEvent(MotionEvent event) {
        // Process the Gestures
        return mGestureDetector.onTouchEvent(event);
    }

    /**
     * Process Double Tap event for showing the Camera options menu
     */
    private class GestureListener extends
            GestureDetector.SimpleOnGestureListener {
        public boolean onDown(MotionEvent e) {
            return true;
        }

        public boolean onSingleTapUp(MotionEvent e) {
            // Calls the Autofocus Native Method
            autofocus();

            return true;
        }
    }

    private void showLoupe(boolean isActive) {
        int width = mScreenWidth;
        int height = mScreenHeight;

        // width of margin is :
        // 5% of the width of the screen for a phone
        // 20% of the width of the screen for a tablet
        int marginWidth = mIsTablet ? (width * 20) / 100 : (width * 5) / 100;

        // loupe height is :
        // 33% of the screen height for a phone
        // 20% of the screen height for a tablet
        int loupeHeight = mIsTablet ? (height * 10) / 100 : (height / 2);

        // lupue width takes the width of the screen minus 2 margins
        int loupeWidth = width - (2 * marginWidth);

        int wordListHeight = height - (loupeHeight + marginWidth);

        // definition of the region of interest
        setROI(width / 2, marginWidth + (loupeHeight / 2), loupeWidth,
                loupeHeight);

        // Gets a reference to the loading dialog
        View loadingIndicator = mUILayout.findViewById(R.id.loading_indicator);

        LinearLayout loupeLayout = (LinearLayout) mUILayout
                .findViewById(R.id.loupeLayout);

        ImageView topMargin = (ImageView) mUILayout
                .findViewById(R.id.topMargin);

        ImageView leftMargin = (ImageView) mUILayout
                .findViewById(R.id.leftMargin);
        
        ImageView rightMargin = (ImageView) mUILayout
                .findViewById(R.id.rightMargin);
        
        ImageView loupeArea = (ImageView) mUILayout.findViewById(R.id.loupe);

        LinearLayout wordListLayout = (LinearLayout) mUILayout
                .findViewById(R.id.wordList);

        wordListLayout.setBackgroundColor(COLOR_OPAQUE);

        if (isActive) {
            topMargin.getLayoutParams().height = marginWidth;
            topMargin.getLayoutParams().width = width;
            
            leftMargin.getLayoutParams().width = marginWidth;
            leftMargin.getLayoutParams().height = loupeHeight;
            
            rightMargin.getLayoutParams().width = marginWidth;
            rightMargin.getLayoutParams().height = loupeHeight;
            
            LinearLayout.LayoutParams params;

            params = (LinearLayout.LayoutParams) loupeLayout.getLayoutParams();
            params.height = loupeHeight;
            loupeLayout.setLayoutParams(params);
            
            loupeArea.getLayoutParams().width = loupeWidth;
            loupeArea.getLayoutParams().height = loupeHeight;
            loupeArea.setVisibility(View.VISIBLE);
            
            params = (LinearLayout.LayoutParams) wordListLayout.getLayoutParams();
            params.height = wordListHeight;
            params.width = width;
            wordListLayout.setLayoutParams(params);


            loadingIndicator.setVisibility(View.GONE);
            loupeArea.setVisibility(View.VISIBLE);
            topMargin.setVisibility(View.VISIBLE);
            loupeLayout.setVisibility(View.VISIBLE);
            wordListLayout.setVisibility(View.VISIBLE);

        } else {
            loadingIndicator.setVisibility(View.VISIBLE);
            loupeArea.setVisibility(View.GONE);
            topMargin.setVisibility(View.GONE);
            loupeLayout.setVisibility(View.GONE);
            wordListLayout.setVisibility(View.GONE);
        }


    }

    // the funtions returns 3 values in an array of ints
    // [0] : the text size
    // [1] : the text component height
    // [2] : the number of words we can display
    private int[] fontSizeForTextHeight(int totalTextHeight, int nbWords,
            int textWidth, int textSizeMax, int textSizeMin) {

        int[] result = new int[3];
        String text = "Agj";
        TextView tv = new TextView(this);
        tv.setText(text);
        tv.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT,
                LayoutParams.WRAP_CONTENT));
        tv.setGravity(Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL);
        // tv.setTextSize(30);
        // tv.setHeight(textHeight);
        int textSize = 0;
        int layoutHeight = 0;

        final float densityMultiplier = getResources().getDisplayMetrics().density;

        for (textSize = textSizeMax; textSize >= textSizeMin; textSize -= 2) {
            // Get the font size setting
            float fontScale = Settings.System.getFloat(getContentResolver(),
                    Settings.System.FONT_SCALE, 1.0f);
            // Text view line spacing multiplier
            float spacingMult = 1.0f * fontScale;
            // Text view additional line spacing
            float spacingAdd = 0.0f;
            TextPaint paint = new TextPaint(tv.getPaint());
            paint.setTextSize(textSize * densityMultiplier);
            // Measure using a static layout
            StaticLayout layout = new StaticLayout(text, paint, textWidth,
                    Alignment.ALIGN_NORMAL, spacingMult, spacingAdd, true);
            layoutHeight = layout.getHeight();
            if ((layoutHeight * nbWords) < totalTextHeight) {
                result[0] = textSize;
                result[1] = layoutHeight;
                result[2] = nbWords;
                return result;
            }
        }

        // we won't be able to display all the fonts
        result[0] = textSize;
        result[1] = layoutHeight;
        result[2] = totalTextHeight / layoutHeight;
        return result;
    }
}
