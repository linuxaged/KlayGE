#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
namespace
{
	using namespace KlayGE;

	void CompressNormal(std::vector<uint8_t>& normals)
	{
		for (size_t i = 0; i < normals.size() / 4; ++ i)
		{
			normals[i * 4 + 1] = normals[i * 4 + 1];
			normals[i * 4 + 3] = normals[i * 4 + 2];
			normals[i * 4 + 0] = 0;
			normals[i * 4 + 2] = 0;		
		}
	}

	TexturePtr CompressNormalMapCube(TexturePtr normal_map, ElementFormat new_format)
	{
		uint32_t const size = normal_map->Width(0);
		TexturePtr new_normal_map = Context::Instance().RenderFactoryInstance().MakeTextureCube(size, 1, new_format);
		std::vector<uint8_t> normals(size * size * 4);

		for (int i = 0; i < 6; ++ i)
		{
			normal_map->CopyToMemoryCube(static_cast<Texture::CubeFaces>(i), 0, &normals[0]);
			CompressNormal(normals);
			new_normal_map->CopyMemoryToTextureCube(static_cast<Texture::CubeFaces>(i), 0,
				&normals[0], EF_ARGB8, size, size, 0, 0, size, size);
		}

		return new_normal_map;
	}

	TexturePtr CompressNormalMap2D(TexturePtr normal_map, ElementFormat new_format)
	{
		uint32_t const width = normal_map->Width(0);
		uint32_t const height = normal_map->Height(0);
		TexturePtr new_normal_map = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, new_format);
		std::vector<uint8_t> normals(width * height * 4);

		{
			normal_map->CopyToMemory2D(0, &normals[0]);
			CompressNormal(normals);
			new_normal_map->CopyMemoryToTexture2D(0, &normals[0], EF_ARGB8,
				width, height, 0, 0, width, height);
		}

		return new_normal_map;
	}

	TexturePtr CompressNormalMap(TexturePtr normal_map, ElementFormat new_format)
	{
		TexturePtr new_normal_map;

		switch (normal_map->Type())
		{
		case Texture::TT_2D:
			{
				TexturePtr temp = Context::Instance().RenderFactoryInstance().MakeTexture2D(
					normal_map->Width(0), normal_map->Height(0), 1, EF_ARGB8);
				normal_map->CopyToTexture(*temp);
				new_normal_map = CompressNormalMap2D(temp, new_format);
			}
			break;

		case Texture::TT_Cube:
			{
				TexturePtr temp = Context::Instance().RenderFactoryInstance().MakeTextureCube(
					normal_map->Width(0), 1, EF_ARGB8);
				normal_map->CopyToTexture(*temp);
				new_normal_map = CompressNormalMapCube(temp, new_format);
			}
			break;

		default:
			cout << "Unsupported texture type" << endl;
			break;
		}

		return new_normal_map;
	}
}

class EmptyApp : public KlayGE::App3DFramework
{
public:
	void DoUpdate(uint32_t /*pass*/)
	{
	}
};

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc < 3)
	{
		cout << "ʹ�÷���: NormalMapCompressor xxx.dds yyy.dds [AL8 | DXT5]" << endl;
		return 1;
	}

	EmptyApp app;

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;

	app.Create("NormalMapCompressor", settings);

	ElementFormat new_format = EF_DXT5;
	if (argc >= 4)
	{
		std::string format_str(argv[3]);
		if ("AL8" == format_str)
		{
			new_format = EF_AL8;
		}
	}

	TexturePtr new_normal_map = CompressNormalMap(LoadTexture(argv[1]), new_format);
	SaveTexture(new_normal_map, argv[2]);

	cout << "Normal map is saved to " << argv[2] << endl;

	return 0;
}
