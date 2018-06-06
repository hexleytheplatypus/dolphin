#include <array>

#include "input.h"


void setWiiButton(int pad_num, int button , int value) {

    if (button <= OEWiiMoteButtonHome || button == OEWiiMoteShake || button == OEWiiSideways)
    {
        setWiimoteButton(pad_num, button , value);
        
        return;
    }
    if (button >= OEWiiClassicButtonUp)
    {
        setWiiClassicButton(pad_num, button , value);
        return;
    }
    if (button >=  OEWiiNunchukButtonC)
    {
        setWiiNunchukButton(pad_num, button , value);
        return;
    }
    
    //static_cast<WiimoteEmu::Wiimote*>(s_config.GetController(pad_num))->Update()
}

void setWiiAxis(int pad_num, int button , float value){
    if (button >= OEWiiClassicButtonUp)
    {
        setWiiClassicAxis(pad_num, button , value);
        return;
    }
    if (button >=  OEWiiNunchukAnalogUp)
    {
        setWiiNunchukAxis(pad_num, button , value);
        return;
    }
    if (button <= OEWiiMoteButtonHome)
    {
        setWiimoteAxis(pad_num, button , value);
    }
}

int getWiiExtension (int pad_num){
    int ext = WiiRemotes[pad_num].extension;

    return ext;
}
