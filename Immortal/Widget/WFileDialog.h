/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "FileSystem/FileSystem.h"
#include "Shared/Async.h"
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
    WFileDialog(Widget *v = nullptr);

    void DrawTreeNodes(const std::vector<FileSystem::DirectoryEntry> &entries, bool isVolume);

    void DrawListDirectories();

    void DrawDirectories();

    void DrawImageDirectories();

    void OnButtonClicked(const FileSystem::DirectoryEntry &dir);

    WFileDialog *Source(const FileSystem::Path &_path);

public:
    std::string Source() const
    {
        return path.string();
    }

    template <class T>
    WidgetType *CallBack(T &&_callback)
    {
        callback = _callback;
        return this;
    }

    template <class T>
    WidgetType *OnSelected(T &&_callback)
    {
		onSelected = _callback;
		return this;
    }

protected:
    std::mutex mutex;

    ImGuiID selectedId = 0;

    FileSystem::DirectoryEntry selectedPath;

    FileSystem::Path path;

    FileSystem::Path lastPath;

    std::vector<FileSystem::DirectoryEntry> directories;

    std::function<void(const String &path)> callback;

    std::function<void(const String &path)> onSelected;

    URef<Image> icon;

    std::array<WImageResource, 8> resources;

    FileSystem::DirectoryEntry *selectEntry;

    URef<Image> dirIcon;

    URef<Image> volumeIcon;

    size_t offset = 0;
};

}
