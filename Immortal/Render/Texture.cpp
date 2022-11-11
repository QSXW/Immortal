#include "Texture.h"
#include "Render.h"

namespace Immortal
{

Texture *Texture::CreateInstance(const std::string& filepath, const Description &description)
{
    return Render::CreateTexture(filepath, description);
}

}
