/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Component.h"
#include "Script/ScriptEngine.h"
#include "FileSystem/FileSystem.h"
#include "Object.h"

namespace Immortal
{

#define ASSET_SET_OBJECT() {                                                  \
        if (!vtable.__setId) {                                                \
            return;                                                           \
        }                                                                     \
        Anonymous params[] = {                                                \
            &id,                                                              \
            &scene,                                                           \
        };                                                                    \
        ScriptEngine::Invoke(vtable.__setId, object, params);                 \
    }

void ScriptComponent::Init(int id, Scene *scene)
{
#if HAVE_MONO
    auto namespaceName = std::string{ "Immortal" };
    className = FileSystem::ExtractFileName(path);

    object = ScriptEngine::CreateObject(namespaceName, className);
    vtable.__setId     = ScriptEngine::Search(object, "GameObject", "SetId_");
    vtable.__update    = ScriptEngine::Search(object, "GameObject", "Update");
    vtable.__onKeyDown = ScriptEngine::Search(object, "GameObject", "OnKeyDown");
#endif
}

void ScriptComponent::Update(int id, Scene *scene, float deltaTime)
{
#if HAVE_MONO
    ASSET_SET_OBJECT()
    Anonymous params[] = { &deltaTime };
    ScriptEngine::Invoke(vtable.__update, object, params);
#endif
}

void ScriptComponent::OnKeyDown(int id, Scene *scene, int keyCode)
{
#if HAVE_MONO
    ASSET_SET_OBJECT()
    Anonymous params[] = { &keyCode };
    ScriptEngine::Invoke(vtable.__onKeyDown, object, params);
#endif
}

}
