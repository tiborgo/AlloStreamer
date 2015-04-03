using UnityEngine;
using System.Collections;

public class CameraParams : MonoBehaviour {

	// Use this for initialization
	void Start () {
		this.GetComponent<Camera>().aspect = (1785.0f/1200.0f);

	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
