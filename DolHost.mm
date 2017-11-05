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

#include "DolHost.h"
#include "input.h"

#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#import  <Cocoa/Cocoa.h>

#include "AudioCommon/AudioCommon.h"

#include "Common/CommonPaths.h"
#include "Common/CommonTypes.h"
#include "Common/Event.h"
#include "Common/Flag.h"
#include "Common/Logging/LogManager.h"
#include "Common/MsgHandler.h"

#include "Core/Analytics.h"
#include "Core/Boot/Boot.h"
#include "Core/BootManager.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/Host.h"
#include "Core/HW/CPU.h"
#include "Core/HW/Wiimote.h"
#include "Core/HW/WiimoteCommon/WiimoteHid.h"
#include "Core/HW/WiimoteReal/WiimoteReal.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "Core/HW/ProcessorInterface.h"
#include "Core/IOS/IOS.h"
#include "Core/IOS/STM/STM.h"
#include "Core/PowerPC/PowerPC.h"
#include "Core/State.h"
#include "Core/WiiUtils.h"

#include "UICommon/CommandLineParse.h"
#include "UICommon/UICommon.h"

#include "InputCommon/InputConfig.h"
#include "InputCommon/ControllerEmu/ControlGroup/Extension.h"
#include "InputCommon/ControllerEmu/ControlGroup/Cursor.h"
#include "InputCommon/ControllerEmu/Control/Control.h"
#include "InputCommon/ControlReference/ControlReference.h"

#include "VideoCommon/RenderBase.h"
#include "VideoCommon/VideoBackendBase.h"
#include "VideoCommon/VideoConfig.h"
#include "VideoCommon/OnScreenDisplay.h"

DolHost* DolHost::m_instance = nullptr;
static Common::Event updateMainFrameEvent;
static Common::Flag s_running{true};

static Common::Flag s_shutdown_requested{false};
static Common::Flag s_tried_graceful_shutdown{false};

DolHost* DolHost::GetInstance()
{
    if (DolHost::m_instance == nullptr)
        DolHost::m_instance = new DolHost();
    return DolHost::m_instance;
}

DolHost::DolHost()
{
}

void DolHost::Init(std::string supportDirectoryPath, std::string cpath)
{
    //Set the game file for the DolHost
    _gamePath = cpath;

    //Configure UI for OpenEmu directory structure
    UICommon::SetUserDirectory(supportDirectoryPath);
    UICommon::CreateDirectories();
    UICommon::Init();

    // Database Settings
    SConfig::GetInstance().m_use_builtin_title_database = true;

    //Setup the CPU Settings
    SConfig::GetInstance().bMMU = true;
    //SConfig::GetInstance().bSkipIdle = true;
#ifdef DEBUG
    SConfig::GetInstance().bEnableCheats = true;
#else
    SConfig::GetInstance().bEnableCheats = false;
#endif
    SConfig::GetInstance().bBootToPause = false;

    //Debug Settings
    SConfig::GetInstance().bEnableDebugging = false;
    SConfig::GetInstance().bOnScreenDisplayMessages = true;
    SConfig::GetInstance().m_ShowFrameCount = false;

    //Video
    SConfig::GetInstance().m_strVideoBackend = "OGL";
    VideoBackendBase::ActivateBackend(SConfig::GetInstance().m_strVideoBackend);

    //Set the Sound
    SConfig::GetInstance().bDSPHLE = true;
    SConfig::GetInstance().bDSPThread = true;
    SConfig::GetInstance().m_Volume = 0;
    SConfig::GetInstance().sBackend = "Cubeb" ;//"OpenAL";

    //Split CPU thread from GPU
    SConfig::GetInstance().bCPUThread = true;

    //Analitics
    SConfig::GetInstance().m_analytics_permission_asked = true;
    SConfig::GetInstance().m_analytics_enabled =  false;
    DolphinAnalytics::Instance()->ReloadConfig();

    //Save them now
    SConfig::GetInstance().SaveSettings();

    //Choose Wiimote Type
    _wiiMoteType = WIIMOTE_SRC_EMU; // WIIMOTE_SRC_EMU, WIIMOTE_SRC_HYBRID or WIIMOTE_SRC_REAL

    //Get game info from file path
    GetGameInfo();

    if (!DiscIO::IsWii(_gameType))
    {
        SConfig::GetInstance().bWii = false;

        //Set the wii format to false
        _wiiWAD = false;


        //Create Memorycards by GameID
        std::string _memCardPath = File::GetUserPath(D_GCUSER_IDX) + DIR_SEP + _gameCountryDir + DIR_SEP + _gameID;
        std::string _memCardA = _memCardPath + "_A." + _gameCountryDir + ".raw";
        std::string _memCardB = _memCardPath +  "_B." + _gameCountryDir + ".raw";

        SConfig::GetInstance().m_strMemoryCardA = _memCardA;
        SConfig::GetInstance().m_strMemoryCardB = _memCardB;

        //Clear the WiiNAND path
        SConfig::GetInstance().m_NANDPath = "";
    }
    else
    {
        SConfig::GetInstance().bWii = true;

        //Set the wii type
        if (_gameType ==  DiscIO::Platform::WII_WAD)
            _wiiWAD = true;
        else
            _wiiWAD = false;

        //clear the GC mem card paths
        SConfig::GetInstance().m_strMemoryCardA = "";
        SConfig::GetInstance().m_strMemoryCardB = "";

        //Set the WiiNAND path
        SConfig::GetInstance().m_NANDPath = supportDirectoryPath  + DIR_SEP + WII_USER_DIR;

        // Disable wiimote continuous scanning
        SConfig::GetInstance().m_WiimoteContinuousScanning = false;

        //Set the Wiimote type
        WiimoteReal::ChangeWiimoteSource(0, _wiiMoteType);
        WiimoteReal::ChangeWiimoteSource(1, _wiiMoteType);
        WiimoteReal::ChangeWiimoteSource(2, _wiiMoteType);
        WiimoteReal::ChangeWiimoteSource(3, _wiiMoteType);
    }
}

# pragma mark - Execution
bool DolHost::LoadFileAtPath()
{

    Core::SetOnStateChangedCallback([](Core::State state) {
        if (state == Core::State::Uninitialized)
            s_running.Clear();
    });

    DolphinAnalytics::Instance()->ReportDolphinStart("openEmu");

    if (_wiiWAD)
        WiiUtils::InstallWAD(_gamePath);
//    else
//        WiiUtils::DoDiscUpdate(nil, _gameRegionName);

   if (!BootManager::BootCore(BootParameters::GenerateFromFile(_gamePath)))
       return false;

    while (!Core::IsRunning())
        updateMainFrameEvent.Wait();
    
    return true;
}

void DolHost::Pause(bool flag)
{
    Core::State state = flag ? Core::State::Paused : Core::State::Running;
    Core::SetState(state);
}

void DolHost::RequestStop()
{
    Core::SetState(Core::State::Running);
    ProcessorInterface::PowerButton_Tap();

    Core::Stop();
    while (CPU::GetState() != CPU::State::PowerDown)
       usleep(1000);

    Core::Shutdown();
    UICommon::Shutdown();
}

void DolHost::Reset()
{
    ProcessorInterface::ResetButton_Tap();
}

void DolHost::UpdateFrame()
{
    Core::HostDispatchJobs();
    updateMainFrameEvent.Set();

    if(_onBoot) _onBoot = false;
}

bool DolHost::CoreRunning()
{
    if (Core::GetState() == Core::State::Running)
        return true;

    return false;
}

# pragma mark - Render FBO
void DolHost::SetPresentationFBO(int RenderFBO)
{
    g_Config.iRenderFBO = RenderFBO;
}

# pragma mark - Audio 
void DolHost::SetVolume(float value)
{
    SConfig::GetInstance().m_Volume = value * 100;
    AudioCommon::UpdateSoundStream();
}

# pragma mark - Save states
bool DolHost::setAutoloadFile(std::string saveStateFile)
{
    Core::SetStateFileName(saveStateFile);

    return true;
}

bool DolHost::SaveState(std::string saveStateFile)
{
    State::SaveAs(saveStateFile);
    return true;
}

bool DolHost::LoadState(std::string saveStateFile)
{
    State::LoadAs(saveStateFile);

    if (DiscIO::IsWii(_gameType))
    {
        // We have to set the wiimote type, cause the gamesave may
        //    have used a different type
        WiimoteReal::ChangeWiimoteSource(0 , _wiiMoteType);
        WiimoteReal::ChangeWiimoteSource(1 , _wiiMoteType);
        WiimoteReal::ChangeWiimoteSource(2 , _wiiMoteType);
        WiimoteReal::ChangeWiimoteSource(3 , _wiiMoteType);

        if( _wiiMoteType != WIIMOTE_SRC_EMU)
            WiimoteReal::Refresh();
    }
    return true;
}

# pragma mark - Cheats
void DolHost::SetCheat(std::string code, std::string type, bool enabled)
{
    NSString* nscode = [NSString stringWithUTF8String:code.c_str()];

    gcode.codes.clear();
    gcode.enabled = enabled;

    // Sanitize
    nscode = [nscode stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

    // Remove any spaces
    nscode = [nscode stringByReplacingOccurrencesOfString:@" " withString:@""];

    NSString *singleCode;
    NSArray *multipleCodes = [nscode componentsSeparatedByString:@"+"];

    Gecko::GeckoCode::Code gcodecode;
    uint32_t cmd_addr, cmd_value;

    for (singleCode in multipleCodes)
    {
        if ([singleCode length] == 16) // Gecko code
        {
            NSString *address = [singleCode substringWithRange:NSMakeRange(0, 8)];
            NSString *value = [singleCode substringWithRange:NSMakeRange(8, 8)];

            bool success_addr = TryParse(std::string("0x") + [address UTF8String], &cmd_addr);
            bool success_val = TryParse(std::string("0x") + [value UTF8String], &cmd_value);

            if (!success_addr || !success_val)
                return;

            gcodecode.address = cmd_addr;
            gcodecode.data = cmd_value;
            gcode.codes.push_back(gcodecode);
        }
        else
        {
            return;
        }
    }

    bool exists = false;

    //  cycle through the codes in our vector
    for (Gecko::GeckoCode& gcompare : gcodes)
    {
        //If the code being modified is the same size as one in the vector, check each value
        if (gcompare.codes.size() == gcode.codes.size())
        {
            for(int i = 0; i < gcode.codes.size() ;i++)
            {
                if (gcompare.codes[i].address == gcode.codes[i].address && gcompare.codes[i].data == gcode.codes[i].data)
                {
                    exists = true;
                }
                else
                {
                    exists = false;
                    // If it's not the same, no need to look through all the codes
                    break;
                }
            }
        }
        if(exists)
        {
            gcompare.enabled = enabled;
            // If it exists, enable it, and we don't need to look at the rest of the codes
            break;
        }
    }

    if(!exists)
        gcodes.push_back(gcode);

    Gecko::SetActiveCodes(gcodes);
}

# pragma mark - Controls
void DolHost::DisplayMessage(std::string message)
{
    Core::DisplayMessage( message, 500);
}


void DolHost::setButtonState(int button, int state, int player)
{
    player -= 1;

    if (_gameType == DiscIO::Platform::GAMECUBE_DISC) {
        setGameCubeButton(player, button, state);
    }
    else
    {
        setWiiButton(player, button, state);
    }


        if (button == OEWiiChangeExtension)
        {
            //set the Extension change state and return.  The next key pressed
            //  while the Change Extension key is held will determine the Extension added
            _wiiChangeExtension[player] = state;
            return;
        }

        if ( _wiiChangeExtension[player] && state == 1)
        {
            if ( button <= OEWiiMoteSwingBackward ) {
                changeWiimoteExtension(WiimoteEmu::EXT_NONE, player);
                Core::DisplayMessage("Extenstion Removed", 1500);
            } else if (button <= OEWiiNunchuckButtonZ ) {
                changeWiimoteExtension(WiimoteEmu::EXT_NUNCHUK, player);
                Core::DisplayMessage("Nunchuk Connected", 1500);
            } else if (button <= OEWiiClassicButtonHome ) {
                changeWiimoteExtension(WiimoteEmu::EXT_CLASSIC, player);
                Core::DisplayMessage("Classic Controller Connected", 1500);
            }
        }
}

void DolHost::SetAxis(int button, float value, int player)
{
    player -= 1;

    if (_gameType == DiscIO::Platform::GAMECUBE_DISC) {
        setGameCubeAxis(player, button, value);
    }
    else
    {
        setWiiAxis(player, button, value);
    }
}

void DolHost::changeWiimoteExtension(int extension, int player)
{
    //Player has already been adjusted befor call
   auto* ce_extension = static_cast<ControllerEmu::Extension*>(Wiimote::GetWiimoteGroup(player, WiimoteEmu::WiimoteGroup::Extension));
    ce_extension->switch_extension = extension;

    WiiRemotes[player].extension = extension;
}

void DolHost::SetIR(int player, float x, float y)
{
    //setWiiIR(player, x,  y);
}

# pragma mark - DVD info

void DolHost::GetGameInfo()
{
     std::unique_ptr<DiscIO::Volume> pVolume = DiscIO::CreateVolumeFromFilename(_gamePath );

    _gameID = pVolume -> GetGameID();
    ///_gameRegion = pVolume -> GetRegion();
    _gameCountry =  DiscIO::CountrySwitch(_gameID[3]);  //pVolume -> GetCountry();
    _gameName = pVolume -> GetInternalName();
    _gameCountryDir = GetDirOfCountry(_gameCountry);
    _gameType = pVolume->GetVolumeType();
}

std::string DolHost::GetNameOfRegion(DiscIO::Region region)
{
    switch (region)
    {

        case DiscIO::Region::NTSC_J:
            return "NTSC_J";

        case DiscIO::Region::NTSC_U:
            return "NTSC_U";

        case DiscIO::Region::PAL:
            return "PAL";

        case DiscIO::Region::NTSC_K:
            return "NTSC_K";

        case DiscIO::Region::UNKNOWN_REGION:
        default:
            return nullptr;
    }
}

std::string DolHost::GetDirOfCountry(DiscIO::Country country)
{
    switch (country)
    {
        case DiscIO::Country::COUNTRY_USA:
            return USA_DIR;

        case DiscIO::Country::COUNTRY_TAIWAN:
        case DiscIO::Country::COUNTRY_KOREA:
        case DiscIO::Country::COUNTRY_JAPAN:
            return JAP_DIR;

        case DiscIO::Country::COUNTRY_AUSTRALIA:
        case DiscIO::Country::COUNTRY_EUROPE:
        case DiscIO::Country::COUNTRY_FRANCE:
        case DiscIO::Country::COUNTRY_GERMANY:
        case DiscIO::Country::COUNTRY_ITALY:
        case DiscIO::Country::COUNTRY_NETHERLANDS:
        case DiscIO::Country::COUNTRY_RUSSIA:
        case DiscIO::Country::COUNTRY_SPAIN:
        case DiscIO::Country::COUNTRY_WORLD:
            return EUR_DIR;

        case DiscIO::Country::COUNTRY_UNKNOWN:
        default:
           return nullptr;
    }
}

# pragma mark - Dolphin Host callbacks
void Host_NotifyMapLoaded() {}
void Host_RefreshDSPDebuggerWindow() {}
void Host_Message(int msg) {
    if  ( msg == WM_USER_CREATE) {
#ifdef DEBUG
         //We have to set FPS display here or it doesn't work
            g_Config.bShowFPS = true;
#endif
        // Core is up,  lets enable Hybric Ubershaders
        g_Config.bPrecompileUberShaders = false;
        g_Config.bBackgroundShaderCompiling = false;
        g_Config.bDisableSpecializedShaders = false;

        //Set the threads to auto (-1)
        g_Config.iShaderCompilerThreads = -1;
        g_Config.iShaderPrecompilerThreads = -1;
    }
}
void* Host_GetRenderHandle() { return nullptr; }
void Host_UpdateTitle(const std::string&) {}
void Host_UpdateDisasmDialog() {}
void Host_UpdateMainFrame() {
    updateMainFrameEvent.Set();
}
void Host_RequestRenderWindowSize(int width, int height) {}
void Host_SetStartupDebuggingParameters()
{
    SConfig& StartUp = SConfig::GetInstance();
    StartUp.bEnableDebugging = false;
    StartUp.bBootToPause = false;
}
bool Host_UINeedsControllerState(){ return false; }
bool Host_RendererHasFocus() { return true; }
bool Host_RendererIsFullscreen() { return false; }
void Host_ShowVideoConfig(void*, const std::string&) {}
void Host_YieldToUI() {}
void Host_UpdateProgressDialog(const char* caption, int position, int total) {

    OSD::AddMessage(StringFromFormat("Processing: %d of %d shaders.", position, total),
                    5000);
}
