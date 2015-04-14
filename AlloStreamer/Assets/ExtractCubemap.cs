using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class ExtractCubemap : MonoBehaviour {

    [DllImport("CubemapExtractionPlugin")]
    private static extern void SetCubemapFaceCountFromUnity(int face);

    [DllImport("CubemapExtractionPlugin")]
    private static extern void SetCubemapFaceTextureFromUnity(System.IntPtr texture, int face);

    private static System.String[] cameraNames = {
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

    private const int cubemapSize = 2048; // 1280;

	// Use this for initialization
    IEnumerator Start() {

        SetCubemapFaceCountFromUnity(cameraNames.Length);

        for (int i = 0; i < cameraNames.Length; i++) {

            GameObject go = GameObject.Find(cameraNames[i]);
            Camera cam = go.GetComponent<Camera>();
            RenderTexture tex = new RenderTexture(cubemapSize, cubemapSize, 1, RenderTextureFormat.ARGB32);
            tex.Create();
            cam.targetTexture = tex;
            cam.aspect = 1;

            SetCubemapFaceTextureFromUnity(tex.GetNativeTexturePtr(), i);
        }

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
