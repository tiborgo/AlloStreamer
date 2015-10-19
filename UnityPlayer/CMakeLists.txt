if(APPLE)
    set(UNITY_EXE "/Applications/Unity/Unity.app/Contents/MacOS/Unity")
elseif(WIN32)
    set(UNITY_EXE "C:\Program Files (x86)\Unity\Editor\Unity.exe")
endif()

add_custom_target(UnityPlayer
	COMMAND ${UNITY_EXE} -quit -batchmode
	-projectPath ${UNITY_PROJECT_DIR}
	-buildOSX64Player ${PROJECT_SOURCE_DIR}/Bin/$<CONFIGURATION>/AlloUnityPlayer.app
)

add_dependencies(UnityPlayer
	UnityRenderingPlugin
	CubemapExtractionPlugin
)