#pragma once

#include "Core/HW/GCPad.h"
#include "Core/HW/Wiimote.h"
#include "Core/HW/WiimoteEmu/Attachment/Classic.h"
#include "Core/HW/WiimoteEmu/Attachment/Nunchuk.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "Common/Common.h"
#include "Common/CommonTypes.h"

#include "Core/Host.h"

#include "InputCommon/GCPadStatus.h"
#include "DolphinGameCore.h"
#include "Wii/OEWiiSystemResponderClient.h"

static unsigned connected_wiimote_type[MAX_BBMOTES];
static int current_mote_id;

typedef struct
{
    int openemuButton;
    unsigned dolphinButton;
    int value;
} keymap;

typedef struct
{
    float Xaxis;
    float Yaxis;
} axismap;

typedef struct
{
    keymap gc_pad_keymap[12] = {
        {OEGCButtonLeft, PAD_BUTTON_LEFT, 0},
        {OEGCButtonRight, PAD_BUTTON_RIGHT, 0},
        {OEGCButtonDown, PAD_BUTTON_DOWN, 0},
        {OEGCButtonUp, PAD_BUTTON_UP, 0},
        {OEGCButtonZ, PAD_TRIGGER_Z, 0},
        {OEGCButtonR, PAD_TRIGGER_R, 0},
        {OEGCButtonL, PAD_TRIGGER_L, 0},
        {OEGCButtonA, PAD_BUTTON_A, 0},
        {OEGCButtonB, PAD_BUTTON_B, 0},
        {OEGCButtonX, PAD_BUTTON_X, 0},
        {OEGCButtonY, PAD_BUTTON_Y, 0},
        {OEGCButtonStart, PAD_BUTTON_START, 0},
    };

    axismap gc_pad_Analog;
    axismap gc_pad_AnalogC;
} gc_pad;

typedef struct
{
    keymap wiimote_keymap[11] = {
        {OEWiiMoteButtonLeft, WiimoteEmu::Wiimote::PAD_LEFT, 0},
        {OEWiiMoteButtonRight, WiimoteEmu::Wiimote::PAD_RIGHT, 0},
        {OEWiiMoteButtonDown, WiimoteEmu::Wiimote::PAD_DOWN, 0},
        {OEWiiMoteButtonUp, WiimoteEmu::Wiimote::PAD_UP, 0},
        {OEWiiMoteButtonPlus, WiimoteEmu::Wiimote::BUTTON_PLUS, 0},
        {OEWiiMoteButton2, WiimoteEmu::Wiimote::BUTTON_TWO, 0},
        {OEWiiMoteButton1, WiimoteEmu::Wiimote::BUTTON_ONE, 0},
        {OEWiiMoteButtonB, WiimoteEmu::Wiimote::BUTTON_B, 0},
        {OEWiiMoteButtonA, WiimoteEmu::Wiimote::BUTTON_A, 0},
        {OEWiiMoteButtonMinus, WiimoteEmu::Wiimote::BUTTON_MINUS, 0},
        {OEWiiMoteButtonHome, WiimoteEmu::Wiimote::BUTTON_HOME, 0},
    };

    keymap sideways_keymap[11] = {
        {OEWiiMoteButtonLeft, WiimoteEmu::Wiimote::PAD_UP, 0},
        {OEWiiMoteButtonRight, WiimoteEmu::Wiimote::PAD_DOWN, 0},
        {OEWiiMoteButtonDown, WiimoteEmu::Wiimote::PAD_LEFT, 0},
        {OEWiiMoteButtonUp, WiimoteEmu::Wiimote::PAD_RIGHT, 0},
        {OEWiiMoteButtonPlus, WiimoteEmu::Wiimote::BUTTON_PLUS, 0},
        {OEWiiMoteButtonB, WiimoteEmu::Wiimote::BUTTON_B, 0},
        {OEWiiMoteButtonA, WiimoteEmu::Wiimote::BUTTON_A, 0},
        {OEWiiMoteButton1, WiimoteEmu::Wiimote::BUTTON_ONE, 0},
        {OEWiiMoteButton2, WiimoteEmu::Wiimote::BUTTON_TWO, 0},
        {OEWiiMoteButtonMinus, WiimoteEmu::Wiimote::BUTTON_MINUS, 0},
        {OEWiiMoteButtonHome, WiimoteEmu::Wiimote::BUTTON_HOME, 0},
    };

    axismap wiimote_tilt;
    axismap wiimote_swing;
    bool emuShake;
    
    keymap nunchuk_keymap[2] = {
        {OEWiiNunchukButtonC, WiimoteEmu::Nunchuk::BUTTON_C, 0},
        {OEWiiNunchuckButtonZ, WiimoteEmu::Nunchuk::BUTTON_Z, 0},
    };

    axismap nunchuck_Analog;

    keymap classic_keymap[15] = {
        {OEWiiClassicButtonRight, WiimoteEmu::Classic::PAD_RIGHT, 0},
        {OEWiiClassicButtonDown, WiimoteEmu::Classic::PAD_DOWN, 0},
        {OEWiiClassicButtonLeft, WiimoteEmu::Classic::TRIGGER_L, 0},
        {OEWiiClassicButtonSelect, WiimoteEmu::Classic::BUTTON_MINUS, 0},
        {OEWiiClassicButtonHome, WiimoteEmu::Classic::BUTTON_HOME, 0},
        {OEWiiClassicButtonStart, WiimoteEmu::Classic::BUTTON_PLUS, 0},
        {OEWiiClassicButtonRight, WiimoteEmu::Classic::TRIGGER_R, 0},
        {OEWiiClassicButtonZl, WiimoteEmu::Classic::BUTTON_ZL, 0},
        {OEWiiClassicButtonB, WiimoteEmu::Classic::BUTTON_B, 0},
        {OEWiiClassicButtonY, WiimoteEmu::Classic::BUTTON_Y, 0},
        {OEWiiClassicButtonX, WiimoteEmu::Classic::BUTTON_X, 0},
        {OEWiiClassicButtonA, WiimoteEmu::Classic::BUTTON_A, 0},
        {OEWiiClassicButtonZr, WiimoteEmu::Classic::BUTTON_ZR, 0},
        {OEWiiClassicButtonLeft, WiimoteEmu::Classic::PAD_LEFT, 0},
        {OEWiiClassicButtonUp, WiimoteEmu::Classic::PAD_UP, 0},
    };

    axismap classic_AnalogLeft;
    axismap classic_AnalogRight;

    int extension;
    ControlState dx, dy;
    
} wii_remote;


static gc_pad GameCubePads[4];
static wii_remote WiiRemotes[4];
static int want_extension[4];


void setGameCubeButton(int pad_num, int button , int value);
void setGameCubeAxis(int pad_num, int button , float value);

void setWiiButton(int pad_num, int button , int value);
void setWiimoteButton(int pad_num, int button , int value);
void setWiiClassicButton(int pad_num, int button , int value);
void setWiiNunchukButton(int pad_num, int button , int value);

void setWiiAxis(int pad_num, int button , float value);
void setWiimoteAxis(int pad_num, int button , float value);
void setWiiClassicAxis(int pad_num, int button , float value);
void setWiiNunchukAxis(int pad_num, int button , float value);
void setWiiIR(int pad_num, float x , float y);

int getWiiExtension (int pad_num);
