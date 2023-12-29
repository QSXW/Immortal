
/**
 * Copyright (C) 2023, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 *
 * dear imgui: Renderer Backend for Immortal Graphics
 */

#pragma once
#ifndef IMGUI_DISABLE
#include "imgui.h"      // IMGUI_IMPL_API

// Called by user code
IMGUI_IMPL_API bool         ImGui_ImplImmortal_Init(Immortal::Device *device, Immortal::Queue *queue, Immortal::Swapchain *swapchain, uint32_t swapchainBufferCount);
IMGUI_IMPL_API void         ImGui_ImplImmortal_Shutdown();
IMGUI_IMPL_API void         ImGui_ImplImmortal_NewFrame();
IMGUI_IMPL_API void         ImGui_ImplImmortal_RenderDrawData(ImDrawData *drawData, Immortal::CommandBuffer *commandBuffer);
IMGUI_IMPL_API bool         ImGui_ImplImmortal_CreateFontsTexture();
IMGUI_IMPL_API void         ImGui_ImplImmortal_InitPlatformInterface();
IMGUI_IMPL_API void         ImGui_ImplImmortal_ShutdownPlatformInterface();
#endif /* #ifndef IMGUI_DISABLE */
