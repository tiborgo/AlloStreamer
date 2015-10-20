set(SOURCES
	MouseLook.cs
	MxrPredistortionMesh.cs
	OSCPhaseSpaceThread.cs
	OSCThread.cs
	ServerThread.cs
	UseRenderingPlugin.cs
)

add_custom_target(UnityScripts_Binoculars	
	SOURCES ${SOURCES}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/MouseLook.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/Control.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/OSCPhaseSpaceThread.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/OSCPhaseSpaceThread.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/OSCThread.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/OSCThread.cs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/ServerThread.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/ServerThread.cs
)
