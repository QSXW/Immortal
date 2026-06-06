/**
 * Copyright (C) 2022-2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "FileSystem/FileSystem.h"
#include "Shared/Async.h"
#include "WFileDialog.h"
#include <stack>

#ifdef _WIN32
#include <shlobj_core.h>
#endif

namespace Immortal
{

#define ICON_DIR "\xF3\xB0\x80\x80"

WFileDialog::WFileDialog(Widget *v) :
    Widget{ v },
    color{ 1.0f, 1.0f, 1.0f, 0.0f }
{
    auto &io = ImGui::GetIO();
    
    dirIcon    = Graphics::CreateTexture("C:/Users/qsxw/Downloads/directory (2).png");
	volumeIcon = Graphics::CreateTexture("C:/Users/qsxw/Downloads/disk.png");

    icon = Graphics::CreateTexture("Assets/Icon/WFileDialogIcons.png");

    constexpr int totalIconCount = 11;
    constexpr float factor = 1.0f / totalIconCount;
    auto width = icon->GetWidth();
    auto height = icon->GetHeight();

    float splitPos = 512.0f / height;
    float xFactor = 64.0f / width;
    float yFactor = 64.0f / height;
    //for (int i = (int)FileType::Directory; i < (int)FileType::BIN; i++)
    //{
    //    resources[(int)i].Resource(icon, { i * factor, 0 }, { (i + 1) * factor, splitPos });
    //}

    Source(std::filesystem::current_path());
    Connect([=, this] {
        if (!path)
        {
            return;
        }

    //ImGui::PushStyleColor(ColorStyle::ChildBg, navigateBackgroundColor);
    //ImGui::BeginChild(IM_ANONY, { ImGui::GetWindowWidth(), 32.0f }, true, ImGuiWindowFlags_AlwaysUseWindowPadding);

    //auto window = ImGui::GetCurrentWindow();
    //MOVEPOS(10, 5);
    //constexpr ImVec2 buttonSize = { 18.0f, 18.0f };
    //ImGui::PushStyleColor(ColorStyle::Button, { 0, 0, 0, 0.0f });
    //ImGui::PushStyleColor(ColorStyle::ButtonHovered, ImGui::RGBA32(0x007bffff));

    //if (ImGui::ImageButton(WIMAGE(icon), buttonSize, { 0 * xFactor, splitPos }, { 1 * xFactor, splitPos + yFactor }, -1, { 0, 0, 0, 0 }, { 1, 1, 1, 1.0f }))
    //{
    //    Source(lastPath);
    //}
    //ImGui::SameLine(0, 5);
    //if (ImGui::ImageButton(WIMAGE(icon), buttonSize, { 1 * xFactor, splitPos + yFactor }, { 0 * xFactor, splitPos }, -1, { 0, 0, 0, 0 }, { 1, 1, 1, 1.0f }))
    //{
    //    Source(path.Parent());
    //}
    //ImGui::SameLine(0, 5);
    //static float toggleIconPos = 2.0f;
    //static bool on = true;
    //{
    //    WidgetLock lock{ "@FDType" };
    //    if (ImGui::ImageButton(WIMAGE(icon), buttonSize, { toggleIconPos * xFactor, splitPos }, { (toggleIconPos + 1.0f) * xFactor, splitPos + yFactor }, -1, { 0, 0, 0, 0 }, { 1, 1, 1, 1.0f }))
    //    {
    //        toggleIconPos = toggleIconPos == 2.0f ? 3.0f : 2.0f;
    //        on = !on;
    //    }
    //}

    //ImGui::SameLine(0, 10);

    //char buffer[4096] = {};
    //auto callback = [](ImGuiInputTextCallbackData *input) -> int {
    //    std::string str = input->Buf;
    //    size_t pos = str.find(' ');
    //    if (str.substr(0, pos++) == "cd")
    //    {
    //        FileSystem::Path path = str.substr(pos);
    //        if (path)
    //        {
    //            ((WFileDialog *)input->UserData)->Source(path);
    //        }
    //    }

    //    return 0;
    //};

    //ImGui::PushFont(GuiLayer::NotoSans.Bold);
    //std::string filepath = path.string();
    //size_t pos = 0;
    //size_t nextPos = 0;
    //while ((nextPos = filepath.find('/', pos)) != std::string::npos ||
    //    (nextPos = filepath.find('\\', pos)) != std::string::npos)
    //{
    //    if (nextPos == 0)
    //    {
    //        pos = nextPos + 1;
    //        continue;
    //    }
    //    filepath[nextPos] = '\0';
    //    if (ImGui::Button(&filepath[pos], { 0, 24.0f }))
    //    {
    //        Source(path.string().substr(0, nextPos));
    //    }
    //    ImGui::SameLine();
    //    ImGui::Button(">");
    //    pos = nextPos + 1;
    //    ImGui::SameLine();
    //}
    //if (pos < filepath.size())
    //{
    //    if (ImGui::Button(&filepath[pos], { 0, 24.0f }))
    //    {

    //    }
    //}
    //ImGui::PopFont();

    //ImGui::PopStyleColor(2);
    //ImGui::EndChild();
    //ImGui::PopStyleColor(1);

    ImGui::PushStyleColor(ColorStyle::ChildBg, backgroundColor);
    //if (on)
    {
		DrawListDirectories();
        // DrawImageDirectories();
    }
    //else
    //{
    //    DrawDirectories();
    //}
    ImGui::PopStyleColor(1);
    });
}

void WFileDialog::DrawTreeNodes(const std::vector<FileSystem::DirectoryEntry> &entries, bool isVolume = false)
{
	for (auto &dir : entries)
	{
		if (dir.IsDirectory())
		{
			auto id = ImGui::GetID(dir.path.c_str());
			bool isOpen = ImGui::TreeNodeUpdateNextOpen(id, ImGuiTreeNodeFlags_None);

            static const char *arrows[] = {
				"\xef\x84\x85",
			    "\xef\x84\x87"
			};

            int arrowIndex = !!isOpen;
            
			bool nodeOpen = ImGui::TreeNodeEx(dir.path.c_str(), ImGuiTreeNodeFlags_FramePadding, "%s", arrows[arrowIndex]);
			ImGui::SameLine();

            auto selectableId = 0;

            if (isVolume)
            {
				selectableId = ImGui::GetID(dir.path.c_str());
            }
            else
            {
				selectableId = ImGui::GetID(dir.GetFileName());
            }

            bool hasPush = false;
            if (selectableId == selectedId)
            {
				hasPush = true;
				ImGui::PushStyleColor(ImGuiCol_Button,        { 0.8, 0.4, 0.6, 0.5 });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.8, 0.4, 0.6, 0.5  });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.8, 0.4, 0.6, 0.5});
            }

            auto itemSize = ImGui::GetItemRectSize();
            if (isVolume)
            {
				ImGui::ImageButton(WIMAGE(volumeIcon), {13, 13});
            }
            else
            {
				ImGui::ImageButton(WIMAGE(dirIcon), {13, 13});
            }
			ImGui::SameLine();


            ImGui::Dummy({ 5.0f, 0.0f });
			ImGui::SameLine();
            bool isButtonClicked = false;
            ImVec2 buttonSize = { ImGui::GetWindowWidth() - itemSize.x, itemSize.y };
            const char *name = dir.path.c_str();

            ImGui::PushID(selectableId);
			if (!isVolume)
			{
				isButtonClicked = ImGui::Button(dir.GetFileName(), buttonSize);
			}
            else
            {
				char volumeName[MAX_PATH] = {};
				sprintf(volumeName, "%s (%c:)", "New Volume", dir.path.c_str()[0]);
				isButtonClicked = ImGui::Button(volumeName, buttonSize);
            }
			ImGui::PopID();

            if (isButtonClicked)
			{
				selectedId = selectableId;
				selectedPath = dir;
				onSelected(selectedPath.path);
			};

            if (hasPush)
            {
				ImGui::PopStyleColor(3);
            }

			if (nodeOpen)
			{
				std::vector<FileSystem::DirectoryEntry> e;
				FileSystem::ListDirectory(dir.path, e);
                if (!e.empty())
                {
					DrawTreeNodes(e);
                }
				ImGui::TreePop();
			}
		}
	}
}

void WFileDialog::DrawListDirectories()
{
	WidgetLock glock{this};
	ImGui::BeginChild("###");
	auto window = ImGui::GetCurrentWindow();
	float width = ImGui::GetWindowWidth();

    ImGui::PushFont(GuiLayer::NotoSans.Bold);

	path = FileSystem::Path::Current();

#ifdef _WIN32
	wchar_t desktop[MAX_PATH] = {};
	SHGetSpecialFolderPathW(0, desktop, CSIDL_DESKTOPDIRECTORY, 0);
	path = desktop;
#endif

	std::vector<FileSystem::DirectoryEntry> entries;
    FileSystem::DirectoryEntry entry = { path.u8string(), FileType::Directory };

	entries.emplace_back(std::move(entry));

    std::vector<std::vector<FileSystem::DirectoryEntry>> volumes;

    wchar_t volumeName[MAX_PATH] = {};
	HANDLE handle = FindFirstVolumeW(volumeName, SL_ARRAY_LENGTH(volumeName));
    if (handle)
    {
        
        do {
			DWORD length = 0;
			wchar_t volumePathName[MAX_PATH] = {};
			if (GetVolumePathNamesForVolumeNameW(volumeName, volumePathName, SL_ARRAY_LENGTH(volumePathName), &length) && volumePathName[0])
            {
				FileSystem::DirectoryEntry entry = {WString2String(volumePathName, StringEncoding::UTF8), FileType::Directory};
				volumes.emplace_back(std::vector<FileSystem::DirectoryEntry>{entry});
            }
		} while (FindNextVolumeW(handle, volumeName, SL_ARRAY_LENGTH(volumeName)));
		FindVolumeClose(handle);
    }

    ImGui::PushStyleColor(ImGuiCol_Text,          {1.0f, 1.0f, 1.0f, 0.95f});
    ImGui::PushStyleColor(ImGuiCol_Button,        {0, 0, 0, 0});
	ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0, 0, 0, 0});
	//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0, 0, 0, 0});
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 5.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0., 0.5 });

	DrawTreeNodes(entries);

    for (auto &volume : volumes)
    {
		DrawTreeNodes(volume, true);
    }

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);

	ImGui::PopFont();

    ImGui::EndChild();
}

void WFileDialog::DrawDirectories()
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
        ImGui::Image(WIMAGE(icon), {height - 6, height - 6 }, uv0, uv1);
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
            ImGui::SetTooltip("%s", dir.path.c_str());
        }
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            static FileSystem::DirectoryEntry file = {
                dir.path,
                FileType::RegularFile
            };

            FileSystem::DirectoryEntry *entry = { &file };
            ImGui::SetDragDropPayload("LOAD_FILE", (void *)&entry, sizeof(&entry));
            ImGui::ImageButton(WIMAGE(icon), {64, 64}, uv0, uv1);
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

void WFileDialog::DrawImageDirectories()
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
            ImGui::ImageButton("###", WIMAGE(icon), { height, height }, uv0, uv1);
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
                            system(("code " + (std::string &)dir.path).c_str());
                            });
                    }
                    else if (dir.type == FileType::EXE)
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
                ImGui::SetTooltip("%s", dir.path.c_str());
            }
            if (ImGui::BeginDragDropSource())
            {
                FileSystem::DirectoryEntry *entry = { &dir };
                ImGui::SetDragDropPayload("LOAD_FILE", (void *)&entry, sizeof(&entry));
                ImGui::ImageButton(WIMAGE(icon), {height, height}, uv0, uv1);
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

void WFileDialog::OnButtonClicked(const FileSystem::DirectoryEntry &dir)
{
    if (dir.IsDirectory())
    {
        Source(dir.path);
    }
    else
    {
        callback(dir.path);
    }
}

WFileDialog *WFileDialog::Source(const FileSystem::Path &_path)
{
    lastPath = path;
    path = _path;

    Async::Execute([&] {
        offset = path.Length() + (path.Parent() == path ? 0 : 1);
        std::vector<FileSystem::DirectoryEntry> entries;
        FileSystem::ListDirectory(path, entries);
        for (auto &dir : entries)
        {
            if (dir.IsRegularFile())
            {
                dir.type = FileSystem::GetFileType(dir.path);
            }
        }

        std::unique_lock lock{ mutex };
        directories = std::move(entries);
    });

    return this;
}

}
