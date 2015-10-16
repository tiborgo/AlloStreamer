using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Reflection;


public class RenderBinoculars : MonoBehaviour
{

    private RenderTexture stereoTexture;
    private RenderTexture leftEyeTexture;
    private RenderTexture rightEyeTexture;
    private Material leftEyeMaterial;
    private Material rightEyeMaterial;
    private GameObject leftEyeCamera;
    private GameObject rightEyeCamera;
    private Camera leftEyeCameraCam;
    private Camera rightEyeCameraCam;

    [SerializeField]
    [HideInInspector]
    private float fieldOfView = 90f;

    [SerializeField]
    [HideInInspector]
    private float eyeSeparation = 0.128f;

    [ExposeProperty]
    public float FieldOfView
    {
        get
        {
            return fieldOfView;
        }
        set
        {
            if (value >= 0.0f && value <= 179f)
            {
                if (leftEyeCameraCam && rightEyeCameraCam)
                {
                    leftEyeCameraCam.fieldOfView = value;
                    rightEyeCameraCam.fieldOfView = value;
                }
                fieldOfView = value;
            }
        }
    }

    [ExposeProperty]
    public float EyeSeparation
    {
        get
        {
            return eyeSeparation;
        }
        set
        {
            if (value >= 0.0f)
            {
                if (leftEyeCamera && rightEyeCamera)
                {
                    leftEyeCamera.transform.localPosition = new Vector3(-value / 2, 0f, 0f);
                    rightEyeCamera.transform.localPosition = new Vector3(value / 2, 0f, 0f);
                }
                eyeSeparation = value;
            }
        }
    }

    // Use this for initialization
    void Start()
    {
        Debug.Log("start");

        // Setup resources (materials, render textures) for binoculars
        stereoTexture = new RenderTexture(1024, 1024, 0, RenderTextureFormat.ARGB32);
        leftEyeTexture = new RenderTexture(1024, 1024, 24, RenderTextureFormat.ARGB32);
        rightEyeTexture = new RenderTexture(1024, 1024, 24, RenderTextureFormat.ARGB32);
        leftEyeMaterial = new Material(Shader.Find("Unlit/Texture"));
        leftEyeMaterial.mainTexture = leftEyeTexture;
        rightEyeMaterial = new Material(Shader.Find("Unlit/Texture"));
        rightEyeMaterial.mainTexture = rightEyeTexture;

        // Setup GameObjects for binoculars
        GameObject binoculars = new GameObject("Binoculars");
        binoculars.transform.parent = transform;
        binoculars.transform.localPosition = Vector3.zero;

        GameObject distanceSphere = GameObject.CreatePrimitive(PrimitiveType.Sphere);
        distanceSphere.transform.parent = binoculars.transform;
        distanceSphere.name = "DistanceSphere";
        Material dinstanceSphereMat = new Material(Shader.Find("Unlit/Color"));
        dinstanceSphereMat.color = new Color(1.0f, 0, 0, 0.5f);
        distanceSphere.GetComponent<MeshRenderer>().material = dinstanceSphereMat;
        // Remove the collider of the sphere. Otherwise distance measurmen would interfere with the sphere.
        GameObject.Destroy(distanceSphere.GetComponent<SphereCollider>());

        GameObject rendererStereo = new GameObject("RendererStereo");
        rendererStereo.transform.parent = binoculars.transform;
        rendererStereo.transform.localPosition = new Vector3(0f, 0f, 0f);
        rendererStereo.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        GameObject camera = new GameObject("Camera");
        camera.transform.parent = rendererStereo.transform;
        camera.transform.localPosition = new Vector3(0f, 0f, 0.1f);
        camera.transform.localRotation = Quaternion.Euler(0f, 180f, 0f);

        camera.AddComponent<UseRenderingPlugin>();

        Camera cameraCam = camera.AddComponent<Camera>();
        cameraCam.clearFlags = CameraClearFlags.SolidColor;
        cameraCam.backgroundColor = Color.black;
        cameraCam.orthographic = true;
        cameraCam.cullingMask = 1 << 8;
        cameraCam.orthographicSize = 0.88f;
        cameraCam.nearClipPlane = 0.01f;
        cameraCam.farClipPlane = 0.11f;
        cameraCam.targetTexture = stereoTexture;

        GameObject leftEyeMesh = new GameObject("LeftEyeMesh");
        leftEyeMesh.transform.parent = rendererStereo.transform;
        leftEyeMesh.transform.localPosition = new Vector3(0.45f, -0.008f, 0f);
        leftEyeMesh.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);
        leftEyeMesh.layer = 8;

        MeshRenderer meshRenderer = leftEyeMesh.AddComponent<MeshRenderer>();
        meshRenderer.material = leftEyeMaterial;
        MeshFilter meshFilter = leftEyeMesh.AddComponent<MeshFilter>();
        MxrPredistortionMesh mxrPredistortionMesh = leftEyeMesh.AddComponent<MxrPredistortionMesh>();
        mxrPredistortionMesh.resolutionX = 50;
        mxrPredistortionMesh.resolutionY = 50;

        GameObject rightEyeMesh = new GameObject("RightEyeMesh");
        rightEyeMesh.transform.parent = rendererStereo.transform;
        rightEyeMesh.transform.localPosition = new Vector3(-0.45f, -0.008f, 0f);
        rightEyeMesh.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);
        rightEyeMesh.layer = 8;

        MeshRenderer meshRenderer2 = rightEyeMesh.AddComponent<MeshRenderer>();
        meshRenderer2.material = rightEyeMaterial;
        MeshFilter meshFilter2 = rightEyeMesh.AddComponent<MeshFilter>();
        MxrPredistortionMesh mxrPredistortionMesh2 = rightEyeMesh.AddComponent<MxrPredistortionMesh>();
        mxrPredistortionMesh2.resolutionX = 50;
        mxrPredistortionMesh2.resolutionY = 50;

        GameObject virtualCameraStereo = new GameObject("VirtualCameraStereo");
        virtualCameraStereo.transform.parent = binoculars.transform;
        virtualCameraStereo.transform.localPosition = new Vector3(0f, 0f, 0f);
        virtualCameraStereo.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        LineRenderer lineRenderer = virtualCameraStereo.AddComponent<LineRenderer>();
        lineRenderer.material = new Material(Shader.Find("Particles/Additive (Soft)"));
        lineRenderer.SetColors(new Color(1.0f, 0, 0, 0.5f), new Color(1.0f, 0, 0, 0.5f));
        lineRenderer.SetWidth(0.1f, 0.1f);
        lineRenderer.SetVertexCount(2);

        virtualCameraStereo.AddComponent<MouseLook>();

        GameObject hmd = new GameObject("HMD");
        hmd.transform.parent = virtualCameraStereo.transform;
        hmd.transform.localPosition = new Vector3(0f, 0f, 0f);
        hmd.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        leftEyeCamera = new GameObject("LeftEyeCamera");
        leftEyeCamera.transform.parent = hmd.transform;
        leftEyeCamera.transform.localPosition = new Vector3(-eyeSeparation / 2, 0f, 0f);
        leftEyeCamera.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        leftEyeCameraCam = leftEyeCamera.AddComponent<Camera>();
        leftEyeCameraCam.targetTexture = leftEyeTexture;
        leftEyeCameraCam.fieldOfView = fieldOfView;

        rightEyeCamera = new GameObject("RightEyeCamera");
        rightEyeCamera.transform.parent = hmd.transform;
        rightEyeCamera.transform.localPosition = new Vector3(eyeSeparation / 2, 0f, 0f);
        rightEyeCamera.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        rightEyeCameraCam = rightEyeCamera.AddComponent<Camera>();
        rightEyeCameraCam.targetTexture = rightEyeTexture;
        rightEyeCameraCam.fieldOfView = fieldOfView;
    }
}