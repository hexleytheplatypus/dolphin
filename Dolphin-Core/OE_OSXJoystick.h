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


#pragma once

#include <IOKit/hid/IOHIDLib.h>

#include "InputCommon/ControllerInterface/Device.h"
#include "InputCommon/ControllerInterface/ForceFeedback/ForceFeedbackDevice.h"

namespace ciface
{
namespace OSX
{

class Joystick : public ForceFeedback::ForceFeedbackDevice
{
private:
	class Button : public Input
	{
	public:
        Button(std::string description);
		std::string GetName() const override;
		ControlState GetState() const override;
        void SetState(ControlState state) override;
        
	private:
		std::string            m_name;
        int                     m_state;
	};

	class Axis : public Input
	{
	public:
		enum direction
		{
			positive = 0,
			negative
		};

		Axis(std::string description, direction dir);
		std::string GetName() const override;
		ControlState GetState() const override;
        void SetState(ControlState state)  ;

	private:
        float                 m_value;
		std::string           m_name;
		const direction       m_direction;
		float                 m_neutral;
		float                 m_scale;
	};

	class Hat : public Input
	{
	public:
		enum direction
		{
			up = 0,
			right,
			down,
			left
		};

		Hat( direction dir);
		std::string GetName() const override;
		ControlState GetState() const override;

	private:
		const char*           m_name;
		const direction       m_direction;
	};

public:
	Joystick(std::string name, int index);
    
	~Joystick();

	std::string GetName() const override;
	std::string GetSource() const override;
	int GetId() const override;

    
    
private:
	
	const std::string    m_device_name;
	const int            m_index;

	ForceFeedback::FFDeviceAdapterReference m_ff_device;
};

}
}
