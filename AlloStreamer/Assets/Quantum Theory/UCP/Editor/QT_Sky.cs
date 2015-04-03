using UnityEngine;
using System.Collections;
using UnityEditor;
[CustomEditor(typeof(QT_SkyBox))]
public class QT_Sky : Editor {
	
	[SerializeField]
	public QT_SkyBox sel;
	public int QT_Layer;
	
	//GUISkin QTSkin;
	
	public override void OnInspectorGUI ()
	{
		QT_Layer = LayerMask.NameToLayer("QT_SkyBox");
		
		GUIStyle g = new GUIStyle(GUI.skin.label);
		g.alignment = TextAnchor.UpperCenter;
		g.fontStyle = FontStyle.BoldAndItalic;
		g.fontSize = 20;
		GUILayout.Label("PhotoReal Skies",g);
		
		GUIStyle yellowtext = new GUIStyle(GUI.skin.label);
		yellowtext.normal.textColor = Color.yellow;
		
		EditorGUILayout.Space();
		if(QT_Layer<0)
			GUILayout.Label("Create a Layer called 'QT_SkyBox' to Begin!",yellowtext);
		
		
		sel=(QT_SkyBox)Selection.activeGameObject.GetComponent("QT_SkyBox");
				
				
		
		GUI.changed=false;
		sel.MainCam=(Camera)EditorGUILayout.ObjectField("Main Camera:",sel.MainCam,typeof(Camera),true);
		if(sel.MainCam)
		{
			if(FPSCheck())
			{
				GUILayout.Label("Ultimate FPSCamera Detected.",yellowtext);
				GUILayout.Label("Set QT_SkyBox script execution order time to 100 to limit jittering.",yellowtext);
				
			}
		}
		sel.SkyBox = (GameObject)EditorGUILayout.ObjectField("Skybox Prefab:",sel.SkyBox,typeof(GameObject),true);
		
		EditorGUILayout.Space();
		
		if(sel.SkyBox)
		{
			sel.SkyRotation = EditorGUILayout.Slider("Sky Rotation:",sel.SkyRotation,0f,359f);
			sel.SkyBox.transform.eulerAngles = new Vector3(sel.SkyBox.transform.eulerAngles.x,sel.SkyRotation,sel.SkyBox.transform.eulerAngles.z);
			sel.animated = EditorGUILayout.Toggle("Rotating:",sel.animated);
			if(sel.animated)
				sel.rate = EditorGUILayout.Slider("Rate:",sel.rate,1f,5f);
					
		}
		
		
		sel.SkyCamera = (Camera)EditorGUILayout.ObjectField("Sky Camera:",sel.SkyCamera,typeof(Camera),true);
		
		
		if(sel.SkyCamera)			
			sel.SkyCamera.depth = Mathf.RoundToInt(EditorGUILayout.Slider("Sky Camera Depth:",sel.SkyCamera.depth,-25,-1));
		
		
		if(GUILayout.Button("Apply Settings"))
		{
			
			
			if(DoChecks())
			{
				
				sel.SkyCamera.cullingMask = 1<<LayerMask.NameToLayer("QT_SkyBox");
				sel.SkyCamera.gameObject.layer = LayerMask.NameToLayer("QT_SkyBox");
				
				sel.SkyBox.layer = LayerMask.NameToLayer("QT_SkyBox");
				
				sel.MainCam.clearFlags = CameraClearFlags.Depth;
				sel.MainCam.cullingMask = ~(1 << LayerMask.NameToLayer("QT_SkyBox"));
				Debug.Log("Quantum Sky Created Successfully. Check the Game View.");
			}
		}
		
		
		if(GUILayout.Button("Help!"))
			Application.OpenURL("http://www.quantumtheoryentertainment.com");
		
		if(GUI.changed)		
			EditorUtility.SetDirty(sel);
		
		
	}
	
	private bool FPSCheck()
	{
		if(sel.MainCam.gameObject.GetComponent("vp_FPSCamera"))
			return true;
		else
			return false;
	}
		
	public bool DoChecks()
	{	
		bool isReady=false;
		bool depthNoGood=false;
		
		
		float depthVar=0;
		string lowDepthCam="";	
		
		Camera[] g = FindSceneObjectsOfType(typeof(Camera)) as Camera[];
		
		foreach(Camera o in g)
		{
			
			//find lowest depth in scene
			if(o.depth < depthVar && o!=sel.SkyCamera)
			{
				depthVar=o.depth;
				lowDepthCam = o.name;					
			}
			
			if(depthVar <= sel.SkyCamera.depth)			
					depthNoGood=true;
		}
		
		if(QT_Layer<0)		
			Debug.LogError("You must first create a layer named, 'QT_SkyBox' for the skybox to work, then setup the parameters in "+sel.name);
		else if (sel.SkyBox==null)
			Debug.LogError("No skybox prefab is specified in "+sel.name);
		else if (sel.MainCam==null)
			Debug.LogError("No Main Camera is specified in "+sel.name);
		else if (sel.SkyCamera.depth >=sel.MainCam.depth)
			Debug.LogError("The depth specified in "+sel.name+" isn't a good value for the Main Camera that is specified. Try a lower depth value.");
		else if (!sel.SkyBox.name.Contains("QT_"))
			Debug.LogError("The skybox prefab associated in the QT_SkyBox script for "+sel.name+" isn't a valid SkyBox.");
		else if (sel.SkyCamera==null)
			Debug.LogError("No Sky Camera is specified in "+sel.name);
		else if (depthNoGood)
			Debug.LogError(lowDepthCam+" has a depth value of "+depthVar+". The SkyCamera's depth has to be less than this.");
		else
			isReady=true;
		
		return isReady;
			
	}
}
