#if UNITY_EDITOR //This script will only work in editor mode. You cannot adjust the scale dynamically in-game!
using UnityEngine;
using System.Collections;
using UnityEditor;

[ExecuteInEditMode]
public class FXScaler : MonoBehaviour 
{
	public float fScale				= 1.0f;
	public bool alsoScaleGameobject	= true;
	//public int renderQueue = 4000;

	float	m_fPrevScale;

	void Start()
	{
		//ParticleSystem ps = GetComponent<ParticleSystem>("ParticleSystem");
		m_fPrevScale = fScale;
	//	this.renderer.material.renderQueue = 2800;
	}

	void Update () 
	{
		if (m_fPrevScale != fScale && fScale > 0)
		//if (Math.Abs(m_fPrevScale - fScale) > 1e-5f && fScale > 0)
		{
			if (alsoScaleGameobject)
				transform.localScale = new Vector3(fScale, fScale, fScale);

			float fScaleFactor = fScale / m_fPrevScale;

			ScaleLegacySystems(fScaleFactor);
			ScaleShurikenSystems(fScaleFactor);

			m_fPrevScale = fScale;
		}
	}

	void ScaleShurikenSystems(float scaleFactor)
	{
		ParticleSystem[] systems = GetComponentsInChildren<ParticleSystem>();

		foreach (ParticleSystem system in systems)
		{
			system.startSpeed			*= scaleFactor;
			system.startSize			*= scaleFactor;
			system.gravityModifier		*= scaleFactor;

			SerializedObject so = new SerializedObject(system);
			
#if UNITY_3_5 
			so.FindProperty("ShapeModule.radius").floatValue					*= scaleFactor;
			so.FindProperty("ShapeModule.boxX").floatValue						*= scaleFactor;
			so.FindProperty("ShapeModule.boxY").floatValue						*= scaleFactor;
			so.FindProperty("ShapeModule.boxZ").floatValue						*= scaleFactor;
#endif
			
			so.FindProperty("VelocityModule.x.scalar").floatValue				*= scaleFactor;
			so.FindProperty("VelocityModule.y.scalar").floatValue				*= scaleFactor;
			so.FindProperty("VelocityModule.z.scalar").floatValue				*= scaleFactor;
			so.FindProperty("ClampVelocityModule.magnitude.scalar").floatValue	*= scaleFactor;
			so.FindProperty("ClampVelocityModule.x.scalar").floatValue			*= scaleFactor;
			so.FindProperty("ClampVelocityModule.y.scalar").floatValue			*= scaleFactor;
			so.FindProperty("ClampVelocityModule.z.scalar").floatValue			*= scaleFactor;
			so.FindProperty("ForceModule.x.scalar").floatValue					*= scaleFactor;
			so.FindProperty("ForceModule.y.scalar").floatValue					*= scaleFactor;
			so.FindProperty("ForceModule.z.scalar").floatValue					*= scaleFactor;
			so.FindProperty("ColorBySpeedModule.range").vector2Value			*= scaleFactor;
			so.FindProperty("SizeBySpeedModule.range").vector2Value				*= scaleFactor;
			so.FindProperty("RotationBySpeedModule.range").vector2Value			*= scaleFactor;

			so.ApplyModifiedProperties();
		}
	}

	void ScaleLegacySystems(float scaleFactor)
	{
		ParticleEmitter[] emitters		= GetComponentsInChildren<ParticleEmitter>();
		ParticleAnimator[] animators	= GetComponentsInChildren<ParticleAnimator>();

		foreach (ParticleEmitter emitter in emitters)
		{
			emitter.minSize				*= scaleFactor;
			emitter.maxSize				*= scaleFactor;
			emitter.worldVelocity		*= scaleFactor;
			emitter.localVelocity		*= scaleFactor;
			emitter.rndVelocity			*= scaleFactor;

			SerializedObject so = new SerializedObject(emitter);

			so.FindProperty("m_Ellipsoid").vector3Value		*= scaleFactor;
			so.FindProperty("tangentVelocity").vector3Value	*= scaleFactor;
			so.ApplyModifiedProperties();
		}
	}
};


#endif