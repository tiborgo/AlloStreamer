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
    

    public int leftLayer = 15, rightLayer = 16;


    public float eyeSep = 0.064f, near = 0.1f, far = 10000f, focal_length = 10f, aperture = 90f;

    public int resolution =1920;

    public float moveSpeed = 1;

    public bool printFPS = false, adjustFOV = false;

    public int faceCount = 6;
    public bool extract = true, disablePlanes=false;
    
    public float fovAdjustValue=0.5f;

    public enum Ceiling_Floor_StereoOption {None, PositiveZ, NegativeZ, PositiveX, NegativeX, All}

    public Ceiling_Floor_StereoOption option;
    
    private float preFOVAdjustValue = 0.5f;


    private GameObject[] CameraL;	//for left eye surround scene
    private GameObject[] CameraR;	//for right eye surround scene
    private GameObject[] CameraCL;  //for left ceiling view
    private GameObject[] CameraCR;  //for right ceiling view
    private GameObject[] CameraFL;  //for left floor view
    private GameObject[] CameraFR;  //for right floor view

    private GameObject[] planes, planeCeiling_4Way, planeFloor_4Way; //planeCeiling_1Way, planeFloor_1Way;
    private GameObject cube, cubeCeiling1, cubeFloor1, cubeCeiling4, cubeFloor4, cameras, cameraCeiling, cameraFloor;

    private Vector3 prePos;
    private Transform OVRCamRig, OVRPlayer;

    
    public RenderTexture[] renderTextures,renderTexturesCeiling,renderTexturesFloor;


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
    private static System.String[] cubemapFloorFaceNames ={
        "LeftEye/Floor_PositiveX",
		"LeftEye/Floor_NegativeX",
		"LeftEye/Floor_PositiveZ",
		"LeftEye/Floor_NegativeZ",
		"RightEye/Floor_PositiveX",
		"RightEye/Floor_NegativeX",
		"RightEye/Floor_PositiveZ",
		"RightEye/Floor_NegativeZ"                                        
    };
    private static System.String[] cubemapCeilingFaceNames ={
        "LeftEye/Ceiling_PositiveX",
		"LeftEye/Ceiling_NegativeX",
		"LeftEye/Ceiling_PositiveZ",
		"LeftEye/Ceiling_NegativeZ",
		"RightEye/Ceiling_PositiveX",
		"RightEye/Ceiling_NegativeX",
		"RightEye/Ceiling_PositiveZ",
		"RightEye/Ceiling_NegativeZ"                                        
    };
    private static Vector3[] cubemapFaceRotations = {
		new Vector3(90, -90, 0),
		new Vector3(90, 90, 0),
		//new Vector3(0, 0, 180),
		//new Vector3(0, 180, 0),
		new Vector3(90, 180, 0),
		new Vector3(90, 0, 0),
        new Vector3(90, -90, 0),
		new Vector3(90, 90, 0),
		//new Vector3(0, 0, 180),
		//new Vector3(0, 180, 0),
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
    private static Vector2[][] ceilingUV = {
        new Vector2[] {new Vector2 (0,0) , new Vector2 (0.5f, 1),new Vector2 (1, 0)},
        new Vector2[] {new Vector2 (1, 0), new Vector2 (0,0),new Vector2 (0.5f, 1)},
        new Vector2[] {new Vector2 (1, 0), new Vector2 (0,0),new Vector2 (0.5f, 1)},
        new Vector2[] {new Vector2 (1, 0), new Vector2 (0,0),new Vector2 (0.5f, 1)},
        new Vector2[] {new Vector2 (0,0) , new Vector2 (0.5f, 1),new Vector2 (1, 0)},
        new Vector2[] {new Vector2 (1, 0), new Vector2 (0,0),new Vector2 (0.5f, 1)},
        new Vector2[] {new Vector2 (1, 0), new Vector2 (0,0),new Vector2 (0.5f, 1)},
        new Vector2[] {new Vector2 (1, 0), new Vector2 (0,0),new Vector2 (0.5f, 1)}
        
        /*new Vector2[] {new Vector2 (1, 0), new Vector2 (0.5f, 0.5f), new Vector2 (1, 1)},
        new Vector2[] {new Vector2 (0,0), new Vector2 (0, 1), new Vector2 (0.5f, 0.5f)},
        new Vector2[] {new Vector2 (0, 1), new Vector2 (1, 1), new Vector2 (0.5f, 0.5f)},
        new Vector2[] {new Vector2 (1,0), new Vector2 (0, 0), new Vector2 (0.5f, 0.5f)}
    
         */
    };
    private static Vector2[][] floorUV = {
        new Vector2[] {new Vector2 (1, 0), new Vector2 (0.5f, 0.5f), new Vector2 (1, 1)},
        new Vector2[] {new Vector2 (0,0), new Vector2 (0, 1), new Vector2 (0.5f, 0.5f)},
        new Vector2[] {new Vector2 (0, 1), new Vector2 (1, 1), new Vector2 (0.5f, 0.5f)},
        new Vector2[] {new Vector2 (1,0), new Vector2 (0, 0), new Vector2 (0.5f, 0.5f)},
        new Vector2[] {new Vector2 (1, 0), new Vector2 (0.5f, 0.5f), new Vector2 (1, 1)},
        new Vector2[] {new Vector2 (0,0), new Vector2 (0, 1), new Vector2 (0.5f, 0.5f)},
        new Vector2[] {new Vector2 (0, 1), new Vector2 (1, 1), new Vector2 (0.5f, 0.5f)},
        new Vector2[] {new Vector2 (1,0), new Vector2 (0, 0), new Vector2 (0.5f, 0.5f)}
    };

    private static int[][] ceilingTriangles ={
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2}
    };

    private static int[][] floorTriangles ={
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2},
        new int[]{0,1,2}
    };


    void PlaneInit()
    {
        Vector3[] cubemapFacePositions = {
            new Vector3(focal_length, 0, 0),
            new Vector3(-focal_length, 0, 0),
            new Vector3(0, 0, focal_length),
            new Vector3(0, 0, -focal_length),

            new Vector3(focal_length, 0, 0),
            new Vector3(-focal_length, 0, 0),
            new Vector3(0, 0, focal_length),
            new Vector3(0, 0, -focal_length)                                                        
        };
        Vector3[][] ceilingVertices ={
            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (0, 0, 0), new Vector3 (focal_length, 0, focal_length) },
            new Vector3[]{new Vector3 (-focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (-focal_length, 0, focal_length), new Vector3 (focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, -focal_length), new Vector3 (0, 0, 0) },

            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (0, 0, 0), new Vector3 (focal_length, 0, focal_length) },
            new Vector3[]{new Vector3 (-focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (-focal_length, 0, focal_length), new Vector3 (focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, -focal_length), new Vector3 (0, 0, 0) }
        };

        Vector3[][] floorVertices ={
            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (0, 0, 0), new Vector3 (focal_length, 0, focal_length) },
            new Vector3[]{new Vector3 (-focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (-focal_length, 0, focal_length), new Vector3 (focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, -focal_length), new Vector3 (0, 0, 0) },

            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (0, 0, 0), new Vector3 (focal_length, 0, focal_length) },
            new Vector3[]{new Vector3 (-focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (-focal_length, 0, focal_length), new Vector3 (focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, -focal_length), new Vector3 (0, 0, 0) }
        };

        renderTextures = new RenderTexture[12];
        for (int i = 0; i < renderTextures.Length; i++)
        {
            renderTextures[i] = new RenderTexture(resolution, resolution, 24);
            renderTextures[i].Create();
        }

        renderTexturesCeiling = new RenderTexture[8];
        for (int i = 0; i < renderTexturesCeiling.Length; i++)
        {
            renderTexturesCeiling[i] = new RenderTexture(resolution, resolution/2, 24);
            renderTexturesCeiling[i].Create();
        }

        renderTexturesFloor = new RenderTexture[8];
        for (int i = 0; i < renderTexturesFloor.Length; i++)
        {
            renderTexturesFloor[i] = new RenderTexture(resolution, resolution/2, 24);
            renderTexturesFloor[i].Create();
        }
        cube = new GameObject();
        //cubeCeiling1 = new GameObject();
        //cubeFloor1 = new GameObject();
        cubeCeiling4 = new GameObject();
        cubeFloor4 = new GameObject();

        cube.transform.parent = transform;
        //cubeCeiling1.transform.parent = transform;
        //cubeFloor1.transform.parent = transform;
        cubeCeiling4.transform.parent = transform;
        cubeFloor4.transform.parent = transform;
        cube.transform.localPosition = new Vector3(0, 0, 0);
        //cubeCeiling1.transform.localPosition = new Vector3(0, 0, 0);
        //cubeFloor1.transform.localPosition = new Vector3(0, 0, 0);
        cubeCeiling4.transform.localPosition = new Vector3(0, 0, 0);
        cubeFloor4.transform.localPosition = new Vector3(0, 0, 0);

        cube.name = "Cube Walls";
        //cubeCeiling1.name = "Cube Ceiling 1 Way Stereo";
        //cubeFloor1.name = "Cube Floor 1 Way Stereo";
        cubeCeiling4.name = "Cube Ceiling 4 Way Stereo";
        cubeFloor4.name = "Cube Floor 4 Way Stereo";

        planes = new GameObject[12];
        planeCeiling_4Way = new GameObject[8];
        planeFloor_4Way = new GameObject[8];
        //planeCeiling_1Way = new GameObject[8];
        //planeFloor_1Way = new GameObject[8];
        int k = 0;
        for (int i = 0; i < 8; i++)
        {
            if (k == 2 || k == 8)
                k += 2;
            planes[i] = GameObject.CreatePrimitive(PrimitiveType.Plane);
            planes[i].transform.parent = cube.transform;
            planes[i].transform.localScale = new Vector3(focal_length / 5, 1, focal_length / 5);

            /*
            planeCeiling_1Way[i] = GameObject.CreatePrimitive(PrimitiveType.Plane);
            planeCeiling_1Way[i].transform.parent = cubeCeiling1.transform;
            planeCeiling_1Way[i].transform.localScale = new Vector3(focal_length / 5, 1, focal_length / 5);
            
            planeFloor_1Way[i] = GameObject.CreatePrimitive(PrimitiveType.Plane);
            planeFloor_1Way[i].transform.parent = cubeFloor1.transform;
            planeFloor_1Way[i].transform.localScale = new Vector3(focal_length / 5, 1, focal_length / 5);
            */
            planeCeiling_4Way[i] = new GameObject();
            planeCeiling_4Way[i].transform.parent = cubeCeiling4.transform;
            planeCeiling_4Way[i].transform.localScale = new Vector3(1, 1, 1);
            planeCeiling_4Way[i].AddComponent<MeshFilter>();
            planeCeiling_4Way[i].AddComponent<MeshRenderer>();

            planeFloor_4Way[i] = new GameObject();
            planeFloor_4Way[i].transform.parent = cubeFloor4.transform;
            planeFloor_4Way[i].transform.localScale = new Vector3(1, 1,1);
            planeFloor_4Way[i].AddComponent<MeshFilter>();
            planeFloor_4Way[i].AddComponent<MeshRenderer>();

            Mesh mesh = planeCeiling_4Way[i].GetComponent<MeshFilter>().mesh;
            mesh.Clear();
            mesh.vertices = ceilingVertices[i];
            mesh.uv = ceilingUV[i];
            mesh.triangles = ceilingTriangles[i];

            mesh = planeFloor_4Way[i].GetComponent<MeshFilter>().mesh;
            mesh.Clear();
            mesh.vertices = floorVertices[i];
            mesh.uv = floorUV[i];
            mesh.triangles = floorTriangles[i];

            Renderer[] r = new Renderer[] { planes[i].GetComponent<Renderer>(), planeCeiling_4Way[i].GetComponent<Renderer>(), planeFloor_4Way[i].GetComponent<Renderer>()};//, planeCeiling_1Way[i].GetComponent<Renderer>(), planeFloor_1Way[i].GetComponent<Renderer>() };
            for (int j = 0; j < r.Length; j++) {
                r[j].shadowCastingMode = ShadowCastingMode.Off;
                r[j].receiveShadows = false;
                r[j].useLightProbes = false;
                r[j].reflectionProbeUsage = ReflectionProbeUsage.Off;
                r[j].material.shader = Shader.Find("Unlit/Texture");
            }
               
            r[0].material.mainTexture = renderTextures[k];
            r[1].material.mainTexture = renderTexturesCeiling[i];
            r[2].material.mainTexture = renderTexturesFloor[i];
            //r[3].material.mainTexture = renderTexturesCeiling[i];
            //r[4].material.mainTexture = renderTexturesFloor[i];

            if (i < 4)
            {
                planes[i].layer = leftLayer;
                planeCeiling_4Way[i].layer = leftLayer;
                planeFloor_4Way[i].layer = leftLayer;
            //    planeCeiling_1Way[i].layer = leftLayer;
            //    planeFloor_1Way[i].layer = leftLayer;
            }
            else {
                planes[i].layer = rightLayer;
                planeCeiling_4Way[i].layer = rightLayer;
                planeFloor_4Way[i].layer = rightLayer;
            //    planeCeiling_1Way[i].layer = rightLayer;
            //    planeFloor_1Way[i].layer = rightLayer;
            }
            
            planes[i].name = cubemapFaceNames[k];
            planes[i].transform.localPosition = cubemapFacePositions[i];
            planes[i].transform.eulerAngles = cubemapFaceRotations[i];

            planeCeiling_4Way[i].name = cubemapCeilingFaceNames[i];
            planeCeiling_4Way[i].transform.localPosition = new Vector3(0, focal_length,0) ;
            planeCeiling_4Way[i].transform.eulerAngles =new Vector3(0, 180, 180);

            planeFloor_4Way[i].name = cubemapFloorFaceNames[i];
            planeFloor_4Way[i].transform.localPosition = new Vector3(0,-focal_length, 0);
            planeFloor_4Way[i].transform.eulerAngles = new Vector3(0, 0, 0);
            /*
            planeCeiling_1Way[i].name = cubemapCeilingFaceNames[i];
            planeCeiling_1Way[i].transform.localPosition = new Vector3(0, focal_length, 0);
            planeCeiling_1Way[i].transform.eulerAngles = new Vector3(0, 180, 180);

            planeFloor_1Way[i].name = cubemapFloorFaceNames[i];
            planeFloor_1Way[i].transform.localPosition = new Vector3(0, -focal_length, 0);
            planeFloor_1Way[i].transform.eulerAngles = new Vector3(0, 0, 0);
            */
            k++;
        }
        k = 2;
        for (int i = 8; i < 12; i++) {
            if (k == 4)
                k =8;
            planes[i] = GameObject.CreatePrimitive(PrimitiveType.Plane);
            planes[i].transform.parent = cube.transform;
            planes[i].transform.localScale = new Vector3(focal_length / 5, 1, focal_length / 5);

            Renderer r = planes[i].GetComponent<Renderer>();
            r.shadowCastingMode = ShadowCastingMode.Off;
            r.receiveShadows = false;
            r.useLightProbes = false;
            r.reflectionProbeUsage = ReflectionProbeUsage.Off;
            r.material.shader = Shader.Find("Unlit/Texture");
            r.material.mainTexture = renderTextures[k];

            if (i < 10)
                planes[i].layer = leftLayer;
            else
                planes[i].layer = rightLayer;

            planes[i].name = cubemapFaceNames[k];

            if (i % 2 == 0)
            {
                planes[i].transform.localPosition = new Vector3(0, focal_length, 0);
                planes[i].transform.eulerAngles = new Vector3(0, 0, 180);
            }
            else {
                planes[i].transform.localPosition = new Vector3(0, -focal_length, 0);
                planes[i].transform.eulerAngles = new Vector3(0, 180, 0);
            }
            
            k++;
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
        CameraCL = new GameObject[4];
        CameraCR = new GameObject[4];
        CameraFL = new GameObject[4];
        CameraFR = new GameObject[4];

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


            if (i < 2 || i > 3)
            {
                cL.cullingMask = ~((1 << leftLayer) | (1 << rightLayer));
                cR.cullingMask = ~((1 << leftLayer) | (1 << rightLayer));
            }
            else
            {
                cL.cullingMask = (1 << leftLayer) ;
                cR.cullingMask = (1 << rightLayer);
            }
            cL.enabled = true;
            cR.enabled = true;

            cL.targetTexture = renderTextures[i];
            cR.targetTexture = renderTextures[i + 6];

            //angle += 90;
        }
        int angle = 90;
        int k = 0;
        for (int i = 0; i < 4; i++)
        {
            if (k == 2){
                 k = 4;
                angle=180;
            }
               
            //
            CameraCL[i] = new GameObject(); CameraCR[i] = new GameObject();
            CameraFL[i] = new GameObject(); CameraFR[i] = new GameObject();

            CameraCL[i].name = "CameraC_" + cubemapFaceNames[k]; CameraCR[i].name = "CameraC_" + cubemapFaceNames[k+6];
            CameraFL[i].name = "CameraF_" + cubemapFaceNames[k]; CameraFR[i].name = "CameraF_" + cubemapFaceNames[k+6];

            CameraCL[i].AddComponent<Camera>(); CameraCR[i].AddComponent<Camera>();
            CameraFL[i].AddComponent<Camera>(); CameraFR[i].AddComponent<Camera>();

            //position the center camera inside the cube
            CameraCL[i].transform.parent = transform; CameraCR[i].transform.parent = transform;
            CameraFL[i].transform.parent = transform; CameraFR[i].transform.parent = transform;
            CameraCL[i].transform.position = transform.position; CameraCR[i].transform.position = transform.position;
            CameraFL[i].transform.position = transform.position; CameraFR[i].transform.position = transform.position;
            CameraCL[i].transform.eulerAngles = new Vector3(270, angle, 0); CameraCR[i].transform.eulerAngles = new Vector3(270, angle, 0);
            CameraFL[i].transform.eulerAngles = new Vector3(90, angle, 0); CameraFR[i].transform.eulerAngles = new Vector3(90, angle, 0);

            CameraCL[i].transform.Translate(cubemapCamPositions[k], transform); CameraCR[i].transform.Translate(cubemapCamPositions[k+6], transform);
            CameraFL[i].transform.Translate(cubemapCamPositions[k], transform); CameraFR[i].transform.Translate(cubemapCamPositions[k + 6], transform);

            //camera setting
            Camera[] c = new Camera[] { CameraCL[i].GetComponent<Camera>(), CameraCR[i].GetComponent<Camera>(), CameraFL[i].GetComponent<Camera>(), CameraFR[i].GetComponent<Camera>() };
            for (int j = 0; j < c.Length; j++)
            {
                if (option == Ceiling_Floor_StereoOption.All)
                {
                    c[j].aspect = 2;
                    c[j].fieldOfView = aperture/2;
                }
                else {
                    c[j].aspect = 1;
                    c[j].fieldOfView = aperture;
                }
                
                c[j].nearClipPlane = near;
                c[j].farClipPlane = far;
                c[j].cullingMask = ~((1 << leftLayer) | (1 << rightLayer));
                c[j].enabled = true;
            }
            c[0].targetTexture = renderTexturesCeiling[i]; c[1].targetTexture = renderTexturesCeiling[i+4];
            c[2].targetTexture = renderTexturesFloor[i]; c[3].targetTexture = renderTexturesFloor[i+4];


            k++;
            angle += 180;
            angle %= 360;
        }
        

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
            track.Find("LeftEyeAnchor").gameObject.GetComponent<Camera>().cullingMask = 1<<leftLayer;
            track.Find("RightEyeAnchor").gameObject.GetComponent<Camera>().cullingMask = 1<<rightLayer;
            track.Find("CenterEyeAnchor").gameObject.GetComponent<AudioListener>().enabled = false;
        }

        CameraProjectionInit();
    }
    Matrix4x4[] CalculateProjectionMatrix(float near, float far, float aperture, float eyeSep, float focal_length, bool fourway=false) {
        //for asymmetric frustum for left and right eyes 
        Matrix4x4[] mats = new Matrix4x4[2];

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
        
        if (fourway) {
            top = 0;
            //Left Eye
            l_L = -wid_div_2 + shift;
            r_L = wid_div_2 + shift;
            //Right Eye
            l_R = -wid_div_2 - shift;
            r_R = wid_div_2 - shift;
        }

        
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

        mats[0] = mat;

        //Right Eye
        x = 2 * near / (r_R - l_R);
        a = (r_R + l_R) / (r_R - l_R);

        mat[0, 0] = x;
        mat[0, 2] = a;

        mats[1] = mat;

        return mats;
    }
    void CameraProjectionInit()
    {

        Matrix4x4[] mat = CalculateProjectionMatrix(near, far, aperture, eyeSep, focal_length);
        Matrix4x4[] matCF;
        if(option==Ceiling_Floor_StereoOption.All)
             matCF= CalculateProjectionMatrix(near, far, aperture, eyeSep, focal_length,true);
        else
            matCF = CalculateProjectionMatrix(near, far, aperture, eyeSep, focal_length);
        for (int i = 0; i < CameraL.Length; i++)
        {
            CameraL[i].GetComponent<Camera>().projectionMatrix = mat[0];
            if (i == 1)
                i += 2;
        }

        for (int i = 0; i < CameraCL.Length; i++) {
            CameraCL[i].GetComponent<Camera>().projectionMatrix = matCF[0];
            CameraFL[i].GetComponent<Camera>().projectionMatrix = matCF[0];
        }

        for (int i = 0; i < CameraR.Length; i++)
        {
            CameraR[i].GetComponent<Camera>().projectionMatrix = mat[1];
            if (i == 1)
                i += 2;
        }

        for (int i = 0; i < CameraCR.Length; i++)
        {
            CameraCR[i].GetComponent<Camera>().projectionMatrix = matCF[1];
            CameraFR[i].GetComponent<Camera>().projectionMatrix = matCF[1];
        }

           
    }
    void enablePlanes(bool enable) {
        cube.SetActive(enable);
        //cubeCeiling1.SetActive(enable);
        //cubeFloor1.SetActive(enable);
        if (enable || option != Ceiling_Floor_StereoOption.All)
        {
            cubeCeiling4.SetActive(enable);
            cubeFloor4.SetActive(enable);
        }
        else {
            cubeCeiling4.SetActive(true);
            cubeFloor4.SetActive(true);
        }
        if(enable)
            Plane_CameraSetUp();
    }
    void OneWayStereoPlaneCameraSetUp(int side) {
        CameraL[2].SetActive(false); CameraR[2].SetActive(false);
        CameraL[3].SetActive(false); CameraR[3].SetActive(false);

        for (int i = 0; i < CameraCL.Length; i++)
        {
            CameraCL[i].SetActive(false);
            CameraCR[i].SetActive(false);
            CameraFL[i].SetActive(false);
            CameraFR[i].SetActive(false);
        }
        
        CameraCL[side].SetActive(true);
        CameraCR[side].SetActive(true);
        CameraFL[side].SetActive(true);
        CameraFR[side].SetActive(true);

        if (disablePlanes)
        {
            enablePlanes(false);
            return;
        }

        for (int i = 0; i < 8; i++)
        {
            //planeCeiling_1Way[i].SetActive(false);
            //planeFloor_1Way[i].SetActive(false);
            planeCeiling_4Way[i].SetActive(false);
            planeFloor_4Way[i].SetActive(false);

        }

        for (int i = 8; i < 12; i++)
        {
            planes[i].SetActive(true);
            if (i % 2 == 0)
                planes[i].GetComponent<Renderer>().material.mainTexture = renderTexturesCeiling[side];
            else
            {
                planes[i].GetComponent<Renderer>().material.mainTexture = renderTexturesFloor[side];
                side += 4;
            }

        }
        
    }
    void Plane_CameraSetUp() { 
        switch(option){
            case Ceiling_Floor_StereoOption.None:
                CameraL[2].SetActive(true);    CameraR[2].SetActive(true);
                CameraL[3].SetActive(true);    CameraR[3].SetActive(true);

                CameraL[2].GetComponent<Camera>().cullingMask= ~((1 << leftLayer) | (1 << rightLayer));
                CameraR[2].GetComponent<Camera>().cullingMask = ~((1 << leftLayer) | (1 << rightLayer));
                CameraL[3].GetComponent<Camera>().cullingMask= ~((1 << leftLayer) | (1 << rightLayer));
                CameraR[3].GetComponent<Camera>().cullingMask = ~((1 << leftLayer) | (1 << rightLayer));

                for (int i = 0; i < CameraCL.Length; i++)
                {
                    CameraCL[i].SetActive(false);
                    CameraCR[i].SetActive(false);
                    CameraFL[i].SetActive(false);
                    CameraFR[i].SetActive(false); 
                }

                if (disablePlanes)
                {
                    enablePlanes(false);
                    break;
                }

                for (int i = 0; i < 8; i++)
                {
                    //planeCeiling_1Way[i].SetActive(false);
                    //planeFloor_1Way[i].SetActive(false);
                    planeCeiling_4Way[i].SetActive(false);
                    planeFloor_4Way[i].SetActive(false);

                }
                   
                for (int i = 8; i < 12; i++)
                    planes[i].SetActive(true);
                
                break;
            case Ceiling_Floor_StereoOption.PositiveX:
                OneWayStereoPlaneCameraSetUp(0);
                break;
            case Ceiling_Floor_StereoOption.NegativeX:
                OneWayStereoPlaneCameraSetUp(1);
                break;
            case Ceiling_Floor_StereoOption.PositiveZ:
                OneWayStereoPlaneCameraSetUp(2);
                break;
            case Ceiling_Floor_StereoOption.NegativeZ:
                OneWayStereoPlaneCameraSetUp(3);
                break;
            case Ceiling_Floor_StereoOption.All:
                CameraL[2].SetActive(true);    CameraR[2].SetActive(true);
                CameraL[3].SetActive(true);    CameraR[3].SetActive(true);

                CameraL[2].GetComponent<Camera>().cullingMask = 1 << leftLayer;
                CameraR[2].GetComponent<Camera>().cullingMask = 1 << rightLayer;
                CameraL[3].GetComponent<Camera>().cullingMask = 1 << leftLayer;
                CameraR[3].GetComponent<Camera>().cullingMask = 1 << rightLayer;

                for (int i = 0; i < CameraCL.Length; i++)
                {
                    CameraCL[i].SetActive(true);
                    CameraCR[i].SetActive(true);
                    CameraFL[i].SetActive(true);
                    CameraFR[i].SetActive(true); 
                }

                if (disablePlanes)
                {
                    enablePlanes(false);
                    break;
                }

                for (int i = 0; i < 8; i++)
                {
                    //planeCeiling_1Way[i].SetActive(false);
                    //planeFloor_1Way[i].SetActive(false);
                    planeCeiling_4Way[i].SetActive(true);
                    planeFloor_4Way[i].SetActive(true);

                }
                for (int i = 8; i < 12; i++)
                    planes[i].SetActive(false);

                break;

        }
    }
    System.IntPtr[] OneWayStereoExtractionSetUp(int side) {
        System.IntPtr[] texturePtrs = new System.IntPtr[faceCount];
        for (int i = 0; i < Math.Min(faceCount, 12); i++)
        {
            if (i != 2 || i != 3 || i != 8 || i != 9)
                texturePtrs[i] = renderTextures[i].GetNativeTexturePtr();
        }
        
        texturePtrs[2] = renderTexturesCeiling[side].GetNativeTexturePtr();
        texturePtrs[3] = renderTexturesFloor[side].GetNativeTexturePtr();
        texturePtrs[8] = renderTexturesCeiling[side+4].GetNativeTexturePtr();
        texturePtrs[9] = renderTexturesFloor[side+4].GetNativeTexturePtr();
        
        return texturePtrs;
    }
    System.IntPtr[] ExtractionSetUp() {
        System.IntPtr[] texturePtrs = new System.IntPtr[faceCount];
        switch (option) { 
            case Ceiling_Floor_StereoOption.None:
            case Ceiling_Floor_StereoOption.All:
                for (int i = 0; i < Math.Min(faceCount, 12); i++)
                {
                    texturePtrs[i] = renderTextures[i].GetNativeTexturePtr();
                }
                break;
            case Ceiling_Floor_StereoOption.PositiveX:
                return OneWayStereoExtractionSetUp(0);
            case Ceiling_Floor_StereoOption.NegativeX:
                return OneWayStereoExtractionSetUp(1);
            case Ceiling_Floor_StereoOption.PositiveZ:
                return OneWayStereoExtractionSetUp(2);
            case Ceiling_Floor_StereoOption.NegativeZ:
                return OneWayStereoExtractionSetUp(3);
        }
        
        return texturePtrs;
    }
    IEnumerator Start()
    {
        PlaneInit();
        CameraInit();

        Plane_CameraSetUp();

        /*System.IntPtr[] texturePtrs = new System.IntPtr[faceCount];
        for (int i = 0; i < Math.Min(faceCount, 12); i++)
        {
            texturePtrs[i] = renderTextures[i].GetNativeTexturePtr();
        }
        */System.IntPtr[] texturePtrs=ExtractionSetUp();


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
        if (Input.GetKey(KeyCode.U)) {
            transform.Translate(new Vector3(0, 1, 0) * moveSpeed);//*Time.deltaTime);
        }
        if (Input.GetKey(KeyCode.O)) {
            transform.Translate(new Vector3(0, -1, 0) * moveSpeed);//*Time.deltaTime);
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
        /*if (preFOVAdjustValue != fovAdjustValue)
        {
            CameraProjectionInit();
            preFOVAdjustValue = fovAdjustValue;
        }*/
            
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
