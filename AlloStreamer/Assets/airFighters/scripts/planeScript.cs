using UnityEngine;
using System.Collections;

public class planeScript : MonoBehaviour {

	// Use this for initialization
	public float speed;
	public float rotSpeed;
	public float upSpeed;
	public float resetRotTime=33.33f;
	void Start () {
	
		
		//rigidbody.freezeRotation=true;
		
	}
	
	// Update is called once per frame
	void Update () {
	
		
		
		transform.Translate(Vector3.forward * speed* Time.deltaTime);
		
		if( Input.GetKey(KeyCode.D) ||  Input.GetKey(KeyCode.RightArrow))
		{
			
			//transform.Rotate( Vector3.up* rotSpeed * Time.deltaTime);
			
			transform.Rotate( 0,0,-rotSpeed * Time.deltaTime);
		//	 transform.Rotate( Vector3.right * -upSpeed * Time.deltaTime);
		
		}
		
		  if( Input.GetKey(KeyCode.A)  ||  Input.GetKey(KeyCode.LeftArrow))
		{
			transform.Rotate( 0,0, rotSpeed * Time.deltaTime);
			//transform.Rotate( Vector3.up * -rotSpeed * Time.deltaTime);
			// transform.Rotate( Vector3.right * -upSpeed * Time.deltaTime);
		}
		
		
		  if( Input.GetKey(KeyCode.W)  ||  Input.GetKey(KeyCode.UpArrow) )
		{
			
			transform.Rotate( Vector3.right* upSpeed * Time.deltaTime);
		}
		
		  if( Input.GetKey(KeyCode.S)  ||  Input.GetKey(KeyCode.DownArrow) )
		{
			
			transform.Rotate( Vector3.right * -upSpeed * Time.deltaTime);
		}
		
	   
		
		
	}
}
