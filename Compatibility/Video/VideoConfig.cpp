// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <algorithm>

#include "Common/CPUDetect.h"
#include "Common/CommonTypes.h"
#include "Common/StringUtil.h"
#include "Core/Config/GraphicsSettings.h"
#include "Core/Core.h"
#include "Core/Movie.h"
#include "VideoCommon/OnScreenDisplay.h"
#include "VideoCommon/VideoCommon.h"
#include "VideoCommon/VideoConfig.h"

VideoConfig g_Config;
VideoConfig g_ActiveConfig;
static bool s_has_registered_callback = false;

void UpdateActiveConfig()
{
    if (Movie::IsPlayingInput() && Movie::IsConfigSaved())
        Movie::SetGraphicsConfig();
    g_ActiveConfig = g_Config;
}

VideoConfig::VideoConfig()
{
    // Needed for the first frame, I think
    fAspectRatioHackW = 1;
    fAspectRatioHackH = 1;

    // disable all features by default
    backend_info.api_type = APIType::Nothing;
    backend_info.MaxTextureSize = 16384;
    backend_info.bSupportsExclusiveFullscreen = false;
    backend_info.bSupportsMultithreading = false;
    backend_info.bSupportsInternalResolutionFrameDumps = false;
    backend_info.bSupportsST3CTextures = false;
    backend_info.bSupportsBPTCTextures = false;

    bEnableValidationLayer = false;
    bBackendMultithreading = true;

    //  OE set Renderbuffer to default renderbuffer
       iRenderFBO = 0;
}

void VideoConfig::Refresh()
{
    if (!s_has_registered_callback)
    {
        Config::AddConfigChangedCallback([]() { g_Config.Refresh(); });
        s_has_registered_callback = true;
    }

    bVSync = Config::Get(Config::GFX_VSYNC);
    iAdapter = Config::Get(Config::GFX_ADAPTER);

    bWidescreenHack = Config::Get(Config::GFX_WIDESCREEN_HACK);
    const int aspect_ratio = Config::Get(Config::GFX_ASPECT_RATIO);
    if (aspect_ratio == ASPECT_AUTO)
        iAspectRatio = Config::Get(Config::GFX_SUGGESTED_ASPECT_RATIO);
    else
        iAspectRatio = aspect_ratio;
    bCrop = Config::Get(Config::GFX_CROP);
    bUseXFB = Config::Get(Config::GFX_USE_XFB);
    bUseRealXFB = Config::Get(Config::GFX_USE_REAL_XFB);
    iSafeTextureCache_ColorSamples = Config::Get(Config::GFX_SAFE_TEXTURE_CACHE_COLOR_SAMPLES);
    bShowFPS = Config::Get(Config::GFX_SHOW_FPS);
    bShowNetPlayPing = Config::Get(Config::GFX_SHOW_NETPLAY_PING);
    bShowNetPlayMessages = Config::Get(Config::GFX_SHOW_NETPLAY_MESSAGES);
    bLogRenderTimeToFile = Config::Get(Config::GFX_LOG_RENDER_TIME_TO_FILE);
    bOverlayStats = Config::Get(Config::GFX_OVERLAY_STATS);
    bOverlayProjStats = Config::Get(Config::GFX_OVERLAY_PROJ_STATS);
    bDumpTextures = Config::Get(Config::GFX_DUMP_TEXTURES);
    bHiresTextures = Config::Get(Config::GFX_HIRES_TEXTURES);
    bConvertHiresTextures = Config::Get(Config::GFX_CONVERT_HIRES_TEXTURES);
    bCacheHiresTextures = Config::Get(Config::GFX_CACHE_HIRES_TEXTURES);
    bDumpEFBTarget = Config::Get(Config::GFX_DUMP_EFB_TARGET);
    bDumpFramesAsImages = Config::Get(Config::GFX_DUMP_FRAMES_AS_IMAGES);
    bFreeLook = Config::Get(Config::GFX_FREE_LOOK);
    bUseFFV1 = Config::Get(Config::GFX_USE_FFV1);
    sDumpFormat = Config::Get(Config::GFX_DUMP_FORMAT);
    sDumpCodec = Config::Get(Config::GFX_DUMP_CODEC);
    sDumpPath = Config::Get(Config::GFX_DUMP_PATH);
    iBitrateKbps = Config::Get(Config::GFX_BITRATE_KBPS);
    bInternalResolutionFrameDumps = Config::Get(Config::GFX_INTERNAL_RESOLUTION_FRAME_DUMPS);
    bEnableGPUTextureDecoding = Config::Get(Config::GFX_ENABLE_GPU_TEXTURE_DECODING);
    bEnablePixelLighting = Config::Get(Config::GFX_ENABLE_PIXEL_LIGHTING);
    bFastDepthCalc = Config::Get(Config::GFX_FAST_DEPTH_CALC);
    iMultisamples = Config::Get(Config::GFX_MSAA);
    bSSAA = Config::Get(Config::GFX_SSAA);
    iEFBScale = Config::Get(Config::GFX_EFB_SCALE);
    bTexFmtOverlayEnable = Config::Get(Config::GFX_TEXFMT_OVERLAY_ENABLE);
    bTexFmtOverlayCenter = Config::Get(Config::GFX_TEXFMT_OVERLAY_CENTER);
    bWireFrame = Config::Get(Config::GFX_ENABLE_WIREFRAME);
    bDisableFog = Config::Get(Config::GFX_DISABLE_FOG);
    bBorderlessFullscreen = Config::Get(Config::GFX_BORDERLESS_FULLSCREEN);
    bEnableValidationLayer = Config::Get(Config::GFX_ENABLE_VALIDATION_LAYER);
    bBackendMultithreading = Config::Get(Config::GFX_BACKEND_MULTITHREADING);
    iCommandBufferExecuteInterval = Config::Get(Config::GFX_COMMAND_BUFFER_EXECUTE_INTERVAL);
    bShaderCache = Config::Get(Config::GFX_SHADER_CACHE);
    bBackgroundShaderCompiling = Config::Get(Config::GFX_BACKGROUND_SHADER_COMPILING);
    bDisableSpecializedShaders = Config::Get(Config::GFX_DISABLE_SPECIALIZED_SHADERS);
    bPrecompileUberShaders = Config::Get(Config::GFX_PRECOMPILE_UBER_SHADERS);
    iShaderCompilerThreads = Config::Get(Config::GFX_SHADER_COMPILER_THREADS);
    iShaderPrecompilerThreads = Config::Get(Config::GFX_SHADER_PRECOMPILER_THREADS);

    bZComploc = Config::Get(Config::GFX_SW_ZCOMPLOC);
    bZFreeze = Config::Get(Config::GFX_SW_ZFREEZE);
    bDumpObjects = Config::Get(Config::GFX_SW_DUMP_OBJECTS);
    bDumpTevStages = Config::Get(Config::GFX_SW_DUMP_TEV_STAGES);
    bDumpTevTextureFetches = Config::Get(Config::GFX_SW_DUMP_TEV_TEX_FETCHES);
    drawStart = Config::Get(Config::GFX_SW_DRAW_START);
    drawEnd = Config::Get(Config::GFX_SW_DRAW_END);

    bForceFiltering = Config::Get(Config::GFX_ENHANCE_FORCE_FILTERING);
    iMaxAnisotropy = Config::Get(Config::GFX_ENHANCE_MAX_ANISOTROPY);
    sPostProcessingShader = Config::Get(Config::GFX_ENHANCE_POST_SHADER);
    bForceTrueColor = Config::Get(Config::GFX_ENHANCE_FORCE_TRUE_COLOR);

    iStereoMode = Config::Get(Config::GFX_STEREO_MODE);
    iStereoDepth = Config::Get(Config::GFX_STEREO_DEPTH);
    iStereoConvergencePercentage = Config::Get(Config::GFX_STEREO_CONVERGENCE_PERCENTAGE);
    bStereoSwapEyes = Config::Get(Config::GFX_STEREO_SWAP_EYES);
    iStereoConvergence = Config::Get(Config::GFX_STEREO_CONVERGENCE);
    bStereoEFBMonoDepth = Config::Get(Config::GFX_STEREO_EFB_MONO_DEPTH);
    iStereoDepthPercentage = Config::Get(Config::GFX_STEREO_DEPTH_PERCENTAGE);

    bEFBAccessEnable = Config::Get(Config::GFX_HACK_EFB_ACCESS_ENABLE);
    bBBoxEnable = Config::Get(Config::GFX_HACK_BBOX_ENABLE);
    bBBoxPreferStencilImplementation =
    Config::Get(Config::GFX_HACK_BBOX_PREFER_STENCIL_IMPLEMENTATION);
    bForceProgressive = Config::Get(Config::GFX_HACK_FORCE_PROGRESSIVE);
    bSkipEFBCopyToRam = Config::Get(Config::GFX_HACK_SKIP_EFB_COPY_TO_RAM);
    bCopyEFBScaled = Config::Get(Config::GFX_HACK_COPY_EFB_ENABLED);
    bEFBEmulateFormatChanges = Config::Get(Config::GFX_HACK_EFB_EMULATE_FORMAT_CHANGES);
    bVertexRounding = Config::Get(Config::GFX_HACK_VERTEX_ROUDING);

    phack.m_enable = Config::Get(Config::GFX_PROJECTION_HACK) == 1;
    phack.m_sznear = Config::Get(Config::GFX_PROJECTION_HACK_SZNEAR) == 1;
    phack.m_szfar = Config::Get(Config::GFX_PROJECTION_HACK_SZFAR) == 1;
    phack.m_znear = Config::Get(Config::GFX_PROJECTION_HACK_ZNEAR);
    phack.m_zfar = Config::Get(Config::GFX_PROJECTION_HACK_ZFAR);
    bPerfQueriesEnable = Config::Get(Config::GFX_PERF_QUERIES_ENABLE);

    VerifyValidity();
}

void VideoConfig::VerifyValidity()
{
    // TODO: Check iMaxAnisotropy value
    if (iAdapter < 0 || iAdapter > ((int)backend_info.Adapters.size() - 1))
        iAdapter = 0;

    if (std::find(backend_info.AAModes.begin(), backend_info.AAModes.end(), iMultisamples) ==
        backend_info.AAModes.end())
        iMultisamples = 1;

    if (iStereoMode > 0)
    {
        if (!backend_info.bSupportsGeometryShaders)
        {
            OSD::AddMessage(
                            "Stereoscopic 3D isn't supported by your GPU, support for OpenGL 3.2 is required.",
                            10000);
            iStereoMode = 0;
        }

        if (bUseXFB && bUseRealXFB)
        {
            OSD::AddMessage("Stereoscopic 3D isn't supported with Real XFB, turning off stereoscopy.",
                            10000);
            iStereoMode = 0;
        }
    }
}

bool VideoConfig::IsVSync()
{
    return bVSync && !Core::GetIsThrottlerTempDisabled();
}

static u32 GetNumAutoShaderCompilerThreads()
{
    // Automatic number. We use clamp(cpus - 3, 1, 4).
    return static_cast<u32>(std::min(std::max(cpu_info.num_cores - 3, 1), 4));
}

u32 VideoConfig::GetShaderCompilerThreads() const
{
    if (iShaderCompilerThreads >= 0)
        return static_cast<u32>(iShaderCompilerThreads);
    else
        return GetNumAutoShaderCompilerThreads();
}

u32 VideoConfig::GetShaderPrecompilerThreads() const
{
    if (iShaderPrecompilerThreads >= 0)
        return static_cast<u32>(iShaderPrecompilerThreads);
    else
        return GetNumAutoShaderCompilerThreads();
}

bool VideoConfig::CanPrecompileUberShaders() const
{
    // We don't want to precompile ubershaders if they're never going to be used.
    return bPrecompileUberShaders && (bBackgroundShaderCompiling || bDisableSpecializedShaders);
}

bool VideoConfig::CanBackgroundCompileShaders() const
{
    // We require precompiled ubershaders to background compile shaders.
    return bBackgroundShaderCompiling && bPrecompileUberShaders;
}

//// Copyright 2008 Dolphin Emulator Project
//// Licensed under GPLv2+
//// Refer to the license.txt file included.
//
//#include <algorithm>
//#include <cmath>
//
//#include "Common/CommonTypes.h"
//#include "Common/FileUtil.h"
//#include "Common/IniFile.h"
//#include "Common/StringUtil.h"
//#include "Core/ConfigManager.h"
//#include "Core/Core.h"
//#include "Core/Movie.h"
//#include "VideoCommon/OnScreenDisplay.h"
//#include "VideoCommon/VideoCommon.h"
//#include "OE_VideoConfig.h"
//
//VideoConfig g_Config;
//VideoConfig g_ActiveConfig;
//
//void UpdateActiveConfig()
//{
//    if (Movie::IsPlayingInput() && Movie::IsConfigSaved())
//        Movie::SetGraphicsConfig();
//    g_ActiveConfig = g_Config;
//}
//
//VideoConfig::VideoConfig()
//{
//    bRunning = false;
//
//    // Exclusive fullscreen flags
//    bFullscreen = false;
//    bExclusiveMode = false;
//
//    // Needed for the first frame, I think
//    fAspectRatioHackW = 1;
//    fAspectRatioHackH = 1;
//
//    // disable all features by default
//    backend_info.APIType = API_NONE;
//    backend_info.bSupportsExclusiveFullscreen = false;
//
//    //  OE set Renderbuffer to default renderbuffer
//    iRenderFBO = 0;
//
//}
//
//void VideoConfig::Load(const std::string& ini_file)
//{
//    IniFile iniFile;
//    iniFile.Load(ini_file);
//
//    IniFile::Section* hardware = iniFile.GetOrCreateSection("Hardware");
//    hardware->Get("VSync", &bVSync, 0);
//    hardware->Get("Adapter", &iAdapter, 0);
//
//    IniFile::Section* settings = iniFile.GetOrCreateSection("Settings");
//    settings->Get("wideScreenHack", &bWidescreenHack, false);
//    settings->Get("AspectRatio", &iAspectRatio, (int)ASPECT_AUTO);
//    settings->Get("Crop", &bCrop, false);
//    settings->Get("UseXFB", &bUseXFB, 0);
//    settings->Get("UseRealXFB", &bUseRealXFB, 0);
//    settings->Get("SafeTextureCacheColorSamples", &iSafeTextureCache_ColorSamples, 128);
//    settings->Get("ShowFPS", &bShowFPS, false);
//    settings->Get("LogRenderTimeToFile", &bLogRenderTimeToFile, false);
//    settings->Get("OverlayStats", &bOverlayStats, false);
//    settings->Get("OverlayProjStats", &bOverlayProjStats, false);
//    settings->Get("DumpTextures", &bDumpTextures, 0);
//    settings->Get("HiresTextures", &bHiresTextures, 0);
//    settings->Get("ConvertHiresTextures", &bConvertHiresTextures, 0);
//    settings->Get("CacheHiresTextures", &bCacheHiresTextures, 0);
//    settings->Get("DumpEFBTarget", &bDumpEFBTarget, 0);
//    settings->Get("FreeLook", &bFreeLook, 0);
//    settings->Get("UseFFV1", &bUseFFV1, 0);
//    settings->Get("EnablePixelLighting", &bEnablePixelLighting, 0);
//    settings->Get("FastDepthCalc", &bFastDepthCalc, true);
//    settings->Get("MSAA", &iMultisamples, 1);
//    settings->Get("SSAA", &bSSAA, false);
//    settings->Get("EFBScale", &iEFBScale, (int)SCALE_1X); // native
//    settings->Get("TexFmtOverlayEnable", &bTexFmtOverlayEnable, 0);
//    settings->Get("TexFmtOverlayCenter", &bTexFmtOverlayCenter, 0);
//    settings->Get("WireFrame", &bWireFrame, 0);
//    settings->Get("DisableFog", &bDisableFog, 0);
//    settings->Get("EnableShaderDebugging", &bEnableShaderDebugging, false);
//    settings->Get("BorderlessFullscreen", &bBorderlessFullscreen, false);
//
//    settings->Get("SWZComploc", &bZComploc, true);
//    settings->Get("SWZFreeze", &bZFreeze, true);
//    settings->Get("SWDumpObjects", &bDumpObjects, false);
//    settings->Get("SWDumpTevStages", &bDumpTevStages, false);
//    settings->Get("SWDumpTevTexFetches", &bDumpTevTextureFetches, false);
//    settings->Get("SWDrawStart", &drawStart, 0);
//    settings->Get("SWDrawEnd", &drawEnd, 100000);
//
//
//    IniFile::Section* enhancements = iniFile.GetOrCreateSection("Enhancements");
//    enhancements->Get("ForceFiltering", &bForceFiltering, 0);
//    enhancements->Get("MaxAnisotropy", &iMaxAnisotropy, 0);  // NOTE - this is x in (1 << x)
//    enhancements->Get("PostProcessingShader", &sPostProcessingShader, "");
//
//    IniFile::Section* stereoscopy = iniFile.GetOrCreateSection("Stereoscopy");
//    stereoscopy->Get("StereoMode", &iStereoMode, 0);
//    stereoscopy->Get("StereoDepth", &iStereoDepth, 20);
//    stereoscopy->Get("StereoConvergencePercentage", &iStereoConvergencePercentage, 100);
//    stereoscopy->Get("StereoSwapEyes", &bStereoSwapEyes, false);
//
//    IniFile::Section* hacks = iniFile.GetOrCreateSection("Hacks");
//    hacks->Get("EFBAccessEnable", &bEFBAccessEnable, true);
//    hacks->Get("BBoxEnable", &bBBoxEnable, false);
//    hacks->Get("ForceProgressive", &bForceProgressive, true);
//    hacks->Get("EFBToTextureEnable", &bSkipEFBCopyToRam, true);
//    hacks->Get("EFBScaledCopy", &bCopyEFBScaled, true);
//    hacks->Get("EFBEmulateFormatChanges", &bEFBEmulateFormatChanges, false);
//
//    // hacks which are disabled by default
//    iPhackvalue[0] = 0;
//    bPerfQueriesEnable = false;
//
//    // Load common settings
//    iniFile.Load(File::GetUserPath(F_DOLPHINCONFIG_IDX));
//    IniFile::Section* interface = iniFile.GetOrCreateSection("Interface");
//    bool bTmp;
//    interface->Get("UsePanicHandlers", &bTmp, true);
//    SetEnableAlert(bTmp);
//
//    // Shader Debugging causes a huge slowdown and it's easy to forget about it
//    // since it's not exposed in the settings dialog. It's only used by
//    // developers, so displaying an obnoxious message avoids some confusion and
//    // is not too annoying/confusing for users.
//    //
//    // XXX(delroth): This is kind of a bad place to put this, but the current
//    // VideoCommon is a mess and we don't have a central initialization
//    // function to do these kind of checks. Instead, the init code is
//    // triplicated for each video backend.
//    if (bEnableShaderDebugging)
//        OSD::AddMessage("Warning: Shader Debugging is enabled, performance will suffer heavily", 15000);
//
//    VerifyValidity();
//}
//
//void VideoConfig::GameIniLoad()
//{
//    bool gfx_override_exists = false;
//
//    // XXX: Again, bad place to put OSD messages at (see delroth's comment above)
//    // XXX: This will add an OSD message for each projection hack value... meh
//#define CHECK_SETTING(section, key, var) do { \
//        decltype(var) temp = var; \
//        if (iniFile.GetIfExists(section, key, &var) && var != temp) { \
//            std::string msg = StringFromFormat("Note: Option \"%s\" is overridden by game ini.", key); \
//            OSD::AddMessage(msg, 7500); \
//            gfx_override_exists = true; \
//        } \
//    } while (0)
//
//    IniFile iniFile = SConfig::GetInstance().LoadGameIni();
//
//    CHECK_SETTING("Video_Hardware", "VSync", bVSync);
//
//    CHECK_SETTING("Video_Settings", "wideScreenHack", bWidescreenHack);
//    CHECK_SETTING("Video_Settings", "AspectRatio", iAspectRatio);
//    CHECK_SETTING("Video_Settings", "Crop", bCrop);
//    CHECK_SETTING("Video_Settings", "UseXFB", bUseXFB);
//    CHECK_SETTING("Video_Settings", "UseRealXFB", bUseRealXFB);
//    CHECK_SETTING("Video_Settings", "SafeTextureCacheColorSamples", iSafeTextureCache_ColorSamples);
//    CHECK_SETTING("Video_Settings", "HiresTextures", bHiresTextures);
//    CHECK_SETTING("Video_Settings", "ConvertHiresTextures", bConvertHiresTextures);
//    CHECK_SETTING("Video_Settings", "CacheHiresTextures", bCacheHiresTextures);
//    CHECK_SETTING("Video_Settings", "EnablePixelLighting", bEnablePixelLighting);
//    CHECK_SETTING("Video_Settings", "FastDepthCalc", bFastDepthCalc);
//    CHECK_SETTING("Video_Settings", "MSAA", iMultisamples);
//    CHECK_SETTING("Video_Settings", "SSAA", bSSAA);
//
//    int tmp = -9000;
//    CHECK_SETTING("Video_Settings", "EFBScale", tmp); // integral
//    if (tmp != -9000)
//    {
//        if (tmp != SCALE_FORCE_INTEGRAL)
//        {
//            iEFBScale = tmp;
//        }
//        else // Round down to multiple of native IR
//        {
//            switch (iEFBScale)
//            {
//            case SCALE_AUTO:
//                iEFBScale = SCALE_AUTO_INTEGRAL;
//                break;
//            case SCALE_1_5X:
//                iEFBScale = SCALE_1X;
//                break;
//            case SCALE_2_5X:
//                iEFBScale = SCALE_2X;
//                break;
//            default:
//                break;
//            }
//        }
//    }
//
//    CHECK_SETTING("Video_Settings", "DisableFog", bDisableFog);
//
//    CHECK_SETTING("Video_Enhancements", "ForceFiltering", bForceFiltering);
//    CHECK_SETTING("Video_Enhancements", "MaxAnisotropy", iMaxAnisotropy);  // NOTE - this is x in (1 << x)
//    CHECK_SETTING("Video_Enhancements", "PostProcessingShader", sPostProcessingShader);
//
//    // These are not overrides, they are per-game stereoscopy parameters, hence no warning
//    iniFile.GetIfExists("Video_Stereoscopy", "StereoConvergence", &iStereoConvergence, 20);
//    iniFile.GetIfExists("Video_Stereoscopy", "StereoEFBMonoDepth", &bStereoEFBMonoDepth, false);
//    iniFile.GetIfExists("Video_Stereoscopy", "StereoDepthPercentage", &iStereoDepthPercentage, 100);
//
//    CHECK_SETTING("Video_Stereoscopy", "StereoMode", iStereoMode);
//    CHECK_SETTING("Video_Stereoscopy", "StereoDepth", iStereoDepth);
//    CHECK_SETTING("Video_Stereoscopy", "StereoSwapEyes", bStereoSwapEyes);
//
//    CHECK_SETTING("Video_Hacks", "EFBAccessEnable", bEFBAccessEnable);
//    CHECK_SETTING("Video_Hacks", "BBoxEnable", bBBoxEnable);
//    CHECK_SETTING("Video_Hacks", "ForceProgressive", bForceProgressive);
//    CHECK_SETTING("Video_Hacks", "EFBToTextureEnable", bSkipEFBCopyToRam);
//    CHECK_SETTING("Video_Hacks", "EFBScaledCopy", bCopyEFBScaled);
//    CHECK_SETTING("Video_Hacks", "EFBEmulateFormatChanges", bEFBEmulateFormatChanges);
//
//    CHECK_SETTING("Video", "ProjectionHack", iPhackvalue[0]);
//    CHECK_SETTING("Video", "PH_SZNear", iPhackvalue[1]);
//    CHECK_SETTING("Video", "PH_SZFar", iPhackvalue[2]);
//    CHECK_SETTING("Video", "PH_ZNear", sPhackvalue[0]);
//    CHECK_SETTING("Video", "PH_ZFar", sPhackvalue[1]);
//    CHECK_SETTING("Video", "PerfQueriesEnable", bPerfQueriesEnable);
//
//    if (gfx_override_exists)
//        OSD::AddMessage("Warning: Opening the graphics configuration will reset settings and might cause issues!", 10000);
//}
//
//void VideoConfig::VerifyValidity()
//{
//    // TODO: Check iMaxAnisotropy value
//    if (iAdapter < 0 || iAdapter > ((int)backend_info.Adapters.size() - 1))
//        iAdapter = 0;
//
//    if (std::find(backend_info.AAModes.begin(), backend_info.AAModes.end(), iMultisamples) == backend_info.AAModes.end())
//        iMultisamples = 1;
//
//    if (iStereoMode > 0)
//    {
//        if (!backend_info.bSupportsGeometryShaders)
//        {
//            OSD::AddMessage("Stereoscopic 3D isn't supported by your GPU, support for OpenGL 3.2 is required.", 10000);
//            iStereoMode = 0;
//        }
//
//        if (bUseXFB && bUseRealXFB)
//        {
//            OSD::AddMessage("Stereoscopic 3D isn't supported with Real XFB, turning off stereoscopy.", 10000);
//            iStereoMode = 0;
//        }
//    }
//}
//
//void VideoConfig::Save(const std::string& ini_file)
//{
//    IniFile iniFile;
//    iniFile.Load(ini_file);
//
//    IniFile::Section* hardware = iniFile.GetOrCreateSection("Hardware");
//    hardware->Set("VSync", bVSync);
//    hardware->Set("Adapter", iAdapter);
//
//    IniFile::Section* settings = iniFile.GetOrCreateSection("Settings");
//    settings->Set("AspectRatio", iAspectRatio);
//    settings->Set("Crop", bCrop);
//    settings->Set("wideScreenHack", bWidescreenHack);
//    settings->Set("UseXFB", bUseXFB);
//    settings->Set("UseRealXFB", bUseRealXFB);
//    settings->Set("SafeTextureCacheColorSamples", iSafeTextureCache_ColorSamples);
//    settings->Set("ShowFPS", bShowFPS);
//    settings->Set("LogRenderTimeToFile", bLogRenderTimeToFile);
//    settings->Set("OverlayStats", bOverlayStats);
//    settings->Set("OverlayProjStats", bOverlayProjStats);
//    settings->Set("DumpTextures", bDumpTextures);
//    settings->Set("HiresTextures", bHiresTextures);
//    settings->Set("ConvertHiresTextures", bConvertHiresTextures);
//    settings->Set("CacheHiresTextures", bCacheHiresTextures);
//    settings->Set("DumpEFBTarget", bDumpEFBTarget);
//    settings->Set("FreeLook", bFreeLook);
//    settings->Set("UseFFV1", bUseFFV1);
//    settings->Set("EnablePixelLighting", bEnablePixelLighting);
//    settings->Set("FastDepthCalc", bFastDepthCalc);
//    settings->Set("MSAA", iMultisamples);
//    settings->Set("SSAA", bSSAA);
//    settings->Set("EFBScale", iEFBScale);
//    settings->Set("TexFmtOverlayEnable", bTexFmtOverlayEnable);
//    settings->Set("TexFmtOverlayCenter", bTexFmtOverlayCenter);
//    settings->Set("Wireframe", bWireFrame);
//    settings->Set("DisableFog", bDisableFog);
//    settings->Set("EnableShaderDebugging", bEnableShaderDebugging);
//    settings->Set("BorderlessFullscreen", bBorderlessFullscreen);
//
//    settings->Set("SWZComploc", bZComploc);
//    settings->Set("SWZFreeze", bZFreeze);
//    settings->Set("SWDumpObjects", bDumpObjects);
//    settings->Set("SWDumpTevStages", bDumpTevStages);
//    settings->Set("SWDumpTevTexFetches", bDumpTevTextureFetches);
//    settings->Set("SWDrawStart", drawStart);
//    settings->Set("SWDrawEnd", drawEnd);
//
//    IniFile::Section* enhancements = iniFile.GetOrCreateSection("Enhancements");
//    enhancements->Set("ForceFiltering", bForceFiltering);
//    enhancements->Set("MaxAnisotropy", iMaxAnisotropy);
//    enhancements->Set("PostProcessingShader", sPostProcessingShader);
//
//    IniFile::Section* stereoscopy = iniFile.GetOrCreateSection("Stereoscopy");
//    stereoscopy->Set("StereoMode", iStereoMode);
//    stereoscopy->Set("StereoDepth", iStereoDepth);
//    stereoscopy->Set("StereoConvergencePercentage", iStereoConvergencePercentage);
//    stereoscopy->Set("StereoSwapEyes", bStereoSwapEyes);
//
//    IniFile::Section* hacks = iniFile.GetOrCreateSection("Hacks");
//    hacks->Set("EFBAccessEnable", bEFBAccessEnable);
//    hacks->Set("BBoxEnable", bBBoxEnable);
//    hacks->Set("ForceProgressive", bForceProgressive);
//    hacks->Set("EFBToTextureEnable", bSkipEFBCopyToRam);
//    hacks->Set("EFBScaledCopy", bCopyEFBScaled);
//    hacks->Set("EFBEmulateFormatChanges", bEFBEmulateFormatChanges);
//
//    iniFile.Save(ini_file);
//}
//
//bool VideoConfig::IsVSync()
//{
//    return bVSync && !Core::GetIsThrottlerTempDisabled();
//}

