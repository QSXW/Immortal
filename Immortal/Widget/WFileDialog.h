/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "FileSystem/FileSystem.h"
#include "Framework/Async.h"
#include "Widget.h"

namespace Immortal
{

class IMMORTAL_API WFileDialog : public Widget
{
public:
    WIDGET_SET_PROPERTIES(WFileDialog)
    WIDGET_PROPERTY_COLOR
    WIDGET_PROPERTY_BACKGROUND_COLOR
    WIDGET_PROPERTY_VAR_COLOR(NavigateBackgroundColor, navigateBackgroundColor)

public:
    WFileDialog(Widget *v = nullptr) :
        Widget{ v },
        color{ 1.0f, 1.0f, 1.0f, 0.0f }
    {
        icon = Render::CreateImage("Assets/Icon/WFileDialogIcons.png");
        constexpr int totalIconCount = 11;
        constexpr float factor = 1.0f / totalIconCount;
        auto width = icon->Width();
        auto height = icon->Height();

        float splitPos = 512.0f / height;
        float xFactor = 64.0f / width;
        float yFactor = 64.0f / height;
        for (int i = (int)FileType::Directory; i < (int)FileType::Binary; i++)
        {
            resources[(int)i].Resource(icon, { i * factor, 0 }, { (i + 1) * factor, splitPos });
        }

        Source(std::filesystem::current_path());
        Connect([=, this] {
            if (!path)
            {
                return;
            }

        ImGui::PushStyleColor(ColorStyle::ChildBg, navigateBackgroundColor);
        ImGui::BeginChild(IM_ANONY, { ImGui::GetWindowWidth(), 32.0f }, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding);

        auto window = ImGui::GetCurrentWindow();
        MOVEPOS(10, 5);
        constexpr ImVec2 buttonSize = { 18.0f, 18.0f };
        ImGui::PushStyleColor(ColorStyle::Button, { 0, 0, 0, 0.0f });
        ImGui::PushStyleColor(ColorStyle::ButtonHovered, ImGui::RGBA32(0x007bffff));

        if (ImGui::ImageButton(WIMAGE(*icon), buttonSize, { 0 * xFactor, splitPos }, { 1 * xFactor, splitPos + yFactor }, -1, { 0, 0, 0, 0 }, { 1, 1, 1, 1.0f }))
        {
            Source(lastPath);
        }
        ImGui::SameLine(0, 5);
        if (ImGui::ImageButton(WIMAGE(*icon), buttonSize, { 1 * xFactor, splitPos + yFactor }, { 0 * xFactor, splitPos }, -1, { 0, 0, 0, 0 }, { 1, 1, 1, 1.0f }))
        {
            Source(path.Parent());
        }
        ImGui::SameLine(0, 5);
        static float toggleIconPos = 2.0f;
        static bool on = true;
        {
            WidgetLock lock{ "@FDType" };
            if (ImGui::ImageButton(WIMAGE(*icon), buttonSize, { toggleIconPos * xFactor, splitPos }, { (toggleIconPos + 1.0f) * xFactor, splitPos + yFactor }, -1, { 0, 0, 0, 0 }, { 1, 1, 1, 1.0f }))
            {
                toggleIconPos = toggleIconPos == 2.0f ? 3.0f : 2.0f;
                on = !on;
            }
        }

        ImGui::SameLine(0, 10);

        char buffer[4096] = {};
        auto callback = [](ImGuiInputTextCallbackData *input) -> int {
            std::string str = input->Buf;
            size_t pos = str.find(' ');
            if (str.substr(0, pos++) == "cd")
            {
                FileSystem::Path path = str.substr(pos);
                if (path)
                {
                    ((WFileDialog *)input->UserData)->Source(path);
                }
            }

            return 0;
        };

        ImGui::PushFont(GuiLayer::NotoSans.Bold);
        std::string filepath = path.string();
        size_t pos = 0;
        size_t nextPos = 0;
        while ((nextPos = filepath.find('/', pos)) != std::string::npos ||
            (nextPos = filepath.find('\\', pos)) != std::string::npos)
        {
            if (nextPos == 0)
            {
                pos = nextPos + 1;
                continue;
            }
            filepath[nextPos] = '\0';
            if (ImGui::Button(&filepath[pos], { 0, 24.0f }))
            {
                Source(path.string().substr(0, nextPos));
            }
            ImGui::SameLine();
            ImGui::Button(">");
            pos = nextPos + 1;
            ImGui::SameLine();
        }
        if (pos < filepath.size())
        {
            if (ImGui::Button(&filepath[pos], { 0, 24.0f }))
            {

            }
        }
        ImGui::PopFont();

        ImGui::PopStyleColor(2);
        ImGui::EndChild();
        ImGui::PopStyleColor(1);

        ImGui::PushStyleColor(ColorStyle::ChildBg, backgroundColor);
        if (on)
        {
            DrawImageDirectories();
        }
        else
        {
            DrawDirectories();
        }
        ImGui::PopStyleColor(1);
        });
    }

    void DrawDirectories()
    {
        WidgetLock glock{ this };
        ImGui::BeginChild("###");
        auto window = ImGui::GetCurrentWindow();
        float width = ImGui::GetWindowWidth();
        constexpr float height = 24.0f;

        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { padding.right, padding.bottom });
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0, 0 });

        std::unique_lock lock{ mutex };
        for (size_t i = 0; i < directories.size(); i++)
        {
            auto &dir = directories[i];
            window->DC.CursorPos += ImVec2{ padding.left, padding.top };

            auto &uv0 = resources[(int)dir.type].uv._0;
            auto &uv1 = resources[(int)dir.type].uv._1;
            MOVEPOS(12, 12);
            ImGui::Image(WIMAGE(*icon), {height - 6, height - 6 }, uv0, uv1);
            ImGui::SameLine();
            MOVEY(-4);
            WidgetLock lock{ &dir };
            ImGui::Button(dir.path.c_str() + offset, { width, height });
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseDoubleClicked(0))
            {
                OnButtonClicked(dir);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(dir.path.c_str());
            }
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                static FileSystem::DirectoryEntry file = {
                    .path = dir.path,
                    .type = FileType::RegularFile
                };

                FileSystem::DirectoryEntry *entry = { &file };
                ImGui::SetDragDropPayload("LOAD_FILE", (void *)&entry, sizeof(&entry));
                ImGui::ImageButton((ImTextureID)(uint64_t)*icon, { 64, 64 }, uv0, uv1);
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 64);
                ImGui::Text("%s", dir.path.c_str() + offset);
                ImGui::PopTextWrapPos();
                ImGui::EndDragDropSource();
            }
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(1);

        ImGui::EndChild();
    }

    void DrawImageDirectories()
    {
        WidgetLock glock{ this };
        ImGui::BeginChild("###");
        EXPORT_WINDOW
        MOVEPOS(20, 8);
        float width = ImGui::GetWindowWidth();
        constexpr float height = 60.0f;
        size_t num = width / (height + 40);
        window = ImGui::GetCurrentWindow();
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { padding.right, padding.bottom });
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0, 0 });

        std::unique_lock lock{ mutex };
        for (size_t i = 0; i < directories.size(); i += num)
        {
            ImGui::BeginColumns("FileColumn", num, ImGuiOldColumnFlags_NoBorder | ImGuiOldColumnFlags_NoForceWithinWindow);
            for (size_t j = 0; j < num; j++)
            {
                if (i + j >= directories.size())
                {
                    break;
                }

                auto &dir = directories[i + j];
                MOVEPOS(20, 20);

                auto &uv0 = resources[(int)dir.type].uv._0;
                auto &uv1 = resources[(int)dir.type].uv._1;

                WidgetLock lock{ &dir };
                ImGui::ImageButton("###", (ImTextureID)(uint64_t)*icon, { height, height }, uv0, uv1);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
                {
                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        OnButtonClicked(dir);
                    }
                }
                if (ImGui::BeginPopupContextItem())
                {
                    MOVEX(4);
                    if (ImGui::Button(dir.IsDirectory() ? "Open Directory" : "Open File", { 168.0f, 24.0f }))
                    {
                        if (dir.type == FileType::CPP)
                        {
                            Async::Execute([=]() {
                                system(("code " + dir.path).c_str());
                                });
                        }
                        else if (dir.type == FileType::Executable)
                        {
                            Async::Execute([=]() {
                                system(dir.path.c_str());
                                });
                        }
                        else
                        {
                            OnButtonClicked(dir);
                        }
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
                else if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
                {
                    ImGui::SetTooltip(dir.path.c_str());
                }
                if (ImGui::BeginDragDropSource())
                {
                    FileSystem::DirectoryEntry *entry = { &dir };
                    ImGui::SetDragDropPayload("LOAD_FILE", (void *)&entry, sizeof(&entry));
                    ImGui::ImageButton((ImTextureID)(uint64_t)*icon, { height, height }, uv0, uv1);
                    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + height);
                    ImGui::Text("%s", dir.path.c_str() + offset);
                    ImGui::PopTextWrapPos();
                    ImGui::EndDragDropSource();
                }

                MOVEX(20);
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + height);
                ImGui::Text("%s", dir.path.c_str() + offset);
                ImGui::PopTextWrapPos();
                ImGui::NextColumn();
            }
            ImGui::EndColumns();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(1);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("@RC");
        }

        if (ImGui::BeginPopup("@RC"))
        {
            if (ImGui::Button("Create Directory", { 168.0f, 24.0f }))
            {
                FileSystem::CreateDirectory((path / "New Directory").string());
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::EndChild();
    }

    void OnButtonClicked(const FileSystem::DirectoryEntry &dir)
    {
        if (dir.IsDirectory())
        {
            Source(UTF82Byte(dir.path));
        }
        else
        {
            callback(UTF82Byte(dir.path));
        }
    }

    WidgetType *Source(const FileSystem::Path &_path)
    {
        lastPath = path;
        path = _path;

        Async::Execute([&] {
            offset = path.Length() + (path.Parent() == path ? 0 : 1);
            std::vector<FileSystem::DirectoryEntry> entries;
            FileSystem::List(path, entries);
            for (auto &dir : entries)
            {
                if (dir.IsRegularFile())
                {
                    dir.type = FileSystem::GetFileType(dir.path);
                }
                dir.path = Byte2UTF8(dir.path);
            }

            std::unique_lock lock{ mutex };
            directories = std::move(entries);
        });

        return this;
    }

    const std::string &Source() const
    {
        return path.string();
    }

    template <class T>
    WidgetType *CallBack(T &&_callback)
    {
        callback = _callback;
        return this;
    }

protected:
    std::mutex mutex;

    FileSystem::Path path;

    FileSystem::Path lastPath;

    std::vector<FileSystem::DirectoryEntry> directories;

    std::function<void(const std::string &path)> callback;

    URef<Image> icon;

    std::array<WImageResource, 8> resources;

    FileSystem::DirectoryEntry *selectEntry;

    size_t offset = 0;
};

}
