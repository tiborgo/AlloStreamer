using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class NotifyBox : MonoBehaviour {
	public GameObject player;
	public GameObject sys;
	public GameObject leftPlane;
	public UseRenderingPlugin renderingPluginScript;
	public GameObject monoCamera;
	public GameObject leftCamera;
	private MainScript mainScript;
	Camera magicLensCam;
	Vector3 intersectionCoords;
	
	[DllImport ("UnityServerPlugin")]
	private static extern float getTouchX ();
	
	[DllImport ("UnityServerPlugin")]
	private static extern float getTouchY ();
	
	[DllImport ("UnityServerPlugin")]
	private static extern float getDragX ();
	
	[DllImport ("UnityServerPlugin")]
	private static extern float getDragY ();
	
	void Start () {
		player = this.gameObject;
		
				
		monoCamera = GameObject.Find("MonoCamera");
		//leftCamera = GameObject.Find("LeftEyeCamera");
		//renderingPluginScript = monoCamera.GetComponent<UseRenderingPlugin>();
		magicLensCam = monoCamera.GetComponent<Camera>();
		
		sys = GameObject.Find("System");
		mainScript = sys.GetComponent<MainScript>();

		leftPlane = GameObject.Find ("PlaneLeft");
		
	}

	void Update () {
		//Vector3 screenPosition = magicLensCam.WorldToScreenPoint(this.transform.position);
		//checkClear();
		checkObjectSelection();

	}
	
	 
//	void checkClear()
//	{
//		if(mainScript.getClear())
//			Destroy (this.gameObject);
//	}
	bool dragged = false;
	int dragCounter = 0;
	void checkObjectSelection()
	{
		/**
		 * This is for a selection via the optical axis of the ML camera
		 * /

//		intersectionCoords = new Vector3(magicLensCam.pixelWidth/2 , magicLensCam.pixelHeight/2 , 0); // choose center pixels
//		
//		Ray ray = magicLensCam.ScreenPointToRay (intersectionCoords);
//		
//		Debug.DrawLine(ray.origin, ray.GetPoint (1000));
//		
//		Vector3 ray3D = magicLensCam.ScreenToWorldPoint(new Vector3(magicLensCam.pixelWidth/2, magicLensCam.pixelHeight/2, 1)); //center pixels again
//		RaycastHit hit = new RaycastHit();
//		Physics.SphereCast(ray.origin, 1f, ray.direction, out hit, 1000);
//		if (hit.collider == this.gameObject.collider)/*collider.Raycast (ray, out hit, 10000.0f)*/ 
//		{
//			
//			mainScript.setSelectedPlane(this.gameObject);
//			//renderingPluginScript.drawSelectionIndication();
//			//Destroy (this.gameObject);
//			
//			
//		}

		/**
		 * These were for the multi interface study, but the first block handles typical magic lens touch input for object selection
		 */
		//Figure out from the main script what interface we are trying to use (Magic Lens, WIM or pointing)
//		if(/*mainScript.getExperimentInterface() != (int)MainScript.interfaces.Pointing*/ false)
//		{
			float touchX = getTouchX ();
			float touchY = getTouchY ();

			if(touchX!=0 && touchY!=0)
			{
				if(dragCounter < 15)
				{
					intersectionCoords = new Vector3(touchX, touchY + 224, 0);
					
					Ray ray = magicLensCam.ScreenPointToRay (intersectionCoords);
					
					Debug.DrawLine(ray.origin, ray.GetPoint (500));
					
					Vector3 ray3D = magicLensCam.ScreenToWorldPoint(new Vector3(touchX, touchY + 224, 1));
				    RaycastHit hit = new RaycastHit();
				    Physics.SphereCast(ray.origin, 1f, ray.direction, out hit, 500);
					if (hit.collider == this.gameObject.GetComponent<Collider>())/*collider.Raycast (ray, out hit, 10000.0f)*/ 
					{
	
						mainScript.setSelectedPlane(this.gameObject);
						//renderingPluginScript.drawSelectionIndication();
						//Destroy (this.gameObject);
						
						
				    }
				}
				dragCounter = 0;
				
			}
			
			if(getDragX() != 0 || getDragY() != 0)
			{
				dragCounter++;
			}
//			
//			
//		}
//		else if (false)
//		{
//			//Find intersection of device cameras optical axis with objects
//			
//			float touchX = getTouchX ();
//			float touchY = getTouchY ();
//			if(touchX!=0 && touchY!=0)
//			{
//				intersectionCoords = new Vector3(magicLensCam.pixelWidth/2.0f, magicLensCam.pixelHeight/2.0f, 0.0f);
//				Ray ray = magicLensCam.ScreenPointToRay (intersectionCoords);
//			    RaycastHit hit = new RaycastHit();
//				Physics.SphereCast(ray.origin, 0.75f, ray.direction, out hit, 500);
//				if (hit.collider == this.gameObject.collider)/*collider.Raycast (ray, out hit, 10000.0f)*/ 
//				{
//					mainScript.setSelectedPlane(this.gameObject);
//					//Destroy (this.gameObject);
//				
//			    }
//			}
//			
//		}
			
			
	}

}
