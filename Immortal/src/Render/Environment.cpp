#include "impch.h"
#include "Environment.h"

#include "Shader.h"

namespace Immortal {

	Environment::Environment(Ref<TextureCube> &skyboxTexture)
	{
		constexpr uint32_t IrradianceMapSize = 32;
		constexpr uint32_t BRDFLookUpTableSize = 256;

		// Compute diffuse irradiance cubemap.
		{
			auto irradianceShader = Shader::Create("assets/shaders/irmap_cs.glsl");

			IrradianceMap = TextureCube::Create(IrradianceMapSize, IrradianceMapSize, Texture::Description{ Texture::Format::RGBA16F, Texture::Wrap::Clamp, Texture::Filter::Linear }, 1);

			irradianceShader->Map();
			skyboxTexture->Map();
			IrradianceMap->BindImageTexture(true);
			irradianceShader->DispatchCompute(IrradianceMap->Width() / 32, IrradianceMap->Height() / 32, 6);
		}

		// Compute Cook-Torrance BRDF 2D LUT for split-sum approximation.
		{
			auto specularBRDFShader = Shader::Create("assets/shaders/spbrdf_cs.glsl");

			SpecularBRDFLookUpTable = Texture2D::Create(BRDFLookUpTableSize, BRDFLookUpTableSize,
				Texture::Description{ Texture::Format::RG16F, Texture::Wrap::Clamp, Texture::Filter::Linear }, 1);

			specularBRDFShader->Map();
			SpecularBRDFLookUpTable->BindImageTexture(false);
			specularBRDFShader->DispatchCompute(SpecularBRDFLookUpTable->Width() / 32, SpecularBRDFLookUpTable->Height() / 32, 1);
		}
	}

	Environment::~Environment()
	{

	}
}

