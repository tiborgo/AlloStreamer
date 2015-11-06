using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System.Threading;

/// MouseLook rotates the transform based on the mouse delta.
/// Minimum and Maximum values can be used to constrain the possible rotation

public class GuideHelicopter : MonoBehaviour
{

    [DllImport("HelicopterControlPlugin")]
    private static extern float getRecentX();

    [DllImport("HelicopterControlPlugin")]
    private static extern float getRecentY();

    [DllImport("HelicopterControlPlugin")]
    private static extern float getSubmitX();

    [DllImport("HelicopterControlPlugin")]
    private static extern float getSubmitY();

    [DllImport("HelicopterControlPlugin")]
    private static extern float endServer();

    public GameObject marker;

    void Update()
    {
        float recentX = getRecentX();
        float recentY = getRecentY();
        float submitX = getSubmitX();
        float submitY = getSubmitY();
        Debug.Log ("(" + recentX + "," + recentY + ") -> (" + submitX + "," + submitY + ")");
        if (marker != null)
        {
            marker.transform.position = new Vector3(recentX, marker.transform.position.y, -recentY);
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
