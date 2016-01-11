set(CONFIG_PATH "$<$<NOT:$<STREQUAL:$<CONFIGURATION>,''>>:$<CONFIGURATION>>")
message(STATUS ${CONFIG_PATH})

if(APPLE)
	add_custom_target(AlloUnityPlayer ALL
		COMMAND /Applications/Unity/Unity.app/Contents/MacOS/Unity -quit -batchmode
		-projectPath ${UNITY_PROJECT_DIR}
		-buildOSX64Player ${PROJECT_SOURCE_DIR}/Bin${CONFIG_PATH}/AlloUnityPlayer.app
	)
elseif(WIN32)
    add_custom_target(AlloUnityPlayer ALL
		COMMAND "C:\\Program Files\\Unity\\Editor\\Unity.exe" -quit -batchmode
		-projectPath ${UNITY_PROJECT_DIR}
		-buildWindows64Player ${PROJECT_SOURCE_DIR}/Bin${CONFIG_PATH}/AlloUnityPlayer.exe
	)
endif()


add_dependencies(AlloUnityPlayer
	UnityRenderingPlugin
	CubemapExtractionPlugin
	UnityScripts_Binoculars
	RenderingPlugin_Binoculars
)