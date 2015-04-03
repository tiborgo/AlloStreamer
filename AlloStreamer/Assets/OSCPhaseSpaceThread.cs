using UnityEngine;
using System;
using System.Threading;
using System.Runtime.InteropServices;

public class OSCThread : MonoBehaviour
{
   // This method that will be called when the thread is started
	
   [DllImport ("UnityServerPlugin")]
   private static extern int oscPhaseSpaceStart ();
	
   public void startServer()
   {
		Debug.Log ("OSC Thread");
		oscPhaseSpaceStart ();
		
   }
};
