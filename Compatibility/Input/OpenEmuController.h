

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
    keymap gc_pad_keymap[20] = {
        {OEGCButtonUp, OEDolJoypadUp, 0},
        {OEGCButtonDown,OEDolJoypadDown, 0},
        {OEGCButtonLeft, OEDolJoypadLeft, 0},
        {OEGCButtonRight, OEDolJoypadRight, 0},
        {OEGCAnalogUp, OEGCAnalog, 0},
        {OEGCAnalogDown, OEGCAnalog, 0},
        {OEGCAnalogLeft, OEGCAnalog, 0},
        {OEGCAnalogRight, OEGCAnalog, 0},
        {OEGCAnalogCUp, OEGCAnalogC, 0},
        {OEGCAnalogCDown, OEGCAnalogC, 0},
        {OEGCAnalogCLeft, OEGCAnalogC, 0},
        {OEGCAnalogCRight, OEGCAnalogC, 0},
        {OEGCButtonA, OEDolJoypadA, 0},
        {OEGCButtonB, OEDolJoypadB, 0},
        {OEGCButtonX, OEDolJoypadX, 0},
        {OEGCButtonY, OEDolJoypadY, 0},
        {OEGCButtonL, OEDolJoypadL, 0},
        {OEGCButtonR, OEDolJoypadR, 0},
        {OEGCButtonZ, OEDolJoypadR2, 0},
        {OEGCButtonStart, OEDolJoypadStart, 0},
    };
    
//    axismap gc_pad_Analog;
//    axismap gc_pad_AnalogC;
    axismap gc_pad_Trigger;
} gc_pad;


void setGameCubeButton(int pad_num, int button , int value);
void setGameCubeAxis(int pad_num, int button , float value);
void init_Callback();

static gc_pad GameCubePads[4];

//
//static unsigned connected_wiimote_type[MAX_BBMOTES];
//static int current_mote_id;
//static InputConfig s_config(WIIMOTE_INI_NAME, _trans("Wii Remote"), "Wiimote");

//
//typedef struct
//{
//    keymap wiimote_keymap[12] = {
//        {OEWiiMoteButtonLeft, OEDolJoypadLeft, 0},
//        {OEWiiMoteButtonRight, OEDolJoypadRight, 0},
//        {OEWiiMoteButtonDown, OEDolJoypadDown, 0},
//        {OEWiiMoteButtonUp, OEDolJoypadUp, 0},
//        {OEWiiMoteButtonPlus, OEDolJoypadStart, 0},
//        {OEWiiMoteButton2, OEDolJoypadY, 0},
//        {OEWiiMoteButton1, OEDolJoypadX, 0},
//        {OEWiiMoteButtonB, OEDolJoypadB, 0},
//        {OEWiiMoteButtonA, OEDolJoypadA, 0},
//        {OEWiiMoteButtonMinus, OEDolJoypadSelect, 0},
//        {OEWiiMoteButtonHome, OEDolJoypadR3, 0},
//        {OEWiiMoteShake, OEDolJoypadR2, 0},
//    };
//
//    keymap sideways_keymap[12] = {
//        {OEWiiMoteButtonLeft, OEDolJoypadUp, 0},
//        {OEWiiMoteButtonRight, OEDolJoypadDown, 0},
//        {OEWiiMoteButtonDown, OEDolJoypadLeft, 0},
//        {OEWiiMoteButtonUp, OEDolJoypadRight, 0},
//        {OEWiiMoteButtonPlus, OEDolJoypadStart, 0},
//        {OEWiiMoteButton2, OEDolJoypadB, 0},
//        {OEWiiMoteButton1, OEDolJoypadA, 0},
//        {OEWiiMoteButtonB, OEDolJoypadY, 0},
//        {OEWiiMoteButtonA, OEDolJoypadX, 0},
//        {OEWiiMoteButtonMinus, OEDolJoypadSelect, 0},
//        {OEWiiMoteButtonHome, OEDolJoypadR3, 0},
//        {OEWiiMoteShake, OEDolJoypadR2, 0},
//};
//
//    axismap wiimote_tilt_LR;
//    axismap wiimote_tilt_FB;
//
//    keymap nunchuk_keymap[3] = {
//        {OEWiiNunchukButtonC, OEDolJoypadL, 0},
//        {OEWiiNunchukButtonZ, OEDolJoypadR, 0},
//        {OEWiiNunchukShake, OEDolJoypadL3, 0},
//    };
//
//    axismap nunchuck_Analog;
//
//    keymap classic_keymap[15] = {
//        {OEWiiClassicButtonRight, WiimoteEmu::Classic::PAD_RIGHT, 0},
//        {OEWiiClassicButtonDown, WiimoteEmu::Classic::PAD_DOWN, 0},
//        {OEWiiClassicButtonL, WiimoteEmu::Classic::TRIGGER_L, 0},
//        {OEWiiClassicButtonSelect, WiimoteEmu::Classic::BUTTON_MINUS, 0},
//        {OEWiiClassicButtonHome, WiimoteEmu::Classic::BUTTON_HOME, 0},
//        {OEWiiClassicButtonStart, WiimoteEmu::Classic::BUTTON_PLUS, 0},
//        {OEWiiClassicButtonR, WiimoteEmu::Classic::TRIGGER_R, 0},
//        {OEWiiClassicButtonZl, WiimoteEmu::Classic::BUTTON_ZL, 0},
//        {OEWiiClassicButtonB, WiimoteEmu::Classic::BUTTON_B, 0},
//        {OEWiiClassicButtonY, WiimoteEmu::Classic::BUTTON_Y, 0},
//        {OEWiiClassicButtonX, WiimoteEmu::Classic::BUTTON_X, 0},
//        {OEWiiClassicButtonA, WiimoteEmu::Classic::BUTTON_A, 0},
//        {OEWiiClassicButtonZr, WiimoteEmu::Classic::BUTTON_ZR, 0},
//        {OEWiiClassicButtonLeft, WiimoteEmu::Classic::PAD_LEFT, 0},
//        {OEWiiClassicButtonUp, WiimoteEmu::Classic::PAD_UP, 0},
//    };
//
//    axismap classic_AnalogLeft;
//    axismap classic_AnalogRight;
//    axismap classic_TriggerLeft;
//    axismap classic_TriggerRight;
//
//    int extension;
//    ControlState dx, dy;
//
//} wii_remote;

// static gc_pad GameCubePads[4];
// static wii_remote WiiRemotes[4];
// static int want_extension[4];

//void setWiiButton(int pad_num, int button , int value);
//void setWiimoteButton(int pad_num, int button , int value);
//void setWiiClassicButton(int pad_num, int button , int value);
//void setWiiNunchukButton(int pad_num, int button , int value);
//
//void setWiiAxis(int pad_num, int button , float value);
//void setWiimoteAxis(int pad_num, int button , float value);
//void setWiiClassicAxis(int pad_num, int button , float value);
//void setWiiNunchukAxis(int pad_num, int button , float value);
//void setWiiIR(int pad_num, float x , float y);
//
//int getWiiExtension (int pad_num);
