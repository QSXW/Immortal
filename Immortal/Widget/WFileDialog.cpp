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

WFileDialog::WFileDialog(Widget *v) :
    Widget{ v }
{

}

}
