using UnityEngine;
using System.Collections;

class Noise
{
	Vector2 seed;
	float speed;
	
	public Noise(float speed)
	{
		this.seed = new Vector2(Random.value, Random.value);	
		this.speed = speed;
	}
	
	public float Update()
	{
		seed += Vector2.one * speed * Time.deltaTime;
		return Mathf.PerlinNoise(seed.x, seed.y) * 2 - 1;
	}
}


public class CameraShake : MonoBehaviour
{
	Transform myTransform;
	Vector3 initPos;
	
	public float bigRadius = 3.0f;
	public float bigSpeed = 0.5f;
	public float smallRadius = 0.5f;
	public float smallSpeed = 2.0f;
	
	Noise noiseBigX;
	Noise noiseBigY;
	Noise noiseSmallX;
	Noise noiseSmallY;
	
	void Start()
	{
		myTransform = this.transform;
		initPos = myTransform.position;
		noiseBigX = new Noise(bigSpeed);
		noiseBigY = new Noise(bigSpeed);
		noiseSmallX = new Noise(smallSpeed);
		noiseSmallY = new Noise(smallSpeed);
	}
	
	void Update()
	{
		myTransform.position = initPos + new Vector3(noiseBigX.Update() * bigRadius + noiseSmallX.Update() * smallRadius, noiseBigY.Update() * bigRadius + noiseSmallY.Update() * smallRadius, 0); 
	}
}
