/* s3dGyroCamSBSEditor.js - revised 3/11/13
 * Please direct any bugs/comments/suggestions to hoberman@usc.edu.
 * FOV2GO for Unity Copyright (c) 2011-13 Perry Hoberman & MxR Lab. All rights reserved.
 * Usage: Put this script in the Editor folder. It provides a custom inspector for s3dGyroCamSBS.js
 */

@CustomEditor (s3dGyroCamSBS)
class s3dGyroCamSBSEditor extends Editor {
    function OnInspectorGUI () {
	    EditorGUILayout.BeginVertical("box");
	   		target.touchRotatesHeading = EditorGUILayout.Toggle(GUIContent("Touch Rotates Heading","Horizontal Swipe Rotates Heading"), target.touchRotatesHeading);
	   		target.setZeroToNorth = EditorGUILayout.Toggle(GUIContent("Set Zero Heading to North","Face Compass North on Startup"), target.setZeroToNorth);
	   		target.checkForAutoRotation = EditorGUILayout.Toggle(GUIContent("Check For Auto Rotation","Leave off unless Auto Rotation is on"), target.checkForAutoRotation);
	    EditorGUILayout.EndVertical();
        if (GUI.changed) {
            EditorUtility.SetDirty (target);
		}
    }
}		

// end s3dGyroCamSBSEditor.js