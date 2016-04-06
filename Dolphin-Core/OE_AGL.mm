/*
Copyright (c) 2016, OpenEmu Team

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the OpenEmu Team nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "OE_AGL.h"
#include "DolphinGameCore.h"
#include "Core/ConfigManager.h"

void cInterfaceAGL::Swap()
{
    GET_CURRENT_OR_RETURN();
    
    [current swapBuffers];
}

// Create rendering window.
// Call browser: Core.cpp:EmuThread() > main.cpp:Video_Initialize()
bool cInterfaceAGL::Create(void *window_handle, bool core)
{
    // Control window size and picture scaling
    if(SConfig::GetInstance().bWii){
        s_backbuffer_width = 854;
        s_backbuffer_height = 480;
    } else {
        s_backbuffer_width = 640;
        s_backbuffer_height = 480;
    }
    return true;
}


bool cInterfaceAGL::MakeCurrent()
{
//    GET_CURRENT_OR_RETURN();
//    
//    [current makeCurrent];
    return true;
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
}

void cInterfaceAGL::SwapInterval(int interval)
{
}

