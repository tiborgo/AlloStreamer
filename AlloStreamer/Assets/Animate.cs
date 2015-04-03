using UnityEngine;
using System.Collections;

public class Animate : MonoBehaviour {
	
	private MainScript mainScript;
	private GameObject sys;
	// Use this for initialization
	GameObject[] wpNodes;
	public GameObject player;
	
	Vector3 startPosition;
	float pathLength = 100; //default path length
	bool dropWater;
	float velocity;
	
	void Awake () {
		sys = GameObject.Find("System");
		mainScript = sys.GetComponent<MainScript>();

		this.gameObject.transform.Rotate (0,250,0);
		startPosition = this.gameObject.transform.position;//default start position;
		
	}
	Vector3 forwardDirection;
	GameObject waterFall;
	bool water = false;
	// Update is called once per frame
	void Update () {
//		if (this.targetIndex > 1 && this.targetIndex < 3 && !water)
//		{
//			//Drop water
//			GameObject waterFall1 = (GameObject)Resources.Load("WaterFall");
//			waterFall = (GameObject) Instantiate(waterFall1, this.transform.position, Quaternion.identity);
//			water = true;
//		}
//		if(this.targetIndex > 4 && water)
//		{
//			Destroy(waterFall);
//			waterFall = null;
//			water = false;
//		}
//
//		if(water)
//		{
//			waterFall.transform.position = this.transform.position;
//		}



		
		Vector3 forward = transform.forward;
		//forward = new Vector3(forward.x, forward.y, forward.z);
		this.gameObject.transform.position += this.gameObject.transform.forward * Time.deltaTime*velocity;
		Debug.Log("Velocity: " + velocity);
		Debug.Log (Vector3.Distance(this.startPosition, this.gameObject.transform.position));

		if(checkPathLength () || velocity ==0)
		{
			this.gameObject.transform.position = startPosition;
		}
		
	}

	public bool checkPathLength()
	{

		if(Vector3.Distance(this.startPosition, this.gameObject.transform.position) > pathLength)
			return true;
		else
			return false;
	}

	public void setVelocity(float vel)
	{
		this.velocity = vel;
	}
	public float getVelocity()
	{
		return this.velocity;
	}

	public void setStartPosition(Vector3 pos)
	{
		this.startPosition = pos;
		//this.transform.position = startPosition;
	}
	public Vector3 getStartPosition()
	{
		return this.startPosition;
	}

	public void setPathLength(float length)
	{
		this.pathLength = length;
	}
	public float getPathLength()
	{
		return this.pathLength;
	}

	public void setDirection(Vector3 dir)
	{
		transform.localEulerAngles = dir;
	}

	public Vector3 getDirection()
	{
		return transform.localEulerAngles;
	}



}
