#include <cassert>

#include "input.h"


GCPadStatus Pad::GetStatus(int pad_num)
{
    //   DEBUG_VAR(pad_num);
    GCPadStatus pad = {};

    unsigned i;

    for (i = 0; i < (sizeof( GameCubePads[pad_num].gc_pad_keymap) / sizeof(* GameCubePads[pad_num].gc_pad_keymap)); i++)
        pad.button |=  GameCubePads[pad_num].gc_pad_keymap[i].value ?  GameCubePads[pad_num].gc_pad_keymap[i].dolphinButton : 0;


    if (pad.button & PAD_BUTTON_A)
        pad.analogA = 0xFF;

    if (pad.button & PAD_BUTTON_B)
        pad.analogB = 0xFF;

    pad.stickX = static_cast<u8>(GCPadStatus::MAIN_STICK_CENTER_X + ( GameCubePads[pad_num].gc_pad_Analog.Xaxis * GCPadStatus::MAIN_STICK_RADIUS));
    pad.stickY = static_cast<u8>(GCPadStatus::MAIN_STICK_CENTER_Y + ( GameCubePads[pad_num].gc_pad_Analog.Yaxis * GCPadStatus::MAIN_STICK_RADIUS));
    pad.substickX =static_cast<u8>(GCPadStatus::C_STICK_CENTER_X + ( GameCubePads[pad_num].gc_pad_AnalogC.Xaxis * GCPadStatus::C_STICK_RADIUS));
    pad.substickY =  static_cast<u8>(GCPadStatus::C_STICK_CENTER_Y + ( GameCubePads[pad_num].gc_pad_AnalogC.Yaxis * GCPadStatus::C_STICK_RADIUS));

    if (pad.button & PAD_TRIGGER_L)
        pad.triggerLeft = 0xFF;

    if (pad.button & PAD_TRIGGER_R)
        pad.triggerRight = 0xFF;

    return pad;
}

void Pad::Rumble(const int pad_num, const ControlState strength)
{
    /* todo: determine actual range used by caller */

    if (pad_num > 0)
        return;

    uint16_t str;

    if (strength < 0.0)
        str = 0;
    else if (strength > 1.0)
        str = 0xFFFF;
    else
        str = (uint16_t)(0xFFFF * strength);

    //    rumble.set_rumble_state(pad_num, RETRO_RUMBLE_WEAK, str);
    //    rumble.set_rumble_state(pad_num, RETRO_RUMBLE_STRONG, str);
}

bool Pad::GetMicButton(const int pad_num)
{
    //   return input_cb(pad_num, JOYPAD, 0,  ID_JOYPAD_L2);
    return false;
}

/* stubs */
ControllerEmu::ControlGroup* Pad::GetGroup(int pad_num, PadGroup group)
{
    return nullptr;
}

void Pad::Shutdown()
{
}

void Pad::Initialize()
{
}

void Pad::LoadConfig()
{
}

#include "InputCommon/InputConfig.h"
InputConfig* Pad::GetConfig()
{
    // return nullptr;
    static InputConfig s_config("GCPadNew", _trans("Pad"), "GCPad");
    return &s_config;
}

void setGameCubeButton(int pad_num, int button , int value) {
    for (unsigned i = 0; i < (sizeof( GameCubePads[pad_num].gc_pad_keymap) / sizeof(* GameCubePads[pad_num].gc_pad_keymap)); i++)
        if ( GameCubePads[pad_num].gc_pad_keymap[i].openemuButton == button)
        {
             GameCubePads[pad_num].gc_pad_keymap[i].value = value;
            return;
        }
}

void setGameCubeAxis(int pad_num, int button , float value) {
  switch (button)
    {
        case OEGCAnalogUp:
             GameCubePads[pad_num].gc_pad_Analog.Yaxis = value;
            break;
        case OEGCAnalogDown:
             GameCubePads[pad_num].gc_pad_Analog.Yaxis = -value;
            break;

        case OEGCAnalogLeft:
             GameCubePads[pad_num].gc_pad_Analog.Xaxis = -value;
            break;
        case OEGCAnalogRight:
             GameCubePads[pad_num].gc_pad_Analog.Xaxis = value;
            break;

        case OEGCAnalogCUp:
             GameCubePads[pad_num].gc_pad_AnalogC.Yaxis = value;
            break;
        case OEGCAnalogCDown:
             GameCubePads[pad_num].gc_pad_AnalogC.Yaxis = -value;
            break;

        case OEGCAnalogCLeft:
             GameCubePads[pad_num].gc_pad_AnalogC.Xaxis = -value;
            break;
        case OEGCAnalogCRight:
             GameCubePads[pad_num].gc_pad_AnalogC.Xaxis = value;
            break;

        default:
            break;

    }
}
