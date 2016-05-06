// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cstring>

#include "Common/Common.h"
#include "Common/CommonTypes.h"
#include "Common/MathUtil.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "Core/HW/WiimoteEmu/Attachment/Nunchuk.h"

namespace WiimoteEmu
{

static const u8 nunchuk_id[] = { 0x00, 0x00, 0xa4, 0x20, 0x00, 0x00 };

static const u8 nunchuk_button_bitmasks[] =
{
	Nunchuk::BUTTON_C,
	Nunchuk::BUTTON_Z,
};


Nunchuk::Nunchuk(WiimoteEmu::ExtensionReg& _reg) : Attachment(_trans("Nunchuk"), _reg)
{
	// buttons
	groups.emplace_back(m_buttons = new Buttons("Buttons"));
	m_buttons->controls.emplace_back(new ControlGroup::Input("OEWiiNunchukButtonC"));
	m_buttons->controls.emplace_back(new ControlGroup::Input("OEWiiNunchukButtonZ"));

	// stick
	groups.emplace_back(m_stick = new AnalogStick("Nunchuk Stick", DEFAULT_ATTACHMENT_STICK_RADIUS));

	// swing
	groups.emplace_back(m_swing = new Force("Swing"));

	// tilt
	groups.emplace_back(m_tilt = new Tilt("Tilt"));

	// shake
	groups.emplace_back(m_shake = new Buttons("Shake"));
	m_shake->controls.emplace_back(new ControlGroup::Input("OEWiiNunchukAccelX"));
	m_shake->controls.emplace_back(new ControlGroup::Input("OEWiiNunchukAccelY"));
	m_shake->controls.emplace_back(new ControlGroup::Input("OEWiiNunchukAccelZ"));

	// id
	memcpy(&id, nunchuk_id, sizeof(nunchuk_id));

	// this should get set to 0 on disconnect, but it isn't, o well
    memset(m_shake_step, 0, sizeof(m_shake_step));
}

void Nunchuk::UpdateAccelData(float X, float Y, float Z)
{
    m_accel.x = X;
    m_accel.y = Y;
    m_accel.z = Z;
}

void Nunchuk::GetState(u8* const data)
{
    wm_nc* const ncdata = (wm_nc*)data;
    ncdata->bt.hex = 0;
    
    // stick
	double jx, jy;
	m_stick->GetState(&jx, &jy);

	ncdata->jx = u8(STICK_CENTER + jx * STICK_RADIUS);
	ncdata->jy = u8(STICK_CENTER + jy * STICK_RADIUS);

	// Some terribly coded games check whether to move with a check like
	//
	//     if (x != 0 && y != 0)
	//         do_movement(x, y);
	//
	// With keyboard controls, these games break if you simply hit
	// of the axes. Adjust this if you're hitting one of the axes so that
	// we slightly tweak the other axis.
	if (ncdata->jx != STICK_CENTER || ncdata->jy != STICK_CENTER)
	{
		if (ncdata->jx == STICK_CENTER)
			++ncdata->jx;
		if (ncdata->jy == STICK_CENTER)
			++ncdata->jy;
	}

    //AccelData accel;
   
    //	// tilt
    //	EmulateTilt(&accel, m_tilt);
    //
    //	// swing
    //	EmulateSwing(&accel, m_swing);
    //	// shake
    //	EmulateShake(&accel, m_shake, m_shake_step);
    // buttons
    m_buttons->GetState(&ncdata->bt.hex, nunchuk_button_bitmasks);

    // flip the button bits :/
    ncdata->bt.hex ^= 0x03;

    ncdata->ax = m_accel.x;
    ncdata->ay = m_accel.y;
    ncdata->az = m_accel.z;
    ncdata->bt.acc_x_lsb = m_accel.x;
    ncdata->bt.acc_y_lsb = m_accel.y;
    ncdata->bt.acc_z_lsb = m_accel.z;
}

    bool Nunchuk::IsButtonPressed() const
    {
        u8 buttons = 0;
        m_buttons->GetState(&buttons, nunchuk_button_bitmasks);
        return buttons != 0;
    }

    void Nunchuk::LoadDefaults(const ControllerInterface& ciface)
    {
        // Stick
        m_stick->SetControlExpression(0, "OEWiiNunchukAnalogUp"); // up
        m_stick->SetControlExpression(1, "OEWiiNunchukAnalogDown"); // down
        m_stick->SetControlExpression(2, "OEWiiNunchukAnalogLeft"); // left
        m_stick->SetControlExpression(3, "OEWiiNunchukAnalogRight"); // right

        // Buttons
        m_buttons->SetControlExpression(0, "OEWiiNunchukButtonC"); // C
        m_buttons->SetControlExpression(1, "OEWiiNunchukButtonZ");   // Z

        //Accelerometer
        m_shake->SetControlExpression(0, "OEWiiNunchukAccelX");
        m_shake->SetControlExpression(1, "OEWiiNunchukAccelY");
        m_shake->SetControlExpression(2, "OEWiiNunchukAccelZ");
        
    }

}
