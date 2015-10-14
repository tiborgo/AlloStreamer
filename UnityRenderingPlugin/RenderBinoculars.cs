using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;
using System.Reflection;



public class RenderBinoculars : MonoBehaviour
{

    // Use this for initialization
    void Start()
    {

        GameObject binoculars = new GameObject("Binoculars");
        binoculars.transform.parent = transform;
        binoculars.transform.localPosition = Vector3.zero;

        GameObject rendererStereo = new GameObject("RendererStereo");
        rendererStereo.transform.parent = binoculars.transform;
        rendererStereo.transform.localPosition = new Vector3(-320.953f, -9.005943f, -136.6786f);
        rendererStereo.transform.localRotation = Quaternion.Euler(0f, 164.2724f, 0f);

        GameObject camera = new GameObject("Camera");
        camera.transform.parent = rendererStereo.transform;
        camera.transform.localPosition = new Vector3(0f, 0f, 0.1f);
        camera.transform.localRotation = Quaternion.Euler(0f, 180f, 0f);

        camera.AddComponent<UseRenderingPlugin>();

        Camera cameraCam = camera.AddComponent<Camera>();
        cameraCam.clearFlags = CameraClearFlags.SolidColor;
        cameraCam.backgroundColor = Color.black;
        cameraCam.cullingMask = 1 << 8;
        cameraCam.orthographicSize = 0.88f;
        cameraCam.nearClipPlane = 0.01f;
        cameraCam.farClipPlane = 0.11f;
        cameraCam.targetTexture = Resources.Load("Binoculars/StereoTexture", typeof(RenderTexture)) as RenderTexture;

        GameObject leftEyeMesh = new GameObject("LeftEyeMesh");
        leftEyeMesh.transform.parent = rendererStereo.transform;
        leftEyeMesh.transform.localPosition = new Vector3(0.45f, -0.008f, 0f);
        leftEyeMesh.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        MeshRenderer meshRenderer = leftEyeMesh.AddComponent<MeshRenderer>();
        meshRenderer.material = Resources.Load("Binoculars/LeftEyeMaterial", typeof(Material)) as Material;
        MeshFilter meshFilter = leftEyeMesh.AddComponent<MeshFilter>();
        MxrPredistortionMesh mxrPredistortionMesh = leftEyeMesh.AddComponent<MxrPredistortionMesh>();
        mxrPredistortionMesh.resolutionX = 50;
        mxrPredistortionMesh.resolutionY = 50;

        GameObject rightEyeMesh = new GameObject("RightEyeMesh");
        rightEyeMesh.transform.parent = rendererStereo.transform;
        rightEyeMesh.transform.localPosition = new Vector3(-0.45f, -0.008f, 0f);
        rightEyeMesh.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        MeshRenderer meshRenderer2 = rightEyeMesh.AddComponent<MeshRenderer>();
        meshRenderer2.material = Resources.Load("Binoculars/RightEyeMaterial", typeof(Material)) as Material;
        MeshFilter meshFilter2 = rightEyeMesh.AddComponent<MeshFilter>();
        MxrPredistortionMesh mxrPredistortionMesh2 = rightEyeMesh.AddComponent<MxrPredistortionMesh>();
        mxrPredistortionMesh2.resolutionX = 50;
        mxrPredistortionMesh2.resolutionY = 50;

        GameObject virtualCameraStereo = new GameObject("VirtualCameraStereo");
        virtualCameraStereo.transform.parent = binoculars.transform;
        virtualCameraStereo.transform.localPosition = new Vector3(-320.953f, -15.1f, -134.3123f);
        virtualCameraStereo.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        GameObject hmd = new GameObject("HMD");
        hmd.transform.parent = virtualCameraStereo.transform;
        hmd.transform.localPosition = new Vector3(0f, 0f, 0f);
        hmd.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        GameObject leftEyeCamera = new GameObject("LeftEyeCamera");
        leftEyeCamera.transform.parent = hmd.transform;
        leftEyeCamera.transform.localPosition = new Vector3(0f, 0f, 0f);
        leftEyeCamera.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);

        GameObject rightEyeCamera = new GameObject("RightEyeCamera");
        rightEyeCamera.transform.parent = hmd.transform;
        rightEyeCamera.transform.localPosition = new Vector3(0f, 0f, 0f);
        rightEyeCamera.transform.localRotation = Quaternion.Euler(0f, 0f, 0f);
    }
}