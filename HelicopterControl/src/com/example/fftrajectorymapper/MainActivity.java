package com.example.fftrajectorymapper;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Arrays;

import com.illposed.osc.OSCMessage;
import com.illposed.osc.OSCPortOut;
import java.net.InetAddress;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.EmbossMaskFilter;
import android.graphics.MaskFilter;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PointF;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import android.view.Window;

public class MainActivity extends Activity{
        //implements ColorPickerDialog.OnColorChangedListener {

	//OSC start
    OSCPortOut sender = null;
    OSCMessage message;
    Object args[];
    float playerX = 200;
    float playerY = 200;
    float mapWidth = 400;
    float mapHeight = 400;
    float sX, sY, fX, fY, prevX, prevY, picX, picY;
    Bitmap  mBitmap;
    int OSCPort = 7244;
    
    //OSC end

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(new MyView(this));

        View decorView = getWindow().getDecorView();

        int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN |View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN ;
        decorView.setSystemUiVisibility(uiOptions);
        
        mPaint = new Paint();
        mPaint.setAntiAlias(true);
        mPaint.setDither(true);
        mPaint.setColor(0xFF0000FF);
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setStrokeJoin(Paint.Join.ROUND);
        mPaint.setStrokeCap(Paint.Cap.ROUND);
        mPaint.setStrokeWidth(12);

        mEmboss = new EmbossMaskFilter(new float[] { 1, 1, 1 },
                                       0.4f, 6, 3.5f);

        mBlur = new BlurMaskFilter(8, BlurMaskFilter.Blur.NORMAL);
        
        //OSC start
        SharedPreferences p = PreferenceManager.getDefaultSharedPreferences(this);
        startActivity(new Intent(this, SettingsActivity.class));
        
        String defaultClient = this.getResources().getString(R.string.defaultClient);
        playerX = Float.parseFloat(this.getResources().getString(R.string.playerX));
        playerY = Float.parseFloat(this.getResources().getString(R.string.playerY));
        mapWidth = Float.parseFloat(this.getResources().getString(R.string.mapWidth));
        mapHeight = Float.parseFloat(this.getResources().getString(R.string.mapHeight));
		String prefClient = "169.231.116.127";//p.getString("Client", defaultClient);
        
        try{
        	//This expects a string w/ a url
        	
            sender = new OSCPortOut(InetAddress.getByName(prefClient), OSCPort);
        }catch (Exception e){
            Log.w("dbg", "oscoutport: " + e.toString() + ": " + prefClient);
        }
        //OSC end
    }

    private Paint       mPaint;
    private MaskFilter  mEmboss;
    private MaskFilter  mBlur;

    public void colorChanged(int color) {
        mPaint.setColor(color);
    }
    
    public void sendMessage(float sX,float sY,float fX,float fY,float picX,float picY,Bitmap mBitmap){
    	Log.w("oscthread", "it works");
    	float unityStartX = ((sX - picX)/mBitmap.getWidth())*mapWidth;
		float unityStartY = ((sY - picY)/mBitmap.getHeight())*mapHeight;
		float unityEndX = ((fX - picX)/mBitmap.getWidth())*mapWidth;
		float unityEndY = ((fY - picY)/mBitmap.getHeight())*mapHeight;
        args = new Object[4];
        args[0] = unityStartX;
        args[1] = unityStartY;
        args[2] = unityEndX;
        args[3] = unityEndY;
    	message = new OSCMessage("/coords",Arrays.asList(args));
        try{
            sender.send(message);
        }catch (Exception e){
            Log.w("oscthread", "sender: " + e.toString());
        }
    }

    private class oscthread extends AsyncTask<String, Integer, String> {

		@Override
		protected String doInBackground(String... params) {
			sendMessage(sX,sY,fX,fY,picX,picY,mBitmap);
			return null;
		}
    	
    }
    
    public class MyView extends View {

        private static final float MINP = 0.25f;
        private static final float MAXP = 0.75f;

        private Canvas  mCanvas;
        private Paint   mBitmapPaint;
        private float prevX, prevY;
        private float maxLength = 200;
        private boolean mapMove = false;

        public MyView(Context c) {
            super(c);
            mBitmapPaint = new Paint(Paint.DITHER_FLAG);
        }

        @Override
        protected void onSizeChanged(int w, int h, int oldw, int oldh) {
            super.onSizeChanged(w, h, oldw, oldh);
            String path = Environment.getExternalStorageDirectory() + "/FFTrajectoryBackground.jpg";
            Bitmap tempBitmap = BitmapFactory.decodeFile(Environment.getExternalStorageDirectory() + "/FFTrajectoryBackground.jpg");//Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
            mBitmap = tempBitmap.copy(Bitmap.Config.ARGB_8888, true);
            mCanvas = new Canvas(mBitmap);
            
            //Log.i("DRBRG", "Width: " + w + " Height: " + h);
            
        }

        protected void calcFinalPoints(float x,float y){
        	double dist = Math.sqrt(Math.pow((sX - x),2) + Math.pow((sY - y),2));
        	if(dist > maxLength){
        		double theta;
        		if(x > sX){
        			theta = Math.atan((sY-y)/(sX-x));
        		} else if (sX > x){
        			theta = Math.atan((sY-y)/(sX-x)) + Math.PI; 
        		} else {
        			if(y > sY){
        				theta = Math.PI/2;
        			} else {
        				theta = (3*Math.PI)/2;
        			}
        		}
        		fX = sX + (float) (maxLength * Math.cos(theta));
        		fY = sY + (float) (maxLength * Math.sin(theta));
        	} else {
        		fX = x;
        		fY = y;
        	}
        }
        
        @Override
        protected void onDraw(Canvas canvas) {
            canvas.drawColor(0xFFAAAAAA);

            canvas.drawBitmap(mBitmap, picX, picY, mBitmapPaint);

            canvas.drawLine(sX,sY,fX,fY,mPaint);
            
            //Draw a circle at the player location
            canvas.drawCircle(mBitmap.getWidth() * (playerX/mapWidth) + picX, mBitmap.getHeight() * (playerY/mapHeight) + picY, 8, mPaint);
        }

        private static final float TOUCH_TOLERANCE = 4;

        private void touch_start(float x, float y) {
        	if(!mapMove && x > 80 && y > 40){
        		sX = x;
            	sY = y;
            	fX = x;
            	fY = y;
        	}
            prevX = x;
            prevY = y;
        }
        int pathListIndex = 0;
        private void touch_move(float x, float y) {
        	if(mapMove){
        		float deltX = x - prevX;
        		float deltY = y - prevY;
        		picX += deltX;
        		picY += deltY;
        		sX += deltX;
        		sY += deltY;
        		fX += deltX;
        		fY += deltY;
        		prevX = x;
        		prevY = y;
        	} else {
        		calcFinalPoints(x,y);
        	}
        }
        private void touch_up(float x, float y) {
        	if(mapMove){
        		if(x < 80 && y < 80){
        			mapMove = false;
        		}
        	} else {
        		if(x < 80 && y < 80){
        			mapMove = true;
        		} else {
        			new oscthread().execute("test");
        		}
        	}
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            float x = event.getX();
            float y = event.getY();

            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    touch_start(x, y);
                    invalidate();
                    break;
                case MotionEvent.ACTION_MOVE:
                    touch_move(x, y);
                    invalidate();
                    break;
                case MotionEvent.ACTION_UP:
                    touch_up(x, y);
                    invalidate();
                    break;
            }
            return true;
        }
    }

    private static final int COLOR_MENU_ID = Menu.FIRST;
    private static final int EMBOSS_MENU_ID = Menu.FIRST + 1;
    private static final int BLUR_MENU_ID = Menu.FIRST + 2;
    private static final int ERASE_MENU_ID = Menu.FIRST + 3;
    private static final int SRCATOP_MENU_ID = Menu.FIRST + 4;

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);

        menu.add(0, COLOR_MENU_ID, 0, "Color").setShortcut('3', 'c');
        menu.add(0, EMBOSS_MENU_ID, 0, "Emboss").setShortcut('4', 's');
        menu.add(0, BLUR_MENU_ID, 0, "Blur").setShortcut('5', 'z');
        menu.add(0, ERASE_MENU_ID, 0, "Erase").setShortcut('5', 'z');
        menu.add(0, SRCATOP_MENU_ID, 0, "SrcATop").setShortcut('5', 'z');

        /****   Is this the mechanism to extend with filter effects?
        Intent intent = new Intent(null, getIntent().getData());
        intent.addCategory(Intent.CATEGORY_ALTERNATIVE);
        menu.addIntentOptions(
                              Menu.ALTERNATIVE, 0,
                              new ComponentName(this, NotesList.class),
                              null, intent, 0, null);
        *****/
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        mPaint.setXfermode(null);
        mPaint.setAlpha(0xFF);

        switch (item.getItemId()) {
            case COLOR_MENU_ID:
                //new ColorPickerDialog(this, this, mPaint.getColor()).show();
                return true;
            case EMBOSS_MENU_ID:
                if (mPaint.getMaskFilter() != mEmboss) {
                    mPaint.setMaskFilter(mEmboss);
                } else {
                    mPaint.setMaskFilter(null);
                }
                return true;
            case BLUR_MENU_ID:
                if (mPaint.getMaskFilter() != mBlur) {
                    mPaint.setMaskFilter(mBlur);
                } else {
                    mPaint.setMaskFilter(null);
                }
                return true;
            case ERASE_MENU_ID:
                mPaint.setXfermode(new PorterDuffXfermode(
                                                        PorterDuff.Mode.CLEAR));
                return true;
            case SRCATOP_MENU_ID:
                mPaint.setXfermode(new PorterDuffXfermode(
                                                    PorterDuff.Mode.SRC_ATOP));
                mPaint.setAlpha(0x80);
                return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
