package com.mathieu.alloclient.javadecoder;

import java.nio.ByteBuffer;

public class NalUnit {
	//ByteBuffer buffer;
	byte[] buffer;
	int frameSize;
	long pts;
	
//	public NalUnit(ByteBuffer buffer, int frameSize, long pts)
//	{
//		this.buffer = buffer;
//		this.frameSize = frameSize;
//		this.pts = pts;
//	}
	
	public NalUnit(int maxBufferSize)
	{
		//buffer = ByteBuffer.allocate(maxBufferSize);
		buffer = new byte[maxBufferSize];
	}
	
	public void setBuffer(final byte[] nativeBuffer, int frameSize, long pts)
	{
		
		//buffer.clear();
		//buffer.limit(frameSize+1);
		//buffer.position(0);
		//buffer.put(nativeBuffer);
		this.buffer = nativeBuffer;
		this.frameSize = frameSize;
		this.pts = pts;
	}
	
	
	

}
