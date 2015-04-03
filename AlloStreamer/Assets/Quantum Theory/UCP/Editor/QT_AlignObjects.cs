using UnityEngine;
using System.Collections;
using UnityEditor;

[CustomEditor(typeof(MeshFilter))]
public class QT_AlignObjects : Editor
{
    //private Transform t;
	
    ////restore the look of the standard inspector transform rollout
   //public override void OnInspectorGUI()
   // {

    //    if(Selection.gameObjects.Length>0)
    //    {
    //    t = Selection.gameObjects[0].transform;
    //    if(!t.parent)
    //    {
    //    //worldspace
    //    t.position = EditorGUILayout.Vector3Field("Position",t.position);
    //    t.eulerAngles = EditorGUILayout.Vector3Field("Rotation",t.eulerAngles);
    //    t.localScale = EditorGUILayout.Vector3Field("Scale",t.localScale);
    //    }
    //    else
    //    {
    //    t.localPosition = EditorGUILayout.Vector3Field("Position",t.localPosition);
    //    t.localEulerAngles = EditorGUILayout.Vector3Field("Rotation",t.localEulerAngles);
    //    t.localScale= EditorGUILayout.Vector3Field("Scale",t.localScale);
    //    }
    //    }
		
    //}
    
	void OnSceneGUI()
	{

        Event e = Event.current;

        if (e.type == EventType.keyDown)
        {
            if (e.shift && e.keyCode == KeyCode.R)
            {
                Ray ray = HandleUtility.GUIPointToWorldRay(Event.current.mousePosition);
                RaycastHit hit;
                if (Physics.Raycast(ray, out hit))
                {
                    Undo.RegisterSceneUndo("Moved GameObjects.");
                    Selection.gameObjects[0].transform.position = hit.transform.gameObject.transform.position;
                    Selection.gameObjects[0].transform.rotation = hit.transform.gameObject.transform.rotation;
                    Debug.Log("Aligned " + Selection.gameObjects[0].name + " to " + hit.transform.gameObject.name);
                }
                else
                    Debug.LogError("Ray cast didn't hit. Make sure objects you want to align to has a collider.");
            }
            if (e.shift && e.keyCode == KeyCode.T)
            {
                Ray ray = HandleUtility.GUIPointToWorldRay(Event.current.mousePosition);
                RaycastHit hit;
                if (Physics.Raycast(ray, out hit))
                {
                    Undo.RegisterSceneUndo("Moved GameObjects.");
                    Selection.gameObjects[0].transform.position = hit.point;
                    Debug.Log("Teleported " + Selection.gameObjects[0].name + " to " + hit.point);
                }
                else
                    Debug.LogError("Ray cast didn't hit. Check to see if the surface you're teleporting to has a collider.");
            }

        }
               
	}
	
	
}
