using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Reflection;



public class RenderAndExtractCubemap : MonoBehaviour {

    [DllImport("CubemapExtractionPlugin")]
    private static extern void SetCubemapFaceCountFromUnity(int face);
    [DllImport("CubemapExtractionPlugin")]
    private static extern void SetCubemapFaceTextureFromUnity(System.IntPtr texture, int face);
    [DllImport("CubemapExtractionPlugin")]
    private static extern void StartFromUnity();
    [DllImport("CubemapExtractionPlugin")]
    private static extern void StopFromUnity();

    private static System.String[] cubemapFaceNames = {
        "LeftEye/PositiveX",
        "LeftEye/NegativeX",
        "LeftEye/PositiveY",
        "LeftEye/NegativeY",
        "LeftEye/PositiveZ",
        "LeftEye/NegativeZ"/*,
        "RightEye/PositiveX",
        "RightEye/NegativeX",
        "RightEye/PositiveY",
        "RightEye/NegativeY",
        "RightEye/PositiveZ",
        "RightEye/NegativeZ"*/
    };

    private static Vector3[] cubemapFaceRotations = {
        new Vector3(  0,  90, 0),
        new Vector3(  0, 270, 0),
        new Vector3(270,   0, 0),
        new Vector3( 90,   0, 0),
        new Vector3(  0,   0, 0),
        new Vector3(  0, 180, 0),
    };

    

    private const int cubemapSize = 2048; // 1280;

	// Use this for initialization
    IEnumerator Start() {

        // Set up 6 cameras for cubemap
        Camera thisCam = GetComponent<Camera>();
        GameObject cubemap = GameObject.CreatePrimitive(PrimitiveType.Cube); //new GameObject("Cubemap");
        //cubemap.transform.SetParent(transform, false);
        cubemap.transform.parent = transform;
        
        // This has no effect for some reason
        //cubemap.transform.localEulerAngles = new Vector3(0, 0, 0);
        //cubemap.transform.localPosition = Vector3.zero;
        //cubemap.transform.Translate(new Vector3(50, 50, 50));

        SetCubemapFaceCountFromUnity(cubemapFaceNames.Length);

        for (int i = 0; i < cubemapFaceNames.Length; i++)
        {
            GameObject go = new GameObject(cubemapFaceNames[i]);
            Camera cam = go.AddComponent<Camera>()/*.GetCopyOf(thisCam)*/;

            //GameObject go = GameObject.Find(cameraNames[i]);
            //Camera cam = go.GetComponent<Camera>();

            // Set render texture
            RenderTexture tex = new RenderTexture(cubemapSize, cubemapSize, 1, RenderTextureFormat.ARGB32);
            tex.Create();
            cam.targetTexture = tex;
            cam.aspect = 1;

            // Set orientation
            cam.fieldOfView = 90;
            go.transform.eulerAngles = cubemapFaceRotations[i];
            go.transform.parent = cubemap.transform;

            SetCubemapFaceTextureFromUnity(tex.GetNativeTexturePtr(), i);
        }

        // Tell native plugin that rendering has started
        StartFromUnity();

        yield return StartCoroutine("CallPluginAtEndOfFrames");
	}

    void OnDestroy()
    {
        StopFromUnity();
    }


    private IEnumerator CallPluginAtEndOfFrames()
    {
        while (true)
        {
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(1);
        }
    }
}

public static class MyExtensions
{
    // From http://answers.unity3d.com/questions/530178/how-to-get-a-component-from-an-object-and-add-it-t.html
    public static T GetCopyOf<T>(this Component comp, T other) where T : Component
    {
        Type type = comp.GetType();
        if (type != other.GetType()) return null; // type mis-match
        BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Default | BindingFlags.DeclaredOnly;
        PropertyInfo[] pinfos = type.GetProperties(flags);
        foreach (var pinfo in pinfos)
        {
            if (pinfo.CanWrite)
            {
                try
                {
                    pinfo.SetValue(comp, pinfo.GetValue(other, null), null);
                }
                catch { } // In case of NotImplementedException being thrown. For some reason specifying that exception didn't seem to catch it, so I didn't catch anything specific.
            }
        }
        FieldInfo[] finfos = type.GetFields(flags);
        foreach (var finfo in finfos)
        {
            finfo.SetValue(comp, finfo.GetValue(other));
        }
        return comp as T;
    }
}