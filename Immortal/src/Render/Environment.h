#pragma once

#include "ImmortalCore.h"

#include "Texture.h"

namespace Immortal {

    class IMMORTAL_API Environment
    {
    public:
        /* Specular Bidirectional Reflectance Distribution Function Look up table */
        Ref<Texture2D> SpecularBRDFLookUpTable;
        Ref<TextureCube> IrradianceMap;

        Environment(Ref<TextureCube> &skyboxTexture);
        ~Environment();
    };

}

