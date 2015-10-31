using UnityEngine;
using System;
using System.Threading;
using System.Runtime.InteropServices;

public class OSCHelicopterThread
{
   // This method that will be called when the thread is started
	
   [DllImport ("HelicopterControlPlugin")]
   private static extern int oscStartHelicopterControl ();
	
   public void startServer()
   {
		Debug.Log ("OSC Thread");
		oscStartHelicopterControl ();
		
   }
};
