using UnityEngine;
using System.Collections;
//simple script to stop rendering a gameobject when the distance from a camera, or the maincamera, is a certain value.
//Similar to LOD culling in Unity Pro.
//Fixedupdate used so the script isn't calling every frame, saves a bit of time.

public class QT_DistanceDropout : MonoBehaviour {
	public float DistanceFromCamera = 25.0f;	
		
	public Camera CustomCamera;

	void FixedUpdate()
	{
		if(CustomCamera) //if the user put a specific camera in the slot, use that for measurement.
		{
			if(Vector3.Distance(CustomCamera.transform.position,this.transform.position) >= DistanceFromCamera)
				this.gameObject.GetComponent<Renderer>().enabled=false;
			else
				this.gameObject.GetComponent<Renderer>().enabled=true;
		}
		else //otherwise, use the main camera. Useful if there are several main cameras in the scene and they switch.
		{
			if(Vector3.Distance(Camera.main.transform.position,this.transform.position) >= DistanceFromCamera)
				this.gameObject.GetComponent<Renderer>().enabled=false;
			else
				this.gameObject.GetComponent<Renderer>().enabled=true;
		}
		
	}
	
	
}
