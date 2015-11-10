using UnityEngine;
using System;
using System.Threading;
using System.Runtime.InteropServices;

public class OSCRotationThread : MonoBehaviour
{
   // This method that will be called when the thread is started
	
   [DllImport ("UnityServerPlugin")]
   private static extern int oscRotationStart ();
	
   public void startServer()
   {
		Debug.Log ("OSC RotationThread");
		oscRotationStart ();
		
   }
};
