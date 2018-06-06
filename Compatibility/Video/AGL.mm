// Copyright 2012 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.
#include "DolphinGameCore.h"
#include "Core/ConfigManager.h"

#include "Common/GL/GLInterface/AGL.h"
#include "Common/Logging/Log.h"
#include <OpenGL/gl3.h>

void cInterfaceAGL::Swap()
{
    [_current.renderDelegate didRenderFrameOnAlternateThread];
}

bool cInterfaceAGL::MakeCurrent()
{
    [_current.renderDelegate willRenderFrameOnAlternateThread];

    // Set the background color of the context to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    Swap();

    return true;
}

static bool UpdateCachedDimensions(NSView* view, u32* width, u32* height)
{
    return true;
}

static bool AttachContextToView(NSOpenGLContext* context, NSView* view, u32* width, u32* height)
{
    return true;
}


// Create rendering window.
// Call browser: Core.cpp:EmuThread() > main.cpp:Video_Initialize()
bool cInterfaceAGL::Create(void* window_handle, bool stereo, bool core)
{
    MakeCurrent();

    // Control window size and picture scaling
    if(SConfig::GetInstance().bWii) {
        s_backbuffer_width = 854;
        s_backbuffer_height = 480;
    } else {
        s_backbuffer_width = 640;
        s_backbuffer_height = 480;
    }
    return true;
}

bool cInterfaceAGL::Create(cInterfaceBase* main_context)
{
    return true;
}

std::unique_ptr<cInterfaceBase> cInterfaceAGL::CreateSharedContext()
{
    return nullptr;
}

bool cInterfaceAGL::ClearCurrent()
{
    return true;
}

// Close backend
void cInterfaceAGL::Shutdown()
{
}

void cInterfaceAGL::Update()
{
    return;
}

void cInterfaceAGL::SwapInterval(int interval)
{
}

