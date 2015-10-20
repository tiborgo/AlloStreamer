using UnityEngine;
using UnityEngine.Rendering;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Reflection;


//[ExecuteInEditMode]
public class RenderStereoCubemap : MonoBehaviour
{
    public int leftLayer = 15, rightLayer = 16;
    public int[] DoNotDisplayLayers = new int[1]{8};
    public float eyeSep = 0.064f *2, near = 0.1f, far = 10000f, focal_length = 10f, aperture = 90f;
    public int resolution =1920;
    public int faceCount = 6;

    public float moveSpeed = 1;
    public bool printFPS = false;
    public int fps = -1;
    
    public bool extract = true, disablePlanes=false;
    
    public enum Ceiling_Floor_StereoOption {None, PositiveZ, NegativeZ, PositiveX, NegativeX, All}

    public Ceiling_Floor_StereoOption CeilingFloorStereoOption;

    private int DoNotDisplayCullingMask = 0;

    //private float preFOVAdjustValue = 0.5f;
    private bool adjustFOV = false;
    private float fovAdjustValue = 0.5f;

    private GameObject[] CameraL;	//for left eye surround scene
    private GameObject[] CameraR;	//for right eye surround scene
    private GameObject[] CameraCL;  //for left ceiling view
    private GameObject[] CameraCR;  //for right ceiling view
    private GameObject[] CameraFL;  //for left floor view
    private GameObject[] CameraFR;  //for right floor view

    private GameObject[] planes, planeCeiling_4Way, planeFloor_4Way; //planeCeiling_1Way, planeFloor_1Way;
    private GameObject cube, cubeCeiling4, cubeFloor4, cameras, camerasCeiling, camerasFloor;

    private Vector3 prePos;
    private Transform OVRCamRig, OVRPlayer;

    
    private RenderTexture[] renderTextures,renderTexturesCeiling,renderTexturesFloor;
    private RenderTexture[] inTextures;
    private RenderTexture[] outTextures;
    private ComputeShader shader;

    private bool didPlay = false;

    [DllImport("CubemapExtractionPlugin")]
    private static extern void ConfigureCubemapFromUnity(System.IntPtr[] texturePtrs, int cubemapFacesCount, int width, int height);
    [DllImport("CubemapExtractionPlugin")]
    private static extern void StopFromUnity();

    private static System.String[] cubemapFaceNames = {
		"LeftEye/PositiveX",
		"LeftEye/NegativeX",
		"LeftEye/PositiveZ",
		"LeftEye/NegativeZ",
        "LeftEye/PositiveY",
        "LeftEye/NegativeY",

        "RightEye/PositiveX",
		"RightEye/NegativeX",
		"RightEye/PositiveZ",
		"RightEye/NegativeZ",
        "RightEye/PositiveY",
        "RightEye/NegativeY"
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
		
		new Vector3(90, 180, 0),
		new Vector3(90, 0, 0),

        new Vector3(90, -90, 0),
		new Vector3(90, 90, 0),
		
		new Vector3(90, 180, 0),
		new Vector3(90, 0, 0),
	};

    private static Vector3[] cubemapCamRotations = {
		new Vector3(  0,  90, 0),
		new Vector3(  0, 270, 0),
		new Vector3(  0,   0, 0),
		new Vector3(  0, 180, 0),
        new Vector3(270,   0, 0),
        new Vector3( 90,   0, 0),

        new Vector3(  0,  90, 0),
        new Vector3(  0, 270, 0),
        new Vector3(  0,   0, 0),
        new Vector3(  0, 180, 0),
        new Vector3(270,   0, 0),
        new Vector3( 90,   0, 0)
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
        
    };
    private static Vector2[][] floorUV = {
        new Vector2[] {new Vector2 (1, 1),new Vector2 (0.5f, 0),new Vector2 (0,1) },
        new Vector2[] { new Vector2 (0,1),new Vector2 (1, 1),new Vector2 (0.5f, 0)},
        new Vector2[] { new Vector2 (0,1),new Vector2 (1, 1),new Vector2 (0.5f, 0)},
        new Vector2[] { new Vector2 (0,1),new Vector2 (1, 1),new Vector2 (0.5f, 0)},
        
        new Vector2[] {new Vector2 (1, 1),new Vector2 (0.5f, 0),new Vector2 (0,1) },
        new Vector2[] { new Vector2 (0,1),new Vector2 (1, 1),new Vector2 (0.5f, 0)},
        new Vector2[] { new Vector2 (0,1),new Vector2 (1, 1),new Vector2 (0.5f, 0)},
        new Vector2[] { new Vector2 (0,1),new Vector2 (1, 1),new Vector2 (0.5f, 0)}
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
        //Init Vertices
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
            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, -focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (-focal_length, 0, focal_length), new Vector3 (focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            
            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (0, 0, 0), new Vector3 (focal_length, 0, focal_length) },
            new Vector3[]{new Vector3 (-focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (focal_length, 0, -focal_length), new Vector3 (-focal_length, 0, -focal_length), new Vector3 (0, 0, 0) },
            new Vector3[]{new Vector3 (-focal_length, 0, focal_length), new Vector3 (focal_length, 0, focal_length), new Vector3 (0, 0, 0) }
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
        //Create RenderTextures
        renderTextures = new RenderTexture[faceCount];
        outTextures = new RenderTexture[faceCount];
        for (int i = 0; i < renderTextures.Length; i++)
        {
            renderTextures[i] = new RenderTexture(resolution, resolution, 1, RenderTextureFormat.ARGB32);
            renderTextures[i].Create();
            if (SystemInfo.graphicsDeviceVersion.StartsWith("Direct3D"))
            {
                outTextures[i] = new RenderTexture(resolution, resolution, 1, RenderTextureFormat.ARGB32);
                outTextures[i].enableRandomWrite = true;
                outTextures[i].Create();
            }
            else
            {
                outTextures[i] = renderTextures[i];
            }
        }

        renderTexturesCeiling = new RenderTexture[8];
        for (int i = 0; i < renderTexturesCeiling.Length; i++)
        {
            if(CeilingFloorStereoOption==Ceiling_Floor_StereoOption.All)
                renderTexturesCeiling[i] = new RenderTexture(resolution, resolution / 2, 1, RenderTextureFormat.ARGB32);
            else
                renderTexturesCeiling[i] = new RenderTexture(resolution, resolution, 1, RenderTextureFormat.ARGB32);
            renderTexturesCeiling[i].Create();
        }

        renderTexturesFloor = new RenderTexture[8];
        for (int i = 0; i < renderTexturesFloor.Length; i++)
        {
            if (CeilingFloorStereoOption == Ceiling_Floor_StereoOption.All)
                renderTexturesFloor[i] = new RenderTexture(resolution, resolution / 2, 1, RenderTextureFormat.ARGB32);
            else
                renderTexturesFloor[i] = new RenderTexture(resolution, resolution, 1, RenderTextureFormat.ARGB32);
            renderTexturesFloor[i].Create();
        }

        //Parent Object for Organization
        
        cube = new GameObject();
        cubeCeiling4 = new GameObject();
        cubeFloor4 = new GameObject();

        cube.transform.parent = transform;
        cubeCeiling4.transform.parent = transform;
        cubeFloor4.transform.parent = transform;
        cube.transform.localPosition = new Vector3(0, 0, 0);
        cubeCeiling4.transform.localPosition = new Vector3(0, 0, 0);
        cubeFloor4.transform.localPosition = new Vector3(0, 0, 0);

        cube.name = "Cube Walls";
        cubeCeiling4.name = "Cube Ceiling 4 Way Stereo";
        cubeFloor4.name = "Cube Floor 4 Way Stereo";

        //Create Planes for the Cube

        planes = new GameObject[12];
        planeCeiling_4Way = new GameObject[8];
        planeFloor_4Way = new GameObject[8];
        
        int k = 0;
        for (int i = 0; i < 8; i++)
        {
            if (k == 4) //Init Ceilings and Floors for 1 directional stereo or no stereo later
                k += 2;
            if (k >= faceCount) //you are done with surround faces
                break;
            //Create Planes for Walls
            planes[i] = GameObject.CreatePrimitive(PrimitiveType.Plane);
            planes[i].transform.parent = cube.transform;
            planes[i].transform.localScale = new Vector3(focal_length / 5, 1, focal_length / 5);

            //Game Object for Ceiling
            planeCeiling_4Way[i] = new GameObject();
            planeCeiling_4Way[i].transform.parent = cubeCeiling4.transform;
            planeCeiling_4Way[i].transform.localScale = new Vector3(1, 1, 1);
            planeCeiling_4Way[i].AddComponent<MeshFilter>();
            planeCeiling_4Way[i].AddComponent<MeshRenderer>();

            //Game Object for Floor
            planeFloor_4Way[i] = new GameObject();
            planeFloor_4Way[i].transform.parent = cubeFloor4.transform;
            planeFloor_4Way[i].transform.localScale = new Vector3(1, 1,1);
            planeFloor_4Way[i].AddComponent<MeshFilter>();
            planeFloor_4Way[i].AddComponent<MeshRenderer>();

            //Plane for Ceiling
            Mesh mesh = planeCeiling_4Way[i].GetComponent<MeshFilter>().mesh;
            mesh.Clear();
            mesh.vertices = ceilingVertices[i];
            mesh.uv = ceilingUV[i];
            mesh.triangles = ceilingTriangles[i];

            //Plane for Floor
            mesh = planeFloor_4Way[i].GetComponent<MeshFilter>().mesh;
            mesh.Clear();
            mesh.vertices = floorVertices[i];
            mesh.uv = floorUV[i];
            mesh.triangles = floorTriangles[i];

            //Rendering Setting
            Renderer[] r = new Renderer[] { planes[i].GetComponent<Renderer>(), planeCeiling_4Way[i].GetComponent<Renderer>(), planeFloor_4Way[i].GetComponent<Renderer>()};//, planeCeiling_1Way[i].GetComponent<Renderer>(), planeFloor_1Way[i].GetComponent<Renderer>() };
            for (int j = 0; j < r.Length; j++) {
                r[j].shadowCastingMode = ShadowCastingMode.Off;
                r[j].receiveShadows = false;
                r[j].useLightProbes = false;
                r[j].reflectionProbeUsage = ReflectionProbeUsage.Off;
                r[j].material.shader = Shader.Find("Unlit/Texture");
            }

            //Set main texture to RenderTexture
            r[0].material.mainTexture = renderTextures[k];
            r[1].material.mainTexture = renderTexturesCeiling[i];
            r[2].material.mainTexture = renderTexturesFloor[i];

            //Set Layers for each plane
            if (i < 4)  //left eye
            {
                planes[i].layer = leftLayer;
                planeCeiling_4Way[i].layer = leftLayer;
                planeFloor_4Way[i].layer = leftLayer;
            
            }
            else {  //right eye
                planes[i].layer = rightLayer;
                planeCeiling_4Way[i].layer = rightLayer;
                planeFloor_4Way[i].layer = rightLayer;
            
            }
            
            //Name the planes and set their position and orientation
            planes[i].name = cubemapFaceNames[k];
            planes[i].transform.localPosition = cubemapFacePositions[i];
            planes[i].transform.eulerAngles = cubemapFaceRotations[i];

            planeCeiling_4Way[i].name = cubemapCeilingFaceNames[i];
            planeCeiling_4Way[i].transform.localPosition = new Vector3(0, focal_length,0) ;
            planeCeiling_4Way[i].transform.eulerAngles =new Vector3(0, 180, 180);

            planeFloor_4Way[i].name = cubemapFloorFaceNames[i];
            planeFloor_4Way[i].transform.localPosition = new Vector3(0,-focal_length, 0);
            planeFloor_4Way[i].transform.eulerAngles = new Vector3(0, 0, 0);
            
            k++;
        }
        
        //Set Ceiling and Floor for no stereo or 1 directional stereo
        k = 4;
        for (int i = 8; i < 12; i++) {
            if (k == 6)
                k = 10;
            if (k >= faceCount) //you are done with surround faces
                break;
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
        //Init Camera Positions
        Vector3[] cubemapCamPositions = {
            new Vector3(0, 0, eyeSep/2),
            new Vector3(0, 0, -eyeSep/2),
            new Vector3(-eyeSep/2, 0, 0),
            new Vector3(eyeSep/2, 0, 0),
            new Vector3(0, 0, 0),
            new Vector3(0, 0, 0),
            
            new Vector3(0, 0, -eyeSep/2),
            new Vector3(0, 0, eyeSep/2),
            new Vector3(eyeSep/2, 0, 0),
            new Vector3(-eyeSep/2, 0, 0),
            new Vector3(0, 0, 0),
            new Vector3(0, 0, 0)                                        
        };

        //Parent Object for Organization
        cameras = new GameObject();
        camerasCeiling = new GameObject();
        camerasFloor = new GameObject();

        cameras.transform.parent = transform;
        camerasCeiling.transform.parent = transform;
        camerasFloor.transform.parent = transform;
        cameras.transform.localPosition = new Vector3(0, 0, 0);
        camerasCeiling.transform.localPosition = new Vector3(0, 0, 0);
        camerasFloor.transform.localPosition = new Vector3(0, 0, 0);

        cameras.name = "Cameras";
        camerasCeiling.name = "Cameras Ceiling 4 Way Stereo";
        camerasFloor.name = "Cameras Floor 4 Way Stereo";

        //Camera Objects
        CameraL = new GameObject[Math.Min(6, faceCount)];
        CameraR = new GameObject[Math.Max(0, faceCount - 6)];
        CameraCL = new GameObject[Math.Min(4, faceCount)];
        CameraCR = new GameObject[Math.Min(Math.Max(0, faceCount - 6), 4)];
        CameraFL = new GameObject[Math.Min(4, faceCount)];
        CameraFR = new GameObject[Math.Min(Math.Max(0, faceCount - 6), 4)];

        //For DoNotDisplayLayers 
        
        for (int j = 0; j < DoNotDisplayLayers.Length; j++)
        {
            DoNotDisplayCullingMask = DoNotDisplayCullingMask | (1 << DoNotDisplayLayers[j]);
        }

        //Init Cameras
        for (int i = 0; i < 6; i++)
        {
            if (i >= faceCount)
                break;
            //Create Cameras
            CameraL[i] = new GameObject();
            CameraL[i].name = "Camera_" + cubemapFaceNames[i];
            CameraL[i].AddComponent<Camera>();
           
            //position and orient the left and right camera inside the cube
            CameraL[i].transform.parent = cameras.transform;           
            CameraL[i].transform.position = cameras.transform.position;           
            CameraL[i].transform.eulerAngles = cubemapCamRotations[i];
                
            if(i<4 || i>5)
            {
                CameraL[i].transform.Translate(cubemapCamPositions[i], cameras.transform);
            }
            
            Camera cL = CameraL[i].GetComponent<Camera>();
            
            //Stub for camera config (later overriden by CameraProjectionInit())
            cL.aspect = 1;
            cL.nearClipPlane = near;
            cL.farClipPlane = far;

            if (adjustFOV)
            {
                float newAperture = (float) (180.0/Math.PI * 2.0 * Math.Atan(resolution / (resolution - fovAdjustValue)));
                aperture = newAperture;
            }
            
            cL.fieldOfView = aperture;

            //Set Culling Mask so that camera do not see the planes
            if (i < 4 || i > 5)
            {
                cL.cullingMask = ~((1 << leftLayer) | (1 << rightLayer) | DoNotDisplayCullingMask);
            }
            else//exception for 4 directional stereo (cameras for +Y and -Y directions are used to capture textures for ceiling and floor)
            {
                cL.cullingMask = (1 << leftLayer) ;
            }
            //enables the cameras
            cL.enabled = true;

            //Set target texture
            cL.targetTexture = renderTextures[i];

            if (i + 6 < faceCount)
            {
                CameraR[i] = new GameObject();
                CameraR[i].name = "Camera_" + cubemapFaceNames[i + 6];
                CameraR[i].AddComponent<Camera>();
                CameraR[i].transform.parent = cameras.transform;
                CameraR[i].transform.position = cameras.transform.position;
                CameraR[i].transform.eulerAngles = cubemapCamRotations[i];

                if (i < 4 || i > 5)
                {
                    CameraR[i].transform.Translate(cubemapCamPositions[i + 6], cameras.transform);
                }

                Camera cR = CameraR[i].GetComponent<Camera>();
                cR.aspect = 1;
                cR.nearClipPlane = near;
                cR.farClipPlane = far;
                cR.fieldOfView = aperture;

                //Set Culling Mask so that camera do not see the planes
                if (i < 4 || i > 5)
                {
                    cR.cullingMask = ~((1 << leftLayer) | (1 << rightLayer) | DoNotDisplayCullingMask);
                }
                else//exception for 4 directional stereo (cameras for +Y and -Y directions are used to capture textures for ceiling and floor)
                {
                    cR.cullingMask = (1 << rightLayer);
                }

                cR.enabled = true;
                cR.targetTexture = renderTextures[i + 6];
            }
             
        }
        int angle = 90;
        int k = 0;
        for (int i = 0; i < 4; i++)
        {
            if (k == 2)
                angle = 0;
            
            if (i >= faceCount)
                break;
                           
            CameraCL[i] = new GameObject();                     
            CameraFL[i] = new GameObject();

            CameraCL[i].name = "CameraC_" + cubemapFaceNames[k];
            CameraFL[i].name = "CameraF_" + cubemapFaceNames[k];

            CameraCL[i].AddComponent<Camera>();                 
            CameraFL[i].AddComponent<Camera>();                 

            //position the cameras for stereo ceiling and floor inside the cube
            CameraCL[i].transform.parent = camerasCeiling.transform;            
            CameraFL[i].transform.parent = camerasFloor.transform;              
            CameraCL[i].transform.position = camerasCeiling.transform.position; 
            CameraFL[i].transform.position = camerasFloor.transform.position;   
            CameraCL[i].transform.eulerAngles = new Vector3(270, angle, 0);     
            CameraFL[i].transform.eulerAngles = new Vector3(90, angle, 0);      

            CameraCL[i].transform.Translate(cubemapCamPositions[k], camerasCeiling.transform);      
            CameraFL[i].transform.Translate(cubemapCamPositions[k], camerasFloor.transform);        

            //camera setting
            Camera[] c = new Camera[] { CameraCL[i].GetComponent<Camera>(),  CameraFL[i].GetComponent<Camera>() };
            for (int j = 0; j < c.Length; j++)
            {
                if (CeilingFloorStereoOption == Ceiling_Floor_StereoOption.All)
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
                c[j].cullingMask = ~((1 << leftLayer) | (1 << rightLayer) | DoNotDisplayCullingMask);
                c[j].enabled = true;
            }
            c[0].targetTexture = renderTexturesCeiling[i];      
            c[1].targetTexture = renderTexturesFloor[i];        


            if (i + 6 < faceCount)
            {
                CameraCR[i] = new GameObject();
                CameraFR[i] = new GameObject();

                CameraCR[i].name = "CameraC_" + cubemapFaceNames[k + 6];
                CameraFR[i].name = "CameraF_" + cubemapFaceNames[k + 6];

                CameraCR[i].AddComponent<Camera>();
                CameraFR[i].AddComponent<Camera>();

                CameraCR[i].transform.parent = camerasCeiling.transform;
                CameraFR[i].transform.parent = camerasFloor.transform;
                CameraCR[i].transform.position = camerasCeiling.transform.position;
                CameraFR[i].transform.position = camerasFloor.transform.position;
                CameraCR[i].transform.eulerAngles = new Vector3(270, angle, 0);
                CameraFR[i].transform.eulerAngles = new Vector3(90, angle, 0);

                CameraCR[i].transform.Translate(cubemapCamPositions[k + 6], camerasCeiling.transform);
                CameraFR[i].transform.Translate(cubemapCamPositions[k + 6], camerasFloor.transform);

                c = new Camera[] { CameraCR[i].GetComponent<Camera>(), CameraFR[i].GetComponent<Camera>() };
                for (int j = 0; j < c.Length; j++)
                {
                    if (CeilingFloorStereoOption == Ceiling_Floor_StereoOption.All)
                    {
                        c[j].aspect = 2;
                        c[j].fieldOfView = aperture / 2;
                    }
                    else
                    {
                        c[j].aspect = 1;
                        c[j].fieldOfView = aperture;
                    }

                    c[j].nearClipPlane = near;
                    c[j].farClipPlane = far;
                    c[j].cullingMask = ~((1 << leftLayer) | (1 << rightLayer) | DoNotDisplayCullingMask);
                    c[j].enabled = true;
                }

                c[0].targetTexture = renderTexturesCeiling[i + 4];
                c[1].targetTexture = renderTexturesFloor[i + 4];
            }

            k++;
            angle += 180;
            angle %= 360;
        }
        angle = 0;
        //Check if OVRPlayer or OVRCamRig is attached to the Game Object
        //if so, enable Oculus Rift support
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
    /*
     * Returns asymmetric projection matrix for left and right eyes 
     */
    Matrix4x4[] CalculateProjectionMatrix(float near, float far, float aperture, float eyeSep, float focal_length, int op=0) //op: 0=not 4way stereo, 1= 4way stereo ceiling, 2= 4way stereo floor
    {  
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

        /*if (op != 0) {
            //Left Eye
            l_L =2* -wid_div_2 + shift;
            r_L =2* wid_div_2 + shift;
            //Right Eye
            l_R =2* -wid_div_2 - shift;
            r_R =2* wid_div_2 - shift;
        }*/
        if (op==1) {
            top = 0;
            //bot *= 2;
        }
        else if (op == 2) {
            bot = 0;
            //top *= 2;
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
    /*
     * Init the camera's projection matrix with correct stereo option
     */
    void CameraProjectionInit()
    {

        Matrix4x4[] mat = CalculateProjectionMatrix(near, far, aperture, eyeSep/2, focal_length);
        Matrix4x4[] matC, matF;
        if (CeilingFloorStereoOption == Ceiling_Floor_StereoOption.All)
        {
            matC = CalculateProjectionMatrix(near, far, aperture/*/2*/, eyeSep/2, focal_length, 1);
            matF = CalculateProjectionMatrix(near, far, aperture/*/2*/, eyeSep/2, focal_length, 2);
        }

        else {
            matC = CalculateProjectionMatrix(near, far, aperture, eyeSep/2, focal_length);
            matF = CalculateProjectionMatrix(near, far, aperture, eyeSep/2, focal_length);
        }
            
        for (int i = 0; i < Math.Min(CameraL.Length, 4); i++)
        {
            CameraL[i].GetComponent<Camera>().projectionMatrix = mat[0];
            //if (i == 1)
            //    i += 2;
        }

        for (int i = 0; i < CameraCL.Length; i++) {
            CameraCL[i].GetComponent<Camera>().projectionMatrix = matC[0];
            CameraFL[i].GetComponent<Camera>().projectionMatrix = matF[0];
        }

        for (int i = 0; i < Math.Min(CameraR.Length, 4); i++)
        {
            CameraR[i].GetComponent<Camera>().projectionMatrix = mat[1];
            //if (i == 1)
            //    i += 2;
        }

        for (int i = 0; i < CameraCR.Length; i++)
        {
            CameraCR[i].GetComponent<Camera>().projectionMatrix = matC[1];
            CameraFR[i].GetComponent<Camera>().projectionMatrix = matF[1];
        }

           
    }
    //enables or disables unnecessary planes
    void enablePlanes(bool enable) {
        cube.SetActive(enable);
        if (enable || CeilingFloorStereoOption != Ceiling_Floor_StereoOption.All)
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
    //Configures the Planes and Cameras for 1 directional stereo
    void OneWayStereoPlaneCameraSetUp(int side) {
        if (CameraL.Length > 4)
        {
            CameraL[4].SetActive(false);
        }
        if (CameraR.Length > 4)
        {
            CameraR[4].SetActive(false);
        }
        if (CameraL.Length > 5)
        {
            CameraL[5].SetActive(false); 
        }
        if (CameraR.Length > 5)
        {
            CameraR[5].SetActive(false); 
        }

        for (int i = 0; i < CameraCL.Length; i++)
        {
            CameraCL[i].SetActive(false);  
            CameraFL[i].SetActive(false);           
        }
        for (int i = 0; i < CameraCR.Length; i++)
        {
            CameraCR[i].SetActive(false);
            CameraFR[i].SetActive(false);
        }
        if (side < CameraCL.Length)
        {
            CameraCL[side].SetActive(true);
            CameraFL[side].SetActive(true);
            CameraCL[side].transform.eulerAngles = new Vector3(270, 0, 0);
            CameraFL[side].transform.eulerAngles = new Vector3(90, 0, 0);
        }
        if (side < CameraCR.Length)
        {
            CameraCR[side].SetActive(true);
            CameraFR[side].SetActive(true);
            CameraCR[side].transform.eulerAngles = new Vector3(270, 0, 0);
            CameraFR[side].transform.eulerAngles = new Vector3(90, 0, 0);
        }
        
        if (disablePlanes)
        {
            enablePlanes(false);
            return;
        }

        for (int i = 0; i < 8; i++)
        {
            if (planeCeiling_4Way[i] != null)
            {
                planeCeiling_4Way[i].SetActive(false);
                planeFloor_4Way[i].SetActive(false);
            }
        }
        Vector3 v= CameraCL[side].transform.eulerAngles;
        v.x=0;
        v.z = 180;
        for (int i = 8; i < 12; i++)
        {
            if (planes[i] == null)
                break;
            planes[i].SetActive(true);
            if (i % 2 == 0){
                planes[i].GetComponent<Renderer>().material.mainTexture = renderTexturesCeiling[side];
                planes[i].transform.eulerAngles = v;
            }
                
            else
            {
                planes[i].GetComponent<Renderer>().material.mainTexture = renderTexturesFloor[side];
                side += 4;
            }

        }
        
    }
    //Configures the Planes and Cameras in accordance to stereo option
    void Plane_CameraSetUp() { 
        switch(CeilingFloorStereoOption){
            case Ceiling_Floor_StereoOption.None:
                if (CameraL.Length > 4)
                {
                    CameraL[4].SetActive(true);
                    CameraL[4].GetComponent<Camera>().cullingMask = ~((1 << leftLayer) | (1 << rightLayer) | DoNotDisplayCullingMask);
                }
                if (CameraR.Length > 4)
                {
                    CameraR[4].SetActive(true);
                    CameraR[4].GetComponent<Camera>().cullingMask = ~((1 << leftLayer) | (1 << rightLayer) | DoNotDisplayCullingMask);
                }
                if (CameraL.Length > 5)
                {
                    CameraL[5].SetActive(true);
                    CameraL[5].GetComponent<Camera>().cullingMask = ~((1 << leftLayer) | (1 << rightLayer) | DoNotDisplayCullingMask);
                }
                if (CameraR.Length > 5)
                {
                    CameraR[5].SetActive(true);
                    CameraR[5].GetComponent<Camera>().cullingMask = ~((1 << leftLayer) | (1 << rightLayer) | DoNotDisplayCullingMask);
                }

                for (int i = 0; i < CameraCL.Length; i++)
                {
                    CameraCL[i].SetActive(false);
                    CameraFL[i].SetActive(false);
                }
                for (int i = 0; i < CameraCR.Length; i++)
                {
                    CameraCR[i].SetActive(false);
                    CameraFR[i].SetActive(false);
                }

                if (disablePlanes)
                {
                    enablePlanes(false);
                    break;
                }

                for (int i = 0; i < 8; i++)
                {
                    if (planeCeiling_4Way[i] != null)
                    {
                        planeCeiling_4Way[i].SetActive(false);
                        planeFloor_4Way[i].SetActive(false);
                    }
                }

                for (int i = 8; i < 12; i++)
                {
                    if (planes[i] == null)
                        break;
                    planes[i].SetActive(true);
                }
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
                if (CameraL.Length > 4)
                {
                    CameraL[4].SetActive(true);
                    CameraL[4].GetComponent<Camera>().cullingMask = 1 << leftLayer;
                }
                if (CameraR.Length > 4)
                {
                    CameraR[4].SetActive(true);
                    CameraR[4].GetComponent<Camera>().cullingMask = 1 << rightLayer;
                }
                if (CameraL.Length > 5)
                {
                    CameraL[5].SetActive(true);
                    CameraL[5].GetComponent<Camera>().cullingMask = 1 << leftLayer;
                }
                if (CameraR.Length > 5)
                {
                    CameraR[5].SetActive(true);
                    CameraR[5].GetComponent<Camera>().cullingMask = 1 << rightLayer;
                }

                for (int i = 0; i < CameraCL.Length; i++)
                {
                    CameraCL[i].SetActive(true);
                    CameraFL[i].SetActive(true);
                }
                for (int i = 0; i < CameraCR.Length; i++)
                {
                    CameraCR[i].SetActive(true);
                    CameraFR[i].SetActive(true);
                }

                if (disablePlanes)
                {
                    enablePlanes(false);
                    break;
                }

                if (faceCount < 5)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        if (planeCeiling_4Way[i] != null)
                        {
                            planeCeiling_4Way[i].SetActive(false);

                        }
                    }
                }
                if (faceCount < 6)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        if (planeCeiling_4Way[i] != null)
                        {
                            planeFloor_4Way[i].SetActive(false);
                        }
                    }
                }
                if (faceCount < 10)
                {
                    for (int i = 4; i < 8; i++)
                    {
                        if (planeCeiling_4Way[i] != null)
                        {
                            planeCeiling_4Way[i].SetActive(false);

                        }
                    }
                }
                if (faceCount < 11)
                {
                    for (int i = 4; i < 8; i++)
                    {
                        if (planeCeiling_4Way[i] != null)
                        {
                            planeFloor_4Way[i].SetActive(false);
                        }
                    }
                }

                for (int i = 8; i < 12; i++)
                {
                    if (planes[i] == null)
                        break;
                    planes[i].SetActive(false);
                }

                break;

        }
    }
    //Cubemap extraction setup for 1 directional stereo
    void OneWayStereoExtractionSetUp(int side) {

        for (int i = 0; i < Math.Min(faceCount, 12); i++)
        {
            if (i != 4 || i != 5 || i != 10 || i != 11) //if not ceiling or floor
            {
                inTextures[i] = renderTextures[i];
            }
                
            if (i == 4 || i == 5)
            {
                inTextures[i] = renderTexturesCeiling[side];
            }
            if (i == 10 || i == 11)
            {
                inTextures[i] = renderTexturesCeiling[side + 4];
            }
        }
    }
    //Cubemap extraction setup
    void ExtractionSetUp() {
        inTextures  = new RenderTexture[faceCount];
        switch (CeilingFloorStereoOption) { 
            case Ceiling_Floor_StereoOption.None:
            case Ceiling_Floor_StereoOption.All:
                for (int i = 0; i < Math.Min(faceCount, 12); i++)
                {
                    inTextures[i] = renderTextures[i];
                }
                break;
            case Ceiling_Floor_StereoOption.PositiveX:
                OneWayStereoExtractionSetUp(0);
                break;
            case Ceiling_Floor_StereoOption.NegativeX:
                OneWayStereoExtractionSetUp(1);
                break;
            case Ceiling_Floor_StereoOption.PositiveZ:
                OneWayStereoExtractionSetUp(2);
                break;
            case Ceiling_Floor_StereoOption.NegativeZ:
                OneWayStereoExtractionSetUp(3);
                break;
        }
        
        
    }
    IEnumerator Start()
    {
        if (fps != -1)
        {
            // VSync must be disabled. However QualitySettings.vSyncCount = 0; crashes app in player mode.
            // So, make sure Edit > Project Settings > Quality > V Sync Count is set to "Do't Sync"
            if (QualitySettings.vSyncCount != 0)
            {
                Debug.LogError("Set Edit > Project Settings > Quality > V Sync Count to \"Do't Sync\". Otherwise, FPS limit will no have any effect.");
            }
            Application.targetFrameRate = fps;
        }

        // Setup convert shader
        shader = Resources.Load("AlloUnity/ConvertRGBtoYUV420p") as ComputeShader;
        

        PlaneInit();
        CameraInit();

        Plane_CameraSetUp();
        ExtractionSetUp();

        System.IntPtr[] texturePtrs = new System.IntPtr[faceCount];
        for (int i = 0; i < Math.Min(faceCount, 12); i++)
        {
            texturePtrs[i] = outTextures[i].GetNativeTexturePtr();
        }
        
        //System.IntPtr[] texturePtrs=ExtractionSetUp();


        // Tell native plugin that rendering has started
        ConfigureCubemapFromUnity(texturePtrs, faceCount, resolution, resolution);
        didPlay = true;

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
        if (Input.GetKey(KeyCode.I))//cubemap moves foward
        {
            angleY = getAngle();
            transform.Translate(new Vector3(Mathf.Sin(angleY), 0, Mathf.Cos(angleY)) * moveSpeed*Time.deltaTime);
        }
        else if (Input.GetKey(KeyCode.L))//cubemap moves rightward
        {
            angleY = getAngle();
            transform.Translate(new Vector3(Mathf.Cos(angleY), 0, -Mathf.Sin(angleY)) * moveSpeed*Time.deltaTime);
        }
        else if (Input.GetKey(KeyCode.J))//cubema moves leftward
        {
            angleY = getAngle();
            transform.Translate(new Vector3(-Mathf.Cos(angleY), 0, Mathf.Sin(angleY)) * moveSpeed*Time.deltaTime);
        }
        else if (Input.GetKey(KeyCode.K))//cubemap moves backward
        {
            angleY = getAngle();
            transform.Translate(new Vector3(-Mathf.Sin(angleY), 0, -Mathf.Cos(angleY)) * moveSpeed*Time.deltaTime);
        }
        else if (Input.GetKey(KeyCode.U))//cubemap moves upward
        {
            transform.Translate(new Vector3(0, 1, 0) * moveSpeed*Time.deltaTime);
        }
        else if (Input.GetKey(KeyCode.O)) //cubemap moves downward
        {
            transform.Translate(new Vector3(0, -1, 0) * moveSpeed*Time.deltaTime);
        }
        else if (OVRPlayer != null) {
            if (Input.GetKey(KeyCode.W))//player moves foward
            {
                angleY = getAngle();
                OVRPlayer.transform.Translate(new Vector3(Mathf.Sin(angleY), 0, Mathf.Cos(angleY)) * moveSpeed * Time.deltaTime);
            }
            else if (Input.GetKey(KeyCode.D))//player moves rightward
            {
                angleY = getAngle();
                OVRPlayer.transform.Translate(new Vector3(Mathf.Cos(angleY), 0, -Mathf.Sin(angleY)) * moveSpeed * Time.deltaTime);
            }
            else if (Input.GetKey(KeyCode.A)) //player moves leftward
            {
                angleY = getAngle();
                OVRPlayer.transform.Translate(new Vector3(-Mathf.Cos(angleY), 0, Mathf.Sin(angleY)) * moveSpeed * Time.deltaTime);
            }
            else if (Input.GetKey(KeyCode.S))//player moves backward
            {
                angleY = getAngle();
                OVRPlayer.transform.Translate(new Vector3(-Mathf.Sin(angleY), 0, -Mathf.Cos(angleY)) * moveSpeed * Time.deltaTime);
            }
            else if (Input.GetKey(KeyCode.Z) ||Input.GetKey(KeyCode.UpArrow))//player moves upward
            {
                OVRPlayer.transform.Translate(new Vector3(0, 1, 0) * moveSpeed * Time.deltaTime);
            }
            else if (Input.GetKey(KeyCode.X) || Input.GetKey(KeyCode.DownArrow))//player moves downward
            {
                OVRPlayer.transform.Translate(new Vector3(0, -1, 0) * moveSpeed * Time.deltaTime);
            }
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
                if (SystemInfo.graphicsDeviceVersion.StartsWith("Direct3D"))
                {
                    for (int i = 0; i < faceCount; i++)
                    {
                        RenderTexture inTex = inTextures[i];
                        RenderTexture outTex = outTextures[i];
                        
                        shader.SetInt("Width", resolution);
                        shader.SetInt("Height", resolution);
                        shader.SetTexture(shader.FindKernel("Convert"), "In", inTex);
                        shader.SetTexture(shader.FindKernel("Convert"), "Out", outTex);
                        shader.Dispatch(shader.FindKernel("Convert"), (resolution / 8) * (resolution / 2), 1, 1);
                    }
                }

                GL.IssuePluginEvent(1);
            }
        }
    }
}
