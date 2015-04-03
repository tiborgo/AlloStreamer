using UnityEngine;
using System.Collections;

public class QT_SkyBox : MonoBehaviour {
	[SerializeField]
	public GameObject SkyBox;
	
	[SerializeField]
	public Camera MainCam,SkyCamera;
	
	[SerializeField]
	public float SkyRotation;
	
	public bool animated=true;
	public float rate=1.0f;
	
	private Vector3 axis = new Vector3(0,1,0);

	
	
	void LateUpdate () 
	{		
		if(MainCam)
		{
		SkyCamera.transform.rotation = MainCam.transform.rotation;
			
		SkyCamera.GetComponent<Camera>().fieldOfView = MainCam.GetComponent<Camera>().fieldOfView;
			if(animated)			
				SkyBox.transform.RotateAround(axis,(rate*0.005f)*Time.deltaTime);
			
		}
		else
			Debug.LogError(this.name + " has no Main Camera associated with it. The skybox is broken!");		
		
	}
	
}
