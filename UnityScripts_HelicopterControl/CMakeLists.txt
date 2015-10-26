set(SOURCES
	GuideHelicopter.cs
)

add_custom_target(UnityScripts_HelicopterControl	
	SOURCES ${SOURCES}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/GuideHelicopter.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/GuideHelicopter.cs
)
