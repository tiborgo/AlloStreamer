using UnityEngine;
using System;
using System.Threading;
using System.Runtime.InteropServices;

public class ServerThread : MonoBehaviour
{
   // This method that will be called when the thread is started
	
   [DllImport ("UnityServerPlugin")]
   private static extern int initInterprocessMemory ();
	
   public void startServer()
   {
		Debug.Log ("Thread");
		initInterprocessMemory ();
		
   }
};

