package com.mathieu.alloclient.javadecoder;

import java.util.ArrayList;
import java.util.concurrent.Semaphore;

import javax.microedition.khronos.egl.EGLSurface;

import android.view.Surface;
import android.view.SurfaceHolder;

public class NativeLib {
	public static native boolean csdAvailable();
    public static native boolean init(String rtspServer);
    public static native void stream(String rtspServer);
    public static native void shutdownClient();
    public static native boolean decodeFrame();
    public static native NalUnit constructNalObject();
    public static native NalUnit getSPSPPS();
    //public static native void stream(String);
	public static NalUnit spspps;
	public static boolean initializeDecoder = false;
	public static String rtspServer;
    
}
