if(APPLE)
	add_custom_target(AlloUnityPlayer
		COMMAND /Applications/Unity/Unity.app/Contents/MacOS/Unity -quit -batchmode
		-projectPath ${UNITY_PROJECT_DIR}
		-buildOSX64Player ${PROJECT_SOURCE_DIR}/Bin/$<CONFIGURATION>/AlloUnityPlayer.app
	)
elseif(WIN32)
    add_custom_target(AlloUnityPlayer
		COMMAND "C:\\Program Files\\Unity\\Editor\\Unity.exe" -quit -batchmode
		-projectPath ${UNITY_PROJECT_DIR}
		-buildWindows64Player ${PROJECT_SOURCE_DIR}/Bin/$<CONFIGURATION>/AlloUnityPlayer.exe
	)
endif()



add_dependencies(AlloUnityPlayer
	UnityRenderingPlugin
	CubemapExtractionPlugin
)