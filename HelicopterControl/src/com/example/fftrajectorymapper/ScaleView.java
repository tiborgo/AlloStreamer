package com.example.fftrajectorymapper;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

public class ScaleView extends View {

	private Paint mPaint;
	private Paint mTextPaint;
	private float mMapWidth;
	
	public void setMapWidth(float mapWidth)
	{
		mMapWidth = mapWidth;
	}
	
	public ScaleView(Context context, AttributeSet attrs) {
        super(context, attrs);
        
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mPaint.setColor(0xFF000000);
        mPaint.setStyle(Paint.Style.STROKE);
        //mPaint.setStrokeJoin(Paint.Join.ROUND);
        //mPaint.setStrokeCap(Paint.Cap.ROUND);
        mPaint.setStrokeWidth(3);
        
        mTextPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mTextPaint.setColor(0xFF000000);
        mTextPaint.setStrokeWidth(1);
        mTextPaint.setTextSize(30);
        mTextPaint.setTextAlign(Paint.Align.CENTER);
    }
	
	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		
		float unityWidth = getWidth() * 0.6f * (mMapWidth / 4000.0f);
		
		float meters = (int)(unityWidth / 250.0f) * 250.0f;
		float feet = (int)(unityWidth * 3.28084f / 1000.0f) * 1000.0f;
		
		float metersScaleWidth = meters * (4000.0f / mMapWidth);
		float feetScaleWidth = feet / 3.28084f * (4000.0f / mMapWidth);

		canvas.drawLine((getWidth() - metersScaleWidth) / 2, getHeight()-50, (getWidth() - metersScaleWidth) / 2 + metersScaleWidth, getHeight()-50, mPaint);
		canvas.drawText(meters + " m", getWidth() / 2, getHeight()-60, mTextPaint);
		
		canvas.drawLine((getWidth() - feetScaleWidth) / 2, 50, (getWidth() - feetScaleWidth) / 2 + feetScaleWidth, 50, mPaint);
		canvas.drawText(feet + " ft", getWidth() / 2, 90, mTextPaint);
	}
}
