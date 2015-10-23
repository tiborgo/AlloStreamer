using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
/// MouseLook rotates the transform based on the mouse delta.
/// Minimum and Maximum values can be used to constrain the possible rotation

public class GuideHelicopter : MonoBehaviour
{
	
	[DllImport("HelicopterControlPlugin")]
	private static extern float getStartX();
	
	void Update()
	{
		float test = getStartX ();
		Debug.Log (test);
	}
	
	void Start()
	{
		
		
	}
}