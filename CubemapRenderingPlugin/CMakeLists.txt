set(SOURCES
	RenderCubemap.cs
)

add_custom_target(CubemapRenderingPlugin	
	SOURCES ${SOURCES}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/RenderCubemap.cs
        ${UNITY_PROJECT_ASSETS_DIR}/RenderCubemap.cs
)
