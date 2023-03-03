//// Copyright 2008 Dolphin Emulator Project
//// Licensed under GPLv2+
// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

// IMPORTANT: UI etc should modify g_Config. Graphics code should read g_ActiveConfig.
// The reason for this is to get rid of race conditions etc when the configuration
// changes in the middle of a frame. This is done by copying g_Config to g_ActiveConfig
// at the start of every frame. Noone should ever change members of g_ActiveConfig
// directly.

#pragma once

#include <string>
#include <vector>

#include "Common/CommonTypes.h"
#include "VideoCommon/GraphicsModSystem/Config/GraphicsModGroup.h"
#include "VideoCommon/VideoCommon.h"

enum class APIType;

// Log in two categories, and save three other options in the same byte
#define CONF_LOG 1
#define CONF_PRIMLOG 2
#define CONF_SAVETARGETS 8
#define CONF_SAVESHADERS 16

constexpr int EFB_SCALE_AUTO_INTEGRAL = 0;

enum class AspectMode : int
{
  Auto,
  AnalogWide,
  Analog,
  Stretch,
};

enum class StereoMode : int
{
  Off,
  SBS,
  TAB,
  Anaglyph,
  QuadBuffer,
  Passive
};

enum class ShaderCompilationMode : int
{
  Synchronous,
  SynchronousUberShaders,
  AsynchronousUberShaders,
  AsynchronousSkipRendering
};

// NEVER inherit from this class.
struct VideoConfig final
{
  VideoConfig() = default;
  void Refresh();
  void VerifyValidity();

  // General
  bool bVSync;
  bool bVSyncActive;
  bool bWidescreenHack;
  AspectMode aspect_mode;
  AspectMode suggested_aspect_mode;
  bool bCrop;  // Aspect ratio controls.
  bool bShaderCache;

  // Enhancements
  u32 iMultisamples;
  bool bSSAA;
  int iEFBScale;
  bool bForceFiltering;
  int iMaxAnisotropy;
  std::string sPostProcessingShader;
  bool bForceTrueColor;
  bool bDisableCopyFilter;
  bool bArbitraryMipmapDetection;
  float fArbitraryMipmapDetectionThreshold;

  // Information
  bool bShowFPS;
  bool bShowNetPlayPing;
  bool bShowNetPlayMessages;
  bool bOverlayStats;
  bool bOverlayProjStats;
  bool bOverlayScissorStats;
  bool bTexFmtOverlayEnable;
  bool bTexFmtOverlayCenter;
  bool bLogRenderTimeToFile;

  // Render
  bool bWireFrame;
  bool bDisableFog;

  //  OpenEmu render buffer
  int iRenderFBO = 0;

  // Utility
  bool bDumpTextures;
  bool bDumpMipmapTextures;
  bool bDumpBaseTextures;
  bool bHiresTextures;
  bool bCacheHiresTextures;
  bool bDumpEFBTarget;
  bool bDumpXFBTarget;
  bool bDumpFramesAsImages;
  bool bUseFFV1;
  std::string sDumpCodec;
  std::string sDumpPixelFormat;
  std::string sDumpEncoder;
  std::string sDumpFormat;
  std::string sDumpPath;
  bool bInternalResolutionFrameDumps;
  bool bBorderlessFullscreen;
  bool bEnableGPUTextureDecoding;
  int iBitrateKbps;
  bool bGraphicMods = false;
  std::optional<GraphicsModGroupConfig> graphics_mod_config;

  // Hacks
  bool bEFBAccessEnable;
  bool bEFBAccessDeferInvalidation;
  bool bPerfQueriesEnable;
  bool bBBoxEnable;
  bool bForceProgressive;

  bool bEFBEmulateFormatChanges;
  bool bSkipEFBCopyToRam;
  bool bSkipXFBCopyToRam;
  bool bDisableCopyToVRAM;
  bool bDeferEFBCopies;
  bool bImmediateXFB;
  bool bSkipPresentingDuplicateXFBs;
  bool bCopyEFBScaled;
  int iSafeTextureCache_ColorSamples;
  float fAspectRatioHackW, fAspectRatioHackH;
  bool bEnablePixelLighting;
  bool bFastDepthCalc;
  bool bVertexRounding;
  int iEFBAccessTileSize;
  u32 iMissingColorValue;
  bool bFastTextureSampling;
  int iLog;           // CONF_ bits
  int iSaveTargetId;  // TODO: Should be dropped

  // Stereoscopy
  StereoMode stereo_mode;
  int iStereoDepth;
  int iStereoConvergence;
  int iStereoConvergencePercentage;
  bool bStereoSwapEyes;
  bool bStereoEFBMonoDepth;
  int iStereoDepthPercentage;

  // D3D only config, mostly to be merged into the above
  int iAdapter;

  // VideoSW Debugging
  int drawStart;
  int drawEnd;
  bool bZComploc;
  bool bZFreeze;
  bool bDumpObjects;
  bool bDumpTevStages;
  bool bDumpTevTextureFetches;

  // Enable API validation layers, currently only supported with Vulkan.
  bool bEnableValidationLayer;

  // Multithreaded submission, currently only supported with Vulkan.
  bool bBackendMultithreading;

  // Early command buffer execution interval in number of draws.
  // Currently only supported with Vulkan.
  int iCommandBufferExecuteInterval;

  // Shader compilation settings.
  bool bWaitForShadersBeforeStarting;
  ShaderCompilationMode iShaderCompilationMode;

  // Number of shader compiler threads.
  // 0 disables background compilation.
  // -1 uses an automatic number based on the CPU threads.
  int iShaderCompilerThreads;
  int iShaderPrecompilerThreads;

  // Static config per API
  // TODO: Move this out of VideoConfig
  struct
  {
    APIType api_type = APIType::Nothing;

    std::vector<std::string> Adapters;  // for D3D
    std::vector<u32> AAModes;

    // TODO: merge AdapterName and Adapters array
    std::string AdapterName;  // for OpenGL

    u32 MaxTextureSize = 16384;
    bool bUsesLowerLeftOrigin = false;

    bool bSupportsExclusiveFullscreen = false;
    bool bSupportsDualSourceBlend = false;
    bool bSupportsPrimitiveRestart = false;
    bool bSupportsOversizedViewports = false;
    bool bSupportsGeometryShaders = false;
    bool bSupportsComputeShaders = false;
    bool bSupports3DVision = false;
    bool bSupportsEarlyZ = false;         // needed by PixelShaderGen, so must stay in VideoCommon
    bool bSupportsBindingLayout = false;  // Needed by ShaderGen, so must stay in VideoCommon
    bool bSupportsBBox = false;
    bool bSupportsGSInstancing = false;  // Needed by GeometryShaderGen, so must stay in VideoCommon
    bool bSupportsPostProcessing = false;
    bool bSupportsPaletteConversion = false;
    bool bSupportsClipControl = false;  // Needed by VertexShaderGen, so must stay in VideoCommon
    bool bSupportsSSAA = false;
    bool bSupportsFragmentStoresAndAtomics = false;  // a.k.a. OpenGL SSBOs a.k.a. Direct3D UAVs
    bool bSupportsDepthClamp = false;  // Needed by VertexShaderGen, so must stay in VideoCommon
    bool bSupportsReversedDepthRange = false;
    bool bSupportsLogicOp = false;
    bool bSupportsMultithreading = false;
    bool bSupportsGPUTextureDecoding = false;
    bool bSupportsST3CTextures = false;
    bool bSupportsCopyToVram = false;
    bool bSupportsBitfield = false;  // Needed by UberShaders, so must stay in VideoCommon
    // Needed by UberShaders, so must stay in VideoCommon
    bool bSupportsDynamicSamplerIndexing = false;
    bool bSupportsBPTCTextures = false;
    bool bSupportsFramebufferFetch = false;  // Used as an alternative to dual-source blend on GLES
    bool bSupportsBackgroundCompiling = false;
    bool bSupportsLargePoints = false;
    bool bSupportsPartialDepthCopies = false;
    bool bSupportsDepthReadback = false;
    bool bSupportsShaderBinaries = false;
    bool bSupportsPipelineCacheData = false;
    bool bSupportsCoarseDerivatives = false;
    bool bSupportsTextureQueryLevels = false;
    bool bSupportsLodBiasInSampler = false;
    bool bSupportsSettingObjectNames = false;
    bool bSupportsPartialMultisampleResolve = false;
  } backend_info;

  // Utility
  bool MultisamplingEnabled() const { return iMultisamples > 1; }
  bool ExclusiveFullscreenEnabled() const
  {
    return backend_info.bSupportsExclusiveFullscreen && !bBorderlessFullscreen;
  }
  bool UseGPUTextureDecoding() const
  {
    return backend_info.bSupportsGPUTextureDecoding && bEnableGPUTextureDecoding;
  }
  bool UseVertexRounding() const { return bVertexRounding && iEFBScale != 1; }
  bool ManualTextureSamplingWithHiResTextures() const
  {
    // Hi-res textures (including hi-res EFB copies, but not native-resolution EFB copies at higher
    // internal resolutions) breaks the wrapping logic used by manual texture sampling.
    if (bFastTextureSampling)
      return false;
    if (iEFBScale != 1 && bCopyEFBScaled)
      return true;
    return bHiresTextures;
  }
  bool UsingUberShaders() const;
  u32 GetShaderCompilerThreads() const;
  u32 GetShaderPrecompilerThreads() const;
};

extern VideoConfig g_Config;
extern VideoConfig g_ActiveConfig;

// Called every frame.
void UpdateActiveConfig();
