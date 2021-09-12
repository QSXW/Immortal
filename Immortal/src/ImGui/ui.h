#pragma once

#include "ImmortalCore.h"

#include <imgui.h>
#include "Render/Texture.h"

namespace Immortal::UI
{
static int s_UIContextID = 0;
static uint32_t s_Counter = 0;
static char s_IDBuffer[16];

static void PushID()
{
    ImGui::PushID(s_UIContextID++);
    s_Counter = 0;
}

static void PopID()
{
    ImGui::PopID();
    s_UIContextID--;
}

static void BeginPropertyGrid()
{
    PushID();
    ImGui::Columns(2);
}

static bool Property(const char* label, std::string& value, bool error = false)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    char buffer[256];
    strcpy(buffer, value.c_str());

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);

    if (error)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    if (ImGui::InputText(s_IDBuffer, buffer, 256))
    {
        value = buffer;
        modified = true;
    }
    if (error)
        ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static void Property(const char* label, const char* value)
{
    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    ImGui::InputText(s_IDBuffer, (char*)value, 256, ImGuiInputTextFlags_ReadOnly);

    ImGui::PopItemWidth();
    ImGui::NextColumn();
}

static bool Property(const char* label, bool& value)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::Checkbox(s_IDBuffer, &value))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool Property(const char* label, int& value)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::DragInt(s_IDBuffer, &value))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool PropertySlider(const char* label, int& value, int min, int max)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::SliderInt(s_IDBuffer, &value, min, max))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool PropertySlider(const char* label, float& value, float min, float max)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::SliderFloat(s_IDBuffer, &value, min, max))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool PropertySlider(const char* label, glm::vec2& value, float min, float max)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::SliderFloat2(s_IDBuffer, glm::value_ptr(value), min, max))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool PropertySlider(const char* label, glm::vec3& value, float min, float max)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::SliderFloat3(s_IDBuffer, glm::value_ptr(value), min, max))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool PropertySlider(const char* label, glm::vec4& value, float min, float max)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::SliderFloat4(s_IDBuffer, glm::value_ptr(value), min, max))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool Property(const char* label, float& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f, bool readOnly = false)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);

    if (!readOnly)
    {
        if (ImGui::DragFloat(s_IDBuffer, &value, delta, min, max))
            modified = true;
    }
    else
    {
        ImGui::InputFloat(s_IDBuffer, &value, 0.0F, 0.0F, "%.3f", ImGuiInputTextFlags_ReadOnly);
    }

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool Property(const char* label, glm::vec2& value, float delta = 0.1f)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::DragFloat2(s_IDBuffer, glm::value_ptr(value), delta))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool PropertyColor(const char* label, glm::vec3& value)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::ColorEdit3(s_IDBuffer, glm::value_ptr(value)))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool Property(const char* label, glm::vec3& value, float delta = 0.1f)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::DragFloat3(s_IDBuffer, glm::value_ptr(value), delta))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool Property(const char* label, glm::vec4& value, float delta = 0.1f)
{
    bool modified = false;

    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::DragFloat4(s_IDBuffer, glm::value_ptr(value), delta))
        modified = true;

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return modified;
}

static bool PropertyDropdown(const char* label, const char** options, int32_t optionCount, int32_t* selected)
{
    const char* current = options[*selected];
    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    bool changed = false;

    std::string id = "##" + std::string(label);
    if (ImGui::BeginCombo(id.c_str(), current))
    {
        for (int i = 0; i < optionCount; i++)
        {
            bool is_selected = (current == options[i]);
            if (ImGui::Selectable(options[i], is_selected))
            {
                current = options[i];
                *selected = i;
                changed = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return changed;
}

static bool PropertyDropdown(const char* label, const std::vector<std::string>& options, int32_t optionCount, int32_t* selected)
{
    const char* current = options[*selected].c_str();
    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    bool changed = false;

    std::string id = "##" + std::string(label);
    if (ImGui::BeginCombo(id.c_str(), current))
    {
        for (int i = 0; i < optionCount; i++)
        {
            bool is_selected = (current == options[i]);
            if (ImGui::Selectable(options[i].c_str(), is_selected))
            {
                current = options[i].c_str();
                *selected = i;
                changed = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();
    ImGui::NextColumn();

    return changed;
}

template<typename T>
static bool PropertyAssetReference(const char* label, Ref<T>& object, UINT64 supportedType)
{
    bool modified = false;

    /*ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    if (object)
    {
        if (object->Type != AssetType::Missing)
        {
            char* assetName = ((Ref<Asset>&)object)->FileName.data();
            ImGui::InputText("##assetRef", assetName, 256, ImGuiInputTextFlags_ReadOnly);
        }
        else
        {
            ImGui::InputText("##assetRef", "Missing", 256, ImGuiInputTextFlags_ReadOnly);
        }
    }
    else
    {
        ImGui::InputText("##assetRef", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
    }

    if (ImGui::BeginDragDropTarget())
    {
        auto data = ImGui::AcceptDragDropPayload("asset_payload");

        if (data)
        {
            AssetHandle assetHandle = *(AssetHandle*)data->Data;
            Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
            if (asset->Type == supportedType)
            {
                object = asset.As<T>();
                modified = true;
            }
        }
    }

    ImGui::PopItemWidth();
    ImGui::NextColumn();*/
    return modified;
}

static void EndPropertyGrid()
{
    ImGui::Columns(1);
    PopID();
}

static bool BeginTreeNode(const char* name, bool defaultOpen = true)
{
    ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
    if (defaultOpen)
        treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

    return ImGui::TreeNodeEx(name, treeNodeFlags);
}

static void EndTreeNode()
{
    ImGui::TreePop();
}

static int s_CheckboxCount = 0;

static void BeginCheckboxGroup(const char* label)
{
    ImGui::Text(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
}

static bool PropertyCheckboxGroup(const char* label, bool& value)
{
    bool modified = false;

    if (++s_CheckboxCount > 1)
        ImGui::SameLine();

    ImGui::Text(label);
    ImGui::SameLine();

    s_IDBuffer[0] = '#';
    s_IDBuffer[1] = '#';
    memset(s_IDBuffer + 2, 0, 14);
    itoa(s_Counter++, s_IDBuffer + 2, 16);
    if (ImGui::Checkbox(s_IDBuffer, &value))
        modified = true;

    return modified;
}

static void EndCheckboxGroup()
{
    ImGui::PopItemWidth();
    ImGui::NextColumn();
    s_CheckboxCount = 0;
}

}