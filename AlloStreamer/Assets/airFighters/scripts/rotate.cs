using UnityEngine;
using System.Collections;

public class rotate : MonoBehaviour {

	// Use this for initialization
	
	public Vector3 rotate2;
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
		transform.Rotate(rotate2*Time.deltaTime);
		
		
	}
}
