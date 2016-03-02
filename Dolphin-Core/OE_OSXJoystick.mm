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
            Input* up = new Axis("OEGCAnalogUp",Axis::positive);
            Input* down = new Axis("OEGCAnalogDown",Axis::negative);
            Input* left = new Axis("OEGCAnalogLeft",Axis::negative);
            Input* right = new Axis("OEGCAnalogRight",Axis::positive);
            Input* cup = new Axis("OEGCAnalogCUp",Axis::positive);
            Input* cdown = new Axis("OEGCAnalogCDown",Axis::negative);
            Input* cleft = new Axis("OEGCAnalogCLeft",Axis::negative);
            Input* cright = new Axis("OEGCAnalogCRight",Axis::positive);

            // Buttons

            //  OE DPAD buttons
            AddInput(new Button("OEGCButtonUp"));
            AddInput(new Button("OEGCButtonDown"));
            AddInput(new Button("OEGCButtonLeft"));
            AddInput(new Button("OEGCButtonRight"));

            //  OE Add our analog axes as buttons
            AddInput(up);
            AddInput(down);
            AddInput(left);
            AddInput(right);
            AddInput(cup);
            AddInput(cdown);
            AddInput(cleft);
            AddInput(cright);

            // Trigger Buttons
            AddInput(new Button("OEGCButtonA"));
            AddInput(new Button("OEGCButtonB"));
            AddInput(new Button("OEGCButtonX"));
            AddInput(new Button("OEGCButtonY"));
            AddInput(new Button("OEGCButtonL"));
            AddInput(new Button("OEGCButtonR"));
            AddInput(new Button("OEGCButtonZ"));

            // OE StartButtons
            AddInput(new Button("OEGCButtonStart"));

            //OE Create Analog Surfaces for Axis buttons
            AddInput(new FullAnalogSurface(up, down));
            AddInput(new FullAnalogSurface(down, up));

            AddInput(new FullAnalogSurface(left, right));
            AddInput(new FullAnalogSurface(right, left));

            AddInput(new FullAnalogSurface(cup, cdown));
            AddInput(new FullAnalogSurface(cdown, cup));

            AddInput(new FullAnalogSurface(cleft, cright));
            AddInput(new FullAnalogSurface(cright, cleft));
           
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
            m_name = description;
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
            m_name = description;
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
