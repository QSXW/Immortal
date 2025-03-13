#pragma once

#include "Shared/IObject.h"
#include "String/IString.h"
#include "Graphics.h"

namespace Immortal
{

struct Material
{
	Material() :
		Name{"Untitled"},
        AlbedoColor{ 0.995f, 0.995f, 0.995f, 1.0f },
        Metallic{ 1.0f },
        Roughness{ 1.0f },
		Opacity{1.0f}
    {
        Textures.Albedo    = Graphics::Preset()->Textures.White;
		Textures.Normal    = Graphics::Preset()->Textures.Normal;
        Textures.Specular  = Textures.Albedo;
        Textures.Metallic  = Textures.Albedo;
        Textures.Roughness = Textures.Albedo;
		Textures.AmbientOcclusion = Textures.Albedo;
    }

    struct {
        Ref<Texture> Albedo;
        Ref<Texture> Normal;
        Ref<Texture> Specular;
		Ref<Texture> Metallic;
        Ref<Texture> Roughness;
		Ref<Texture> AmbientOcclusion;
    } Textures;

    struct
	{
		String Diffuse;
		String Normal;
		String Specular;
		String Metallic;
		String Roughness;
		String AmbientOcclusion;
	} Pathes;

    std::string Name;
    Vector4 AlbedoColor;
	Vector4 Specular;
	Vector4 Ambient;
	Vector4 Emissive;
    float   Metallic;
    float   Roughness;
    float   Opacity;
};

}
