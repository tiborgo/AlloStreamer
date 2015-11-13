package com.example.fftrajectorymapper;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class SelectionActivity  extends Activity {
	
	public final static String MAP_FILE_NAME = "com.example.fftrajectorymapper.MAP_FILE_NAME";
	
	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_selection);
        
        Button settingsButton = (Button) findViewById(R.id.mSettingsButton);
        Button preTrainingButton = (Button) findViewById(R.id.mPreTrainingButton);
        Button trainingButton = (Button) findViewById(R.id.mTrainingButton);
        Button assessmentButton = (Button) findViewById(R.id.mAssessmentButton);
        
        //EditText editText = (EditText) findViewById(R.id.edit_message);
        //String message = editText.getText().toString();
        //
        
        settingsButton.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				Intent intent = new Intent(SelectionActivity.this, SettingsActivity.class);
				startActivity(intent);
			}
        });
        
        final Intent intent = new Intent(this, MainActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK); 
        
        preTrainingButton.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				intent.putExtra(MAP_FILE_NAME, "PreTrainingMap.jpg");
				startActivity(intent);
				finish();
			}
        });
        
        trainingButton.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				intent.putExtra(MAP_FILE_NAME, "TrainingMap.jpg");
				startActivity(intent);
				finish();
			}
        });
        
        assessmentButton.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				intent.putExtra(MAP_FILE_NAME, "AssessmentMap.jpg");
				startActivity(intent);
				finish();
			}
        });
	}
}
