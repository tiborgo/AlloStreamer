using UnityEngine;
using System.Collections;
[ExecuteInEditMode]
public class QT_TrafficLight : MonoBehaviour {
	[HideInInspector]
	public GameObject BulbGreen,BulbYellow,BulbRed;
	[HideInInspector]
	public bool showLinks=false;
	[HideInInspector]
	public Color linkColor;
	[HideInInspector]
	public Vector3 controllerPosition;
	[HideInInspector]
	public GameObject[] Lights = new GameObject[3];
	
	// Use this for initialization
	void Awake () {
	foreach(Transform t in transform)
		{
			if(t.name.Contains("BulbGreen"))
				BulbGreen = t.gameObject;
			if(t.name.Contains("BulbRed"))
				BulbRed = t.gameObject;
			if(t.name.Contains("BulbYellow"))
				BulbYellow = t.gameObject;
			if(t.name.Contains("Light-BulbGreen"))
				Lights[0]=t.gameObject;
			if(t.name.Contains("Light-BulbYellow"))
				Lights[1] = t.gameObject;
			if(t.name.Contains("Light-BulbRed"))
				Lights[2] = t.gameObject;
			
		}
		
	}
	
	// Update is called once per frame
	void Update () {
		if(showLinks)
		{
			Debug.DrawLine(this.transform.position,controllerPosition,linkColor);
			
		}
	}
}
