using UnityEngine;
using UnityEditor;
using System.Collections;

public class QuantumClouds : EditorWindow {
		[MenuItem ("Window/Quantum MetaClouds %&q")]
	
	static void Init()
	{
		QuantumClouds window =  (QuantumClouds)EditorWindow.GetWindow (typeof (QuantumClouds));
		window.title = "Quantum MetaClouds";
		window.maxSize=new Vector2(200,220);
		window.minSize = window.maxSize;
		window.Show();		
	}
	
	//Unneeded for now.
	//List<Material> MatList = new List<Material>();

	Color AmbientColor;
	Color SunColor;
	float Contrast =1, Bias = 0.5f, Opacity = 1.0f;
	GameObject LookAtObject;
	float CloudThickness; //z scale
	float CloudThicknessMax; //relative to 2x the x scale.

	void OnGUI()
	{
		GameObject[] selectionobjects = Selection.gameObjects;		
	
		GetMaterialValues(selectionobjects);
		
		if(GUILayout.Button("Help"))
		{
			EditorUtility.DisplayDialog("How to Use Quantum MetaClouds",
				"1. Drag and drop some Quantum MetaCloud prefabs into the scene.\n\n" +
				"2. Select the clouds you want to modify. You can adjust the sun and ambient colors using the color swatches.\n\n" +
				"3. Adjust their contrast and density using the sliders.\n\n" +
				"4. You can have them look at an object or the scene camera by clicking on the corresponding button.\n\n" +
				"WARNING: This tool overrides any values you set in the material.\n\nIf you want to make your own cloud textures, the cloud formation goes into the red channel, hard alpha goes into the green channel, soft alpha goes into the blue.","OK");
		}

		GUILayout.Label("Current Selection:");
		AmbientColor = EditorGUILayout.ColorField("Ambient Color: ",AmbientColor);
		SunColor = EditorGUILayout.ColorField("Sun Color: ",SunColor);
		EditorGUILayout.BeginHorizontal();
		GUILayout.Label("Clouds Contrast: ");
		Contrast = GUILayout.HorizontalSlider(Contrast,0.1f,3,null);
		EditorGUILayout.EndHorizontal();
		EditorGUILayout.BeginHorizontal();
		GUILayout.Label("Clouds Density: ");
		Bias = GUILayout.HorizontalSlider(Bias,0.25f,0.95f,null);
		EditorGUILayout.EndHorizontal();
		EditorGUILayout.BeginHorizontal();
		GUILayout.Label("Clouds Opacity: ");
		Opacity = GUILayout.HorizontalSlider(Opacity,1f,0f,null);
		EditorGUILayout.EndHorizontal();	
		EditorGUILayout.BeginHorizontal();
		GUILayout.Label("Cloud Thickness: ");
		CloudThickness = GUILayout.HorizontalSlider(CloudThickness,0.1f,1,null);
		EditorGUILayout.EndHorizontal();
			
		if(selectionobjects.Length!=0)
		{
		for(int x=0;x<selectionobjects.Length;x++)
			{
			float z = (selectionobjects[x].transform.localScale.x * 2) * CloudThickness;
			selectionobjects[x].transform.localScale = new Vector3(selectionobjects[x].transform.localScale.x,selectionobjects[x].transform.localScale.y,z);
			}
		}
		EditorGUILayout.BeginHorizontal();
		GUILayout.Label("Object to LookAt:");
		LookAtObject = (GameObject)EditorGUILayout.ObjectField(LookAtObject,typeof(GameObject),true,null);
		EditorGUILayout.EndHorizontal();

		if(GUILayout.Button("Look at Object"))
		{
			
			if(selectionobjects.Length==0)
					EditorUtility.DisplayDialog("No Selection is Made","Please select objects (clouds) you want to modify.","OK");
			else if (LookAtObject == null)
					EditorUtility.DisplayDialog("Look At Object is Invalid","Please pick an object to look at in the 'Object to LookAt' box.","OK");
			else
			for(int x=0;x<selectionobjects.Length;x++)
			{
					selectionobjects[x].transform.LookAt(LookAtObject.transform);	
			}
		}
		
		if(GUILayout.Button("Look at Scene Cam"))
		{			
			if(selectionobjects.Length==0)
					EditorUtility.DisplayDialog("No Selection is Made","Please select objects (clouds) you want to modify.","OK");
			else
				{
				for(int x=0;x<selectionobjects.Length;x++)
				{					
					selectionobjects[x].transform.LookAt(SceneView.lastActiveSceneView.camera.transform);
				}
			}
		}	
		if(selectionobjects.Length>0)
			UpdateMaterialColors(selectionobjects);
		
	}
	
	void UpdateMaterialColors(GameObject[] selection)
	{		
		for(int x=0;x<selection.Length;x++)
			{
			if(selection[x].GetComponent<Renderer>().sharedMaterial.shader.name.Contains("QuantumMetaCloud"))
				{
				selection[x].GetComponent<Renderer>().sharedMaterial.SetColor("_AmbientColor",AmbientColor);
				selection[x].GetComponent<Renderer>().sharedMaterial.SetColor("_SunColor",SunColor);
				selection[x].GetComponent<Renderer>().sharedMaterial.SetFloat("_CloudContrast",Contrast);
				selection[x].GetComponent<Renderer>().sharedMaterial.SetFloat("_Bias",Bias);
				selection[x].GetComponent<Renderer>().sharedMaterial.SetFloat("_AlphaCutoff",Opacity);
				}			
			}
	}
	
	
	
	void GetMaterialValues(GameObject[] selection)
	{
		if(selection.Length>0)
		{
		SunColor = selection[0].GetComponent<Renderer>().sharedMaterial.GetColor("_SunColor");
		AmbientColor = selection[0].GetComponent<Renderer>().sharedMaterial.GetColor("_AmbientColor");
		Contrast = selection[0].GetComponent<Renderer>().sharedMaterial.GetFloat("_CloudContrast");
		Bias = selection[0].GetComponent<Renderer>().sharedMaterial.GetFloat("_Bias");
		Opacity = selection[0].GetComponent<Renderer>().sharedMaterial.GetFloat("_AlphaCutoff");
					}
		
	}
	
	#region Unused Functions but kept around just in case.
	
//	void UpdateMaterialColors()
//	{
//		if(MatList.Count>0)
//		{				
//				for(int x=0;x<MatList.Count;x++)
//					{
//						MatList[x].SetColor("_AmbientColor",AmbientColor);
//						MatList[x].SetColor("_SunColor",SunColor);
//						MatList[x].SetFloat("_CloudContrast",Contrast);
//						MatList[x].SetFloat("_Bias",Bias);
//						MatList[x].SetFloat("_AlphaCutoff",Opacity);
//					}
//		}
//	}
//	void UpdateSceneCache()
//	{
//	
//			MatList.Clear();
//				//Material test = new Material(Shader.Find("QuantumClouds"));
//			Material[] tempmaterial = (Material[])Resources.FindObjectsOfTypeAll(typeof(Material));
//			for(int x=0;x<tempmaterial.Length;x++)
//			{
//				if(tempmaterial[x].shader.name.Contains("QuantumMetaCloud"))
//				{
//				MatList.Add(tempmaterial[x]);
//				}			
//			}	
//		
//	}
	#endregion
	
}
