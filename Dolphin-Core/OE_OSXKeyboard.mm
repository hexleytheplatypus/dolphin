// Copyright 2014 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <sstream>

#include <Foundation/Foundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <Cocoa/Cocoa.h>

#include "OE_OSXKeyboard.h"

namespace ciface
{
namespace OSX
{

Keyboard::Keyboard(std::string name, int index)
	:m_device_name(name)
	, m_index(index)
{
}

void Keyboard::UpdateInput()
{
}

std::string Keyboard::GetName() const
{
	return m_device_name;
}

std::string Keyboard::GetSource() const
{
	return 0;
}

int Keyboard::GetId() const
{
	return m_index;
}

Keyboard::Key::Key()
	
{
}

ControlState Keyboard::Key::GetState() const
{
		return 0;
}

ControlState Keyboard::Cursor::GetState() const
{
    return 0;
}

ControlState Keyboard::Button::GetState() const
{
	return 0;
}

std::string Keyboard::Cursor::GetName() const
{
	return 0;
}

std::string Keyboard::Button::GetName() const
{
	return 0;
}

std::string Keyboard::Key::GetName() const
{
	return 0;
}


}
}
