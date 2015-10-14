using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Reflection;


public class RenderCubemap : MonoBehaviour
{
    
    // Parameters
    public int width  = 1024;
    public int height = 1024;
    public int faceCount = 6;
    public bool extract = true;
    public int fps = -1;
    
    [DllImport("CubemapExtractionPlugin")]
    private static extern void ConfigureCubemapFromUnity(System.IntPtr[] texturePtrs, int cubemapFacesCount, int width, int height);
    [DllImport("CubemapExtractionPlugin")]
    private static extern void StopFromUnity();
    
    private static System.String[] cubemapFaceNames = {
        "LeftEye/PositiveX",
        "LeftEye/NegativeX",
        "LeftEye/PositiveY",
        "LeftEye/NegativeY",
        "LeftEye/PositiveZ",
        "LeftEye/NegativeZ",
        "RightEye/PositiveX",
        "RightEye/NegativeX",
        "RightEye/PositiveY",
        "RightEye/NegativeY",
        "RightEye/PositiveZ",
        "RightEye/NegativeZ"
    };
    
    private static Vector3[] cubemapFaceRotations = {
        new Vector3(  0,  90, 0),
        new Vector3(  0, 270, 0),
        new Vector3(270,   0, 0),
        new Vector3( 90,   0, 0),
        new Vector3(  0,   0, 0),
        new Vector3(  0, 180, 0),
        new Vector3(  0,  90, 0),
        new Vector3(  0, 270, 0),
        new Vector3(270,   0, 0),
        new Vector3( 90,   0, 0),
        new Vector3(  0,   0, 0),
        new Vector3(  0, 180, 0)
    };
    
    private ComputeShader shader;
    private RenderTexture[] inTextures;
    private RenderTexture[] outTextures;

    private bool didPlay = false;
    
    // Use this for initialization
    IEnumerator Start()
    {
        if (fps != -1)
        {
            QualitySettings.vSyncCount = 0;  // VSync must be disabled
            Application.targetFrameRate = fps;
        }
        
        // Setup convert shader
        shader = Resources.Load("ConvertRGBtoYUV420p") as ComputeShader;
        
        
        // Setup the cameras for cubemap
        Camera thisCam = GetComponent<Camera>();
        GameObject cubemap = new GameObject("Cubemap");
        cubemap.transform.parent = transform;
        
        // Move the cubemap to the origin of the parent cam
        cubemap.transform.localPosition = Vector3.zero;
        
        System.IntPtr[] texturePtrs = new System.IntPtr[faceCount];
        inTextures = new RenderTexture[faceCount];
        outTextures = new RenderTexture[faceCount];
        
        for (int i = 0; i < faceCount; i++)
        {
            GameObject go = new GameObject(cubemapFaceNames[i]);
            Camera cam = go.AddComponent<Camera>();
            
            // Set render texture
            RenderTexture inTex = new RenderTexture(width, height, 1, RenderTextureFormat.ARGB32);
            inTex.Create();
            inTextures[i] = inTex;
            
            cam.targetTexture = inTex;
            cam.aspect = (float)width / height;
            
            // Set orientation
            cam.fieldOfView = 90;
            go.transform.eulerAngles = cubemapFaceRotations[i];
            go.transform.parent = cubemap.transform;
            
            // Move the cubemap to the origin of the parent cam
            cam.transform.localPosition = Vector3.zero;
            
            
            RenderTexture outTex;
            
            if (shader)
            {
                outTex = new RenderTexture(width, height, 1, RenderTextureFormat.ARGB32);
                outTex.enableRandomWrite = true;
                outTex.Create();
            }
            else
            {
                outTex = inTex;
            }
            
            outTextures[i] = outTex;
            texturePtrs[i] = outTex.GetNativeTexturePtr();
        }
        
        // Tell native plugin that rendering has started
        ConfigureCubemapFromUnity(texturePtrs, faceCount, width, height);
        didPlay = true;
        
        yield return StartCoroutine("CallPluginAtEndOfFrames");
    }
    
    void OnDestroy()
    {
        if (didPlay)
        {
            StopFromUnity();
            didPlay = false;
        }
    }
    
    private IEnumerator CallPluginAtEndOfFrames()
    {
        while (true)
        {
            yield return new WaitForEndOfFrame();
            
            if (extract)
            {
                if (shader)
                {
                    for (int i = 0; i < faceCount; i++)
                    {
                        RenderTexture inTex = inTextures[i];
                        RenderTexture outTex = outTextures[i];
                        
                        shader.SetInt("Width", width);
                        shader.SetInt("Height", height);
                        shader.SetTexture(shader.FindKernel("Convert"), "In", inTex);
                        shader.SetTexture(shader.FindKernel("Convert"), "Out", outTex);
                        shader.Dispatch(shader.FindKernel("Convert"), (width/8) * (height/2), 1, 1);
                    }
                }
                
                GL.IssuePluginEvent(1);
            }
        }
    }
}