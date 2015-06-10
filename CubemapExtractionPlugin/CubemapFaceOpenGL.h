#pragma once

#include "CubemapExtractionPlugin.h"
#include "AlloShared/CubemapFace.h"

#if SUPPORT_OPENGL

class CubemapFaceOpenGL : public CubemapFace
{
public:
	typedef boost::interprocess::offset_ptr<CubemapFaceOpenGL> Ptr;

	const GLuint gpuTextureID;

	template <typename SegmentManager>
	static CubemapFaceOpenGL* create(GLuint textureID,
		int face,
		Allocator<SegmentManager>& allocator);

protected:

	template <typename SegmentManager>
	CubemapFaceOpenGL(
		boost::uint32_t width,
		boost::uint32_t height,
		int index,
		Allocator<SegmentManager>& allocator,
		GLuint gpuTextureID
		);
};

#include "CubemapFaceOpenGL.inl"

#endif