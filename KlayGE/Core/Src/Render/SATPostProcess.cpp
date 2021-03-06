// SATPostProcess.cpp
// KlayGE Summed-Area Table后期处理类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
//
// 3.7.0
// 初次建立 (2006.10.10)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/PostProcess.hpp>
#include <KlayGE/SATPostProcess.hpp>

namespace KlayGE
{
	SATSeparableScanSweepPostProcess::SATSeparableScanSweepPostProcess(RenderTechniquePtr const & tech)
			: PostProcess(L"SATSeparableScanSweep",
					std::vector<std::string>(),
					std::vector<std::string>(1, "src_tex"),
					std::vector<std::string>(1, "output"),
					tech)
	{
		if (technique_)
		{
			child_tex_ep_ = technique_->Effect().ParameterByName("child_tex");
			addr_offset_ep_ = technique_->Effect().ParameterByName("addr_offset");
			length_ep_ = technique_->Effect().ParameterByName("length");
			scale_ep_ = technique_->Effect().ParameterByName("scale");
		}
	}

	void SATSeparableScanSweepPostProcess::ChildBuffer(TexturePtr const & tex)
	{
		*child_tex_ep_ = tex;
	}

	void SATSeparableScanSweepPostProcess::AddrOffset(float3 const & offset)
	{
		*addr_offset_ep_ = offset;
	}

	void SATSeparableScanSweepPostProcess::Length(int32_t length)
	{
		length_ = length;
		*length_ep_ = length;
	}

	void SATSeparableScanSweepPostProcess::Scale(float scale)
	{
		*scale_ep_ = scale;
	}


	SATPostProcess::SATPostProcess()
		: PostProcessChain(L"SAT")
	{
	}

	void SATPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		if (0 == index)
		{
			uint32_t const tex_width = tex->Width(0);
			uint32_t const tex_height = tex->Height(0);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			uint32_t width = tex_width;
			uint32_t height = tex_height;

			std::vector<uint32_t> widths;
			while (width >= 1)
			{
				widths.push_back(width);

				if (1 == width)
				{
					break;
				}
				width = (width + 3) / 4;
			}

			std::vector<uint32_t> heights;
			while (height >= 1)
			{
				heights.push_back(height);

				if (1 == height)
				{
					break;
				}
				height = (height + 3) / 4;
			}

			std::vector<TexturePtr> inter_tex_x_up(widths.size());
			std::vector<TexturePtr> inter_tex_x_down(widths.size());
			std::vector<TexturePtr> inter_tex_y_up(heights.size());
			std::vector<TexturePtr> inter_tex_y_down(heights.size());

			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			ElementFormat fmt;
			if (caps.rendertarget_format_support(EF_ABGR32F, 1, 0))
			{
				fmt = EF_ABGR32F;
			}
			else
			{
				BOOST_ASSERT(caps.rendertarget_format_support(EF_ABGR16F, 1, 0));

				fmt = EF_ABGR16F;
			}

			{
				inter_tex_x_up[0] = tex;
			}
			for (size_t i = 1; i < widths.size(); ++ i)
			{
				inter_tex_x_up[i] = rf.MakeTexture2D(widths[i], tex_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			}
			{
				inter_tex_x_down[0] = inter_tex_x_up.back();
			}
			for (size_t i = 1; i < widths.size(); ++ i)
			{
				inter_tex_x_down[i] = rf.MakeTexture2D(widths[widths.size() - 1 - i], tex_height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			}
			{
				inter_tex_y_up[0] = inter_tex_x_down.back();
			}
			for (size_t i = 1; i < heights.size(); ++ i)
			{
				inter_tex_y_up[i] = rf.MakeTexture2D(tex_width, heights[i], 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			}
			{
				inter_tex_y_down[0] = inter_tex_y_up.back();
			}
			for (size_t i = 1; i < heights.size(); ++ i)
			{
				inter_tex_y_down[i] = rf.MakeTexture2D(tex_width, heights[heights.size() - 1 - i], 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, nullptr);
			}

			for (size_t i = 0; i < inter_tex_x_up.size() - 1; ++ i)
			{
				uint32_t const parent_length = inter_tex_x_up[i + 1]->Width(0);
				uint32_t const child_length = inter_tex_x_up[i]->Width(0);

				RenderEffectPtr effect = SyncLoadRenderEffect("SAT.fxml");
				SATSeparableScanSweepPostProcessPtr pp = MakeSharedPtr<SATSeparableScanSweepPostProcess>(effect->TechniqueByName("SATScanXUpSweep"));
				pp->Length(child_length);
				pp->AddrOffset(float3(0.5f / child_length, 1.5f / child_length, 0));
				pp->Scale((parent_length * 4.0f) / child_length);
				pp->InputPin(0, inter_tex_x_up[i]);
				pp->OutputPin(0, inter_tex_x_up[i + 1]);

				this->Append(pp);
			}
			for (size_t i = 0; i < inter_tex_x_down.size() - 1; ++ i)
			{
				uint32_t const parent_length = inter_tex_x_down[i]->Width(0);
				uint32_t const child_length = inter_tex_x_down[i + 1]->Width(0);

				RenderEffectPtr effect = SyncLoadRenderEffect("SAT.fxml");
				SATSeparableScanSweepPostProcessPtr pp = MakeSharedPtr<SATSeparableScanSweepPostProcess>(effect->TechniqueByName("SATScanXDownSweep"));
				pp->Length(child_length);
				pp->InputPin(0, inter_tex_x_down[i]);
				pp->ChildBuffer(inter_tex_x_up[inter_tex_x_down.size() - 2 - i]);
				pp->AddrOffset(float3(1.0f / parent_length, 1.0f / child_length, 2.0f / child_length));
				pp->Scale(child_length / (parent_length * 4.0f));
				pp->OutputPin(0, inter_tex_x_down[i + 1]);

				this->Append(pp);
			}
			for (size_t i = 0; i < inter_tex_y_up.size() - 1; ++ i)
			{
				uint32_t const parent_length = inter_tex_y_up[i + 1]->Height(0);
				uint32_t const child_length = inter_tex_y_up[i]->Height(0);

				RenderEffectPtr effect = SyncLoadRenderEffect("SAT.fxml");
				SATSeparableScanSweepPostProcessPtr pp = MakeSharedPtr<SATSeparableScanSweepPostProcess>(effect->TechniqueByName("SATScanYUpSweep"));
				pp->Length(child_length);
				pp->AddrOffset(float3(0.5f / child_length, 1.5f / child_length, 0));
				pp->Scale((parent_length * 4.0f) / child_length);
				pp->InputPin(0, inter_tex_y_up[i]);
				pp->OutputPin(0, inter_tex_y_up[i + 1]);

				this->Append(pp);
			}
			for (size_t i = 0; i < inter_tex_y_down.size() - 1; ++ i)
			{
				uint32_t const parent_length = inter_tex_y_down[i]->Height(0);
				uint32_t const child_length = inter_tex_y_down[i + 1]->Height(0);

				RenderEffectPtr effect = SyncLoadRenderEffect("SAT.fxml");
				SATSeparableScanSweepPostProcessPtr pp = MakeSharedPtr<SATSeparableScanSweepPostProcess>(effect->TechniqueByName("SATScanYDownSweep"));
				pp->Length(child_length);
				pp->InputPin(0, inter_tex_y_down[i]);
				pp->ChildBuffer(inter_tex_y_up[inter_tex_y_down.size() - 2 - i]);
				pp->AddrOffset(float3(1.0f / parent_length, 1.0f / child_length, 2.0f / child_length));
				pp->Scale(child_length / (parent_length * 4.0f));
				pp->OutputPin(0, inter_tex_y_down[i + 1]);

				this->Append(pp);
			}
		}
	}


	uint32_t const BLOCK_SIZE = 128;

	SATPostProcessCS::SATPostProcessCS()
		: PostProcessChain(L"SATCS")
	{
	}

	void SATPostProcessCS::InputPin(uint32_t index, TexturePtr const & tex)
	{
		if (0 == index)
		{
			uint32_t const tex_width = tex->Width(0);
			uint32_t const tex_height = tex->Height(0);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			TexturePtr inter_tex = rf.MakeTexture2D(tex_width, tex_height, 1, 1, EF_ABGR16F, 1, 0,
				EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered, nullptr);

			{
				int32_t const wave_x = (tex_width + BLOCK_SIZE - 1) / BLOCK_SIZE;
				PostProcessPtr pp = SyncLoadPostProcess("SAT.ppml", "in_block_scan_cs_x");
				pp->CSPixelPerThreadX(wave_x);
				pp->SetParam(0, wave_x);
				pp->InputPin(0, tex);
				pp->OutputPin(0, inter_tex);
				this->Append(pp);
			}
			{
				int32_t const wave_y = (tex_height + BLOCK_SIZE - 1) / BLOCK_SIZE;
				PostProcessPtr pp = SyncLoadPostProcess("SAT.ppml", "in_block_scan_cs_y");
				pp->CSPixelPerThreadY(wave_y);
				pp->SetParam(0, wave_y);
				pp->InputPin(0, inter_tex);
				pp->OutputPin(0, tex);
				this->Append(pp);
			}
		}
	}
}
