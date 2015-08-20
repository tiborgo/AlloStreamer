using UnityEngine;
using UnityEngine.Rendering;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Reflection;


//[ExecuteInEditMode]
public class RenderStereoCubemap : MonoBehaviour
{
    //public Cubemap left;
    //public Cubemap right;
    public GameObject[] CameraL;	//for left eye surround scene
    public GameObject[] CameraR;	//for right eye surround scene
    public GameObject[] CameraC;	//for top and bottom view (no stereo)

    public float eyeSep = 0.064f, near = 0.1f, far = 10000f, focal_length = 10f, aperture = 90f;

    public int resolution =1920;

    public float moveSpeed = 1;

    public bool printFPS = false, adjustFOV = false;


    public int faceCount = 6;
    public bool extract = true;


    public float fovAdjustValue=0.5f;
    private float preFOVAdjustValue = 0.5f;

    private GameObject[] planes;
    //Plane1L,Plane2L,Plane3L,Plane4L,Plane1R,Plane2R,Plane3R,Plane4R,Plane5,Plane6;

    private Vector3 prePos;
    private Transform OVRCamRig, OVRPlayer;

    private Texture2D[] screenShot;
    private RenderTexture[] renderTextures;


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
		new Vector3(90, -90, 0),
		new Vector3(90, 90, 0),
		new Vector3(0, 0, 180),
		new Vector3(0, 180, 0),
		new Vector3(90, 180, 0),
		new Vector3(90, 0, 0),
        new Vector3(90, -90, 0),
		new Vector3(90, 90, 0),
		new Vector3(0, 0, 180),
		new Vector3(0, 180, 0),
		new Vector3(90, 180, 0),
		new Vector3(90, 0, 0),
	};

    private static Vector3[] cubemapCamRotations = {
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

    void PlaneInit()
    {
        Vector3[] cubemapFacePositions = {
            new Vector3(focal_length, 0, 0),
            new Vector3(-focal_length, 0, 0),
            new Vector3(0, focal_length, 0),
            new Vector3(0, -focal_length, 0),
            new Vector3(0, 0, focal_length),
            new Vector3(0, 0, -focal_length),
            new Vector3(focal_length, 0, 0),
            new Vector3(-focal_length, 0, 0),
            new Vector3(0, focal_length, 0),
            new Vector3(0, -focal_length, 0),
            new Vector3(0, 0, focal_length),
            new Vector3(0, 0, -focal_length)                                                        
        };
        screenShot = new Texture2D[12];
        for (int i = 0; i < screenShot.Length; i++)
        {
            screenShot[i] = new Texture2D(resolution, resolution, TextureFormat.RGB24, false);
        }
        renderTextures = new RenderTexture[12];
        for (int i = 0; i < renderTextures.Length; i++)
        {
            renderTextures[i] = new RenderTexture(resolution, resolution, 24);
            renderTextures[i].Create();
        }

        planes = new GameObject[12];
        for (int i = 0; i < planes.Length; i++)
        {
            planes[i] = GameObject.CreatePrimitive(PrimitiveType.Plane);
            planes[i].transform.parent = transform;
            planes[i].transform.localScale = new Vector3(focal_length / 5, 1, focal_length / 5);

            Renderer r = planes[i].GetComponent<Renderer>();
            r.shadowCastingMode = ShadowCastingMode.Off;
            r.receiveShadows = false;
            r.useLightProbes = false;
            r.reflectionProbeUsage = ReflectionProbeUsage.Off;
            r.material.shader = Shader.Find("Unlit/Texture");
            r.material.mainTexture = renderTextures[i];

            if(i<6)
                planes[i].layer = 14;
            else
                planes[i].layer = 15;

            planes[i].name = cubemapFaceNames[i];
            planes[i].transform.localPosition = cubemapFacePositions[i];
            planes[i].transform.eulerAngles = cubemapFaceRotations[i];
            /*string name;
            if (i < 4)
            {
                name = "L";
                planes[i].layer = 14;
            }
            else if (i < 8)
            {
                name = "R";
                planes[i].layer = 15;
            }
            else
            {
                name = "";
                planes[i].layer = 16;
            }
            switch (i)
            {
                case 0:
                case 4:
                    planes[i].name = "Plane1" + name;
                    planes[i].transform.localPosition = new Vector3(0, 0, focal_length);

                    planes[i].transform.eulerAngles = new Vector3(90, 180, 0);
                    break;
                case 1:
                case 5:
                    planes[i].name = "Plane2" + name;
                    planes[i].transform.localPosition = new Vector3(focal_length, 0, 0);

                    planes[i].transform.eulerAngles = new Vector3(90, -90, 0);
                    break;
                case 2:
                case 6:
                    planes[i].name = "Plane3" + name;
                    planes[i].transform.localPosition = new Vector3(0, 0, -focal_length);

                    planes[i].transform.eulerAngles = new Vector3(90, 0, 0);
                    break;
                case 3:
                case 7:
                    planes[i].name = "Plane4" + name;
                    planes[i].transform.localPosition = new Vector3(-focal_length, 0, 0);

                    planes[i].transform.eulerAngles = new Vector3(90, 90, 0);
                    break;
                case 8:
                    planes[i].name = "Plane5";
                    planes[i].transform.localPosition = new Vector3(0, focal_length, 0);

                    planes[i].transform.eulerAngles = new Vector3(0, 0, 180);
                    break;
                case 9:
                    planes[i].name = "Plane6";
                    planes[i].transform.localPosition = new Vector3(0, -focal_length, 0);

                    planes[i].transform.eulerAngles = new Vector3(0, 180, 0);
                    break;
            }
            */
        }
    }

    void CameraInit()
    {

        Vector3[] cubemapCamPositions = {
            new Vector3(0, 0, eyeSep),
            new Vector3(0, 0, -eyeSep),
            new Vector3(0, 0, 0),
            new Vector3(0, 0, 0),
            new Vector3(-eyeSep, 0, 0),
            new Vector3(eyeSep, 0, 0),
            new Vector3(0, 0, -eyeSep),
            new Vector3(0, 0, eyeSep),
            new Vector3(0, 0, 0),
            new Vector3(0, 0, 0),
            new Vector3(eyeSep, 0, 0),
            new Vector3(-eyeSep, 0, 0)                                                     
        };

        CameraL = new GameObject[6];
        CameraR = new GameObject[6];
        //CameraC = new GameObject[2];

        //int angle = 0;
        for (int i = 0; i < 6; i++)
        {
            CameraL[i] = new GameObject();
            CameraR[i] = new GameObject();

            CameraL[i].name = "Camera_" + cubemapFaceNames[i];
            CameraR[i].name = "Camera_" + cubemapFaceNames[i+6];

            CameraL[i].AddComponent<Camera>();
            CameraR[i].AddComponent<Camera>();

            //position the left and right camera inside the cube
            CameraL[i].transform.parent = transform;
            CameraR[i].transform.parent = transform;
            CameraL[i].transform.position = transform.position;
            CameraR[i].transform.position = transform.position;
            CameraL[i].transform.eulerAngles = cubemapCamRotations[i];
            CameraR[i].transform.eulerAngles = cubemapCamRotations[i];
            
            if(i<2 || i>3){
                CameraL[i].transform.Translate(cubemapCamPositions[i], transform);
                CameraR[i].transform.Translate(cubemapCamPositions[i+6], transform);
            }
            
            //CameraL[i].transform.RotateAround(transform.position, new Vector3(0, 1, 0), angle);
            //CameraR[i].transform.RotateAround(transform.position, new Vector3(0, 1, 0), angle);

            Camera cL = CameraL[i].GetComponent<Camera>();
            Camera cR = CameraR[i].GetComponent<Camera>();

            cL.aspect = 1;
            cL.nearClipPlane = near;
            cL.farClipPlane = far;
            cR.aspect = 1;
            cR.nearClipPlane = near;
            cR.farClipPlane = far;

            if (adjustFOV)
            {
                float newAperture = (float) (180.0/Math.PI * 2.0 * Math.Atan(resolution / (resolution - fovAdjustValue)));
                aperture = newAperture;
            }
            
            cL.fieldOfView = aperture;
            cR.fieldOfView = aperture;
            


            cL.cullingMask = 0x00001fff;
            cR.cullingMask = 0x00001fff;

            cL.enabled = true;
            cR.enabled = true;

            cL.targetTexture = renderTextures[i];
            cR.targetTexture = renderTextures[i + 6];

            //angle += 90;
        }
        /*angle = -90;
        for (int i = 0; i < 2; i++)
        {
            CameraC[i] = new GameObject();

            CameraC[i].name = "CameraC" + (i + 1);

            CameraC[i].AddComponent<Camera>();

            //position the center camera inside the cube
            CameraC[i].transform.parent = transform;
            CameraC[i].transform.position = transform.position;

            CameraC[i].transform.RotateAround(transform.position, new Vector3(1, 0, 0), angle);

            Camera c = CameraC[i].GetComponent<Camera>();

            c.aspect = 1;
            c.fieldOfView = 90;
            c.nearClipPlane = near;
            c.farClipPlane = far;

            c.cullingMask = 0x00001fff;

            c.enabled = true;

            c.targetTexture = renderTextures[i + 8];

            angle += 180;
        }
        */

        if ((OVRPlayer = transform.Find("OVRPlayerController")) != null)
        {
            OVRCamRig = OVRPlayer.Find("OVRCameraRig");

        }
        else
        {
            OVRPlayer = null;
        }
        if (OVRCamRig != null || (OVRCamRig = transform.Find("OVRCameraRig")) != null)
        {
            Transform track = OVRCamRig.Find("TrackingSpace");
            track.Find("LeftEyeAnchor").gameObject.GetComponent<Camera>().cullingMask = 0x00014000;
            track.Find("RightEyeAnchor").gameObject.GetComponent<Camera>().cullingMask = 0x00018000;
            track.Find("CenterEyeAnchor").gameObject.GetComponent<AudioListener>().enabled = false;
        }

        CameraProjectionInit();
    }

    void CameraProjectionInit()
    {
        //for asymmetric frustum for left and right eyes 
        Matrix4x4 mat = new Matrix4x4();

        float l_L, l_R, r_L, r_R, top, bot, wid_div_2, shift;
        wid_div_2 = near * Mathf.Tan(0.5f * aperture * Mathf.PI / 180);
        shift = 0.5f * eyeSep * near / focal_length;

        top = wid_div_2;
        bot = -wid_div_2;

        //Left Eye
        l_L = -wid_div_2 + shift;
        r_L = wid_div_2 + shift;
        //Right Eye
        l_R = -wid_div_2 - shift;
        r_R = wid_div_2 - shift;

        float x, y, a, b, c, d, e;

        //Left Eye
        x = 2 * near / (r_L - l_L);
        y = 2 * near / (top - bot);

        a = (r_L + l_L) / (r_L - l_L);
        b = (top + bot) / (top - bot);
        c = -(far + near) / (far - near);
        d = -1f;
        e = (-2 * far * near) / (far - near);

        mat[0, 0] = x; mat[0, 1] = 0; mat[0, 2] = a; mat[0, 3] = 0;
        mat[1, 0] = 0; mat[1, 1] = y; mat[1, 2] = b; mat[1, 3] = 0;
        mat[2, 0] = 0; mat[2, 1] = 0; mat[2, 2] = c; mat[2, 3] = e;
        mat[3, 0] = 0; mat[3, 1] = 0; mat[3, 2] = d; mat[3, 3] = 0;

        for (int i = 0; i < 4; i++)
            CameraL[i].GetComponent<Camera>().projectionMatrix = mat;

        //Right Eye
        x = 2 * near / (r_R - l_R);
        a = (r_R + l_R) / (r_R - l_R);

        mat[0, 0] = x;
        mat[0, 2] = a;

        for (int i = 0; i < 4; i++)
            CameraR[i].GetComponent<Camera>().projectionMatrix = mat;
    }

    IEnumerator Start()
    {
        PlaneInit();
        CameraInit();
        //int[] map = {1,3,8,9,0,2,5,7,8,9,4,6};
        System.IntPtr[] texturePtrs = new System.IntPtr[faceCount];
        //int j = 0;
        for (int i = 0; i < Math.Min(faceCount,12); i++)
        {
            texturePtrs[i] = renderTextures[i].GetNativeTexturePtr();
            /*j++;
            if (j == 4)
                j = 8;
            else if (j == 10)
                j = 4;*/
        }
        
       


        // Tell native plugin that rendering has started
        ConfigureCubemapFromUnity(texturePtrs, faceCount, resolution);

        yield return StartCoroutine("CallPluginAtEndOfFrames");

        //UpdateCubeTexture ();
        //prePos = transform.position;
    }
    float getAngle()
    {
        float angleY = 0;// OVRCamRig.eulerAngles.y;
        if (OVRPlayer != null)
        {
            angleY += OVRPlayer.eulerAngles.y;
            angleY %= 360;
        }
        return Mathf.Deg2Rad * angleY;
    }
    void UpdateCharacter()
    {
        float angleY = 0;
        if (Input.GetKey(KeyCode.I))
        {
            angleY = getAngle();
            transform.Translate(new Vector3(Mathf.Sin(angleY), 0, Mathf.Cos(angleY)) * moveSpeed);//*Time.deltaTime);
        }
        if (Input.GetKey(KeyCode.L))
        {
            angleY = getAngle();
            transform.Translate(new Vector3(Mathf.Cos(angleY), 0, -Mathf.Sin(angleY)) * moveSpeed);//*Time.deltaTime);
        }
        if (Input.GetKey(KeyCode.J))
        {
            angleY = getAngle();
            transform.Translate(new Vector3(-Mathf.Cos(angleY), 0, Mathf.Sin(angleY)) * moveSpeed);//*Time.deltaTime);
        }
        if (Input.GetKey(KeyCode.K))
        {
            angleY = getAngle();
            transform.Translate(new Vector3(-Mathf.Sin(angleY), 0, -Mathf.Cos(angleY)) * moveSpeed);//*Time.deltaTime);
        }
    }

    // Update is called once per frame
    void Update()
    {
        UpdateCharacter();


        //if (transform.position==prePos) {
        //	return;
        //}
        //UpdateCubeTexture ();
        //prePos = transform.position;
    }
    void LateUpdate()
    {
        if (printFPS)
            print(1.0f / Time.deltaTime);
        if (preFOVAdjustValue != fovAdjustValue)
        {
            CameraProjectionInit();
            preFOVAdjustValue = fovAdjustValue;
        }
            
    }
    //For static update
    void UpdateCubeTexture()
    {

        Camera c;
        GameObject[] go;

        for (int e = 0; e < 2; e++)
        {
            //string planeType;
            if (e == 0)
            { //left eye
                go = CameraL;

                //planeType="L";
            }
            else
            { //right eye
                go = CameraR;
                //planeType="R";
            }
            for (int i = 0; i < 4; i++)
            {
                c = go[i].GetComponent<Camera>();
                //take screenshot for horizontal surrounding
                //Texture2D screenShot = new Texture2D(resolution, resolution, TextureFormat.RGB24, false);

                c.Render();
                /*RenderTexture.active = c.targetTexture;
                screenShot[i+4*e].ReadPixels(new Rect(0, 0, resolution, resolution), 0, 0);
                screenShot[i+4*e].Apply();
                RenderTexture.active = null; // JC: added to avoid errors
                //Transform plane=transform.Find("Plane"+i+planeType);
                //plane.localScale=new Vector3(focal_length*2,1,focal_length*2);
                //Renderer r=plane.gameObject.GetComponent<Renderer>();
                Renderer r=planes[i+4*e].GetComponent<Renderer>();

                //Destroy(r.material.mainTexture);
                r.material.mainTexture = screenShot[i+4*e];
*/
            }
        }
        go = CameraC;
        for (int i = 5; i <= 6; i++)
        {
            c = go[i - 5].GetComponent<Camera>();
            //take screenshot for top and bottom
            //Texture2D screenShot = new Texture2D(resolution, resolution, TextureFormat.RGB24, false);

            c.Render();
            /*RenderTexture.active = c.targetTexture;
            screenShot[i+3].ReadPixels(new Rect(0, 0, resolution, resolution), 0, 0);
            screenShot[i+3].Apply();
            RenderTexture.active = null; // JC: added to avoid errors
            //Transform plane=transform.Find("Plane"+i);
            //plane.localScale=new Vector3(focal_length*2,1,focal_length*2);
            //Renderer r=plane.gameObject.GetComponent<Renderer>();
            Renderer r= planes[i+3].GetComponent<Renderer>();

            //Destroy(r.material.mainTexture);
            r.material.mainTexture = screenShot[i+3];
*/
        }
        //		Resources.UnloadUnusedAssets();
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
            if (extract)
            {
                GL.IssuePluginEvent(1);
            }
        }
    }
}
