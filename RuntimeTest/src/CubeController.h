#pragma once

#include <Immortal/Scene/ScriptableObject.h>

namespace Immortal
{
    class __declspec(dllexport) CubeController : public ScriptableObject
    {
    public:
        void OnStart();
        void OnDestroy();
        void OnUpdate();
    };

}