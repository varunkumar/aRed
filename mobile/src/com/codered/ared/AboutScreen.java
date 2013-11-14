package com.codered.ared;

import com.codered.ared.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class AboutScreen extends Activity implements OnClickListener
{
    private TextView mAboutText;
    private Button mStartButton, mStartVideoButton;


    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.about_screen);

        mAboutText = (TextView) findViewById(R.id.about_text);
        mAboutText.setText(Html.fromHtml(getString(R.string.about_text)));
        mAboutText.setMovementMethod(LinkMovementMethod.getInstance());

        // Setups the link color
        mAboutText.setLinkTextColor(getResources().getColor(
                R.color.holo_light_blue));

        mStartButton = (Button) findViewById(R.id.button_start);
        mStartButton.setOnClickListener(this);
        
        mStartVideoButton = (Button) findViewById(R.id.button_start_video);
        mStartVideoButton.setOnClickListener(this);
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
