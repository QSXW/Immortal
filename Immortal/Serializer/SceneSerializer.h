#pragma once

#include "Core.h"

namespace Immortal
{

class Scene;
class IMMORTAL_API SceneSerializer
{
public:
    SceneSerializer();

    ~SceneSerializer() = default;
        
    void Serialize(Scene *scene, const std::string &filepath);

    bool Deserialize(Scene *scene, const std::string &filepath);
};

}
