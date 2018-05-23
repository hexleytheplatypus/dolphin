// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Core/HW/WiimoteEmu/Attachment/Classic.h"

#include "input.h"

#include <array>
#include <cassert>

#include "Common/Common.h"
#include "Common/CommonTypes.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"

#include "InputCommon/ControllerEmu/Control/Input.h"
#include "InputCommon/ControllerEmu/ControlGroup/AnalogStick.h"
#include "InputCommon/ControllerEmu/ControlGroup/Buttons.h"
#include "InputCommon/ControllerEmu/ControlGroup/ControlGroup.h"
#include "InputCommon/ControllerEmu/ControlGroup/MixedTriggers.h"

namespace WiimoteEmu
{
    constexpr std::array<u8, 6> classic_id{{0x00, 0x00, 0xa4, 0x20, 0x01, 0x01}};

    // Classic Controller calibration
    constexpr std::array<u8, 0x10> classic_calibration{{
        0xff, 0x00, 0x80, 0xff, 0x00, 0x80, 0xff, 0x00, 0x80, 0xff, 0x00, 0x80, 0x00, 0x00, 0x51, 0xa6,
    }};

    constexpr std::array<u16, 9> classic_button_bitmasks{{
        Classic::BUTTON_A, Classic::BUTTON_B, Classic::BUTTON_X, Classic::BUTTON_Y,

        Classic::BUTTON_ZL, Classic::BUTTON_ZR,

        Classic::BUTTON_MINUS, Classic::BUTTON_PLUS,

        Classic::BUTTON_HOME,
    }};

    constexpr std::array<const char*, 9> classic_button_names{{
        "A", "B", "X", "Y", "ZL", "ZR", "-", "+", "Home",
    }};

    constexpr std::array<u16, 2> classic_trigger_bitmasks{{
        Classic::TRIGGER_L, Classic::TRIGGER_R,
    }};

    constexpr std::array<const char*, 4> classic_trigger_names{{
        // i18n: The left trigger button (labeled L on real controllers)
        _trans("L"),
        // i18n: The right trigger button (labeled R on real controllers)
        _trans("R"),
        // i18n: The left trigger button (labeled L on real controllers) used as an analog input
        _trans("L-Analog"),
        // i18n: The right trigger button (labeled R on real controllers) used as an analog input
        _trans("R-Analog"),
    }};

    constexpr std::array<u16, 4> classic_dpad_bitmasks{{
        Classic::PAD_UP, Classic::PAD_DOWN, Classic::PAD_LEFT, Classic::PAD_RIGHT,
    }};

    Classic::Classic(ExtensionReg& reg) : Attachment(_trans("Classic"), reg)
    {
        // buttons
        groups.emplace_back(m_buttons = new ControllerEmu::Buttons(_trans("Buttons")));
        for (const char* button_name : classic_button_names)
        {
            const std::string& ui_name = (button_name == std::string("Home")) ? "HOME" : button_name;
//            m_buttons->controls.emplace_back(new ControllerEmu::Input(button_name, ui_name));
        }

        // sticks
        groups.emplace_back(m_left_stick = new ControllerEmu::AnalogStick(
                                                                          _trans("Left Stick"), DEFAULT_ATTACHMENT_STICK_RADIUS));
        groups.emplace_back(m_right_stick = new ControllerEmu::AnalogStick(
                                                                           _trans("Right Stick"), DEFAULT_ATTACHMENT_STICK_RADIUS));

        // triggers
        groups.emplace_back(m_triggers = new ControllerEmu::MixedTriggers(_trans("Triggers")));
        for (const char* trigger_name : classic_trigger_names)
//            m_triggers->controls.emplace_back(new ControllerEmu::Input(trigger_name));

        // dpad
        groups.emplace_back(m_dpad = new ControllerEmu::Buttons(_trans("D-Pad")));
        for (const char* named_direction : named_directions)
//            m_dpad->controls.emplace_back(new ControllerEmu::Input(named_direction));

        // Set up register
        m_calibration = classic_calibration;
        m_id = classic_id;
    }

    void Classic::GetState(u8* const data)
    {
        wm_classic_extension* const ccdata = reinterpret_cast<wm_classic_extension* const>(data);
        ccdata->bt.hex = 0;

        // not using calibration data, o well

        int pad_num= 0;

        // left stick
        {
            //ControlState x, y;
            //m_left_stick->GetState(&x, &y);

            ccdata->regular_data.lx =
            static_cast<u8>(Classic::LEFT_STICK_CENTER_X + ( WiiRemotes[pad_num].classic_AnalogLeft.Xaxis * Classic::LEFT_STICK_RADIUS));
            ccdata->regular_data.ly =
            static_cast<u8>(Classic::LEFT_STICK_CENTER_Y + ( WiiRemotes[pad_num].classic_AnalogLeft.Yaxis * Classic::LEFT_STICK_RADIUS));

        }

        // right stick
        {
            //ControlState x, y;
            u8 x_, y_;
            x_ = static_cast<u8>(Classic::RIGHT_STICK_CENTER_X + ( WiiRemotes[pad_num].classic_AnalogRight.Xaxis * Classic::RIGHT_STICK_RADIUS));
            y_ = static_cast<u8>(Classic::RIGHT_STICK_CENTER_Y + ( WiiRemotes[pad_num].classic_AnalogRight.Yaxis * Classic::RIGHT_STICK_RADIUS));
            
            ccdata->rx1 = x_;
            ccdata->rx2 = x_ >> 1;
            ccdata->rx3 = x_ >> 3;
            ccdata->ry = y_;
            
        }

        // triggers
        {
            ControlState trigs[2] = {0, 0};
            u8 lt, rt;
            m_triggers->GetState(&ccdata->bt.hex, classic_trigger_bitmasks.data(), trigs);

            lt = static_cast<u8>(WiiRemotes[pad_num].classic_TriggerLeft.Xaxis * Classic::LEFT_TRIGGER_RANGE);
            rt = static_cast<u8>(WiiRemotes[pad_num].classic_TriggerRight.Xaxis * Classic::RIGHT_TRIGGER_RANGE);

            ccdata->lt1 = lt;
            ccdata->lt2 = lt >> 3;
            ccdata->rt = rt;
        }

        // buttons
        ccdata->bt.hex = 0;

        for (unsigned i = 0; i < (sizeof( WiiRemotes[pad_num].classic_keymap) / sizeof(* WiiRemotes[pad_num].classic_keymap)); i++)
            ccdata->bt.hex |=  WiiRemotes[pad_num].classic_keymap[i].value ?  WiiRemotes[pad_num].classic_keymap[i].dolphinButton : 0;

        // flip button bits
        ccdata->bt.hex ^= 0xFFFF;
    }

    bool Classic::IsButtonPressed() const
    {
        u16 buttons = 0;
        std::array<ControlState, 2> trigs{};
        m_buttons->GetState(&buttons, classic_button_bitmasks.data());
        m_dpad->GetState(&buttons, classic_dpad_bitmasks.data());
        m_triggers->GetState(&buttons, classic_trigger_bitmasks.data(), trigs.data());

        int pad_num = 0;

//        for (pad_num = 0; pad_num < MAX_BBMOTES && this != classic_controllers_map[pad_num]; pad_num++)
//            ;
//
//        if (pad_num > 4)
//            return false;

        for (unsigned i = 0; i < (sizeof( WiiRemotes[pad_num].classic_keymap) / sizeof(* WiiRemotes[pad_num].classic_keymap)); i++)
            if ( WiiRemotes[pad_num].classic_keymap[i].value)
                return true;

        return false;

//        return buttons != 0;
    }

    ControllerEmu::ControlGroup* Classic::GetGroup(ClassicGroup group)
    {
        switch (group)
        {
            case ClassicGroup::Buttons:
                return m_buttons;
            case ClassicGroup::Triggers:
                return m_triggers;
            case ClassicGroup::DPad:
                return m_dpad;
            case ClassicGroup::LeftStick:
                return m_left_stick;
            case ClassicGroup::RightStick:
                return m_right_stick;
            default:
                assert(false);
                return nullptr;
        }
    }
}


void setWiiClassicButton(int pad_num, int button , int value)
{
    for (unsigned i = 0; i < (sizeof( WiiRemotes[pad_num].classic_keymap) / sizeof(* WiiRemotes[pad_num].classic_keymap)); i++) {
        if ( WiiRemotes[pad_num].classic_keymap[i].openemuButton == button) {
            WiiRemotes[pad_num].classic_keymap[i].value = value;
            return;
        }
    }
}

void setWiiClassicAxis(int pad_num, int button , float value)
{
    switch (button)
    {
        case OEWiiClassicAnalogLUp:
            WiiRemotes[pad_num].classic_AnalogLeft.Yaxis = value;
            break;
        case OEWiiClassicAnalogLDown:
            WiiRemotes[pad_num].classic_AnalogLeft.Yaxis = -value;
            break;

        case OEWiiClassicAnalogLLeft:
            WiiRemotes[pad_num].classic_AnalogLeft.Xaxis = -value;
            break;
        case OEWiiClassicAnalogLRight:
            WiiRemotes[pad_num].classic_AnalogLeft.Xaxis = value;
            break;

        case OEWiiClassicAnalogRUp:
            WiiRemotes[pad_num].classic_AnalogRight.Yaxis = value;
            break;
        case OEWiiClassicAnalogRDown:
            WiiRemotes[pad_num].classic_AnalogRight.Yaxis = -value;
            break;

        case OEWiiClassicAnalogRLeft:
            WiiRemotes[pad_num].classic_AnalogRight.Xaxis = -value;
            break;
        case OEWiiClassicAnalogRRight:
            WiiRemotes[pad_num].classic_AnalogRight.Xaxis = value;
            break;

        case OEWiiClassicButtonL:
            WiiRemotes[pad_num].classic_TriggerLeft.Xaxis = value;
            break;
        case OEWiiClassicButtonR:
            WiiRemotes[pad_num].classic_TriggerRight.Xaxis = value;
            break;
            
        default:
            break;

    }
}

