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


#include <sstream>

#include <Foundation/Foundation.h>
//#include <IOKit/hid/IOHIDLib.h>

#include "OE_OSXJoystick.h"
#include "InputCommon/ControllerEmu.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"


namespace ciface
{
    namespace OSX
    {
        Joystick::Joystick( std::string name, int index)
        : m_device_name(name)
        , m_index(index)
        , m_ff_device(nullptr)
        {
            // Buttons

            //  OE DPAD buttons
            AddInput(new Button("Dpad_UP"));
            AddInput(new Button("Dpad_Down"));
            AddInput(new Button("Dpad_Left"));
            AddInput(new Button("Dpad_Right"));

            // OE StartButtons
            AddInput(new Button("Start"));

            // Trigger Buttons
            AddInput(new Button("A"));
            AddInput(new Button("B"));
            AddInput(new Button("X"));
            AddInput(new Button("Y"));

            // Trigger Buttons
            AddInput(new Button("L"));
            AddInput(new Button("R"));
            AddInput(new Button("Z"));

            //  OE Add out analog axes
            AddAnalogInputs(new Axis("X",Axis::negative),
                            new Axis("X",Axis::positive));

            AddAnalogInputs(new Axis("Y",Axis::negative),
                            new Axis("Y",Axis::positive));

            AddAnalogInputs(new Axis("Cx",Axis::negative),
                            new Axis("Cx",Axis::positive));

            AddAnalogInputs(new Axis("Cy",Axis::negative),
                            new Axis("Cy",Axis::positive));

            //Maybe someday this can be added
            //	// Force Feedback
            //	FFCAPABILITIES ff_caps;
            //	if (SUCCEEDED(ForceFeedback::FFDeviceAdapter::Create(IOHIDDeviceGetService(m_device), &m_ff_device)) &&
            //		SUCCEEDED(FFDeviceGetForceFeedbackCapabilities(m_ff_device->m_device, &ff_caps)))
            //	{
            //		InitForceFeedback(m_ff_device, ff_caps.numFfAxes);
            //	}
        }

        Joystick::~Joystick()
        {
            if (m_ff_device)
                m_ff_device->Release();
        }

        std::string Joystick::GetName() const
        {
            return m_device_name;
        }

        std::string Joystick::GetSource() const
        {
            return "Input";
        }

        int Joystick::GetId() const
        {
            return m_index;
        }

        Joystick::Button::Button(std::string description)
        {
            m_name = std::string("Button ") + description;
            m_state=0;
        }

        std::string Joystick::Button::GetName() const
        {

            return  m_name;
        }

        ControlState Joystick::Button::GetState() const
        {
            return m_state;
        }

        void Joystick::Button::SetState(ControlState state)
        {
            m_state = state;
        }

        Joystick::Axis::Axis(std::string description, direction dir)
        :m_direction(dir)
        {
            m_name = std::string("Axis ") + description;
            m_name.append((m_direction == positive) ? "+" : "-");
        }

        ControlState Joystick::Axis::GetState() const
        {
            return m_value;
        }
        
        void Joystick::Axis::SetState(ControlState value)
        {
            m_value = value;
        }
        
        std::string Joystick::Axis::GetName() const
        {
            return m_name;
        }
        
        Joystick::Hat::Hat( direction dir): m_direction(dir)
        {
        }
        
        ControlState Joystick::Hat::GetState() const
        {
            return 0;
        }
        
        std::string Joystick::Hat::GetName() const
        {
            return m_name;
        }
        
        
    }
}
