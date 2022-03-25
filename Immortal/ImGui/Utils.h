#pragma once

#include "Core.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Immortal::UI
{

struct Context
{
    int ID = 0;
    uint32_t Counter = 0;
};

static inline Context context;

static inline void PushID()
{
    ImGui::PushID(context.ID++);
    context.Counter = 0;
}

static inline void PopID()
{
    ImGui::PopID();
    context.ID--;
}

static inline void BeginPropertyGrid()
{
    PushID();
    ImGui::Columns(2);
}

static inline void EndPropertyGrid()
{
    ImGui::Columns(1);
    PopID();
}

static bool DrawVec3Control(const std::string &label, Vector3 &values, float speed = 0.01f, float resetValue = 0.0f, float columnWidth = 64.0f)
{
    bool modified = false;

    const ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[1];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y  *2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("X", buttonSize))
    {
        values.x = resetValue;
        modified = true;
    }

    ImGui::PopFont();
    ImGui::PopStyleColor(4);

    ImGui::SameLine();
    modified |= ImGui::DragFloat("##X", &values.x, speed, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Y", buttonSize))
    {
        values.y = resetValue;
        modified = true;
    }

    ImGui::PopFont();
    ImGui::PopStyleColor(4);

    ImGui::SameLine();
    modified |= ImGui::DragFloat("##Y", &values.y, speed, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Z", buttonSize))
    {
        values.z = resetValue;
        modified = true;
    }

    ImGui::PopFont();
    ImGui::PopStyleColor(4);

    ImGui::SameLine();
    modified |= ImGui::DragFloat("##Z", &values.z, speed, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();

    return modified;
}

}
