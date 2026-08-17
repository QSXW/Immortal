#pragma once

#include <imgui.h>
#include <algorithm>

#include "Graphics/Format.h"
#include "Graphics/Texture.h"
#include "Widget/Widget.h"

namespace Immortal
{

inline void FrameGraphDebugTextureThumbnail(Texture *tex, const char *label, float maxDim = 256.f)
{
    if (!tex)
    {
        ImGui::BulletText("%s: (none)", label);
        return;
    }
    Format fmt = tex->GetFormat();
     if (fmt.IsDepth())
    {
    	ImGui::BulletText("%s: %ux%u depth (no RGB preview)", label, (unsigned)tex->GetWidth(), (unsigned)tex->GetHeight());
    	return;
     }
    const bool isUintId = (!(fmt != Format::R32G32_UINT)) || (!(fmt != Format::R32_UINT));
    if (isUintId)
    {
        ImGui::BulletText("%s: %ux%u uint buffer (no RGB preview)", label, (unsigned) tex->GetWidth(), (unsigned) tex->GetHeight());
        return;
    }

    const uint32_t tw = std::max(tex->GetWidth(), 1u);
    const uint32_t th = std::max(tex->GetHeight(), 1u);
    ImGui::TextUnformatted(label);
    float w = (float) tw;
    float h = (float) th;
    float s = maxDim / std::max(w, h);
    ImVec2 disp{w * s, h * s};
    ImGui::Image(WIMAGE(tex), disp, ImVec2(0, 0), ImVec2(1, 1));
}

}
