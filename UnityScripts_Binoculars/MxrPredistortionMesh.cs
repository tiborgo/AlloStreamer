//Open source predistortion code for LEEP optics and aspheric lenses
//University of Southern California Irvine
//Institute for Creative Technologies
//Mixed Reality Lab
//
//Contributors:
//Evan Suma, PhD
//Thai Phan
//Bradley Newman

using UnityEngine;
using System.Collections;

public class MxrPredistortionMesh : MonoBehaviour 
{
    public int resolutionX = 0;
    public int resolutionY = 0;
    public float sizeX = 1.0f;
    public float sizeY = 1.0f;

    public Material grid;
    public Material EyeMat;
	private bool renderGrid = false;

    
    public float PreDistortK {
        get { return preDistortionConstant; }
        set { preDistortionConstant = value; }
    }

    private Vector3[] originalVertices;

	//Uncomment these variables if you need to
	//tweak the amount of pre-distortion
	//using hotkeys
	/*
	private int adjustConstant = 0;
	private float adjustmentValue = .001f; 
	*/  

	// Use this for initialization
	void Start () 
    {
        RebuildMesh();
	}
	
	// Update is called once per frame
	void Update () {
		//Uncomment the section of code below
		//to enable hotkeys to tweak the
		//amount of predistortion for both eyes
        /*
        if (Input.GetKeyDown("["))
        {
            adjustConstant = -1;
        }
        else if (Input.GetKeyDown("]"))
        {
            adjustConstant = 1;
        }
        else if (Input.GetKeyUp("[") || Input.GetKeyUp("]"))
        {
            adjustConstant = 0;
        }

        if (adjustConstant != 0)
        {
            preDistortionConstant += adjustmentValue * adjustConstant;
            UpdateMesh();
        }

        if (Input.GetKeyDown("p"))
        {
            Debug.Log("Predistortion constant: " + preDistortionConstant);
        }
        */

        if (Input.GetKeyDown(KeyCode.G)) {
            renderGrid = !renderGrid;
            if (renderGrid)
                this.GetComponent<MeshRenderer>().material = grid;
            else
                this.GetComponent<MeshRenderer>().material = EyeMat;
        }
	}
	float DC = 0f;
	private float preDistortionConstant = -0.577f;
    void UpdateMesh()
    {
		
//		Mesh mesh = gameObject.GetComponent<MeshFilter>().mesh;
//        Vector3[] vertices = mesh.vertices;
//
//        int numVerticesX = resolutionX + 1;
//        int numVerticesY = resolutionY + 1;
//		
//		
//
//        for (int i = 0; i < numVerticesY; i++)
//        {
//            for (int j = 0; j < numVerticesX; j++)
//            {
//                int currentIndex = i * numVerticesX + j;
//                float xvn = originalVertices[currentIndex].x;
//                float yvn = originalVertices[currentIndex].y;
//				
//				if(i > numVerticesY/2)
//                	vertices[currentIndex].y = yvn + yvn*Mathf.Pow((i/(numVerticesY*1.0f)),4)*DC;
//				else
//					vertices[currentIndex].y = yvn + yvn*Mathf.Pow(((numVerticesY-i)/(numVerticesY*1.0f)),3)*DC;
//				
//				vertices[currentIndex].x = xvn + xvn*Mathf.Pow(((numVerticesX-j)/(numVerticesX*1.0f)),2);
//			
//							
//                //vertices[currentIndex].y = 
//            }
//        }
//
//        mesh.vertices = vertices;
		
		
        Mesh mesh = gameObject.GetComponent<MeshFilter>().mesh;
        Vector3[] vertices = mesh.vertices;

        int numVerticesX = resolutionX + 1;
        int numVerticesY = resolutionY + 1;

        for (int i = 0; i < numVerticesY; i++)
        {
            for (int j = 0; j < numVerticesX; j++)
            {
                int currentIndex = i * numVerticesX + j;
                float xvn = originalVertices[currentIndex].x;
                float yvn = originalVertices[currentIndex].y;

                vertices[currentIndex].x = xvn + preDistortionConstant * xvn * (xvn * xvn + yvn * yvn);
                vertices[currentIndex].y = yvn + preDistortionConstant * yvn * (xvn * xvn + yvn * yvn);
            }
        }

        mesh.vertices = vertices;
    }

    public void RebuildMesh () {
        float vertexDistanceX = sizeX / resolutionX;
        float vertexDistanceY = sizeY / resolutionY;
        int numVerticesX = resolutionX + 1;
        int numVerticesY = resolutionY + 1;

        int numVertices = numVerticesX * numVerticesY;
        Vector3[] vertices = new Vector3[numVertices];
        Vector2[] uvs = new Vector2[numVertices];
        Vector3[] normals = new Vector3[numVertices];

        // generate vertices and normals
        for (int i = 0; i < numVerticesY; i++)
        {
            for (int j = 0; j < numVerticesX; j++)
            {
                int index = i * numVerticesX + j;
                vertices[index] = new Vector3(j * vertexDistanceX - sizeX / 2, i * vertexDistanceY - sizeY / 2, 0);
                uvs[index] = new Vector2(1f - ((j * vertexDistanceX) / sizeX), (i * vertexDistanceY) / sizeY);
                normals[index] = new Vector3(0, 0, -1);
            }
        }


        // generate triangles
        int[] triangles = new int[resolutionX * resolutionY * 6];
        int triangleIndex = 0;
        for (int i = 0; i < numVerticesY - 1; i++)
        {
            for (int j = 0; j < numVerticesX - 1; j++)
            {
                int currentIndex = i * numVerticesX + j;
                // upper left triangle
                triangles[triangleIndex] = currentIndex;
                triangles[triangleIndex + 1] = currentIndex + 1;
                triangles[triangleIndex + 2] = currentIndex + numVerticesY;

                triangleIndex += 3;

                // lower right triangle
                triangles[triangleIndex] = currentIndex + numVerticesY + 1;
                triangles[triangleIndex + 1] = currentIndex + numVerticesY;
                triangles[triangleIndex + 2] = currentIndex + 1;

                triangleIndex += 3;
            }
        }


        Mesh mesh = gameObject.GetComponent<MeshFilter>().mesh;
        mesh.vertices = vertices;
        mesh.normals = normals;
        mesh.uv = uvs;
        mesh.triangles = triangles;

        originalVertices = vertices;

        UpdateMesh();
    }
}