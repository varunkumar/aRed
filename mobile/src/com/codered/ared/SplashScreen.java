package com.codered.ared;

import com.codered.ared.R;

import android.app.Activity;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;

public class SplashScreen extends Activity {
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Sets the Splash Screen Layout
		setContentView(R.layout.splash_screen);

		final Handler handler = new Handler();
		handler.postDelayed(new Runnable() {
			public void run() {
				// Starts the About Screen Activity
				startActivity(new Intent(SplashScreen.this, AboutScreen.class));
			}
		}, 2000L);
	}

	public void onConfigurationChanged(Configuration newConfig) {
		// Manages auto rotation for the Splash Screen Layout
		super.onConfigurationChanged(newConfig);
		setContentView(R.layout.splash_screen);
	}
}
