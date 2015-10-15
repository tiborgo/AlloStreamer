set(SOURCES
	ExposeProperties.cs
	ExposePropertyAttribute.cs
	RenderCubemap.cs
	RenderBinoculars.cs
	RenderBinocularsEditor.cs
	RenderStereoCubemap.cs
	ConvertRGBtoYUV420p.compute
)

add_custom_target(UnityRenderingPlugin	
	SOURCES ${SOURCES}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/ExposeProperties.cs
        ${UNITY_PROJECT_ASSETS_DIR}/ExposeProperties.cs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/ExposePropertyAttribute.cs
        ${UNITY_PROJECT_ASSETS_DIR}/ExposePropertyAttribute.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/RenderCubemap.cs
        ${UNITY_PROJECT_ASSETS_DIR}/RenderCubemap.cs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/RenderBinoculars.cs
        ${UNITY_PROJECT_ASSETS_DIR}/RenderBinoculars.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/RenderBinocularsEditor.cs
        ${UNITY_PROJECT_ASSETS_DIR}/Editor/RenderBinocularsEditor.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/RenderStereoCubemap.cs
        ${UNITY_PROJECT_ASSETS_DIR}/RenderStereoCubemap.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/ConvertRGBtoYUV420p.compute
        ${UNITY_PROJECT_ASSETS_DIR}/Resources/ConvertRGBtoYUV420p.compute
)
