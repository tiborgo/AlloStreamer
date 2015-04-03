/* s3dCameraSBSEditor.js - revised 2/12/13
 * Please direct any bugs/comments/suggestions to hoberman@usc.edu.
 * FOV2GO for Unity Copyright (c) 2011-13 Perry Hoberman & MxR Lab. All rights reserved.
 * Usage: Put this script in the Editor folder. It provides a custom inspector for s3dCameraSBS.js
 */

@CustomEditor (s3dCameraSBS)
class s3dCameraSBSEditor extends Editor {
    function OnInspectorGUI () {
	    EditorGUIUtility.LookLikeControls(110,30);
		var allowSceneObjects : boolean = !EditorUtility.IsPersistent (target);
      	EditorGUILayout.BeginVertical("box");
        target.interaxial = EditorGUILayout.IntSlider (GUIContent("Interaxial (mm)","Distance (in millimeters) between cameras."),target.interaxial, 0, 1000);
        target.zeroPrlxDist = EditorGUILayout.Slider (GUIContent("Zero Prlx Dist (M)","Distance (in meters) at which left and right images converge."),target.zeroPrlxDist, 0.1, 100);
 		target.cameraSelect = EditorGUILayout.EnumPopup(GUIContent("Camera Order","Swap cameras for cross-eyed free-viewing."), target.cameraSelect);
		target.H_I_T = EditorGUILayout.Slider (GUIContent("H I T","Horizontal Image Transform (default 0)"),target.H_I_T, -25, 25);
		target.sideBySideSqueezed = EditorGUILayout.Toggle(GUIContent("Squeezed","For 3DTV frame-compatible format"), target.sideBySideSqueezed);
		target.usePhoneMask = EditorGUILayout.Toggle(GUIContent("Use Phone Mask","Mask for side-by-side mobile phone formats"), target.usePhoneMask);
		if (target.usePhoneMask) {
			EditorGUI.indentLevel = 1;
			target.leftViewRect = EditorGUILayout.Vector4Field("Left View Rect (left bottom width height)", target.leftViewRect);
			target.rightViewRect = EditorGUILayout.Vector4Field("Right View Rect (left bottom width height)", target.rightViewRect);
			EditorGUI.indentLevel = 0;
		}
 		EditorGUILayout.EndVertical();
       if (GUI.changed) {
             EditorUtility.SetDirty (target);
    	}
	}
}

// end s3dCameraSBSEditor.js
