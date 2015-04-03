using UnityEngine;
using System.Collections;

public class CreateRenderTexture : MonoBehaviour {

	public RenderTexture cam1RenderTex;
	public RenderTexture cam2RenderTex;
	int halfAlloWidth = 1785;//3570;
	int alloHeight = 1200;
	public int whichCamera;
	
	// Use this for initialization
	void Start () {
		if(whichCamera==1)
		{
			cam1RenderTex = new RenderTexture(halfAlloWidth, alloHeight, 24);	
			this.gameObject.GetComponent<Camera>().targetTexture = cam1RenderTex;
			Material cam1Material = (Material)Resources.Load("Cam1Material", typeof(Material));
			//this.gameObject.camera.aspect = 1785/1200;
			cam1Material.mainTexture = cam1RenderTex;
			
		}
		else if(whichCamera==2)
		{
			cam2RenderTex = new RenderTexture(halfAlloWidth, alloHeight, 24);
			this.gameObject.GetComponent<Camera>().targetTexture = cam2RenderTex;
			Material cam2Material = (Material)Resources.Load("Cam2Material", typeof(Material));
			//this.gameObject.camera.aspect = 1785/1200;
			cam2Material.mainTexture = cam2RenderTex;
			
		}
		
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
