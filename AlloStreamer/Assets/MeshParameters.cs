using UnityEngine;
using System.Collections;

public class MeshParameters : MonoBehaviour {

	// Use this for initialization
	void Start () {
		Color myColor = this.gameObject.GetComponent<Renderer>().material.color;
		myColor.a = 0.5f;
		this.gameObject.GetComponent<Renderer>().material.color = myColor;
		this.gameObject.GetComponent<Renderer>().material.SetColor ("_Color", myColor);

		Color originalColour = GetComponent<Renderer>().material.color;
		GetComponent<Renderer>().material.color = new Color(originalColour.r, originalColour.g, originalColour.b, 0.5f);
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
