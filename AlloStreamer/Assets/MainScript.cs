using UnityEngine;
using System.Collections;
using System.Collections.Generic;

using System.IO;
public class MainScript : MonoBehaviour {

	private MainScript m_Instance;
	public MainScript Instance { get { return m_Instance; } }
	

	
	public enum interfaces {MagicLens, WIM, Pointing};
	int experimentInterface;
	Camera cam1, cam2, cam3;
	public GameObject crosshairSphere;
	Vector3 intersectionCoords;
	static bool selected;
	GameObject selectedPlane;
	float FOV;


	bool clear = false;
	float hSliderValue;
	public static bool showGUI = false;
	int participantNumber = 0;
	bool startTask = false;
	int FOVGUI;
	//for x > 1000
	// x between (1060, 1080)
	// y between (105, 112)
	// z between (295, 310)
	
	//for y <= 1000
	// x between (990, 1010)
	// y between (105, 112)
	// z between (270, 290)
	
	static string[] planes = {"privateplane", "privateplanenotail", "Marine One", "planenotail", "Marine One", "biplanenotail", "ghostplane", "ghostplanenotail"};
	
	GameObject[] startNodes = new GameObject[7];
	
	float[] heights = {105f, 106f, 107f, 108f, 109f, 110f, 111f, 112f, 113f, 105f, 106f, 107f};
	
	string[,] randomPlanes  = 
	{
		{planes[2], planes[4]}, // 6
		{planes[4], planes[6], planes[4], planes[5], planes[7], planes[6], planes[3], planes[5], planes[7], planes[4], planes[1], planes[1]}, // 6
		{planes[7], planes[0], planes[7], planes[3], planes[2], planes[1], planes[0], planes[3], planes[4], planes[5], planes[5], planes[4]}, // 5
		{planes[5], planes[7], planes[2], planes[4], planes[5], planes[7], planes[2], planes[3], planes[3], planes[6], planes[4], planes[5]}, // 9
		{planes[4], planes[7], planes[1], planes[4], planes[1], planes[1], planes[0], planes[7], planes[0], planes[2], planes[6], planes[6]}, // 9
		{planes[1], planes[1], planes[3], planes[3], planes[4], planes[2], planes[2], planes[5], planes[5], planes[4], planes[2], planes[7]}, // 11
		{planes[3], planes[5], planes[4], planes[5], planes[7], planes[4], planes[7], planes[7], planes[6], planes[6], planes[6], planes[5]}, // 0
		{planes[4], planes[2], planes[5], planes[7], planes[0], planes[4], planes[0], planes[0], planes[6], planes[2], planes[7], planes[5]}, // 8
		{planes[1], planes[5], planes[3], planes[1], planes[2], planes[3], planes[2], planes[0], planes[7], planes[7], planes[0], planes[7]}, // 1
		{planes[0], planes[7], planes[0], planes[6], planes[2], planes[5], planes[6], planes[2], planes[4], planes[7], planes[5], planes[0]}, // 8
		{planes[3], planes[6], planes[7], planes[6], planes[0], planes[5], planes[3], planes[5], planes[2], planes[3], planes[7], planes[2]}, // 4	
		{planes[1], planes[1], planes[2], planes[2], planes[5], planes[4], planes[2], planes[7], planes[5], planes[6], planes[5], planes[7]}, // 5 (4)
		{planes[7], planes[1], planes[4], planes[2], planes[3], planes[4], planes[3], planes[0], planes[0], planes[7], planes[0], planes[2]}, // 1 (1)
		{planes[5], planes[0], planes[2], planes[7], planes[7], planes[0], planes[5], planes[5], planes[3], planes[0], planes[0], planes[3]}, // 2 (2)
		{planes[4], planes[7], planes[7], planes[6], planes[3], planes[1], planes[7], planes[3], planes[1], planes[3], planes[7], planes[4]}, // 3 (6)
		{planes[7], planes[4], planes[3], planes[3], planes[0], planes[1], planes[1], planes[7], planes[7], planes[4], planes[7], planes[1]}, // 4 (0)
		{planes[4], planes[7], planes[0], planes[2], planes[3], planes[0], planes[2], planes[4], planes[2], planes[4], planes[3], planes[0]}, // 1 (7)
		{planes[1], planes[2], planes[0], planes[3], planes[3], planes[4], planes[0], planes[2], planes[0], planes[1], planes[5], planes[4]}, // 10 (5)
		{planes[2], planes[0], planes[2], planes[0], planes[4], planes[5], planes[3], planes[2], planes[5], planes[4], planes[2], planes[4]}, // 6 (3)
		{planes[2], planes[1], planes[7], planes[2], planes[3], planes[5], planes[3], planes[0], planes[7], planes[1], planes[5], planes[1]}, // 7 (0)
		{planes[2], planes[0], planes[1], planes[1], planes[3], planes[4], planes[3], planes[3], planes[2], planes[2], planes[3], planes[0]} // 5 (4)

	};
	
	
	GameObject[] planeObjects;
	
	int[] correctPlanes = {6, 5, 9, 9, 11, 0, 8, 1, 8, 4, 5, 1, 2, 3, 4, 1, 10, 6, 7, 5};
	int[] correctPlaneThumbnails = {3, 1, 6, 2, 7, 3, 6, 5, 4, 0, 4, 1, 2, 6, 0, 7, 5, 3, 0, 4};

	
	int[,] counterbalancedTrials = 
	{
		{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19},
		{19,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18},
		{18,19,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},
		{17,18,19,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16},
		{16,17,18,19,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
		{15,16,17,18,19,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14},
		{14,15,16,17,18,19,0,1,2,3,4,5,6,7,8,9,10,11,12,13},
		{13,14,15,16,17,18,19,0,1,2,3,4,5,6,7,8,9,10,11,12},
		{12,13,14,15,16,17,18,19,0,1,2,3,4,5,6,7,8,9,10,11},
		{11,12,13,14,15,16,17,18,19,0,1,2,3,4,5,6,7,8,9,10},
		{10,11,12,13,14,15,16,17,18,19,0,1,2,3,4,5,6,7,8,9},
		{9,10,11,12,13,14,15,16,17,18,19,0,1,2,3,4,5,6,7,8},
		{8,9,10,11,12,13,14,15,16,17,18,19,0,1,2,3,4,5,6,7},
		{7,8,9,10,11,12,13,14,15,16,17,18,19,0,1,2,3,4,5,6},
		{6,7,8,9,10,11,12,13,14,15,16,17,18,19,0,1,2,3,4,5},
		{5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,0,1,2,3,4},
		{4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,0,1,2,3},
		{3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,0,1,2},
		{2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,0,1},
		{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,0},
		
	};
	
	
						
	Color dayFogColor;
	Color dayAmbientColor;
	Color daySkyboxColor;
	Color sunsetFogColor;
	Color sunsetAmbientColor;
	Color sunsetSkyboxColor;

	
	GUIStyle style = new GUIStyle();
	GUIStyle crosshairStyle = new GUIStyle();
	Texture2D correctSelectionTexture, incorrectSelectionTexture, crosshairTexture;
	Texture2D[] planeThumbnails;
	private MouseLook mouseLookScript;
	//public GameObject magicLens;
	GameObject monoCamera;
	GameObject stereoCamera;
	GameObject virtCamMono;
	Camera magicLensCam;
	GameObject selectionLightObj;
	Simulator simScript;
	Vector3 defaultPosition;
	Vector3 defaultDirection;
	Vector3 startPosition;
	Material aMaterial;
	Texture2D tempTex;
	void Awake()
	{
		m_Instance = this;

		aMaterial = new Material(Shader.Find("Unlit/Texture"));
		//magicLens = GameObject.Find("VirtualCameraMono");
		//magicLens = GameObject.Find ("VirtualCameraStereo");
		
		
		simScript = this.gameObject.GetComponent<Simulator>();

		GameObject virtCamMono = GameObject.Find("VirtualCameraMono");
		mouseLookScript = virtCamMono.GetComponent<MouseLook>();

		
		
		startNodes[0] = GameObject.Find("Node1");
		startNodes[1] = GameObject.Find("Node2");
		startNodes[2] = GameObject.Find("Node3");
		startNodes[3] = GameObject.Find("Node4");
		startNodes[4] = GameObject.Find("Node5");
		startNodes[5] = GameObject.Find("Node6");
		startNodes[6] = GameObject.Find("Node7");
		
		
		crosshairTexture =  (Texture2D)Resources.Load("crosshair", typeof(Texture2D));
		correctSelectionTexture =  (Texture2D)Resources.Load("GUISelectionBox", typeof(Texture2D));
		incorrectSelectionTexture =  (Texture2D)Resources.Load("GUISelectionBoxWrong", typeof(Texture2D));
		planeThumbnails = new Texture2D[8];
		
		planeThumbnails[0] = (Texture2D)Resources.Load("privateplanet", typeof(Texture2D));
		planeThumbnails[1] = (Texture2D)Resources.Load("privateplanenotailt", typeof(Texture2D));
		planeThumbnails[2] = (Texture2D)Resources.Load("planet", typeof(Texture2D));
		planeThumbnails[3] = (Texture2D)Resources.Load("planenotailt", typeof(Texture2D));
		planeThumbnails[4] = (Texture2D)Resources.Load("biplanet", typeof(Texture2D));
		planeThumbnails[5] = (Texture2D)Resources.Load("biplanenotailt", typeof(Texture2D));
		planeThumbnails[6] = (Texture2D)Resources.Load("ghostplanet", typeof(Texture2D));
		planeThumbnails[7] = (Texture2D)Resources.Load("ghostplanenotailt", typeof(Texture2D));
		
		
		planeObjects = new GameObject[12];
		selectedPlane = null;
		cam1 = Camera.allCameras[0];
		cam2 = Camera.allCameras[1];
		//cam3 = Camera.allCameras[2];
		//monoCamera = GameObject.Find ("Camera");
		monoCamera = GameObject.Find("MonoCamera");
		//stereoCamera = GameObject.Find("VirtualCameraStereo");
		magicLensCam = monoCamera.GetComponent<Camera>();


		hSliderValue =  70;
		crosshairSphere = GameObject.Find("Sphere");
		
		experimentInterface = 0;
		FOV = 60;
		//selectByIntersection = true;
		
		sunsetFogColor = new Color32(24, 20, 17, 255);
		sunsetAmbientColor = new Color32(131, 115, 102, 255);
		sunsetSkyboxColor = new Color32(75, 42, 21, 255);
		dayFogColor = new Color32(118, 129, 102, 255);
		dayAmbientColor = new Color32(143, 148, 171, 255);
		daySkyboxColor = new Color32(148, 145, 145, 255);
		

		

//		selectionLightObj = new GameObject("Selection Box Light");
//        selectionLightObj.AddComponent<Light>();
//        selectionLightObj.light.color = Color.yellow;
//		selectionLightObj.light.range = 3.0f;
//		selectionLightObj.light.intensity = 5.0f;
//		selectionLightObj.light.type = LightType.Point;
//		selectionLightObj.layer = LayerMask.NameToLayer("Augmentation");
//		selectionLightObj.light.shadows = LightShadows.None;
		
		RenderSettings.fogDensity = 0.001f;
		
		defaultPosition = new Vector3(1119.56f, 109f, 571.1064f);
		defaultDirection = new Vector3(0,250,0);

		tempTex = new Texture2D(1024,578,TextureFormat.RGB24,false);
		tempTex.Apply();
	}
	
	void OnDestroy()
	{
		m_Instance = null;
	}
	
	bool runTimer = false;
	int trialNum = 0;
	int wrongSelections = 0;
	float timer=0;
	int selGridInt=0;
	
	float jitter = 0;
	float latency = 0;
	float randJitterX = 0;
	float randJitterY = 0;
	int heliCounter = 0;
	void Update()
	{
		//Debug.Log("Width: " + magicLensCam.pixelWidth + " Height: " + magicLensCam.pixelWidth);
		//Select which interface
		
		if(Input.GetKeyDown("1"))
		{
			experimentInterface = (int)interfaces.MagicLens;
		}
		if(Input.GetKeyDown("2"))
		{
			experimentInterface = (int)interfaces.WIM;
		}
		if(Input.GetKeyDown("3"))
		{
			experimentInterface = (int)interfaces.Pointing;
		}
		if(Input.GetKeyDown("4"))
		{
			FOV = 60;
			FOVGUI = 0;
		}
		if(Input.GetKeyDown("5"))
		{
			FOV = 30;
			FOVGUI = 1;
		}
		if(runTimer)
		{
			timer += Time.deltaTime;	
		}
	
		if(Input.GetKeyDown (KeyCode.Return))
		{
			timer = 0;
			runTimer = true;
			//Temporarily changing this for TV demo *update 6-4-2014 changing for EOYS (changed to helicopters)
//			for(int i=0; i</*12*/ 2; i++)
//			{
//				if(planeObjects[i] != null)
//				{
//					Destroy (planeObjects[i]);
//					planeObjects[i] = null;
//				}
//				
//				GameObject plane = (GameObject)Resources.Load(randomPlanes[counterbalancedTrials[0, 0],i]);
//
//				GameObject startNode = startNodes[i%6];
//
//				Vector3 startPosition = new Vector3(startNode.transform.position.x, heights[i], startNode.transform.position.z);
//				
//				
//				planeObjects[i] = (GameObject) Instantiate(plane, startPosition, Quaternion.identity);
//				
//				Animate anScript2 = planeObjects[i].GetComponent<Animate>();
//				
//				anScript2.setStartNode (startNode);
//				anScript2.setTargetNode ((i%6)+1);
//
//
//			}

			GameObject plane = (GameObject)Resources.Load(randomPlanes[counterbalancedTrials[0, 0],0]);
			
			startPosition = new Vector3(1119.56f, 109f, 571.1064f); //Default start position (i.e. always instantiate objects here)
			
			
			planeObjects[0] = (GameObject) Instantiate(plane, startPosition, Quaternion.identity);
			planeObjects[0].name = "Marine Heli " + heliCounter.ToString();

			heliCounter++;
			
		}
		
		
		
		if(Input.GetKeyDown (KeyCode.Space))
			clearPlaneObjects();
		
		
		if(Input.GetKeyDown ("`"))
			showGUI = !showGUI;
		
		if(Input.GetKeyDown ("s"))
			startTask = true;
		

		if(planeObjects[0])
		{
			//simScript.setTrackerJitterLatency(planeObjects[0], jitter, monoCamera.transform.eulerAngles.x, monoCamera.transform.eulerAngles.y);
			//simScript.setTrackerJitterLatency(GameObject.Find("bench001"), jitter, monoCamera.transform.eulerAngles.x, monoCamera.transform.eulerAngles.y);

		}
			
	}

	public void clearPlaneObjects()
	{
		runTimer = false;
		wrongSelections = 0;
		
		for(int i = 0; i<12; i++)
		{
			Destroy(planeObjects[i]);
			planeObjects[i] = null;
		}
		
		trialNum++;
		if(trialNum %20 == 0)
		{
			trialNum = 0;
			//participantNumber++;
			
			
			if(participantNumber%20==0)
				participantNumber = 0;

		}
	}
	
	public void setSelectedPlane(GameObject plane)
	{
		this.selectedPlane = plane;
		
		if(!selectedPlane.Equals(planeObjects[correctPlanes[counterbalancedTrials[participantNumber, trialNum]]]))
			wrongSelections++;
	}
	
	public void recordTrial()
	{
			System.IO.File.AppendAllText(Application.dataPath + "/../User" + userID + ".txt", System.String.Format("{0} {1} {2} {3} {4} \n", trialNum, experimentInterface, FOV, timer, wrongSelections));
			participantNumber = int.Parse (userID);
	}
	
	string userID = "";
	string trialNumString = "";
	
	
	Vector3 worldToMLScreenPoint(Vector3 worldPoint)
	{
		Matrix4x4 modelView = magicLensCam.worldToCameraMatrix;
		Matrix4x4 projection = magicLensCam.projectionMatrix;
		Matrix4x4 modelViewProjection = projection*modelView;
		Vector3 clippingCoord = modelViewProjection.MultiplyPoint(worldPoint);
		
		float screenX = ((clippingCoord.x+1f)/2f)*magicLensCam.pixelWidth;
		
		return new Vector3(screenX, 200f, 0f);
	}
	

	Vector3 unityViewportPoint = Vector3.zero;
	float screenAspect = (float)Screen.width/(float)Screen.height;
	Vector2 scrollPosition = Vector2.zero;
	GameObject objectSelected;
	float objVisibility = 0;
	float objVelocity = 10;
	float pathLength = 100;
	GameObject objOfInterest;

	void OnGUI()
	{

//		Texture2D screenTex = new Texture2D(Screen.width, Screen.height);
//		screenTex.ReadPixels(new Rect(0, 0, screenTex.width, screenTex.height), 0, 0);
//		screenTex.Apply ();
//		byte[] data = screenTex.EncodeToPNG();
//		File.WriteAllBytes(Application.dataPath + "/../screen.png", data);
//
//		RenderTexture temp = magicLensCam.targetTexture;
//
//		RenderTexture.active = temp;
//		Texture2D mlTex = new Texture2D(1024, 1024);
//		mlTex.ReadPixels(new Rect(0, 0, temp.width, temp.height), 0, 0);
//		mlTex.Apply();
//
//		byte[] data2 = mlTex.EncodeToPNG();
//		File.WriteAllBytes(Application.dataPath + "/../ML.png", data2);
//
//		RenderTexture.active = null;

//		RenderTexture.active = magicLensCam.targetTexture;
//		//Mini Augmented view on screen
//
//		tempTex.ReadPixels(new Rect(0, 224 /*192*/, 1024, 578), 0, 0);
//
//
//		RenderTexture.active = null;

		Graphics.DrawTexture(new Rect(600, Screen.height-225, 400, 228), magicLensCam.targetTexture, aMaterial);

		/*
		 * AR Annotations via GUI class
		 * Values to map from world to magic lens space depend on the resolution of the big screen, not the camera's render texture :(
		 * Reactivate when not using 3D annotations
		 */ 
		
//		RenderTexture.active = magicLensCam.targetTexture;
//		
//		if(selectedPlane)
//		{
//			unityViewportPoint = magicLensCam.WorldToViewportPoint(selectedPlane.transform.position);
//			//unityViewportPoint = worldToMLScreenPoint (selectedPlane.transform.position);
//		}
//		
//		unityViewportPoint.x *= Screen.width;
//		float MLHeight = Screen.width/(3450f/1200f);//(3570f/1200f);
//		unityViewportPoint.y = MLHeight - (unityViewportPoint.y * MLHeight);
//		
//		//Should make classes for AR annotations!
//		float boxHeight = 150f;
//		float boxWidth = 150f*screenAspect;
//		
//		GUI.color = Color.white;
//		//GUI.Box(new Rect(unityViewportPoint.x - (boxWidth/2f), unityViewportPoint.y - (boxHeight/2f), boxWidth, boxHeight), magicLensCam.aspect + System.Environment.NewLine + magicLensCam.pixelRect);
//		GUI.DrawTexture(new Rect(unityViewportPoint.x - (boxWidth/2f), unityViewportPoint.y - (boxHeight/2f), boxWidth, boxHeight), correctSelectionTexture, ScaleMode.ScaleToFit, true, 0.0f);
//
//		//GUI.Box (new Rect(400,400,200,200), "BUTTS");
//		RenderTexture.active = null;
		
		

		/*
		 * Task specific GUI elements 
		 */
		//plane preview
		if(startTask)
		{
			Texture2D correctPlaneThumbnail = planeThumbnails[correctPlaneThumbnails[counterbalancedTrials[participantNumber, trialNum]]];
			style.normal.background = correctPlaneThumbnail;	
			GUI.Box (new Rect(0,Screen.height - correctPlaneThumbnail.height, correctPlaneThumbnail.width, correctPlaneThumbnail.height),"", style);
			
			if(trialNum < 5)
			{
				GUI.contentColor = Color.red;
				GUI.Label(new Rect(25, 25, 300, 40), "TRAINING");
			}

				
		}
		
		if(showGUI)
		{


			//FOV Setup *not needed anymore, plus we are using more cameras than the simpler 2 camera setup
			
//			hSliderValue = GUI.HorizontalSlider(new Rect(25, 45, 400, 30), hSliderValue, 40.0F, 80.0F);
//			
//				//(360 (tan^(-1)(80/119 tan((pi*60)/360))))/pi
//			float t1 = (80f/119f)*Mathf.Tan((3.14159f*hSliderValue)/360f); //correct
//			float t2 = 360f*Mathf.Atan(t1)/3.14159f;
//			float vertFOV = t2;
//			
//			cam1.fieldOfView = vertFOV;
//			cam2.fieldOfView = vertFOV;
//			
//		    cam2.transform.localEulerAngles= new Vector3(0f, hSliderValue, 0f);	
//			

			GUI.color = Color.red;
//			GUI.Label(new Rect(25, 25, 40, 20), hSliderValue.ToString());

			GUI.Label(new Rect(0, 0, 600, 140), "Real World Atmosphere Parameters:");
			GUI.Label(new Rect(0, 70, 600, 140), "Real World Scene Object Parameters:");
			GUI.Label(new Rect(300, 0, 600, 140), "Augmenting Device Parameters:");


			/*
			 * Controls to change light settings
			 * Main directional light, fog, ambient light
			 * Day fog (118, 129, 102), exp2, fog start 0, fog end 650, fog density .0009
			 * ambient day light color (143,148,171)
			 * ambient sunset light color (131,115,102), skybox tint sunset color (75, 42, 21)
			 * sunset fog (24, 20, 17), exp, fog start 0, fog end 650, fog density .003
			 * /
			/*
			 * Sliders to control offsets on magic lens
			 */
//			GameObject dayLight = GameObject.Find("Directional light Day");
//			GameObject sunsetLight = GameObject.Find("Directional light Night");
//			Light dayLightSettings = dayLight.GetComponent<Light>();
//			Light sunsetLightSettings = sunsetLight.GetComponent<Light>();
			
			//bool temp = GUI.Toggle(new Rect(25, 45, 100, 30), false, "Sunset");
			GUI.color = Color.white;
//			string[] selStrings = new string[] {"Daytime", "Sunset"};
//			selGridInt = GUI.SelectionGrid(new Rect(25, 40, 100, 100), selGridInt, selStrings, 1);
//			if(selGridInt == 1)
//			{	
//				dayLightSettings.intensity = 0f;
//				sunsetLightSettings.intensity = 2f;
//				RenderSettings.fogColor = sunsetFogColor;
//				RenderSettings.fogMode = FogMode.Exponential;
//				//RenderSettings.fogDensity = .003f;
//
//				//RenderSettings.ambientLight = sunsetAmbientColor;
//				RenderSettings.ambientLight = new Color(.75f,.75f,.75f,1f);
//				RenderSettings.skybox.SetColor("_Tint", sunsetSkyboxColor);
//			}
//			else
//			{
//				
//			}
//			
//			//bool temp1 = GUI.Toggle(new Rect(25, 65, 100, 30), false, "Day");
//			if(selGridInt == 0)
//			{
//				dayLightSettings.intensity = 0.8f;
//				sunsetLightSettings.intensity = .5f;
//				RenderSettings.fogColor = dayFogColor;
//				RenderSettings.fogMode = FogMode.ExponentialSquared;
//				//RenderSettings.fogDensity = .0009f;
//				
//				//RenderSettings.ambientLight = dayAmbientColor;
//				RenderSettings.ambientLight = new Color(.75f,.75f,.75f,1f);
//				RenderSettings.skybox.SetColor("_Tint", daySkyboxColor);
//
//			}


			/*
			 * Set augmenting device parameters
			 */ 
			/*
			 * Set Tracker jitter
			 */

			GUI.Label(new Rect(325, 50, 100, 140), "Tracking Latency");
			float tempLatency = GUI.HorizontalSlider(new Rect(325, 70, 200, 20), latency, 0F, 10F);
			
			if(latency != tempLatency)
				simScript.setTrackerLatency(tempLatency);

			latency = tempLatency;


			GUI.Label(new Rect(325, 20, 100, 140), "Tracking Jitter");
			jitter = GUI.HorizontalSlider(new Rect(325, 40, 200, 20), jitter, 0F, 20F);



			
			//Set FOV
			GUI.Label(new Rect(325, 80, 200, 140), "Field of View: " + FOV);
			FOV = GUI.HorizontalSlider(new Rect(325, 100, 200, 20), FOV, 10F, 90F);
			simScript.setARFOV(monoCamera, FOV);

			/*
			 * Set atmosphere parameters
			 */ 
			/*
			 * Set Visibility
			 */
			GUI.Label(new Rect(25, 20, 100, 140), "Visibility");
			//RenderSettings.fogDensity = GUI.HorizontalSlider(new Rect(25, 180, 200, 20), RenderSettings.fogDensity, 0F, .005F);
			GameObject atmosphere = GameObject.Find("RollingSmoke");
			if(atmosphere)
			{
				objVisibility = GUI.HorizontalSlider(new Rect(25, 40, 200, 20), objVisibility, 1F, 0F);
				simScript.setVisibility(atmosphere, objVisibility);
			}


			/*
			 * Select object
			 */
			if(objOfInterest)
				GUI.Label(new Rect(25, 90, 600, 140),"Object Chosen: " + objOfInterest.name);
		
			GameObject[] gameObjects = FindObjectsOfType(typeof(GameObject)) as GameObject[]; 
			List<bool> buttonPresses = new List<bool>();
			int spacing = 15;
			scrollPosition = GUI.BeginScrollView(new Rect(25, 110, 218, 80), scrollPosition, new Rect(0, 0, 200, gameObjects.Length*spacing));
			int counter=0;

			//buttonPresses.Add (GUI.Button(new Rect(0, spacing*counter, 200, spacing), "Bench 1"));
			//buttonPresses.Add (GUI.Button(new Rect(0, spacing*1/*counter*/, 200, spacing), "Security 2"));
			//buttonPresses.Add (GUI.Button(new Rect(0, spacing*2/*counter*/, 200, spacing), "Security 3"));

			for(int i=0; i<gameObjects.Length; i++)
			{
				if((!gameObjects[i].transform.parent || gameObjects[i].transform.parent.name == "Static")&& gameObjects[i].layer == LayerMask.NameToLayer("SceneObjects"))
				{
					buttonPresses.Add (GUI.Button(new Rect(0, spacing*counter, 200, spacing), gameObjects[i].name));
					//buttonPresses.Add (GUI.Button(new Rect(0, spacing*counter, 200, spacing), GameObject.Find("bench001")));
					counter++;
				}
				else
				{
					buttonPresses.Add (false);
				}
			}
			GUI.EndScrollView();
			//Debug.Log ("buttonpresses: " + buttonPresses.Count + " gameObjects: " + gameObjects.Length);


			for(int i=0; i<buttonPresses.Count; i++)
			{
				if(buttonPresses[i])
				{
					objOfInterest = gameObjects[i];
				}
			}
			buttonPresses.Clear();

			if(this.selectedPlane)
			{
				objOfInterest = this.selectedPlane;
			}

			if(objOfInterest)
				simScript.setTrackerJitterLatency(objOfInterest, jitter, monoCamera.transform.eulerAngles.x, monoCamera.transform.eulerAngles.y);

			/*
			 * Set selected object velocity
			 */
			//GameObject objOfInterest = GameObject.Find(objectSelected.name);
			GameObject sceneCam = GameObject.Find ("HMD1");
			float sceneRotX = sceneCam.transform.localEulerAngles.x;
			float sceneRotY = sceneCam.transform.localEulerAngles.y;
			float sceneRotZ = sceneCam.transform.localEulerAngles.z;

			float mlRotX = mouseLookScript.offsetX;
			float mlRotY = mouseLookScript.offsetY;
			float mlRotZ = mouseLookScript.offsetZ;


		
			GUI.Label(new Rect(600, 230, 300, 140), "Scene rotation x: " + sceneCam.transform.eulerAngles.x);
			sceneRotX = GUI.HorizontalSlider(new Rect(600, 250, 200, 20), sceneRotX, 0F, 360F);
			GUI.Label(new Rect(600, 260, 300, 140), "Scene rotation y: " + sceneCam.transform.eulerAngles.y);
			sceneRotY= GUI.HorizontalSlider(new Rect(600, 280, 200, 20), sceneRotY, 0F, 360F);
			GUI.Label(new Rect(600, 290, 300, 140), "Scene rotation z: " + sceneCam.transform.eulerAngles.z);
			sceneRotZ = GUI.HorizontalSlider(new Rect(600, 310, 200, 20), sceneRotZ, 0F, 360F);

			GUI.Label(new Rect(600, 320, 300, 140), "ML rotation x: " + mouseLookScript.offsetX);
			mouseLookScript.offsetX = GUI.HorizontalSlider(new Rect(600, 340, 200, 20),mouseLookScript.offsetX, 0F, 360F);
			GUI.Label(new Rect(600, 350, 300, 140), "ML rotation y: " +mouseLookScript.offsetY);
			mouseLookScript.offsetY = GUI.HorizontalSlider(new Rect(600, 370, 200, 20),mouseLookScript.offsetY, 0F, 360F);
			GUI.Label(new Rect(600, 380, 300, 140), "ML rotation z: " + mouseLookScript.offsetZ);
			mouseLookScript.offsetZ = GUI.HorizontalSlider(new Rect(600, 400, 200, 20), mouseLookScript.offsetZ, 0F, 360F);


			sceneCam.transform.localEulerAngles = new Vector3(sceneRotX,sceneRotY,sceneRotZ);
			
			if(objOfInterest && objOfInterest.GetComponent<Animate>())
			{
				Vector3 objDirection = simScript.getDirection(objOfInterest);
				Vector3 objStartPosition = simScript.getStart(objOfInterest);
				GUI.Label(new Rect(25, 200, 100, 140), "Object Velocity");
				objVelocity = GUI.HorizontalSlider(new Rect(25, 220, 200, 20), simScript.getVelocity(objOfInterest), 0F, 30F);
				simScript.setVelocity (objOfInterest, objVelocity);

				/*
				 * Set selected object direction
				 */


				GUI.Label(new Rect(25, 230, 300, 140), "Direction x: " + objDirection.x);
				objDirection.x = GUI.HorizontalSlider(new Rect(25, 250, 200, 20), objDirection.x, 0F, 360F);
				GUI.Label(new Rect(25, 260, 300, 140), "Direction y: " + objDirection.y);
				objDirection.y = GUI.HorizontalSlider(new Rect(25, 280, 200, 20), objDirection.y, 0F, 360F);
				GUI.Label(new Rect(25, 290, 300, 140), "Direction z: " + objDirection.z);
				objDirection.z = GUI.HorizontalSlider(new Rect(25, 310, 200, 20), objDirection.z, 0F, 360F);
				simScript.setDirection(objOfInterest, objDirection);

				GUI.Label(new Rect(25, 330, 300, 140), "Start x: " + objStartPosition.x);
				objStartPosition.x = GUI.HorizontalSlider(new Rect(25, 350, 200, 20), objStartPosition.x, 0F, 2000F);
				GUI.Label(new Rect(25, 360, 300, 140), "Start y: " + objStartPosition.y);
				objStartPosition.y = GUI.HorizontalSlider(new Rect(25, 380, 200, 20), objStartPosition.y, 0F, 2000F);
				GUI.Label(new Rect(25, 390, 300, 140), "Start z: " + objStartPosition.z);
				objStartPosition.z = GUI.HorizontalSlider(new Rect(25, 410, 200, 20), objStartPosition.z, 0F, 2000F);
				simScript.setStart(objOfInterest, objStartPosition);
				
				GUI.Label(new Rect(25, 430, 300, 140), "Path Length: " + pathLength);
				pathLength = GUI.HorizontalSlider(new Rect(25, 450, 200, 20), pathLength, 0F, 1000F);
				simScript.setPathLength(objOfInterest, pathLength);
			}
			
			
			
//			GUI.Label(new Rect(400, 400, 80, 80), "X: " + unityViewportPoint.x.ToString());
//			unityViewportPoint.x = GUI.HorizontalSlider(new Rect(400, 420, 800, 20), unityViewportPoint.x, 0f, 1700f);
//		
//			GUI.Label(new Rect(400, 440, 80, 80), "Y: " + unityViewportPoint.y.ToString());
//			unityViewportPoint.y = GUI.HorizontalSlider(new Rect(400, 460, 800, 20), unityViewportPoint.y, -100f, 600f);

		}

		
	}
			
			
			
			
			
//			
//			mouseLookScript.offsetX = GUI.HorizontalSlider(new Rect(25, 45, 400, 30), mouseLookScript.offsetX, 0.0F, 360.0F);
//			mouseLookScript.offsetY = GUI.HorizontalSlider(new Rect(25, 65, 400, 30), mouseLookScript.offsetY, 0.0F, 360.0F);
//			mouseLookScript.offsetZ = GUI.HorizontalSlider(new Rect(25, 85, 400, 30), mouseLookScript.offsetZ, 0.0F, 360.0F);
//		
//			/*
//			 * Participant Entry and Trial entry
//			 */
//			GUI.Label(new Rect(25, 75, 100, 40), "Enter User ID:");
//			userID = GUI.TextField(new Rect(25, 105, 200, 20), userID, 20);
//			
//			GUI.Label(new Rect(500, 75, 100, 40), "Enter Trial Number (optional):");
//			
//			trialNumString = GUI.TextField(new Rect(500, 105, 200, 20), trialNumString, 20);
//			
//			if(GUI.Button(new Rect(500, 140, 80, 30), "Submit"))
//			{
//				trialNum = int.Parse(trialNumString);
//			}
//			string experimentInfo = "Participant: " + participantNumber.ToString() + " Trial: " + trialNum.ToString();
//			GUI.Label(new Rect(500, 25, 300, 40), experimentInfo);
//			GUI.Label(new Rect(500, 40, 300, 40), "Counterbalanced trial num: " + counterbalancedTrials[participantNumber, trialNum].ToString());
//			GUI.Label(new Rect(500, 55, 300, 40), "Interface: " + experimentInterface + " FOV: " + FOVGUI );
//			
//			if(GUI.Button(new Rect(25, 140, 80, 30), "Submit"))
//			{
//				System.IO.File.AppendAllText(Application.dataPath + "/../User" + userID + ".txt", "Trial # / Interface / FOV / Elapsed Time / # Errors\n");
//				
//				participantNumber = int.Parse (userID);
//
//				//participantNumber++;
//				//if(participantNumber%20==0)
//				//	participantNumber = 0;
//			}
//			
//			GUI.color = Color.black;
//			GUI.Label(new Rect(25, 180, 200, 50), "Data file set for user: " + participantNumber.ToString());
//			
//
//		}
//		
//		/*
//		 * Shows a box around current selected plane
//		 */
//		if(selectedPlane != null)
//		{
//			//Check if center point projects to left or right viewport and assign screen coordinates accordingly
//			Vector3 cam1View = cam1.WorldToViewportPoint (selectedPlane.transform.position);
//			Vector3 cam2View = cam2.WorldToViewportPoint (selectedPlane.transform.position);
//			//Debug.Log ( cam1View + " " + cam2View);
//			
//			Vector3 vec = Vector3.zero; //= new Vector3 (b.center.x + b.extents.x, b.center.y + b.extents.y, b.center.z + b.extents.z);
//			
//			if(cam1View.x >= 0 && cam1View.x <= 1 && cam1View.y >= 0 && cam1View.y <= 1 && selectedPlane.renderer.isVisible)
//			{
//				vec = cam1.WorldToScreenPoint (selectedPlane.transform.position);
//			}
//			else if(cam2View.x >= 0 && cam2View.x <= 1 && cam2View.y >= 0 && cam2View.y <= 1 && selectedPlane.renderer.isVisible)
//			{
//				vec = cam2.WorldToScreenPoint (selectedPlane.transform.position);
//			}
//			float boxHeight = 1500f*1/vec.z;
//			float boxWidth = 1500f*1/vec.z;
//	
//			//GUI.color = Color.green;
//			
//			if(selectedPlane.Equals(planeObjects[correctPlanes[counterbalancedTrials[participantNumber, trialNum]]]))
//			{
//				style.normal.background = correctSelectionTexture;
//				recordTrial();
//				clearPlaneObjects ();
//			}
//			else
//			{
//				style.normal.background = incorrectSelectionTexture;
//				
//			}
//			
//			//GUI.backgroundColor = new Color(0,1,0,0.4f);
//			GUI.Box (new Rect(vec.x-boxWidth/2, Screen.height-vec.y-boxHeight/2, boxWidth, boxHeight),"", style);
//			
//		}
//		
//		/*
//		 * Shows "cross hair" for the pointing interface
//		 */
//		if(getExperimentInterface () == (int)interfaces.Pointing)
//		{
//			//Project a ray from camera center along a radius of 1000 units in direction of optical center  
//			intersectionCoords = new Vector3(cam3.pixelWidth/2.0f, cam3.pixelHeight/2.0f, 0.0f);
//			Ray ray = cam3.ViewportPointToRay(new Vector3(0.5f, 0.5f, 0f));//cam3.ScreenPointToRay (intersectionCoords);
//			
//			Vector3 crosshair3DPosition = ray.GetPoint(1000);//new Vector3(ray.direction.x*1000f,ray.direction.y*1000f, ray.direction.z*1000f);;
//			//Debug.DrawLine (ray.origin, ray.GetPoint(1000));
//
//			//Find screen point of end of ray and place crosshair
//			
//			//Check if center point projects to left or right viewport and assign screen coordinates accordingly
//			Vector3 cam1View = cam1.WorldToViewportPoint (crosshair3DPosition);
//			Vector3 cam2View = cam2.WorldToViewportPoint (crosshair3DPosition);
//			
//			Vector3 vec = Vector3.zero; //= new Vector3 (b.center.x + b.extents.x, b.center.y + b.extents.y, b.center.z + b.extents.z);
//			
//			if(cam1View.x >= 0 && cam1View.x <= 1 && cam1View.y >= 0 && cam1View.y <= 1/* && renderer.isVisible*/)
//			{
//				vec = cam1.WorldToScreenPoint (crosshair3DPosition);
//			}
//			else if(cam2View.x >= 0 && cam2View.x <= 1 && cam2View.y >= 0 && cam2View.y <= 1 /*&& renderer.isVisible*/)
//			{
//				vec = cam2.WorldToScreenPoint (crosshair3DPosition);
//			}
//			
//			float boxHeight = 25f;
//			float boxWidth = 25f;
//			//GUI.backgroundColor = Color.green;
//			crosshairStyle.normal.background = crosshairTexture;
//			GUI.Box (new Rect(vec.x-boxWidth/2, Screen.height-vec.y-boxHeight/2, boxWidth, boxHeight),"", crosshairStyle);
//		
//		}
//	}
	
	public int getExperimentInterface()
	{
		return experimentInterface;
	}
	
	public float getFOV()
	{
		return FOV;
	}
	
}

