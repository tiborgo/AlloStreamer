using UnityEngine;
using System.Collections;

public class BoundingPlane : MonoBehaviour {
	

	void OnTriggerEnter(Collider other)
	{
		if(other.gameObject.tag=="biplane" || other.gameObject.tag=="biplanenotail" || other.gameObject.tag=="plane" || other.gameObject.tag=="planenotail"
			|| other.gameObject.tag=="ghostplane" || other.gameObject.tag=="ghostplanenotail" || other.gameObject.tag=="privateplane" || other.gameObject.tag=="privateplanenotail")
		{
			
			//other.gameObject.transform.position += other.gameObject.transform.forward * -150; // trial and error value to bring to other side of screen
			other.gameObject.transform.position = new Vector3(0,other.gameObject.transform.position.y,0);

		}
		
		
	}
}
