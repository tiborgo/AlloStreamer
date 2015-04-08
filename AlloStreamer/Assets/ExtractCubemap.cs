using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class ExtractCubemap : MonoBehaviour {

    [DllImport("CubemapRenderingPlugin")]
    private static extern void SetCubemapFaceTextureFromUnity(System.IntPtr texture, int face);

    private Camera camPositiveX;
    private Camera camNegativeX;
    private Camera camPositiveY;
    private Camera camNegativeY;
    private Camera camPositiveZ;
    private Camera camNegativeZ;

    private RenderTexture texPositiveX;
    private RenderTexture texNegativeX;
    private RenderTexture texPositiveY;
    private RenderTexture texNegativeY;
    private RenderTexture texPositiveZ;
    private RenderTexture texNegativeZ;

    private System.IntPtr texPtrPositiveX;
    private System.IntPtr texPtrNegativeX;
    private System.IntPtr texPtrPositiveY;
    private System.IntPtr texPtrNegativeY;
    private System.IntPtr texPtrPositiveZ;
    private System.IntPtr texPtrNegativeZ;

    private const int cubemapSize = 2048;

	// Use this for initialization
    IEnumerator Start() {

        camPositiveX = GameObject.Find("PositiveX").GetComponent<Camera>();
        camNegativeX = GameObject.Find("NegativeX").GetComponent<Camera>();
        camPositiveY = GameObject.Find("PositiveY").GetComponent<Camera>();
        camNegativeY = GameObject.Find("NegativeY").GetComponent<Camera>();
        camPositiveZ = GameObject.Find("PositiveZ").GetComponent<Camera>();
        camNegativeZ = GameObject.Find("NegativeZ").GetComponent<Camera>();

        texPositiveX = new RenderTexture(cubemapSize, cubemapSize, 1, RenderTextureFormat.ARGB32);
        texNegativeX = new RenderTexture(cubemapSize, cubemapSize, 1, RenderTextureFormat.ARGB32);
        texPositiveY = new RenderTexture(cubemapSize, cubemapSize, 1, RenderTextureFormat.ARGB32);
        texNegativeY = new RenderTexture(cubemapSize, cubemapSize, 1, RenderTextureFormat.ARGB32);
        texPositiveZ = new RenderTexture(cubemapSize, cubemapSize, 1, RenderTextureFormat.ARGB32);
        texNegativeZ = new RenderTexture(cubemapSize, cubemapSize, 1, RenderTextureFormat.ARGB32);
        texPositiveX.Create();
        texNegativeX.Create();
        texPositiveY.Create();
        texNegativeY.Create();
        texPositiveZ.Create();
        texNegativeZ.Create();

        camPositiveX.targetTexture = texPositiveX;
        camNegativeX.targetTexture = texNegativeX;
        camPositiveY.targetTexture = texPositiveY;
        camNegativeY.targetTexture = texNegativeY;
        camPositiveZ.targetTexture = texPositiveZ;
        camNegativeZ.targetTexture = texNegativeZ;

        texPtrPositiveX = texPositiveX.GetNativeTexturePtr();
        texPtrNegativeX = texNegativeX.GetNativeTexturePtr();
        texPtrPositiveY = texPositiveY.GetNativeTexturePtr();
        texPtrNegativeY = texNegativeY.GetNativeTexturePtr();
        texPtrPositiveZ = texPositiveZ.GetNativeTexturePtr();
        texPtrNegativeZ = texNegativeZ.GetNativeTexturePtr();

        SetCubemapFaceTextureFromUnity(texPtrPositiveX, 0);
        SetCubemapFaceTextureFromUnity(texPtrNegativeX, 1);
        SetCubemapFaceTextureFromUnity(texPtrPositiveY, 2);
        SetCubemapFaceTextureFromUnity(texPtrNegativeY, 3);
        SetCubemapFaceTextureFromUnity(texPtrPositiveZ, 4);
        SetCubemapFaceTextureFromUnity(texPtrNegativeZ, 5);

        yield return StartCoroutine("CallPluginAtEndOfFrames");
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
