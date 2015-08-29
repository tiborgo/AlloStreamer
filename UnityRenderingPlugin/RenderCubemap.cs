using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Reflection;



public class RenderCubemap : MonoBehaviour
{

    // Parameters
    public int resolution = 1024;
    public int faceCount = 6;
    public bool extract = true;
    public int fps = -1;
    public ComputeShader shader;

    [DllImport("CubemapExtractionPlugin")]
    private static extern void ConfigureCubemapFromUnity(System.IntPtr[] texturePtrs, int cubemapFacesCount, int resolution);
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

    private RenderTexture[] inTextures;
    private RenderTexture[] outTextures;

    private uint[] pixels;
    private uint[] condensedPixels;
    private ComputeBuffer buffer;
    private ComputeBuffer condensedPixelsBuffer;
    private RenderTexture yuv444Tex;

    // Use this for initialization
    IEnumerator Start()
    {
        pixels = new uint[resolution * resolution * 3];
        condensedPixels = new uint[resolution * resolution];
        buffer = new ComputeBuffer(pixels.Length, sizeof(uint));
        condensedPixelsBuffer = new ComputeBuffer(condensedPixels.Length, sizeof(uint));

        yuv444Tex = new RenderTexture(resolution, resolution, 0, RenderTextureFormat.ARGB32);
        yuv444Tex.enableRandomWrite = true;
        yuv444Tex.Create();

        if (fps != -1)
        {
            QualitySettings.vSyncCount = 0;  // VSync must be disabled
            Application.targetFrameRate = fps;
        }


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
            RenderTexture inTex = new RenderTexture(resolution, resolution, 1, RenderTextureFormat.ARGB32);
            inTex.Create();
            inTextures[i] = inTex;

            cam.targetTexture = inTex;
            cam.aspect = 1;

            // Set orientation
            cam.fieldOfView = 90;
            go.transform.eulerAngles = cubemapFaceRotations[i];
            go.transform.parent = cubemap.transform;

            // Move the cubemap to the origin of the parent cam
            cam.transform.localPosition = Vector3.zero;

            RenderTexture outTex = new RenderTexture(resolution, resolution, 0, RenderTextureFormat.ARGB32);
            outTex.enableRandomWrite = true;
            outTex.Create();
            outTextures[i] = outTex;

            texturePtrs[i] = outTex.GetNativeTexturePtr();
        }

        // Tell native plugin that rendering has started
        ConfigureCubemapFromUnity(texturePtrs, faceCount, resolution);

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

            for (int i = 0; i < faceCount; i++)
            {
                RenderTexture inTex = inTextures[i];
                RenderTexture outTex = outTextures[i];

                

                shader.SetInt("Pitch", resolution);

                
                shader.SetTexture(shader.FindKernel("CSMain"), "In", inTex);
                shader.SetTexture(shader.FindKernel("CSMain"), "OutYUV444", yuv444Tex);
                
                //shader.SetTexture(shader.FindKernel("CSMain"), "Out", outTex);
                shader.Dispatch(shader.FindKernel("CSMain"), resolution / 8, resolution / 8, 1);

                //buffer.GetData(pixels);
                //buffer.SetData(pixels);

                shader.SetInt("Channel", 0);

                shader.SetTexture(shader.FindKernel("Condense"), "Out", outTex);
                shader.SetTexture(shader.FindKernel("Condense"), "InYUV444", yuv444Tex);
                //shader.SetBuffer(shader.FindKernel("Condense"), "Out2", buffer);
                //shader.SetBuffer(shader.FindKernel("Condense"), "Out2", buffer);
                //shader.SetBuffer(shader.FindKernel("Condense"), "CondensedPixels", condensedPixelsBuffer);
                shader.Dispatch(shader.FindKernel("Condense"), resolution * resolution / 4, 1, 1);

                shader.SetInt("Channel", 1);

                shader.SetTexture(shader.FindKernel("Condense"), "Out", outTex);
                shader.SetTexture(shader.FindKernel("Condense"), "InYUV444", yuv444Tex);
                //shader.SetBuffer(shader.FindKernel("Condense"), "Out2", buffer);
                //shader.SetBuffer(shader.FindKernel("Condense"), "Out2", buffer);
                //shader.SetBuffer(shader.FindKernel("Condense"), "CondensedPixels", condensedPixelsBuffer);
                shader.Dispatch(shader.FindKernel("Condense"), resolution * resolution / 4, 1, 1);

                shader.SetInt("Channel", 2);

                shader.SetTexture(shader.FindKernel("Condense"), "Out", outTex);
                shader.SetTexture(shader.FindKernel("Condense"), "InYUV444", yuv444Tex);
                //shader.SetBuffer(shader.FindKernel("Condense"), "Out2", buffer);
                //shader.SetBuffer(shader.FindKernel("Condense"), "Out2", buffer);
                //shader.SetBuffer(shader.FindKernel("Condense"), "CondensedPixels", condensedPixelsBuffer);
                shader.Dispatch(shader.FindKernel("Condense"), resolution * resolution / 4, 1, 1);

                //buffer.GetData(pixels);
                //Debug.Log(pixels[0]);
            }

            if (extract)
            {
                GL.IssuePluginEvent(1);
            }
        }
    }
}