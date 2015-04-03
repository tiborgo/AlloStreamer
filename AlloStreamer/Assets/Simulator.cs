using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Simulator : MonoBehaviour {
	GameObject quad;
	GameObject quad2;
	GameObject quad3;
	GameObject monoCamera;
	GameObject sceneCalibCrosshair;
	GameObject MLCalibCrosshair;
	GameObject stereoCamera;
	Camera magicLensCam;
	GameObject selectedObject;

	// Use this for initialization
	Queue<Vector3> latencyQueue;
	Vector3 planePos;
	Texture2D selectionBox;
	Texture2D calibCrosshair;
	Texture2D waypointtext;

	void Start () 
	{
		monoCamera = GameObject.Find("MonoCamera");

		//stereoCamera = GameObject.Find("VirtualCameraStereo");
		//magicLensCam = stereoCamera.camera;//monoCamera.camera;
		magicLensCam = monoCamera.GetComponent<Camera>();

		sceneCalibCrosshair = GameObject.Find("CalibrationCrosshair1");
		sceneCalibCrosshair.SetActive (false);

		MLCalibCrosshair = GameObject.Find ("CalibrationCrosshair2");
		MLCalibCrosshair.SetActive (false);

		selectionBox = Resources.Load("GUISelectionBox") as Texture2D;
		calibCrosshair = Resources.Load ("crosshair2") as Texture2D;
		waypointtext = Resources.Load("waypointtext") as Texture2D;
		quad = GameObject.CreatePrimitive(PrimitiveType.Quad);
		quad2 = GameObject.CreatePrimitive(PrimitiveType.Quad);
		quad3 = GameObject.CreatePrimitive(PrimitiveType.Quad);
		
		quad.transform.localScale = new Vector3(20f,20f,20f); //20,20,20
		quad2.transform.localScale = new Vector3(4f,4f,4f); //20,20,20
		quad3.transform.localScale = new Vector3(4f,4f,4f); //20,20,20

		//quad.renderer.material.color = Color.red;//new Color(0,0,0,0);
		quad.GetComponent<Renderer>().material.shader = Shader.Find("Unlit/Transparent");
		quad.GetComponent<Renderer>().material.mainTexture = selectionBox;
		quad.layer = LayerMask.NameToLayer("Augmentation");

		quad2.GetComponent<Renderer>().material.shader = Shader.Find("Unlit/Transparent");
		quad2.GetComponent<Renderer>().material.mainTexture = selectionBox;
		quad2.layer = LayerMask.NameToLayer("Augmentation");

		quad3.GetComponent<Renderer>().material.shader = Shader.Find("Unlit/Transparent");
		quad3.GetComponent<Renderer>().material.mainTexture = selectionBox;
		quad3.layer = LayerMask.NameToLayer("Augmentation");
		
		latencyQueue = new Queue<Vector3>();
		planePos = new Vector3(0f,0f,0f);
	}
	
	// Update is called once per frame
	void Update () 
	{
	
	}

	
	int latency = 0;
	public void setTrackerLatency(float lat)
	{
		latency = (int)lat;
	}

	Vector3 unityViewportPoint = Vector3.zero;
	float screenAspect = (float)Screen.width/(float)Screen.height;
	float randJitterX = 0;
	float randJitterY = 0;
	public void setTrackerJitterLatency(GameObject selectedPlane, float jitter, float camAngleX, float camAngleY)
	{
		selectedObject = selectedPlane;
		//Sets the tracking jitter

		planePos = selectedPlane.transform.position;


//		planePos = selectedPlane.transform.position;
//		
//		latencyQueue.Enqueue(selectedPlane.transform.position);
//		
//		while(latencyQueue.Count > latency)
//		{
//			planePos = latencyQueue.Dequeue();
//		}





//		quad.transform.position = planePos;
//		Vector3 pos2 = new Vector3(quad.transform.position.x,quad.transform.position.y ,quad.transform.position.z);
//		quad.transform.position = pos2;
//		//quad.transform.position += /*magicLensCam.transform.forward*/ this.selectedPlane.transform.right * 1.5f;
//		quad.transform.eulerAngles = new Vector3(camAngleX, camAngleY, 0);

//		quad2.transform.position = GameObject.Find("Security 2").transform.position;//planePos;
//		Vector3 pos3 = new Vector3(quad2.transform.position.x,quad2.transform.position.y + 1.75f,quad2.transform.position.z);
//		quad2.transform.position = pos3;
//		//quad.transform.position += /*magicLensCam.transform.forward*/ this.selectedPlane.transform.right * 1.5f;
//		quad2.transform.eulerAngles = new Vector3(camAngleX, camAngleY, 0);
//
//		quad3.transform.position = GameObject.Find("Security 3").transform.position;//planePos;
//		Vector3 pos4 = new Vector3(quad3.transform.position.x,quad3.transform.position.y + 1.75f,quad3.transform.position.z);
//		quad3.transform.position = pos4;
//		//quad.transform.position += /*magicLensCam.transform.forward*/ this.selectedPlane.transform.right * 1.5f;
//		quad3.transform.eulerAngles = new Vector3(camAngleX, camAngleY, 0);
		
		//Get random numbers and apply jitter
		randJitterX = Random.Range(-jitter, jitter);
		randJitterY = Random.Range(-jitter, jitter);

		//quad.transform.position += quad.transform.right*randJitterX;
		//quad.transform.position += quad.transform.up*randJitterY;

	}

	public void positionAnnotation(GameObject simObj, Vector3 position)
	{
		simObj.transform.position = position;
	}





//	public void setSpatialFrequency(GameObject simObj, int num)
//	{
//		//Set desired number of objects in the world
//		//If no spatial distribution is set, default to something (a line? random?)
//
//	}

	public void setSpatialDistribution(GameObject simObj, Vector3 dir, Vector3 center, float distribution) //dir: normal of the plane, center: position of the center of the plane, distribution: a distribution object describing what distribution to spread the objects
	{
		//Spatial distributions--
		//line/plane: any custom distribution as described by the distribution object
		//ForestFire sim-- implement line and plane distributions, and sample distribution object
		//Would be cool if eventually the distribution object could be controlled by an onscreen gui to create some heatmap like thing

		//This will create a number of one type of object in some spatial distribution in the world
		//Always contain objects in an array... and return it so that it may be handled later on.
		//Contained within the array

	}

	public void setDirection(GameObject simObj, Vector3 dir)//dir: direction that front of object will point, start: position of starting point, end: position of ending point, loop: start back at beginning
	{
		//Set direction of an object (overload this to set the direction of multiple objects in an array)
		//contained within the object
		Animate animScript = simObj.GetComponent<Animate>();

		if(animScript)
			animScript.setDirection(dir);

	}

	public Vector3 getDirection(GameObject simObj)
	{
		Animate animScript = simObj.GetComponent<Animate>();

		if(animScript)
			return animScript.getDirection();
		else
			return Vector3.zero;

	}

	public void setStart(GameObject simObj, Vector3 start)
	{
		Animate animScript = simObj.GetComponent<Animate>();
		if(animScript)
			animScript.setStartPosition(start);
	}
	public Vector3 getStart(GameObject simObj)
	{
		Animate animScript = simObj.GetComponent<Animate>();

		if(animScript)
			return animScript.getStartPosition(); 
		else
			return Vector3.zero;
			
				
	}

	public void setPathLength(GameObject simObj, float length)
	{
		Animate animScript = simObj.GetComponent<Animate>();

		if(animScript)
			animScript.setPathLength(length);
	}
	

	public float getVelocity(GameObject simObj)
	{
		Animate animScript = simObj.GetComponent<Animate>();
		if(animScript)
			return animScript.getVelocity();
		else
			return 0f;
	}
	public void setVelocity(GameObject simObj, float vel)
	{
		//Set velocity of an object (overload this to set the velocity of multiple objects in an array)
		//contained within the object
		Animate animScript = simObj.GetComponent<Animate>();
		if(animScript)
			animScript.setVelocity (vel);
	}

	public void setVisibility(GameObject simObj, float vis)
	{
		ParticleSystem ps = simObj.GetComponent<ParticleSystem>();
		//ps.maxParticles = (int)vis;
		ps.startColor = new Color(1,1,1,vis);
	}

	public void setARFOV(GameObject simObj, float FOV)
	{
		simObj.GetComponent<Camera>().fieldOfView = FOV;
	}



	void setPath()
	{
		//Set path for animation to be implemented
	}

	Vector3 boxPosition;
	public GUIText myGUItext;
	bool crosshair = false;
	bool sceneCrosshair = false;
	bool writeRotations = false;
	int frameMarker = 0;
	void OnGUI()
	{ 
//		if(MainScript.showGUI)
//		{
//			if (GUI.Button(new Rect(600,20,200,30),"Write Rotations"))
//				writeRotations = !writeRotations;
//
//			if (GUI.Button(new Rect(600,50,200,30),"Show ML Crosshair"))
//			{
//				crosshair = !crosshair;
//				MLCalibCrosshair.SetActive (crosshair);
//			}
//
//
//			if (GUI.Button(new Rect(600,80,200,30),"Show Scene Crosshair"))
//			{
//				sceneCrosshair = !sceneCrosshair;
//				sceneCalibCrosshair.SetActive (sceneCrosshair);
//
//			}
//			
//			
//		}


		
		if(writeRotations)
		{
			frameMarker++;
			//if(frameMarker >= 5)
			//{
				using (System.IO.StreamWriter file = new System.IO.StreamWriter(@"rotations.txt", true))
				{
					file.WriteLine(monoCamera.transform.eulerAngles);
				}
				//frameMarker = 0;
			//}
			
		}
		
//		if(crosshair)
//		{
//			//			unityViewportPoint = magicLensCam.WorldToViewportPoint(planePos);//selectedObject.transform.position);
//			//			
//			//			unityViewportPoint.x *= Screen.width;
//			//			float MLHeight = Screen.width/(3543f/1200f);//(3570f/1200f);
//			//			unityViewportPoint.y = MLHeight - (unityViewportPoint.y * MLHeight);
//			
//			//Should make classes for AR annotations!
//			float crosshairHeight = 100f;
//			float crosshairWidth = 100f*screenAspect;
//			RenderTexture.active = magicLensCam.targetTexture;
//			GUI.DrawTexture(new Rect(687, 248, crosshairWidth, crosshairHeight), calibCrosshair, ScaleMode.ScaleToFit, true, 3543f/1200f);
//			RenderTexture.active = null;
//		}


		if(selectedObject)
		{
			//First calculate selected object 3D position in screen coordinates

			unityViewportPoint = magicLensCam.WorldToViewportPoint(planePos);//selectedObject.transform.position);

			unityViewportPoint.x *= Screen.width;
			float MLHeight = Screen.width/(3543f/1200f);//(3570f/1200f);
			unityViewportPoint.y = MLHeight - (unityViewportPoint.y * MLHeight);
			
			//Should make classes for AR annotations!
			float boxHeight = 100f;
			float boxWidth = 100f*screenAspect;


			//Now add screen position to queue
			
			latencyQueue.Enqueue(unityViewportPoint);
			
			while(latencyQueue.Count > latency)
			{
				boxPosition = latencyQueue.Dequeue();
			}



			RenderTexture.active = magicLensCam.targetTexture;

			
			GUI.color = Color.white;
			//GUI.Box(new Rect(unityViewportPoint.x - (boxWidth/2f), unityViewportPoint.y - (boxHeight/2f), boxWidth, boxHeight), magicLensCam.aspect + System.Environment.NewLine + magicLensCam.pixelRect);
			GUI.DrawTexture(new Rect(boxPosition.x - (boxWidth/2f)+randJitterX, boxPosition.y - (boxHeight/2f)+randJitterY, boxWidth, boxHeight), selectionBox, ScaleMode.ScaleToFit, true, 3543f/1200f);

			//GUI.DrawTexture(new Rect(boxPosition.x - (boxWidth/2f)+randJitterX, boxPosition.y - (boxHeight/2f)+randJitterY-100, boxWidth, boxHeight-20), waypointtext, ScaleMode.ScaleToFit, true, 3543f/1200f);
			

			RenderTexture.active = null;



		}

	}
	
}
