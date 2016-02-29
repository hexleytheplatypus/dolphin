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

// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cstring>

#import "OE_CoreAudio.h"
#include "DolphinGameCore.h"


#include "AudioCommon/SoundStream.h"
#include "Common/Event.h"

#include "Common/Thread.h"
#include "AudioCommon/Mixer.h"
#include "Core/HW/AudioInterface.h"
#include "Core/HW/SystemTimers.h"



bool CoreAudioSound::Start()
{
    m_run_thread.store(true);
    
    realtimeBuffer = (uint16_t *)malloc(SIZESOUNDBUFFER * sizeof(uint16_t));
    memset(realtimeBuffer, 0, SIZESOUNDBUFFER * sizeof(uint16_t));
    
    thread = std::thread(&CoreAudioSound::AudioThread, this);

    return true;
}

void CoreAudioSound::SetVolume(int volume)
{
}

void CoreAudioSound::SoundLoop()
{
}

void CoreAudioSound::AudioThread()
{
    Common::SetCurrentThreadName("Audio thread");
    
    uint32 numBytesToRender = SIZESOUNDBUFFER/60/2/2; //Sound buffer size/fps/sound channels/Size of short
    
//    while (m_run_thread.load())
//    {
        numBytesToRender = m_mixer->Mix((short*)realtimeBuffer, numBytesToRender);
        
        {
            //soundCriticalSection;
             GET_CURRENT_OR_RETURN();
            
            [[current ringBufferAtIndex:0] write:(uint8_t*)realtimeBuffer maxLength:(numBytesToRender*2) ];
            
//            soundSyncEvent.Set();
//        }
//        
    }

}
void CoreAudioSound::Stop()
{}

void CoreAudioSound::Update()
{
    AudioThread();//soundSyncEvent.Set();
}
