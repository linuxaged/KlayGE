// OGLESMapping.cpp
// KlayGE RenderEngine和OpenGL ES 2本地之间的映射 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Math.hpp>
#include <KFL/Vector.hpp>
#include <KFL/Matrix.hpp>
#include <KFL/Color.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderLayout.hpp>

#include <boost/assert.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGLES/OGLESRenderEngine.hpp>
#include <KlayGE/OpenGLES/OGLESTexture.hpp>
#include <KlayGE/OpenGLES/OGLESMapping.hpp>

namespace KlayGE
{
	// 从KlayGE的Color转换到float[4]
	/////////////////////////////////////////////////////////////////////////////////
	void OGLESMapping::Mapping(GLfloat* clr4, Color const & clr)
	{
		clr4[0] = clr.r();
		clr4[1] = clr.g();
		clr4[2] = clr.b();
		clr4[3] = clr.a();
	}

	// 从RenderEngine::CompareFunction转换到GLenum
	/////////////////////////////////////////////////////////////////////////////////
	GLenum OGLESMapping::Mapping(CompareFunction func)
	{
		switch (func)
		{
		case CF_AlwaysFail:
			return GL_NEVER;

		case CF_AlwaysPass:
			return GL_ALWAYS;

		case CF_Less:
			return GL_LESS;

		case CF_LessEqual:
			return GL_LEQUAL;

		case CF_Equal:
			return GL_EQUAL;

		case CF_NotEqual:
			return GL_NOTEQUAL;

		case CF_GreaterEqual:
			return GL_GEQUAL;

		case CF_Greater:
			return GL_GREATER;

		default:
			BOOST_ASSERT(false);
			return GL_EQUAL;
		};
	}

	// 从RenderEngine::AlphaBlendFactor转换到GLenum
	/////////////////////////////////////////////////////////////////////////////////
	GLenum OGLESMapping::Mapping(AlphaBlendFactor factor)
	{
		switch (factor)
		{
		case ABF_Zero:
			return GL_ZERO;

		case ABF_One:
			return GL_ONE;

		case ABF_Src_Alpha:
			return GL_SRC_ALPHA;

		case ABF_Dst_Alpha:
			return GL_DST_ALPHA;

		case ABF_Inv_Src_Alpha:
			return GL_ONE_MINUS_SRC_ALPHA;

		case ABF_Inv_Dst_Alpha:
			return GL_ONE_MINUS_DST_ALPHA;

		case ABF_Src_Color:
			return GL_SRC_COLOR;

		case ABF_Dst_Color:
			return GL_DST_COLOR;

		case ABF_Inv_Src_Color:
			return GL_ONE_MINUS_SRC_COLOR;

		case ABF_Inv_Dst_Color:
			return GL_ONE_MINUS_DST_COLOR;

		case ABF_Src_Alpha_Sat:
			return GL_SRC_ALPHA_SATURATE;

		case ABF_Blend_Factor:
			return GL_CONSTANT_COLOR;

		case ABF_Inv_Blend_Factor:
			return GL_ONE_MINUS_CONSTANT_COLOR;

		default:
			BOOST_ASSERT(false);
			return GL_ZERO;
		}
	}

	// 从RenderEngine::StencilOperation转换到GLenum
	/////////////////////////////////////////////////////////////////////////////////
	GLenum OGLESMapping::Mapping(StencilOperation op)
	{
		switch (op)
		{
		case SOP_Keep:
			return GL_KEEP;

		case SOP_Zero:
			return GL_ZERO;

		case SOP_Replace:
			return GL_REPLACE;

		case SOP_Incr:
			return GL_INCR;

		case SOP_Decr:
			return GL_DECR;

		case SOP_Invert:
			return GL_INVERT;

		case SOP_Incr_Wrap:
			return GL_INCR_WRAP;

		case SOP_Decr_Wrap:
			return GL_DECR_WRAP;

		default:
			BOOST_ASSERT(false);
			return GL_KEEP;
		};
	}

	GLenum OGLESMapping::Mapping(BlendOperation bo)
	{
		switch (bo)
		{
		case BOP_Add:
			return GL_FUNC_ADD;

		case BOP_Sub:
			return GL_FUNC_SUBTRACT;

		case BOP_Rev_Sub:
			return GL_FUNC_REVERSE_SUBTRACT;

		case BOP_Min:
			if (glloader_GLES_VERSION_3_0())
			{
				return GL_MIN;
			}
			else if (glloader_GLES_EXT_blend_minmax())
			{
				return GL_MIN_EXT;
			}
			else
			{
				THR(errc::function_not_supported);
			}

		case BOP_Max:
			if (glloader_GLES_VERSION_3_0())
			{
				return GL_MAX;
			}
			else if (glloader_GLES_EXT_blend_minmax())
			{
				return GL_MAX_EXT;
			}
			else
			{
				THR(errc::function_not_supported);
			}

		default:
			BOOST_ASSERT(false);
			return GL_FUNC_ADD;
		}
	}

	GLint OGLESMapping::Mapping(TexAddressingMode mode)
	{
		switch (mode)
		{
		case TAM_Wrap:
			return GL_REPEAT;

		case TAM_Mirror:
			return GL_MIRRORED_REPEAT;

		case TAM_Clamp:
			return GL_CLAMP_TO_EDGE;

		case TAM_Border:
			return GL_CLAMP_TO_EDGE;

		default:
			BOOST_ASSERT(false);
			return GL_REPEAT;
		}
	}

	void OGLESMapping::Mapping(GLenum& primType, uint32_t& primCount, RenderLayout const & rl)
	{
		uint32_t const vertexCount = static_cast<uint32_t>(rl.UseIndices() ? rl.NumIndices() : rl.NumVertices());
		primType = GL_POINTS;
		primCount = vertexCount;
		switch (rl.TopologyType())
		{
		case RenderLayout::TT_PointList:
			primType = GL_POINTS;
			primCount = vertexCount;
			break;

		case RenderLayout::TT_LineList:
			primType = GL_LINES;
			primCount = vertexCount / 2;
			break;

		case RenderLayout::TT_LineStrip:
			primType = GL_LINE_STRIP;
			primCount = vertexCount - 1;
			break;

		case RenderLayout::TT_TriangleList:
			primType = GL_TRIANGLES;
			primCount = vertexCount / 3;
			break;

		case RenderLayout::TT_TriangleStrip:
			primType = GL_TRIANGLE_STRIP;
			primCount = vertexCount - 2;
			break;

		case RenderLayout::TT_1_Ctrl_Pt_PatchList:
		case RenderLayout::TT_2_Ctrl_Pt_PatchList:
		case RenderLayout::TT_3_Ctrl_Pt_PatchList:
		case RenderLayout::TT_4_Ctrl_Pt_PatchList:
		case RenderLayout::TT_5_Ctrl_Pt_PatchList:
		case RenderLayout::TT_6_Ctrl_Pt_PatchList:
		case RenderLayout::TT_7_Ctrl_Pt_PatchList:
		case RenderLayout::TT_8_Ctrl_Pt_PatchList:
		case RenderLayout::TT_9_Ctrl_Pt_PatchList:
		case RenderLayout::TT_10_Ctrl_Pt_PatchList:
		case RenderLayout::TT_11_Ctrl_Pt_PatchList:
		case RenderLayout::TT_12_Ctrl_Pt_PatchList:
		case RenderLayout::TT_13_Ctrl_Pt_PatchList:
		case RenderLayout::TT_14_Ctrl_Pt_PatchList:
		case RenderLayout::TT_15_Ctrl_Pt_PatchList:
		case RenderLayout::TT_16_Ctrl_Pt_PatchList:
		case RenderLayout::TT_17_Ctrl_Pt_PatchList:
		case RenderLayout::TT_18_Ctrl_Pt_PatchList:
		case RenderLayout::TT_19_Ctrl_Pt_PatchList:
		case RenderLayout::TT_20_Ctrl_Pt_PatchList:
		case RenderLayout::TT_21_Ctrl_Pt_PatchList:
		case RenderLayout::TT_22_Ctrl_Pt_PatchList:
		case RenderLayout::TT_23_Ctrl_Pt_PatchList:
		case RenderLayout::TT_24_Ctrl_Pt_PatchList:
		case RenderLayout::TT_25_Ctrl_Pt_PatchList:
		case RenderLayout::TT_26_Ctrl_Pt_PatchList:
		case RenderLayout::TT_27_Ctrl_Pt_PatchList:
		case RenderLayout::TT_28_Ctrl_Pt_PatchList:
		case RenderLayout::TT_29_Ctrl_Pt_PatchList:
		case RenderLayout::TT_30_Ctrl_Pt_PatchList:
		case RenderLayout::TT_31_Ctrl_Pt_PatchList:
		case RenderLayout::TT_32_Ctrl_Pt_PatchList:
			if (glloader_GLES_VERSION_3_2())
			{
				primType = GL_PATCHES;
				uint32_t n = rl.TopologyType() - RenderLayout::TT_1_Ctrl_Pt_PatchList + 1;
				glPatchParameteriOES(GL_PATCH_VERTICES, n);
				primCount = vertexCount / 3;
			}
			else if (glloader_GLES_OES_tessellation_shader())
			{
				primType = GL_PATCHES_OES;
				uint32_t n = rl.TopologyType() - RenderLayout::TT_1_Ctrl_Pt_PatchList + 1;
				glPatchParameteriOES(GL_PATCH_VERTICES_OES, n);
				primCount = vertexCount / 3;
			}
			else if (glloader_GLES_EXT_tessellation_shader() || glloader_GLES_ANDROID_extension_pack_es31a())
			{
				primType = GL_PATCHES_EXT;
				uint32_t n = rl.TopologyType() - RenderLayout::TT_1_Ctrl_Pt_PatchList + 1;
				glPatchParameteriEXT(GL_PATCH_VERTICES_EXT, n);
				primCount = vertexCount / 3;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		default:
			THR(errc::function_not_supported);
		}
	}

	void OGLESMapping::MappingFormat(GLint& internalFormat, GLenum& glformat, GLenum& gltype, ElementFormat ef)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		OGLESRenderEngine& ogl_re = *checked_cast<OGLESRenderEngine*>(&re);

		switch (ef)
		{
		case EF_A8:
			internalFormat = GL_ALPHA;
			glformat = GL_ALPHA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case EF_R5G6B5:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB565;
				glformat = GL_RGBA;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_A1RGB5:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB5_A1;
				glformat = GL_RGBA;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_ARGB4:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGBA4;
				glformat = GL_RGBA;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_R8:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R8;
				glformat = GL_RED;
				gltype = GL_UNSIGNED_BYTE;
			}
			else if (glloader_GLES_EXT_texture_rg())
			{
				internalFormat = GL_RED_EXT;
				glformat = GL_RED_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				internalFormat = GL_LUMINANCE;
				glformat = GL_LUMINANCE;
				gltype = GL_UNSIGNED_BYTE;
			}
			break;

		case EF_SIGNED_R8:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R8_SNORM;
				glformat = GL_RED;
				gltype = GL_BYTE;
			}
			else if (glloader_GLES_EXT_texture_rg())
			{
				internalFormat = GL_RED_EXT;
				glformat = GL_RED_EXT;
				gltype = GL_BYTE;
			}
			else
			{
				internalFormat = GL_LUMINANCE;
				glformat = GL_LUMINANCE;
				gltype = GL_BYTE;
			}
			break;

		case EF_GR8:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG8;
				glformat = GL_RG;
				gltype = GL_UNSIGNED_BYTE;
			}
			else if (glloader_GLES_EXT_texture_rg())
			{
				internalFormat = GL_RG_EXT;
				glformat = GL_RG_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_SIGNED_GR8:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG8_SNORM;
				glformat = GL_RG;
				gltype = GL_BYTE;
			}
			else if (glloader_GLES_EXT_texture_rg())
			{
				internalFormat = GL_RG_EXT;
				glformat = GL_RG_EXT;
				gltype = GL_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BGR8:
			internalFormat = GL_RGB8;
			glformat = GL_RGB;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case EF_ARGB8:
			if (glloader_GLES_EXT_texture_format_BGRA8888())
			{
				internalFormat = GL_BGRA_EXT;
				glformat = GL_BGRA_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_ABGR8:
			internalFormat = GL_RGBA;
			glformat = GL_RGBA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case EF_A2BGR10:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB10_A2;
				glformat = GL_RGBA;
				gltype = GL_UNSIGNED_INT_2_10_10_10_REV;
			}
			else if (glloader_GLES_EXT_texture_type_2_10_10_10_REV())
			{
				internalFormat = GL_RGBA;
				glformat = GL_RGBA;
				gltype = GL_UNSIGNED_INT_2_10_10_10_REV_EXT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_R8I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R8I;
				glformat = GL_RED_INTEGER;
				gltype = GL_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_GR8UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG8UI;
				glformat = GL_RG_INTEGER;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_GR8I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG8I;
				glformat = GL_RG_INTEGER;
				gltype = GL_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BGR8UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB8UI;
				glformat = GL_RGB_INTEGER;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BGR8I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB8I;
				glformat = GL_RGB_INTEGER;
				gltype = GL_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_R16UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R16UI;
				glformat = GL_RED_INTEGER;
				gltype = GL_UNSIGNED_SHORT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_R16I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R16I;
				glformat = GL_RED_INTEGER;
				gltype = GL_SHORT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_GR16UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG16UI;
				glformat = GL_RG_INTEGER;
				gltype = GL_UNSIGNED_SHORT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_GR16I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG16I;
				glformat = GL_RG_INTEGER;
				gltype = GL_SHORT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BGR16UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB16UI;
				glformat = GL_RGB_INTEGER;
				gltype = GL_UNSIGNED_SHORT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BGR16I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB16I;
				glformat = GL_RGB_INTEGER;
				gltype = GL_SHORT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_ABGR16UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGBA16UI;
				glformat = GL_RGBA_INTEGER;
				gltype = GL_UNSIGNED_SHORT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_ABGR16I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGBA16I;
				glformat = GL_RGBA_INTEGER;
				gltype = GL_SHORT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_R32UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R32UI;
				glformat = GL_RED_INTEGER;
				gltype = GL_UNSIGNED_INT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_R32I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R32I;
				glformat = GL_RED_INTEGER;
				gltype = GL_INT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_GR32UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG32UI;
				glformat = GL_RG_INTEGER;
				gltype = GL_UNSIGNED_INT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_GR32I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG32I;
				glformat = GL_RG_INTEGER;
				gltype = GL_INT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BGR32UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB32UI;
				glformat = GL_RGB_INTEGER;
				gltype = GL_UNSIGNED_INT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BGR32I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB32I;
				glformat = GL_RGB_INTEGER;
				gltype = GL_INT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_ABGR32UI:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGBA32UI;
				glformat = GL_RGBA_INTEGER;
				gltype = GL_UNSIGNED_INT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_ABGR32I:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGBA32I;
				glformat = GL_RGBA_INTEGER;
				gltype = GL_INT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_R16F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R16F;
				glformat = GL_RED;
				gltype = GL_HALF_FLOAT;
			}
			else if (glloader_GLES_OES_texture_half_float() || ogl_re.HackForTegra())
			{
				internalFormat = GL_LUMINANCE;
				glformat = GL_LUMINANCE;
				gltype = GL_HALF_FLOAT_OES;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_GR16F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG16F;
				glformat = GL_RG;
				gltype = GL_HALF_FLOAT;
			}
			else if (glloader_GLES_OES_texture_half_float() || ogl_re.HackForTegra())
			{
				internalFormat = GL_LUMINANCE_ALPHA;
				glformat = GL_LUMINANCE_ALPHA;
				gltype = GL_HALF_FLOAT_OES;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_B10G11R11F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R11F_G11F_B10F;
				glformat = GL_RGB;
				gltype = GL_UNSIGNED_INT_10F_11F_11F_REV;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BGR16F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB16F;
				glformat = GL_RGB;
				gltype = GL_HALF_FLOAT;
			}
			else if (glloader_GLES_OES_texture_half_float() || ogl_re.HackForTegra())
			{
				internalFormat = GL_RGB;
				glformat = GL_RGB;
				gltype = GL_HALF_FLOAT_OES;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_ABGR16F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGBA16F;
				glformat = GL_RGBA;
				gltype = GL_HALF_FLOAT;
			}
			else if (glloader_GLES_OES_texture_half_float() || ogl_re.HackForTegra())
			{
				internalFormat = GL_RGBA;
				glformat = GL_RGBA;
				gltype = GL_HALF_FLOAT_OES;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_R32F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_R32F;
				glformat = GL_RED;
				gltype = GL_FLOAT;
			}
			else if (glloader_GLES_OES_texture_float())
			{
				internalFormat = GL_LUMINANCE;
				glformat = GL_LUMINANCE;
				gltype = GL_FLOAT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_GR32F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RG32F;
				glformat = GL_RG;
				gltype = GL_FLOAT;
			}
			else if (glloader_GLES_OES_texture_float())
			{
				internalFormat = GL_LUMINANCE_ALPHA;
				glformat = GL_LUMINANCE_ALPHA;
				gltype = GL_FLOAT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BGR32F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGB32F;
				glformat = GL_RGB;
				gltype = GL_FLOAT;
			}
			else if (glloader_GLES_OES_texture_float())
			{
				internalFormat = GL_RGB;
				glformat = GL_RGB;
				gltype = GL_FLOAT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_ABGR32F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_RGBA32F;
				glformat = GL_RGBA;
				gltype = GL_FLOAT;
			}
			else if (glloader_GLES_OES_texture_float())
			{
				internalFormat = GL_RGBA;
				glformat = GL_RGBA;
				gltype = GL_FLOAT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BC1:
			if (glloader_GLES_EXT_texture_compression_dxt1() || glloader_GLES_EXT_texture_compression_s3tc())
			{
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				glformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BC2:
			if (glloader_GLES_EXT_texture_compression_s3tc())
			{
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				glformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BC3:
			if (glloader_GLES_EXT_texture_compression_s3tc())
			{
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				glformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BC4:
			if (glloader_GLES_EXT_texture_compression_latc())
			{
				internalFormat = GL_COMPRESSED_LUMINANCE_LATC1_EXT;
				glformat = GL_COMPRESSED_LUMINANCE_LATC1_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_BC5:
			if (glloader_GLES_EXT_texture_compression_dxt1())
			{
				internalFormat = GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
				glformat = GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_SIGNED_BC4:
			if (glloader_GLES_EXT_texture_compression_latc())
			{
				internalFormat = GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT;
				glformat = GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_SIGNED_BC5:
			if (glloader_GLES_EXT_texture_compression_dxt1())
			{
				internalFormat = GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT;
				glformat = GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_D16:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_DEPTH_COMPONENT16;
			}
			else
			{
				internalFormat = GL_DEPTH_COMPONENT;
			}
			glformat = GL_DEPTH_COMPONENT;
			gltype = GL_UNSIGNED_SHORT;
			break;

		case EF_D24S8:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_DEPTH24_STENCIL8;
				glformat = GL_DEPTH_STENCIL;
				gltype = GL_UNSIGNED_INT_24_8;
			}
			else if (glloader_GLES_OES_packed_depth_stencil())
			{
				internalFormat = GL_DEPTH_STENCIL_OES;
				glformat = GL_DEPTH_STENCIL_OES;
				gltype = GL_UNSIGNED_INT_24_8_OES;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_D32F:
			if (glloader_GLES_VERSION_3_0())
			{
				internalFormat = GL_DEPTH_COMPONENT32F;
				glformat = GL_DEPTH_COMPONENT;
				gltype = GL_FLOAT;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_ABGR8_SRGB:
			if (glloader_GLES_VERSION_3_0() || glloader_GLES_EXT_sRGB())
			{
				internalFormat = GL_SRGB8_ALPHA8;
				glformat = GL_RGBA;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		default:
			THR(errc::function_not_supported);
		}
	}

	void OGLESMapping::MappingVertexFormat(GLenum& gltype, GLboolean& normalized, ElementFormat ef)
	{
		switch (ef)
		{
		case EF_A8:
		case EF_R8:
		case EF_GR8:
		case EF_BGR8:
		case EF_ARGB8:
		case EF_ABGR8:
			gltype = GL_UNSIGNED_BYTE;
			normalized = GL_TRUE;
			break;

		case EF_R8UI:
		case EF_GR8UI:
		case EF_BGR8UI:
		case EF_ABGR8UI:
			gltype = GL_UNSIGNED_BYTE;
			normalized = GL_FALSE;
			break;

		case EF_SIGNED_R8:
		case EF_SIGNED_GR8:
		case EF_SIGNED_BGR8:
		case EF_SIGNED_ABGR8:
			gltype = GL_BYTE;
			normalized = GL_TRUE;
			break;

		case EF_R8I:
		case EF_GR8I:
		case EF_BGR8I:
		case EF_ABGR8I:
			gltype = GL_BYTE;
			normalized = GL_FALSE;
			break;

		case EF_A2BGR10:
			if (glloader_GLES_OES_vertex_type_10_10_10_2())
			{
				gltype = GL_UNSIGNED_INT_10_10_10_2_OES;
				normalized = GL_TRUE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_SIGNED_A2BGR10:
			if (glloader_GLES_OES_vertex_type_10_10_10_2())
			{
				gltype = GL_INT_10_10_10_2_OES;
				normalized = GL_TRUE;
			}
			else
			{
				THR(errc::function_not_supported);
			}
			break;

		case EF_R16:
		case EF_GR16:
		case EF_BGR16:
		case EF_ABGR16:
			gltype = GL_UNSIGNED_SHORT;
			normalized = GL_TRUE;
			break;

		case EF_R16UI:
		case EF_GR16UI:
		case EF_BGR16UI:
		case EF_ABGR16UI:
			gltype = GL_UNSIGNED_SHORT;
			normalized = GL_FALSE;
			break;

		case EF_SIGNED_R16:
		case EF_SIGNED_GR16:
		case EF_SIGNED_BGR16:
		case EF_SIGNED_ABGR16:
			gltype = GL_SHORT;
			normalized = GL_TRUE;
			break;

		case EF_R16I:
		case EF_GR16I:
		case EF_BGR16I:
		case EF_ABGR16I:
			gltype = GL_SHORT;
			normalized = GL_FALSE;
			break;

		case EF_R32UI:
		case EF_GR32UI:
		case EF_BGR32UI:
		case EF_ABGR32UI:
			gltype = GL_UNSIGNED_INT;
			normalized = GL_FALSE;
			break;

		case EF_R32I:
		case EF_GR32I:
		case EF_BGR32I:
		case EF_ABGR32I:
			gltype = GL_INT;
			normalized = GL_FALSE;
			break;

		case EF_R16F:
		case EF_GR16F:
		case EF_BGR16F:
		case EF_ABGR16F:
			gltype = GL_HALF_FLOAT_OES;
			normalized = GL_FALSE;
			break;

		case EF_R32F:
		case EF_GR32F:
		case EF_BGR32F:
		case EF_ABGR32F:
			gltype = GL_FLOAT;
			normalized = GL_FALSE;
			break;

		default:
			THR(errc::function_not_supported);
		}
	}
}
