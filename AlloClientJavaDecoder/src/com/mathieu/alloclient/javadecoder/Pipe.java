package com.mathieu.alloclient.javadecoder;

import java.io.FileDescriptor;

public class Pipe
{
	private FileDescriptor input;
	
	private FileDescriptor output;
	
	public Pipe()
	{
		open();
	}
	
	public FileDescriptor getInput()
	{
		return input;
	}
	
	public FileDescriptor getOutput()
	{
		return output;
	}
	
	public native void closeOutput();
	
	public native void closeInput();
	
	private native void open();
}
