package com.codered.ared;

import android.app.Activity;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class SplashScreen extends Activity implements OnClickListener {
	private Button mStartButton, mStartVideoButton;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Sets the Splash Screen Layout
		setContentView(R.layout.splash_screen);
 
		/*final Handler handler = new Handler();
		handler.postDelayed(new Runnable() {
			public void run() {
				// Starts the About Screen Activity
				startActivity(new Intent(SplashScreen.this, AboutScreen.class));
			}
		}, 2000L);*/
		
		mStartButton = (Button) findViewById(R.id.button_start);
        mStartButton.setOnClickListener(this);
        
        mStartVideoButton = (Button) findViewById(R.id.button_start_video);
        mStartVideoButton.setOnClickListener(this);
	}

	public void onConfigurationChanged(Configuration newConfig) {
		// Manages auto rotation for the Splash Screen Layout
		super.onConfigurationChanged(newConfig);
		setContentView(R.layout.splash_screen);
	}
	
	/** Starts the TextReco main activity */
    private void startARActivity()
    {
        Intent i = new Intent(this, TextReco.class);
        startActivity(i);
    }
    
    /** Starts the VideoPlayback main activity */
    private void startARVideoActivity()
    {
        Intent i = new Intent(this, VideoPlayback.class);
        startActivity(i);
    }


    public void onClick(View v)
    {
        switch (v.getId())
        {
        case R.id.button_start:
            startARActivity();
            break;
        case R.id.button_start_video:
        	startARVideoActivity();
        }
    }
}
