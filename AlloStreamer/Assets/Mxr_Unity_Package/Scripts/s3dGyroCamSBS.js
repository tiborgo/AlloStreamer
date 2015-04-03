/* s3dGyroCamBS.js - revised 3/11/13
 * Please direct any bugs/comments/suggestions to hoberman@usc.edu.
 * FOV2GO for Unity Copyright (c) 2011-13 Perry Hoberman & MxR Lab. All rights reserved.
 * Gyroscope-controlled camera for iPhone & Android revised 5.12.12
 * Usage: Attach this script to main camera.
 * Note: Unity Remote does not currently support gyroscope. 
 * This script uses three techniques to get the correct orientation out of the gyroscope attitude:
   1. creates a parent transform (camParent) and rotates it with eulerAngles
   2. for Android + Unity 3.5 only: remaps gyro.Attitude quaternion values from xyzw to wxyz (quatMap)
   3. multiplies attitude quaternion by quaternion quatMult
 * Also creates a grandparent (camGrandparent) which can be rotated to change heading
   This allows an arbitrary heading to be added to the gyroscope reading
   so that the virtual camera can be facing any direction in the scene, no matter which way the phone is actually facing
 * Option for direct touch input - horizontal swipe controls heading
 * Has companion Editor script (s3dGyroCamSBSEditor.js) for custom inspector 
 */

#pragma strict

static var gyroBool : boolean = false;
private var gyro : Gyroscope;
private var compass : Compass;
private var quatMult : Quaternion;
private var quatMap : Quaternion;
private var prevScreenOrientation: ScreenOrientation;

// camera grandparent node to rotate heading
private var camParent: GameObject;
private var camGrandparent : GameObject;
public var heading : float = 0;
public var Pitch : float = 0;
public var setZeroToNorth : boolean = true;
public var checkForAutoRotation : boolean = false;

// mouse/touch input
public var touchRotatesHeading : boolean;
private var headingAtTouchStart : float;
private var pitchAtTouchStart : float;
private var mouseStartPoint: Vector2;

private var screenSize : Vector2;
@script AddComponentMenu ("Stereoskopix/s3d Gyro Cam")

function Awake() {
	// find the current parent of the camera's transform
	var currentParent = transform.parent;
	// instantiate a new transform
	camParent = new GameObject ("camParent");
	// match the transform to the camera position
	camParent.transform.position = transform.position;
	// make the new transform the parent of the camera transform
	transform.parent = camParent.transform;
	// instantiate a new transform
	camGrandparent = new GameObject ("camGrandParent");
	// match the transform to the camera position
	camGrandparent.transform.position = transform.position;
	// make the new transform the grandparent of the camera transform
	camParent.transform.parent = camGrandparent.transform;
	// make the original parent the great grandparent of the camera transform
	camGrandparent.transform.parent = currentParent;
		
	// check whether device supports gyroscope
	#if UNITY_3_4
		gyroBool = Input.isGyroAvailable;
	#endif
	#if !UNITY_3_4
		gyroBool = SystemInfo.supportsGyroscope;
	#endif
	if (gyroBool) {
		prevScreenOrientation = Screen.orientation;
		gyro = Input.gyro;
		gyro.enabled = true;
		
		if (setZeroToNorth) {
			compass = Input.compass;
			compass.enabled = true;
		}
	
		fixScreenOrientation();
		
	}
	Screen.sleepTimeout = SleepTimeout.NeverSleep;
}

function Start() {
	if (gyroBool) {
		if (setZeroToNorth) {
			turnToFaceNorth();
		}
	}
	screenSize.x = Screen.width;
	screenSize.y = Screen.height;
	#if (UNITY_IPHONE || UNITY_ANDROID) && ! UNITY_EDITOR
		if (checkForAutoRotation) {
			InvokeRepeating("checkAutoRotation",1.0,1.0);
		}
	#endif
}

function turnToFaceNorth() {
	yield WaitForSeconds(1);
	heading = Input.compass.magneticHeading;
}	


function Update () {
	if (gyroBool) {
		#if UNITY_IPHONE || (UNITY_ANDROID && UNITY_4_0)
			quatMap = gyro.attitude;
		#endif
		#if UNITY_ANDROID && UNITY_3_5
			quatMap = Quaternion(gyro.attitude.w,gyro.attitude.x,gyro.attitude.y,gyro.attitude.z);
		#endif
		transform.localRotation = quatMap * quatMult;
	}
	
	if (touchRotatesHeading) {
		GetTouchMouseInput();
	}
	camGrandparent.transform.localEulerAngles.y = heading;
	// only update pitch if in Unity Editor (on device, pitch is handled by gyroscope)
	#if UNITY_EDITOR
		transform.localEulerAngles.x = Pitch;
	#endif
}

function checkAutoRotation() {
	// check if Screen.orientation has changed
	if (prevScreenOrientation != Screen.orientation) {
		// fix gyroscope orientation settings
		fixScreenOrientation();
		// also need to fix camera aspect
	}
	prevScreenOrientation = Screen.orientation;
}

function fixScreenOrientation() {	
	#if UNITY_IPHONE && UNITY_4_0
		camParent.transform.eulerAngles = Vector3(90,90,0);
        quatMult = Quaternion(0,0,1,0);
	#endif
	#if UNITY_IPHONE && UNITY_3_5
		camParent.transform.eulerAngles = Vector3(90,90,0);
		if (Screen.orientation == ScreenOrientation.LandscapeLeft) {
			quatMult = Quaternion(0,0,0.7071,0.7071);
 		} else if (Screen.orientation == ScreenOrientation.LandscapeRight) {
			quatMult = Quaternion(0,0,-0.7071,0.7071);
     	} else if (Screen.orientation == ScreenOrientation.Portrait) {
           	quatMult = Quaternion(0,0,1,0);
		} else if (Screen.orientation == ScreenOrientation.PortraitUpsideDown) {
           	quatMult = Quaternion(0,0,0,1);
		}
	#endif
	#if UNITY_ANDROID && UNITY_4_0
		camParent.transform.eulerAngles = Vector3(90,0,0);
 		quatMult = Quaternion(0,0,1,0);
	#endif
	#if UNITY_ANDROID && UNITY_3_5
		camParent.transform.eulerAngles = Vector3(-90,0,0);
		if (Screen.orientation == ScreenOrientation.LandscapeLeft) {
			quatMult = Quaternion(0,0,0.7071,-0.7071);
		} else if (Screen.orientation == ScreenOrientation.LandscapeRight) {
 			quatMult = Quaternion(0,0,-0.7071,-0.7071);
       	} else if (Screen.orientation == ScreenOrientation.Portrait) {
           	quatMult = Quaternion(0,0,0,1);
		} else if (Screen.orientation == ScreenOrientation.PortraitUpsideDown) {
           	quatMult = Quaternion(0,0,1,0);
		}
	#endif
}

function GetTouchMouseInput() {
    if(Input.GetMouseButtonDown(0)) {
        mouseStartPoint = Input.mousePosition;
        headingAtTouchStart = heading;
		#if UNITY_EDITOR
        	pitchAtTouchStart = Pitch;
		#endif
    } else if (Input.GetMouseButton(0)) {
		var delta : Vector2;
    	var mousePos = Input.mousePosition;
    	delta.x = (mousePos.x - mouseStartPoint.x)/screenSize.x;
		heading = (headingAtTouchStart+delta.x*100);
		heading = heading%360;
		#if UNITY_EDITOR
	    	delta.y = (mousePos.y - mouseStartPoint.y)/screenSize.y;
			Pitch = (pitchAtTouchStart+delta.y*-100);
			Pitch = Mathf.Clamp(Pitch%360, -60, 60);
		#endif
	}
}

// end s3dGyroCamSBS.js
