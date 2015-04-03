using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class QT_TrafficLightController : MonoBehaviour {
	public List<QT_TrafficLight> GreenLights,RedLights,BlinkingLights;
	public float StayOnGreenTime=10.0f;
	public float StayOnYellowTime=4.0f;
	public float RedToGreenTime=4.0f;
	public float BlinkTime=2.0f;
	public byte nothin;
	public bool ShowLinks=false;
	public float GlobalTimeOffset=0.0f;
	public bool UseDynamicLights=false;
	private List<int> greenindices = new List<int>();
	private List<int> redindices= new List<int>();
	private List<int> yellowindices = new List<int>();// since the lists can include nulls, we store the indices of valid objects here.
	
	private bool swapSets = false;
	
	// Use this for initialization
	void Start () {
		UpdateIndices();
		SetupInitialBulbs();
		// you need to do error checks before the coroutines...
		
		if(greenindices.Count>0 && redindices.Count>0)
			StartCoroutine(DoLightLoop());
		if(yellowindices.Count>0)
			StartCoroutine(DoBlinkLoop());
	
	}
	
	private IEnumerator DoLightLoop()
	{
		yield return new WaitForSeconds(GlobalTimeOffset);
		
		while(true)
		{
			yield return new WaitForSeconds(StayOnGreenTime);
			
			if(swapSets==false) //if it's greenlights' turn to be green
			{
				foreach (int i in greenindices) //switch to yellowbulb
				{
					GreenLights[i].BulbGreen.SetActive(false);		
					GreenLights[i].BulbYellow.SetActive(true);
					if(UseDynamicLights)
					{
						GreenLights[i].Lights[0].SetActive(false);
						GreenLights[i].Lights[1].SetActive(true);
					}
				}
				yield return new WaitForSeconds(StayOnYellowTime);
				foreach(int i in greenindices) //switch to redbulb
				{					
					GreenLights[i].BulbYellow.SetActive(false);
					GreenLights[i].BulbRed.SetActive(true);
					if(UseDynamicLights)
					{
						GreenLights[i].Lights[1].SetActive(false);
						GreenLights[i].Lights[2].SetActive(true);
					}
				}
				yield return new WaitForSeconds(RedToGreenTime);
				foreach (int i in redindices)
				{
					RedLights[i].BulbRed.SetActive(false);
					RedLights[i].BulbGreen.SetActive(true);
					if(UseDynamicLights)
					{
						RedLights[i].Lights[2].SetActive(false);
						RedLights[i].Lights[0].SetActive(true);
					}
					
				}
				
			}
			else
			{
				foreach(int i in redindices)
				{
					RedLights[i].BulbGreen.SetActive(false);
					RedLights[i].BulbYellow.SetActive(true);
					if(UseDynamicLights)
					{
						RedLights[i].Lights[0].SetActive(false);
						RedLights[i].Lights[1].SetActive(true);
					}
				}
				yield return new WaitForSeconds(StayOnYellowTime);
				foreach(int i in redindices)
				{				
					RedLights[i].BulbYellow.SetActive(false);
					RedLights[i].BulbRed.SetActive(true);
					if(UseDynamicLights)
					{
						RedLights[i].Lights[1].SetActive(false);
						RedLights[i].Lights[2].SetActive(true);
					}
				}
				yield return new WaitForSeconds(RedToGreenTime);
				foreach(int i in greenindices)
				{
					GreenLights[i].BulbGreen.SetActive(true);
					GreenLights[i].BulbRed.SetActive(false);
					if(UseDynamicLights)
					{
						GreenLights[i].Lights[0].SetActive(true);
						GreenLights[i].Lights[2].SetActive(false);
					}
				}
			}
			
			swapSets = !swapSets;
		}
	}
	
	private IEnumerator DoBlinkLoop()
	{
		yield return new WaitForSeconds(GlobalTimeOffset);
		
		while(true)
		{
			yield return new WaitForSeconds(BlinkTime/2);
			foreach (int i in yellowindices)
			{
				BlinkingLights[i].BulbYellow.SetActive(false);
				if(UseDynamicLights)
					BlinkingLights[i].Lights[1].SetActive(false);
			}
			yield return new WaitForSeconds(BlinkTime/2);
			foreach (int i in yellowindices)
			{
				BlinkingLights[i].BulbYellow.SetActive(true);	
				if(UseDynamicLights)
					BlinkingLights[i].Lights[1].SetActive(true);
			}
			
			
		}
			
	}
	
	//updates the indices with index values for the lists.
	void UpdateIndices()
	{
		greenindices.Clear();
		redindices.Clear();
		yellowindices.Clear();
		
		
		for(int x=0;x<GreenLights.Count;x++)
		{
			if(GreenLights[x]!=null)
				greenindices.Add(x);
		}
		for(int x=0;x<BlinkingLights.Count;x++)
		{
			if(BlinkingLights[x]!=null)
				yellowindices.Add(x);
		}
		for(int x=0;x<RedLights.Count;x++)
		{
			if(RedLights[x]!=null)
				redindices.Add(x);
		}
	}
	
	void SetupInitialBulbs()
	{
		if(greenindices.Count>0) //if green, disable yellow and red.
		{
			foreach (int i in greenindices)
			{
				GreenLights[i].BulbRed.SetActive(false);
				GreenLights[i].BulbYellow.SetActive(false);
				foreach(GameObject l in GreenLights[i].Lights)
					l.SetActive(false);
				if(UseDynamicLights)
					GreenLights[i].Lights[0].SetActive(true);	
			}			
		}
		if(redindices.Count>0)
		{
			foreach (int i in redindices)
			{
				RedLights[i].BulbGreen.SetActive(false);
				RedLights[i].BulbYellow.SetActive(false);
				foreach(GameObject l in RedLights[i].Lights)
					l.SetActive(false);
				if(UseDynamicLights)
					RedLights[i].Lights[2].SetActive(true);
				
			}
		}
		if(yellowindices.Count>0)
		{
			foreach(int i in yellowindices)
			{
				BlinkingLights[i].BulbGreen.SetActive(false);
				BlinkingLights[i].BulbRed.SetActive(false);		
				foreach(GameObject l in BlinkingLights[i].Lights)
					l.SetActive(false);
			}
		}
		
		
		
	}
	
	
	void ResetAll()
	{
		ResetGreen();
		ResetBlinking();
		ResetRed();
	}
	void ResetGreen()
	{
		for(int x=0;x<GreenLights.Count;x++)
			GreenLights[x]=null;
	}
	void ResetRed()
	{
		for(int x=0;x<RedLights.Count;x++)
			RedLights[x]=null;
	}
	void ResetBlinking()
	{
		for(int x=0;x<BlinkingLights.Count;x++)
			BlinkingLights[x]=null;
	}
	void DisplayLinks()
	{
		ShowLinks=!ShowLinks;
		
		UpdateIndices();
		foreach(int i in greenindices)
		{
			GreenLights[i].showLinks=ShowLinks;
			GreenLights[i].linkColor = Color.green;
			GreenLights[i].controllerPosition = this.transform.position;
		}
		foreach(int i in yellowindices)
		{
			BlinkingLights[i].showLinks = ShowLinks;
			BlinkingLights[i].linkColor = Color.yellow;
			BlinkingLights[i].controllerPosition = this.transform.position;
			
		}
		foreach(int i in redindices)
		{
			RedLights[i].showLinks = ShowLinks;
			RedLights[i].linkColor = Color.red;
			RedLights[i].controllerPosition = this.transform.position;
		}
		this.transform.position = this.transform.position; // hacky way of manually updating the scene view.
	}
	void HelpClick()
	{
		Application.OpenURL("http://quantumtheoryentertainment.com/UCP/Documentation/traffic-light-system/");
	}
}
