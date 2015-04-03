#pragma strict
var landing : boolean = false;
var slowmotion : boolean = true;
var engineOff : boolean = false;
var rotorSpeed : int = 100;
var mainRotor : Transform;
var tailRotor : Transform;
var maingearOne : Transform;
var maingearTwo : Transform;
var tailGear : Transform;
var tailWheelCollider : WheelCollider;
var maingearOnelCollider : WheelCollider;
var maingearTwolCollider : WheelCollider;
var nose : GameObject;
//Lights
var navLight : Transform;
var tailLight : Light;
var noseLight : Light;
var gearLight1 : Light;
var gearLight2 : Light;
var landingLight1 : Light;
var landingLight2 : Light;
var landingLight : Transform;
private var shader1 : Shader;
private var shader2 : Shader;
//
private var P1 : float;
private var P2 : float;
private var P3 : float;

private var P4 : float;
private var P5 : float;

	
function Start () {
	//Application.targetFrameRate = 60;
	Physics.gravity = Vector3(0, -8.0, 0);
	GafferMarineOne.helicopter = transform;
	shader1 = Shader.Find("Self-Illumin/Specular");
	shader2 = Shader.Find("Specular");
}
	

 	
 function Update () {
 		mainRotor.Rotate (0,500*Time.deltaTime, 0);
 	 	//mainRotor.Rotate(Vector3.down * (Time.deltaTime*rotorSpeed));
 	 	//mainRotor.localRotation = Quaternion.Euler(0,Time.deltaTime,0);
 		tailRotor.Rotate(Vector3.down * (Time.deltaTime*500));
 		//tailGear.localRotation = Quaternion.Euler(0, 20, 0);
 		
// 	if(slowmotion == false && landing == false) 
// 	{
// 		mainRotor.Rotate(Vector3.down * (Time.deltaTime*rotorSpeed));
// 		tailRotor.Rotate(Vector3.down * (Time.deltaTime*rotorSpeed));
// 		if(transform.position.y > -6.0) 
// 		{
// 			//P1 += Time.deltaTime;
// 			P2 = 0.0;
// 			P3 = 0.0;
// 			//maingearOne.localRotation = Quaternion.Euler(0,90, Mathf.PingPong(Time.timeSinceLevelLoad*30, 72));
// 			//maingearTwo.localRotation = Quaternion.Euler(0,90, Mathf.PingPong(Time.timeSinceLevelLoad*40, 72));
// 			maingearOne.localRotation = Quaternion.Euler(0,90,Mathf.Lerp(maingearOne.localEulerAngles.z, 72, P1/17));
// 			maingearTwo.localRotation = Quaternion.Euler(0,90,Mathf.Lerp(maingearOne.localEulerAngles.z, 72, P1/17));
// 		}
// 		tailGear.localRotation = Quaternion.Euler(0, Mathf.PingPong(P1, 180), 0);
// 	} 
// 	else if(landing == false) 
// 	{
// 		mainRotor.Rotate(Vector3.down * (Time.deltaTime*1000));
// 		tailRotor.Rotate(Vector3.down * (Time.deltaTime*100));
// 		if(transform.position.y > -6.0) 
// 		{
// 			//P2 += Time.deltaTime;
// 			P1 = 0.0;
// 			P3 = 0.0;
// 			//maingearOne.localRotation = Quaternion.Euler(0,90, Mathf.PingPong(Time.timeSinceLevelLoad*15, 72));
// 			//maingearTwo.localRotation = Quaternion.Euler(0,90, Mathf.PingPong(Time.timeSinceLevelLoad*20, 72));
// 			maingearOne.localRotation = Quaternion.Euler(0,90,Mathf.Lerp(maingearOne.localEulerAngles.z, 72, P2/17));
// 			maingearTwo.localRotation = Quaternion.Euler(0,90,Mathf.Lerp(maingearOne.localEulerAngles.z, 72, P2/17));
// 		}
// 		tailGear.localRotation = Quaternion.Euler(0, Mathf.PingPong(Time.time*20, 180), 0);
// 	} else 
// 	{
// 		P3 += Time.deltaTime;
// 		P1 = 0.0;
// 		P2 = 0.0;
// 		mainRotor.Rotate(Vector3.down * (Time.deltaTime*rotorSpeed));
// 		tailRotor.Rotate(Vector3.down * (Time.deltaTime*rotorSpeed));
// 		maingearOne.localRotation = Quaternion.Euler(0,90,Mathf.Lerp(maingearOne.localEulerAngles.z, 0, P3/17));
// 		maingearTwo.localRotation = Quaternion.Euler(0,90,Mathf.Lerp(maingearOne.localEulerAngles.z, 0, P3/17));
// 	}
 	
 	if(tailWheelCollider.isGrounded == true) {
 	//tailGear.localPosition.y = Mathf.Lerp(tailGear.localPosition.y, -0.6, Time.time/30);
 	}
 	
// 	if(engineOff == true) {
// 	P4 += Time.deltaTime/250;
// 	P5 = 0.0;
// 	audio.pitch = Mathf.Lerp(audio.pitch, 0.0, P4/10);
// 	audio.volume = Mathf.Lerp(audio.volume, 0.0, P4/10);
// 	//rotorSpeed = Mathf.Lerp(rotorSpeed, 10, P4/4);
// 	} else 
//	if(slowmotion == false) 
//	{
//	 	P5 += Time.deltaTime/250;
//	 	P4 = 0.0;
//	 	audio.pitch = Mathf.Lerp(audio.pitch, 1.0, P5*2);
//	 	audio.volume = Mathf.Lerp(audio.volume, 1.0, P5*2);
//	 	rotorSpeed = Mathf.Lerp(rotorSpeed, 2000, P5/1.5);
// 	}
 	
 	
 	//P5 += Time.deltaTime/250;
 	//rotorSpeed = Mathf.Lerp(rotorSpeed, 2000, P5/1.5);
 	
 	if(transform.position.y > 0.2 && engineOff == false && landing == false) 
 	{
	 	//transform.rigidbody.isKinematic = true;
	 	//transform.position.y = 0.19;
 	}
 	
 	if(slowmotion == true) {
 	var blinkSpeed = 4;
 	//audio.pitch = 0.2;
 	} else {
 	blinkSpeed = 1;
 	//audio.pitch = 1.0;
 	}
 	
 	//Nav Lights
 	var blink : int = 0;
 	var blink2 : int = 0;
 	blink = Mathf.PingPong((Time.time*6.5)/blinkSpeed, 2);
 	blink2 = Mathf.PingPong((Time.time*8.5)/blinkSpeed, 2);
 	tailLight.intensity = blink;
 	noseLight.intensity = blink2; 
 	
// 	var Hit1 : WheelHit;
// 	if (tailWheelCollider.GetGroundHit(Hit1)) {
// 		var P7 : float;
// 		P7 += 0.1;
// 		tailGear.localPosition.y = Mathf.Lerp(tailGear.localPosition.y, -0.6, P7);
// 		if(Hit1.force > 15) {
// 		tailWheelCollider.audio.volume = Hit1.force/50;
// 		tailWheelCollider.audio.Play();
// 		}
// 	}
// 	var Hit2 : WheelHit;
// 	if (maingearOnelCollider.GetGroundHit(Hit2)) {
// 		if(Hit2.force > 15) {
// 		maingearOnelCollider.audio.volume = Hit2.force/50;
// 		maingearOnelCollider.audio.Play();
// 		}
// 	}
// 	var Hit3 : WheelHit;
// 	if (maingearTwolCollider.GetGroundHit(Hit3)) {
// 		if(Hit3.force > 15) {
// 		maingearTwolCollider.audio.volume = Hit3.force/50;
// 		maingearTwolCollider.audio.Play();
// 		}
// 	}
 }
 
 function OnGUI () {
 
 
// 	GUILayout.BeginArea (Rect (20,20,750,100));
// 	GUILayout.BeginHorizontal ("box");
// 
// 		if(GUILayout.Button ("TOGGLE:\nSLOW MOTION")) {
// 			if(landing == false) {
// 			if(slowmotion == true) {
// 			slowmotion = false;
// 			audio.pitch = 1.0;
// 			nose.audio.Play();
// 			} else {
// 			slowmotion = true;
// 			audio.pitch = 0.2;
// 			nose.audio.Pause();
// 			}
// 			}
// 		}
// 		if(slowmotion == true) {
// 			GUI.contentColor = Color.yellow;
// 			GUILayout.Label ("SLOW MOTION\nACTIVATED");
// 		}
// 		GUI.contentColor = Color.white;
// 		if(GUILayout.Button ("TOGGLE:\nLANDING LIGHTS")) {
// 			if(landingLight1.enabled == false) {
// 			landingLight.renderer.material.shader = shader1;
// 			landingLight1.enabled = true;
// 			landingLight2.enabled = true;
// 			} else {
// 			landingLight.renderer.material.shader = shader2;
// 			landingLight1.enabled = false;
// 			landingLight2.enabled = false;
// 			}
// 		}
// 		if(GUILayout.Button ("TOGGLE:\nNAV LIGHTS")) {
// 			if(tailLight.enabled == false) {
// 			navLight.renderer.material.shader = shader1;
// 			tailLight.enabled = true;
// 			noseLight.enabled = true;
// 			gearLight1.enabled = true;
// 			gearLight2.enabled = true;
// 			} else {
// 			navLight.renderer.material.shader = shader2;
// 			tailLight.enabled = false;
// 			noseLight.enabled = false;
// 			gearLight1.enabled = false;
// 			gearLight2.enabled = false;
// 			}
// 		}
// 		if(engineOff == false && maingearTwolCollider.isGrounded == false) {
// 		if(GUILayout.Button ("TOGGLE:\nLANDING")) {
// 			if(landing == true) {
// 			landing = false;
// 			transform.rigidbody.isKinematic = true;
// 			GearSound();
// 			} else {
// 			landing = true;
// 			slowmotion = false;
// 			audio.pitch = 1.0;
// 			constantForce.force.y = 7.7;
// 			constantForce.relativeTorque.x = -0.1;
// 			transform.rigidbody.isKinematic = false;
// 			GearSound();
// 			}
// 		}
// 		}
// 		if(engineOff == false && maingearTwolCollider.isGrounded == true) {
// 			GUI.contentColor = Color.green;
// 			GUILayout.Label ("LANDING\nCOMPLETED");
// 		}
// 		if(engineOff == false) {
// 		GUI.contentColor = Color.red;
// 		if(GUILayout.Button ("ENGINE:\nOFF")) {
// 			engineOff = true;
// 			landing = true;
// 			slowmotion = false;
// 			audio.pitch = 1.0;
// 			constantForce.force.y = 5.7;
// 			transform.rigidbody.isKinematic = false;
// 		}
// 		}
// 		if(engineOff == true) {
// 			GUI.contentColor = Color.yellow;
// 			GUILayout.Label ("ENGINE IS\nSHUTTING DOWN");
// 		}
// 		if(rotorSpeed < 250 && engineOff == true) {
// 			GUI.contentColor = Color(1,0.5,0);
// 			if(GUILayout.Button ("ENGINE:\nSTART")) {
// 			engineOff = false;
// 			landing = false;
// 			slowmotion = false;
// 			SetLift(8.1, 5);
// 			constantForce.relativeTorque.x = 0.0;
// 			//transform.rigidbody.isKinematic = true;
// 			}
// 		}
// 		GUI.contentColor = Color.white;
// 		if(GUILayout.Button ("RESET DEMO")) {
// 		Application.LoadLevel(0);
// 		}
// 		
// 	GUILayout.EndHorizontal ();
// 	GUILayout.EndArea ();
 	
 }
 function SetLift(power : float, wait : float) {
 	yield WaitForSeconds(wait);
 	if(engineOff == false) {
 	GetComponent.<ConstantForce>().force.y = power;
 	}
 }
 function GearSound() {
 	maingearOne.GetComponent.<AudioSource>().Play();
 	yield WaitForSeconds(0.1);
 	maingearTwo.GetComponent.<AudioSource>().Play();
 }