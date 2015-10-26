using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System.Threading;

/// MouseLook rotates the transform based on the mouse delta.
/// Minimum and Maximum values can be used to constrain the possible rotation

public class GuideHelicopter : MonoBehaviour
{
	
	[DllImport("HelicopterControlPlugin")]
	private static extern float getStartX();

	[DllImport("HelicopterControlPlugin")]
	private static extern float getStartY();

	[DllImport("HelicopterControlPlugin")]
	private static extern float getEndX();
	
	[DllImport("HelicopterControlPlugin")]
	private static extern float getEndY();
	
	void Update()
	{
		float startX = getStartX();
		float startY = getStartY();
		float endX = getEndX();
		float endY = getEndY();
		Debug.Log ("(" + startX + "," + startY + ") -> (" + endX + "," + endY + ")");
	}
	
	void Start()
	{
		OSCHelicopterThread  oscClientHelicopter = new OSCHelicopterThread ();
		Thread osctHelicopter = new Thread(new ThreadStart(oscClientHelicopter.startServer));
        osctHelicopter.Start();

        OSCHelicopterThreadPhaseSpace oscPhaseHelicopter = new OSCHelicopterThreadPhaseSpace ();
		Thread osctPhaseHelicopter = new Thread(new ThreadStart(oscPhaseHelicopter.startServer));
        osctPhaseHelicopter.Start();
		
	}
}

public class OSCHelicopterThread : MonoBehaviour
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
public class OSCHelicopterThreadPhaseSpace : MonoBehaviour
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
