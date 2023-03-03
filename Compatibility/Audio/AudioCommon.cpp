// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "AudioCommon/AudioCommon.h"
#include "AudioCommon/Mixer.h"
#include "AudioCommon/NullSoundStream.h"
#include "OpenEmuAudioStream.h"
#include "Common/Common.h"
#include "Common/FileUtil.h"
#include "Common/Logging/Log.h"
#include "Core/Config/MainSettings.h"
#include "Core/ConfigManager.h"

// This shouldn't be a global, at least not here.
extern std::unique_ptr<SoundStream> g_sound_stream;
std::unique_ptr<SoundStream> g_sound_stream;

static bool s_audio_dump_start = false;
static bool s_sound_stream_running = false;

namespace AudioCommon
{
    static const int AUDIO_VOLUME_MIN = 0;
    static const int AUDIO_VOLUME_MAX = 100;
    
    void InitSoundStream(Core::System& system)
    {
        g_sound_stream = std::make_unique<OpenEmuAudioStream>();
        
        if (!g_sound_stream->Init())
        {
            WARN_LOG_FMT(AUDIO, "Could not initialize backend");
            g_sound_stream = std::make_unique<NullSound>();
        }
        
        UpdateSoundStream(system);
        SetSoundStreamRunning(system, true);
    }
    
    void PostInitSoundStream(Core::System& system)
    {
        // This needs to be called after AudioInterface::Init and SerialInterface::Init (for GBA devices)
        // where input sample rates are set
        UpdateSoundStream(system);
        SetSoundStreamRunning(system, true);
        
        if (Config::Get(Config::MAIN_DUMP_AUDIO) && !s_audio_dump_start)
            StartAudioDump(system);
    }

    void ShutdownSoundStream(Core::System& system)
    {
        INFO_LOG_FMT(AUDIO, "Shutting down sound stream");
        
        if (Config::Get(Config::MAIN_DUMP_AUDIO) && s_audio_dump_start)
            StopAudioDump(system);
        
        SetSoundStreamRunning(system, false);
        g_sound_stream.reset();
        
        INFO_LOG_FMT(AUDIO, "Done shutting down sound stream");
    }
    
    std::string GetDefaultSoundBackend()
    {
        std::string backend = "oeaudio";
        return backend;
    }
    
    std::vector<std::string> GetSoundBackends()
    {
        std::vector<std::string> backends;
        backends.push_back("oeaudio");
        return backends;
    }
    
DPL2Quality GetDefaultDPL2Quality()
{
  return DPL2Quality::High;
}

    bool SupportsDPL2Decoder(const std::string& backend)
    {
        return false;
    }
    
    bool SupportsLatencyControl(const std::string& backend)
    {
        return false;
    }
    
    bool SupportsVolumeChanges(const std::string& backend)
    {
        // FIXME: this one should ask the backend whether it supports it.
        //       but getting the backend from string etc. is probably
        //       too much just to enable/disable a stupid slider...
        return false;
    }
    
    void UpdateSoundStream(Core::System& system)
    {
        if (g_sound_stream)
        {
            int volume = Config::Get(Config::MAIN_AUDIO_MUTED) ? 0 : Config::Get(Config::MAIN_AUDIO_VOLUME);
            g_sound_stream->SetVolume(volume);
        }
    }
    
    void SetSoundStreamRunning(Core::System& system, bool running)
    {
        if (!g_sound_stream)
            return;
        
        if (s_sound_stream_running == running)
            return;
        s_sound_stream_running = running;
        
        if (g_sound_stream->SetRunning(running))
            return;
        if (running)
            ERROR_LOG_FMT(AUDIO, "Error starting stream.");
        else
            ERROR_LOG_FMT(AUDIO, "Error stopping stream.");
    }
    
    void SendAIBuffer(Core::System& system, const short* samples, unsigned int num_samples)
    {
        if (!g_sound_stream)
            return;
        
        if (Config::Get(Config::MAIN_DUMP_AUDIO) && !s_audio_dump_start)
            StartAudioDump(system);
        else if (!Config::Get(Config::MAIN_DUMP_AUDIO) && s_audio_dump_start)
            StopAudioDump(system);
        
        Mixer* pMixer = g_sound_stream->GetMixer();
        
        if (pMixer && samples)
        {
            pMixer->PushSamples(samples, num_samples);
        }
    }
    
    void StartAudioDump(Core::System& system)
    {
        std::string audio_file_name_dtk = File::GetUserPath(D_DUMPAUDIO_IDX) + "dtkdump.wav";
        std::string audio_file_name_dsp = File::GetUserPath(D_DUMPAUDIO_IDX) + "dspdump.wav";
        File::CreateFullPath(audio_file_name_dtk);
        File::CreateFullPath(audio_file_name_dsp);
        g_sound_stream->GetMixer()->StartLogDTKAudio(audio_file_name_dtk);
        g_sound_stream->GetMixer()->StartLogDSPAudio(audio_file_name_dsp);
        s_audio_dump_start = true;
    }
    
    void StopAudioDump(Core::System& system)
    {
        if (!g_sound_stream)
            return;
        g_sound_stream->GetMixer()->StopLogDTKAudio();
        g_sound_stream->GetMixer()->StopLogDSPAudio();
        s_audio_dump_start = false;
    }
    
    void IncreaseVolume(Core::System& system, unsigned short offset)
    {
        Config::SetBaseOrCurrent(Config::MAIN_AUDIO_MUTED, false);
        int currentVolume = Config::Get(Config::MAIN_AUDIO_VOLUME);
        currentVolume += offset;
        if (currentVolume > AUDIO_VOLUME_MAX)
            currentVolume = AUDIO_VOLUME_MAX;
        Config::SetBaseOrCurrent(Config::MAIN_AUDIO_VOLUME, currentVolume);
        UpdateSoundStream(system);
    }
    
    void DecreaseVolume(Core::System& system, unsigned short offset)
    {
        Config::SetBaseOrCurrent(Config::MAIN_AUDIO_MUTED, false);
        int currentVolume = Config::Get(Config::MAIN_AUDIO_VOLUME);
        currentVolume -= offset;
        if (currentVolume < AUDIO_VOLUME_MIN)
            currentVolume = AUDIO_VOLUME_MIN;
        Config::SetBaseOrCurrent(Config::MAIN_AUDIO_VOLUME, currentVolume);
        UpdateSoundStream(system);
    }
    
    void ToggleMuteVolume(Core::System& system)
    {
          bool isMuted = Config::Get(Config::MAIN_AUDIO_MUTED);
          Config::SetBaseOrCurrent(Config::MAIN_AUDIO_MUTED, !isMuted);
          UpdateSoundStream(system);
    }
}  // namespace AudioCommon
