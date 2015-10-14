package com.mathieu.alloclient.javadecoder;


import java.math.RoundingMode;
import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.text.DecimalFormat;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaFormat;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.illposed.osc.OSCMessage;
import com.illposed.osc.OSCPortOut;

public class MainActivity extends Activity implements SurfaceHolder.Callback, SensorEventListener {
	private PlayerThread mPlayer = null;
	private StreamThread mStream = null;
	SurfaceHolder svHolder;
	String preferenceServer;
	String preferenceOSCClient;
	
    OSCPortOut sender = null;
    OSCMessage message;
    Object args[];

	
	static
	{
        try
        {
        	//System.loadLibrary("ffmpeg"); 
            System.loadLibrary("streamer");
        }
        catch(Throwable e)
        {
            throw new RuntimeException(e);
        }
		
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
//		m_sensorManager = (SensorManager) getSystemService(this.SENSOR_SERVICE);
//        registerListeners();
		args = new Object[7];
		
        gyroOrientation[0] = 0.0f;
        gyroOrientation[1] = 0.0f;
        gyroOrientation[2] = 0.0f;
 
        // initialise gyroMatrix with identity matrix
        gyroMatrix[0] = 1.0f; gyroMatrix[1] = 0.0f; gyroMatrix[2] = 0.0f;
        gyroMatrix[3] = 0.0f; gyroMatrix[4] = 1.0f; gyroMatrix[5] = 0.0f;
        gyroMatrix[6] = 0.0f; gyroMatrix[7] = 0.0f; gyroMatrix[8] = 1.0f;
 
        // get sensorManager and initialise sensor listeners
        mSensorManager = (SensorManager) this.getSystemService(SENSOR_SERVICE);
        initListeners();
        
        // wait for one second until gyroscope and magnetometer/accelerometer
        // data is initialised then scedule the complementary filter task
        fuseTimer.scheduleAtFixedRate(new calculateFusedOrientationTask(),
                                      1000, TIME_CONSTANT);
        
		
		Log.i("dbg","onCreate()");

		

        
//        new oscthread().execute("test");
        
		//Remove title bar
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		//Remove notification bar
		this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		
		SharedPreferences p = PreferenceManager.getDefaultSharedPreferences(this);
		String defaultServer = this.getResources().getString(R.string.defaultServer);
		String defaultOSCClient = this.getResources().getString(R.string.defaultOSCClient);
		
		//Cache the latest server that was put into preferences so that we can compare in onResume()
		preferenceServer = p.getString("RTSPServer", defaultServer);
		preferenceOSCClient = p.getString("OSCClient", defaultOSCClient);
		
		if(preferenceServer.equals(defaultServer))
		{
			//First time opening the app, so make the user put in a URL by starting the preference Activity
			startActivity(new Intent(this, SettingsActivity.class));
		}
		else
		{
			//Init Live 555 in native code
			boolean ret = NativeLib.init(preferenceServer);
			if(!ret)
			{
				startActivity(new Intent(this, SettingsActivity.class));
			}
			
		}
		
		if(preferenceOSCClient.equals(defaultOSCClient))
		{
			startActivity(new Intent(this, SettingsActivity.class));
		}
		else
		{
			//Start OSC client
	        try{
	            sender = new OSCPortOut(InetAddress.getByName(preferenceOSCClient), 7000);
	        }catch (Exception e){
	            Log.w("dbg", "oscoutport: " + e.toString());
	        }
		}
		
		SurfaceView sv = new SurfaceView(this);
		svHolder = sv.getHolder();
		sv.getHolder().addCallback(this);
		setContentView(sv);
		
	}
	
	
	float touchX = 0;
	float touchY = 0;
	float dragX = 0;
	float dragY = 0;
	
	@Override
	public boolean onTouchEvent(MotionEvent e) {
	    // MotionEvent reports input details from the touch screen
	    // and other input controls. In this case, you are only
	    // interested in events where the touch position changed.

		switch (e.getAction())
		{
		case MotionEvent.ACTION_UP:
		    touchX = e.getX()/1.856f;
		    touchY = 575 - e.getY()/1.8437f;
		    break;
		case MotionEvent.ACTION_MOVE:
			dragX = e.getX()/1.856f;
			dragY = 575 - e.getY()/1.8437f;
			touchX = 0;
			touchY = 0;
			//Log.i("dbg", "X: " + dragX + " Y: " + dragY);
			
			break;
		}
		
//		DisplayMetrics metrics = this.getResources().getDisplayMetrics();
//		int width = metrics.widthPixels;
//		int height = metrics.heightPixels;
		

	    
	    //Log.i("dbg", "width: " + width + " height: " + height);
	    //Log.i("dbg", "X: " + touchX + " Y: " + touchY);
	    
	    return true;
	}

	protected void onDestroy() {
		super.onDestroy();
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		

	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		if(mStream == null)
		{
			mStream = new StreamThread();
			mStream.start();
		}
		if (mPlayer == null) {
			mPlayer = new PlayerThread(svHolder.getSurface());
			mPlayer.start();
		}
		
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.main, menu);
	    return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    // Handle item selection
	    switch (item.getItemId()) {
	        case R.id.action_settings:
	        	Log.i("dbg", "Starting menu...");
	        	startActivity(new Intent(this, SettingsActivity.class));
	        	Log.i("dbg", "Started menu.");
	            return true;
//	        case R.id.help:
//	            showHelp();
//	            return true;
	        default:
	            return super.onOptionsItemSelected(item);
	    }
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.i("dbg", "surfaceDestroyed");
		if (mPlayer != null) {
			mPlayer.interrupt();
		}
	}
	
	@Override
	public void onPause()
	{
		super.onPause();
		mSensorManager.unregisterListener(this);
		NativeLib.shutdownClient();
		Log.i("dbg", "onPause");
		if (mPlayer != null) {
			mPlayer.interrupt();
		}
	}
	
	@Override
	public void onStop()
	{
		super.onStop();
		mSensorManager.unregisterListener(this);
		NativeLib.shutdownClient();
	}
	
	@Override
	public void onResume()
	{
		super.onResume();
		
		initListeners();
		
		//Handle any changes that might have been made to preferences
		SharedPreferences p = PreferenceManager.getDefaultSharedPreferences(this);
		String defaultServer = this.getResources().getString(R.string.defaultServer);
		String newServer = p.getString("RTSPServer", defaultServer);
		
		String defaultOSCClient = this.getResources().getString(R.string.defaultServer);
		String newOSCClient = p.getString("OSCClient", defaultOSCClient);
		
		Log.i("dbg", "new server: " + newServer);
		Log.i("dbg", "original server: " + defaultServer);
		
		if(!preferenceServer.equals(newServer))
		{
			Log.i("dbg","Opening new server...");
			boolean ret = NativeLib.init(newServer);
			if(!ret)
			{
				startActivity(new Intent(this, SettingsActivity.class));
			}
		}
		if(!preferenceOSCClient.equals(newOSCClient))
		{
	        try{
	            sender = new OSCPortOut(InetAddress.getByName(newOSCClient), 7000);
	        }catch (Exception e){
	            Log.w("dbg", "oscoutport: " + e.toString());
	        }
			
		}
		if(mStream!=null)
		{
			Log.i("dbg", "resuming stream thread...");
			mStream = new StreamThread();
			mStream.start();
		}
		if(mPlayer!=null)
		{
			Log.i("dbg", "resuming decode thread...");
			mPlayer = new PlayerThread(svHolder.getSurface());
			mPlayer.start();
		}
			
	}
	
	
	private class PlayerThread extends Thread {
		private MediaCodec decoder;
		private Surface surface;

		public PlayerThread(Surface surface) {
			this.surface = surface;
		}
		

		@Override
		public void run() {

			decoder = MediaCodec.createDecoderByType("video/avc");

			ByteBuffer[] inputBuffers = null;
			ByteBuffer[] outputBuffers = null;
			BufferInfo info = new BufferInfo();
			boolean isEOS = false;
			
			
			//Put this back where it belongs in the loop, once bug is figured out
			MediaFormat format = MediaFormat.createVideoFormat("video/avc", 1080, 1920);
			format.setInteger(MediaFormat.KEY_BIT_RATE, 400000);
			format.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
			//int fps = format.getInteger(MediaFormat.KEY_FRAME_RATE);
			
//			format.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 2000000);
//			Log.i("dbg","Max buffer size: " + format.getInteger(MediaFormat.KEY_MAX_INPUT_SIZE));
			Log.i("dbg","starting...");
			decoder.configure(format, surface, null, 0);
			
			decoder.start();

			inputBuffers = decoder.getInputBuffers();
			outputBuffers = decoder.getOutputBuffers();
			int framesBeforeCSD = 0;
			boolean noCSD=true;
			long startMS= System.currentTimeMillis();
			NalUnit frame = NativeLib.constructNalObject();
			//Log.i("dbg", "frame size: "+ frame.frameSize);
			long startMSOutsideDecode= 0;
			while (!Thread.interrupted()) 
			{
				
				long stopMSOutsideDecode = System.currentTimeMillis();
				long elapsedOutsideTime = stopMSOutsideDecode - startMSOutsideDecode;
				//Log.i("dbg", "Started decode loop again in: " + elapsedOutsideTime);
				startMS = System.currentTimeMillis();
					//Get next frame from native code (change name of native function)
//					NalUnit frame = NativeLib.decodeFrame(); 
				
//					if (frame != null) 
//					{
				
				
				//long afterFrameTime = stopMSframe - startMS;
				//Log.i("dbg", "time to get frame: " + afterFrameTime);
						int inIndex = decoder.dequeueInputBuffer(1000);
						
						//int fps = format.getInteger(MediaFormat.KEY_FRAME_RATE);
					
						//Log.i("dbg", "fps: " + fps);
						//Log.i("dbg", "frame size: "+ frame.frameSize + " frame pts: " + frame.pts);
						
						if (inIndex >= 0) 
						{
							//Gets the next NAL unit in the queue and sets it to the constructed NalUnit object

							NativeLib.decodeFrame();

							if(frame != null)
							{
								ByteBuffer buffer = inputBuffers[inIndex];
								//Log.i("dbg","frame size: " + frame.frameSize + " buffer size: " + buffer.capacity());
								buffer.clear();
								
								buffer.put(frame.buffer, 0, frame.frameSize);
								int frameSize = frame.frameSize;
								long pts = frame.pts;
								decoder.queueInputBuffer(inIndex, 0, frameSize, pts, 0);
							}
							else
							{
								Log.i("dbg", "Frame is null!");
							}
							
						}
						else
						{
							//Log.i("dbg", "dequeueInputBuffer timed out!");
						}
					//}

				int outIndex = decoder.dequeueOutputBuffer(info, 1000);

				
				//Log.i("dbg", "Time to grab buffer and decode: " );
				
				
				

				
				switch (outIndex) 
				{
					case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
						Log.d("DecodeActivity", "INFO_OUTPUT_BUFFERS_CHANGED");
						noCSD = false;
						outputBuffers = decoder.getOutputBuffers();
//						format.setInteger(MediaFormat.KEY_BIT_RATE, 400000);
//						format.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
						break;
					case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
						Log.d("DecodeActivity", "New format " + decoder.getOutputFormat());
						break;
					case MediaCodec.INFO_TRY_AGAIN_LATER:
						//Log.d("DecodeActivity", "dequeueOutputBuffer timed out!");
						break;
					default:
						ByteBuffer buffer = outputBuffers[outIndex];
						

						
						//Log.v("DecodeActivity", "Rendering buffer:  " + buffer);
	
						//Log.i("dbg", "Frames before CSD: " + framesBeforeCSD);
						// We use a very simple clock to keep the video FPS, or the video
						// playback will be too fast
						//Log.i("dbg", "Presentation time: " + info.presentationTimeUs + " System.currentTimeMillis: " + System.currentTimeMillis() + " startMS: " 
						//+ startMs + " currMS - startMS: " + (System.currentTimeMillis() - startMs) + " PTS/1000: " + (info.presentationTimeUs /100));
						
						decoder.releaseOutputBuffer(outIndex, true);
						break;
				}
				long stopMS = System.currentTimeMillis();
				long elapsedTime = stopMS - startMS;

				//Log.i("dbg", "Finished Java decode iteration in: " + elapsedTime);
				
				startMSOutsideDecode = System.currentTimeMillis();
			}
		    decoder.stop();
			decoder.release();

		}
	}
	
	public class StreamThread extends Thread
	{	
		MainActivity mActivity;
		boolean first = true;

		public void run()
		{

			/*
			 * Get streaming data from FFmpeg via NDK
			 * Synchronize this between player thread
			 */
			
			//Init the data in AlloClient.cpp
			NativeLib.stream(preferenceServer);

			
			//Add the NalUnit to a global queue
		}
		
	}
    private class oscthread extends AsyncTask<String, Integer, String> {
        

        public oscthread (){
//            args = new Object[8];
//            args[0] = (float) 0;
//            args[1] = (float) 0;
//            args[2] = (float) 0;
//            args[3] = (float) 0;
        }

        @Override
        protected String doInBackground(String... params) {
//            args[4] = (float) 1337; //quat.q1;
//            args[5] = (float) 80085; //quat.q2;
//            args[6] = (float) 7175; //quat.q3;
//            args[7] = (float) 455; //quat.q4;
            message = new OSCMessage("/float", args);
            
            try{
                sender.send(message);
            }catch (Exception e){
                Log.w("oscthread", "sender: " + e.toString());
            }
            
            return "All Done!";
        }

            
        
    }
    
private SensorManager mSensorManager = null;
	
    // angular speeds from gyro
    private float[] gyro = new float[3];
 
    // rotation matrix from gyro data
    private float[] gyroMatrix = new float[9];
 
    // orientation angles from gyro matrix
    private float[] gyroOrientation = new float[3];
 
    // magnetic field vector
    private float[] magnet = new float[3];
 
    // accelerometer vector
    private float[] accel = new float[3];
 
    // orientation angles from accel and magnet
    private float[] accMagOrientation = new float[3];
 
    // final orientation angles from sensor fusion
    private float[] fusedOrientation = new float[3];
 
    // accelerometer and magnetometer based rotation matrix
    private float[] rotationMatrix = new float[9];
    
    public static final float EPSILON = 0.000000001f;
    private static final float NS2S = 1.0f / 1000000000.0f;
	private float timestamp;
	private boolean initState = true;
    
	public static final int TIME_CONSTANT = 30;
	public static final float FILTER_COEFFICIENT = 0.99f;//0.98f;
	private Timer fuseTimer = new Timer();
	
	
    
    // This function registers sensor listeners for the accelerometer, magnetometer and gyroscope.
    public void initListeners(){
        mSensorManager.registerListener(this,
            mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
            SensorManager.SENSOR_DELAY_FASTEST);
     
        mSensorManager.registerListener(this,
            mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE),
            SensorManager.SENSOR_DELAY_FASTEST);
     
        mSensorManager.registerListener(this,
            mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD),
            SensorManager.SENSOR_DELAY_FASTEST);
    }

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {		
	}

	@Override
	public void onSensorChanged(SensorEvent event) {
		switch(event.sensor.getType()) {
	    case Sensor.TYPE_ACCELEROMETER:
	        // copy new accelerometer data into accel array and calculate orientation
	        System.arraycopy(event.values, 0, accel, 0, 3);
	        calculateAccMagOrientation();
	        break;
	 
	    case Sensor.TYPE_GYROSCOPE:
	        // process gyro data
	        gyroFunction(event);
	        break;
	 
	    case Sensor.TYPE_MAGNETIC_FIELD:
	        // copy new magnetometer data into magnet array
	        System.arraycopy(event.values, 0, magnet, 0, 3);
	        break;
	    }
	}
	
	// calculates orientation angles from accelerometer and magnetometer output
	public void calculateAccMagOrientation() {
	    if(SensorManager.getRotationMatrix(rotationMatrix, null, accel, magnet)) {
	        SensorManager.getOrientation(rotationMatrix, accMagOrientation);
	    }
	}
	
	// This function is borrowed from the Android reference
	// at http://developer.android.com/reference/android/hardware/SensorEvent.html#values
	// It calculates a rotation vector from the gyroscope angular speed values.
    private void getRotationVectorFromGyro(float[] gyroValues,
            float[] deltaRotationVector,
            float timeFactor)
	{
		float[] normValues = new float[3];
		
		// Calculate the angular speed of the sample
		float omegaMagnitude =
		(float)Math.sqrt(gyroValues[0] * gyroValues[0] +
		gyroValues[1] * gyroValues[1] +
		gyroValues[2] * gyroValues[2]);
		
		// Normalize the rotation vector if it's big enough to get the axis
		if(omegaMagnitude > EPSILON) {
		normValues[0] = gyroValues[0] / omegaMagnitude;
		normValues[1] = gyroValues[1] / omegaMagnitude;
		normValues[2] = gyroValues[2] / omegaMagnitude;
		}
		
		// Integrate around this axis with the angular speed by the timestep
		// in order to get a delta rotation from this sample over the timestep
		// We will convert this axis-angle representation of the delta rotation
		// into a quaternion before turning it into the rotation matrix.
		float thetaOverTwo = omegaMagnitude * timeFactor;
		float sinThetaOverTwo = (float)Math.sin(thetaOverTwo);
		float cosThetaOverTwo = (float)Math.cos(thetaOverTwo);
		deltaRotationVector[0] = sinThetaOverTwo * normValues[0];
		deltaRotationVector[1] = sinThetaOverTwo * normValues[1];
		deltaRotationVector[2] = sinThetaOverTwo * normValues[2];
		deltaRotationVector[3] = cosThetaOverTwo;
	}
	
    // This function performs the integration of the gyroscope data.
    // It writes the gyroscope based orientation into gyroOrientation.
    public void gyroFunction(SensorEvent event) {
        // don't start until first accelerometer/magnetometer orientation has been acquired
        if (accMagOrientation == null)
            return;
     
        // initialisation of the gyroscope based rotation matrix
        if(initState) {
            float[] initMatrix = new float[9];
            initMatrix = getRotationMatrixFromOrientation(accMagOrientation);
            float[] test = new float[3];
            SensorManager.getOrientation(initMatrix, test);
            gyroMatrix = matrixMultiplication(gyroMatrix, initMatrix);
            initState = false;
        }
     
        // copy the new gyro values into the gyro array
        // convert the raw gyro data into a rotation vector
        float[] deltaVector = new float[4];
        if(timestamp != 0) {
            final float dT = (event.timestamp - timestamp) * NS2S;
        System.arraycopy(event.values, 0, gyro, 0, 3);
        getRotationVectorFromGyro(gyro, deltaVector, dT / 2.0f);
        }
     
        // measurement done, save current time for next interval
        timestamp = event.timestamp;
     
        // convert rotation vector into rotation matrix
        float[] deltaMatrix = new float[9];
        SensorManager.getRotationMatrixFromVector(deltaMatrix, deltaVector);
     
        // apply the new rotation interval on the gyroscope based rotation matrix
        gyroMatrix = matrixMultiplication(gyroMatrix, deltaMatrix);
     
        // get the gyroscope based orientation from the rotation matrix
        SensorManager.getOrientation(gyroMatrix, gyroOrientation);
    }
    
    private float[] getRotationMatrixFromOrientation(float[] o) {
        float[] xM = new float[9];
        float[] yM = new float[9];
        float[] zM = new float[9];
     
        float sinX = (float)Math.sin(o[1]);
        float cosX = (float)Math.cos(o[1]);
        float sinY = (float)Math.sin(o[2]);
        float cosY = (float)Math.cos(o[2]);
        float sinZ = (float)Math.sin(o[0]);
        float cosZ = (float)Math.cos(o[0]);
     
        // rotation about x-axis (pitch)
        xM[0] = 1.0f; xM[1] = 0.0f; xM[2] = 0.0f;
        xM[3] = 0.0f; xM[4] = cosX; xM[5] = sinX;
        xM[6] = 0.0f; xM[7] = -sinX; xM[8] = cosX;
     
        // rotation about y-axis (roll)
        yM[0] = cosY; yM[1] = 0.0f; yM[2] = sinY;
        yM[3] = 0.0f; yM[4] = 1.0f; yM[5] = 0.0f;
        yM[6] = -sinY; yM[7] = 0.0f; yM[8] = cosY;
     
        // rotation about z-axis (azimuth)
        zM[0] = cosZ; zM[1] = sinZ; zM[2] = 0.0f;
        zM[3] = -sinZ; zM[4] = cosZ; zM[5] = 0.0f;
        zM[6] = 0.0f; zM[7] = 0.0f; zM[8] = 1.0f;
     
        // rotation order is y, x, z (roll, pitch, azimuth)
        float[] resultMatrix = matrixMultiplication(xM, yM);
        resultMatrix = matrixMultiplication(zM, resultMatrix);
        return resultMatrix;
    }
    
    private float[] matrixMultiplication(float[] A, float[] B) {
        float[] result = new float[9];
     
        result[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6];
        result[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7];
        result[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8];
     
        result[3] = A[3] * B[0] + A[4] * B[3] + A[5] * B[6];
        result[4] = A[3] * B[1] + A[4] * B[4] + A[5] * B[7];
        result[5] = A[3] * B[2] + A[4] * B[5] + A[5] * B[8];
     
        result[6] = A[6] * B[0] + A[7] * B[3] + A[8] * B[6];
        result[7] = A[6] * B[1] + A[7] * B[4] + A[8] * B[7];
        result[8] = A[6] * B[2] + A[7] * B[5] + A[8] * B[8];
     
        return result;
    }
    
    class calculateFusedOrientationTask extends TimerTask {
        public void run() {
            float oneMinusCoeff = 1.0f - FILTER_COEFFICIENT;
            
            /*
             * Fix for 179° <--> -179° transition problem:
             * Check whether one of the two orientation angles (gyro or accMag) is negative while the other one is positive.
             * If so, add 360° (2 * math.PI) to the negative value, perform the sensor fusion, and remove the 360° from the result
             * if it is greater than 180°. This stabilizes the output in positive-to-negative-transition cases.
             */
            
            // azimuth
            if (gyroOrientation[0] < -0.5 * Math.PI && accMagOrientation[0] > 0.0) {
            	fusedOrientation[0] = (float) (FILTER_COEFFICIENT * (gyroOrientation[0] + 2.0 * Math.PI) + oneMinusCoeff * accMagOrientation[0]);
        		fusedOrientation[0] -= (fusedOrientation[0] > Math.PI) ? 2.0 * Math.PI : 0;
            }
            else if (accMagOrientation[0] < -0.5 * Math.PI && gyroOrientation[0] > 0.0) {
            	fusedOrientation[0] = (float) (FILTER_COEFFICIENT * gyroOrientation[0] + oneMinusCoeff * (accMagOrientation[0] + 2.0 * Math.PI));
            	fusedOrientation[0] -= (fusedOrientation[0] > Math.PI)? 2.0 * Math.PI : 0;
            }
            else {
            	fusedOrientation[0] = FILTER_COEFFICIENT * gyroOrientation[0] + oneMinusCoeff * accMagOrientation[0];
            }
            
            // pitch
            if (gyroOrientation[1] < -0.5 * Math.PI && accMagOrientation[1] > 0.0) {
            	fusedOrientation[1] = (float) (FILTER_COEFFICIENT * (gyroOrientation[1] + 2.0 * Math.PI) + oneMinusCoeff * accMagOrientation[1]);
        		fusedOrientation[1] -= (fusedOrientation[1] > Math.PI) ? 2.0 * Math.PI : 0;
            }
            else if (accMagOrientation[1] < -0.5 * Math.PI && gyroOrientation[1] > 0.0) {
            	fusedOrientation[1] = (float) (FILTER_COEFFICIENT * gyroOrientation[1] + oneMinusCoeff * (accMagOrientation[1] + 2.0 * Math.PI));
            	fusedOrientation[1] -= (fusedOrientation[1] > Math.PI)? 2.0 * Math.PI : 0;
            }
            else {
            	fusedOrientation[1] = FILTER_COEFFICIENT * gyroOrientation[1] + oneMinusCoeff * accMagOrientation[1];
            }
            
            // roll
            if (gyroOrientation[2] < -0.5 * Math.PI && accMagOrientation[2] > 0.0) {
            	fusedOrientation[2] = (float) (FILTER_COEFFICIENT * (gyroOrientation[2] + 2.0 * Math.PI) + oneMinusCoeff * accMagOrientation[2]);
        		fusedOrientation[2] -= (fusedOrientation[2] > Math.PI) ? 2.0 * Math.PI : 0;
            }
            else if (accMagOrientation[2] < -0.5 * Math.PI && gyroOrientation[2] > 0.0) {
            	fusedOrientation[2] = (float) (FILTER_COEFFICIENT * gyroOrientation[2] + oneMinusCoeff * (accMagOrientation[2] + 2.0 * Math.PI));
            	fusedOrientation[2] -= (fusedOrientation[2] > Math.PI)? 2.0 * Math.PI : 0;
            }
            else {
            	fusedOrientation[2] = FILTER_COEFFICIENT * gyroOrientation[2] + oneMinusCoeff * accMagOrientation[2];
            }
     
            // overwrite gyro matrix and orientation with fused orientation
            // to comensate gyro drift
            gyroMatrix = getRotationMatrixFromOrientation(fusedOrientation);
            System.arraycopy(fusedOrientation, 0, gyroOrientation, 0, 3);
            
            args[0] = (float)(fusedOrientation[1] * 180/Math.PI);
            args[1] = (float)(fusedOrientation[2] * 180/Math.PI)+90;
            args[2] = (float)(fusedOrientation[0] * 180/Math.PI);
            args[3] = touchX;
            args[4] = touchY;
            args[5] = dragX;
            args[6] = dragY;
            
            touchX = touchY = 0;
            dragX = dragY = 0;
            
            //Log.i("dbg", "Yaw: " + args[0] + " Pitch: " + args[1] + " Roll: " + args[2]);
            //Log.i("dbg", "TouchX: " + args[3] + " TouchY: " + args[4]);
            new oscthread().execute("test");
            
            // update sensor output in GUI
            //mHandler.post(updateOreintationDisplayTask);
        }
    }
    
}