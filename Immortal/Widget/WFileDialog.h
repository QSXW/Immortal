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
};

}
