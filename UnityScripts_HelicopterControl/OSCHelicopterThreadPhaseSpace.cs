using UnityEngine;
using System;
using System.Threading;
using System.Runtime.InteropServices;

public class OSCHelicopterThreadPhaseSpace
{
   // This method that will be called when the thread is started
	
   [DllImport ("HelicopterControlPlugin")]
   private static extern int oscPhaseSpaceStartHelicopterControl ();
	
   public void startServer()
   {
		Debug.Log ("OSC Thread Phase Space");
		oscPhaseSpaceStartHelicopterControl ();
		
   }
};
