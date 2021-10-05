#pragma once

#include "ImmortalCore.h"

#include "Texture.h"

namespace Immortal
{

class IMMORTAL_API Environment
{
public:
    /* Specular Bidirectional Reflectance Distribution Function Look up table */
    std::shared_ptr<Texture2D> SpecularBRDFLookUpTable;

    std::shared_ptr<TextureCube> IrradianceMap;

    Environment(std::shared_ptr<TextureCube> &skyboxTexture);

    ~Environment();
};

}

