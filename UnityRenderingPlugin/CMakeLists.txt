set(SOURCES
	ExposeProperties.cs
	ExposePropertyAttribute.cs
	RenderCubemap.cs
	RenderBinoculars.cs
	RenderBinocularsEditor.cs
	RenderStereoCubemap.cs
	ConvertRGBtoYUV420p.compute
	ParticlesAdditiveSoftMaterialDummy.mat
	UnlitColorMaterialDummy.mat
	UnlitTextureMaterialDummy.mat
)

add_custom_target(UnityRenderingPlugin ALL
	SOURCES ${SOURCES}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/ExposeProperties.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/ExposeProperties.cs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/ExposePropertyAttribute.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/ExposePropertyAttribute.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/RenderCubemap.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/RenderCubemap.cs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/RenderBinoculars.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/RenderBinoculars.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/RenderBinocularsEditor.cs
        ${UNITY_PROJECT_ASSETS_DIR}/Editor/AlloUnity/RenderBinocularsEditor.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/RenderStereoCubemap.cs
        ${UNITY_PROJECT_ASSETS_DIR}/AlloUnity/RenderStereoCubemap.cs
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/ConvertRGBtoYUV420p.compute
        ${UNITY_PROJECT_ASSETS_DIR}/Resources/AlloUnity/ConvertRGBtoYUV420p.compute
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/ParticlesAdditiveSoftMaterialDummy.mat
        ${UNITY_PROJECT_ASSETS_DIR}/Resources/AlloUnity/ParticlesAdditiveSoftMaterialDummy.mat
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/UnlitColorMaterialDummy.mat
        ${UNITY_PROJECT_ASSETS_DIR}/Resources/AlloUnity/UnlitColorMaterialDummy.mat
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/UnlitTextureMaterialDummy.mat
        ${UNITY_PROJECT_ASSETS_DIR}/Resources/AlloUnity/UnlitTextureMaterialDummy.mat
)
