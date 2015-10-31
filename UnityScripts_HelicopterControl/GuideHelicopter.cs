using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System.Threading;

/// MouseLook rotates the transform based on the mouse delta.
/// Minimum and Maximum values can be used to constrain the possible rotation

public class GuideHelicopter : MonoBehaviour
{

    [DllImport("HelicopterControlPlugin")]
    private static extern float getStartX();

    [DllImport("HelicopterControlPlugin")]
    private static extern float getStartY();

    [DllImport("HelicopterControlPlugin")]
    private static extern float getEndX();

    [DllImport("HelicopterControlPlugin")]
    private static extern float getEndY();

    [DllImport("HelicopterControlPlugin")]
    private static extern float endServer();

    public GameObject marker;

    void Update()
    {
        float startX = getStartX();
        float startY = getStartY();
        float endX = getEndX();
        float endY = getEndY();
        //Debug.Log ("(" + startX + "," + startY + ") -> (" + endX + "," + endY + ")");
        if (marker != null)
        {
            marker.transform.position = new Vector3(startX, marker.transform.position.y, -startY);
        }
    }

    void Awake()
    {
        OSCHelicopterThread oscClientHelicopter = new OSCHelicopterThread();
        Thread osctHelicopter = new Thread(new ThreadStart(oscClientHelicopter.startServer));
        osctHelicopter.Start();

        OSCHelicopterThreadPhaseSpace oscPhaseHelicopter = new OSCHelicopterThreadPhaseSpace();
        Thread osctPhaseHelicopter = new Thread(new ThreadStart(oscPhaseHelicopter.startServer));
        osctPhaseHelicopter.Start();

    }

    void OnApplicationQuit()
    {
        endServer();
    }
}
