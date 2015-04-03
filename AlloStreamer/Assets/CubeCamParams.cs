using UnityEngine;
using System.Collections;

public class CubeCamParams : MonoBehaviour {

	Camera cam;
	RenderTexture rtex;
	int cubemapSize = 1024;
	float nearClip = 0.01f;
	float farClip = 1f;
	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		//updateCubemap (63);
	}
	
	void updateCubemap(int faceMask) {
//		if (!cam) {
//			
//			GameObject go = new GameObject("CubeCamera", typeof(Camera));
//			//go.hideFlags = HideFlags.HideAndDontSave;
//			go.transform.position = transform.position;
//			go.transform.rotation = Quaternion.identity;
//			cam = go.camera;
//			//cam.cullingMask = layerMask;
//			cam.nearClipPlane = nearClip;
//			cam.farClipPlane = farClip;
//			cam.enabled = false;
//		}
//		
//		if (!rtex) {	
//			//rtex = new RenderTexture (cubemapSize, cubemapSize, 16);
//			rtex = (RenderTexture)Resources.Load("CubeMapRT", typeof(RenderTexture));
//			//cam.targetTexture = rtex;
//			rtex.isCubemap = true;
//			//rtex.hideFlags = HideFlags.HideAndDontSave;
//			Material cubemapmaterial = (Material)Resources.Load("CMMaterial", typeof(Material));
//			cubemapmaterial.SetTexture("_CM", rtex);
//			renderer.sharedMaterial.SetTexture ("_Cube", rtex);
//		}
//		
//		cam.transform.position = transform.position;
//		cam.RenderToCubemap (rtex, faceMask);
	}
}