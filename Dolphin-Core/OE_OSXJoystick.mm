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
            if(SConfig::GetInstance().bWii){

                //  OE Wiimote DPAD buttons
                AddInput(new Button("OEWiiMoteButtonUp"));
                AddInput(new Button("OEWiiMoteButtonDown"));
                AddInput(new Button("OEWiiMoteButtonLeft"));
                AddInput(new Button("OEWiiMoteButtonRight"));

                // OE Wiimote Buttons
                AddInput(new Button("OEWiiMoteButtonA"));
                AddInput(new Button("OEWiiMoteButtonB"));
                AddInput(new Button("OEWiiMoteButton1"));
                AddInput(new Button("OEWiiMoteButton2"));
                AddInput(new Button("OEWiiMoteButtonPlus"));
                AddInput(new Button("OEWiiMoteButtonMinus"));
                AddInput(new Button("OEWiiMoteButtonHome"));

                //OE Nunchuk Buttons
                AddInput(new Button("OEWiiNunchukButtonC"));
                AddInput(new Button("OEWiiNunchukButtonZ"));

                //OE Nunchuk Axes
                AddAnalogInputs(new Axis("OEWiiNunchukAnalogDown",Axis::negative),
                                new Axis("OEWiiNunchukAnalogUp",Axis::positive));
                AddAnalogInputs(new Axis("OEWiiNunchukAnalogLeft",Axis::negative),
                                new Axis("OEWiiNunchukAnalogRight",Axis::positive));

                //  OE Classic DPAD buttons
                AddInput(new Button("OEWiiClassicButtonUp"));
                AddInput(new Button("OEWiiClassicButtonDown"));
                AddInput(new Button("OEWiiClassicButtonLeft"));
                AddInput(new Button("OEWiiClassicButtonRight"));

                //  OE Add our Classic analog axes as buttons
                AddAnalogInputs(new Axis("OEWiiClassicAnalogLDown",Axis::negative),
                                new Axis("OEWiiClassicAnalogLUp",Axis::positive));
                AddAnalogInputs(new Axis("OEWiiClassicAnalogLLeft",Axis::negative),
                                new Axis("OEWiiClassicAnalogLRight",Axis::positive));
                AddAnalogInputs(new Axis("OEWiiClassicAnalogRDown",Axis::negative),
                                new Axis("OEWiiClassicAnalogRUp",Axis::positive));
                AddAnalogInputs(new Axis("OEWiiClassicAnalogRLeft",Axis::negative),
                                new Axis("OEWiiClassicAnalogRRight",Axis::positive));

                // Wii Classic Buttons
                AddInput(new Button("OEWiiClassicButtonA"));
                AddInput(new Button("OEWiiClassicButtonB"));
                AddInput(new Button("OEWiiClassicButtonX"));
                AddInput(new Button("OEWiiClassicButtonY"));
                AddInput(new Button("OEWiiClassicButtonL"));
                AddInput(new Button("OEWiiClassicButtonR"));
                AddInput(new Button("OEWiiClassicButtonZl"));
                AddInput(new Button("OEWiiClassicButtonZr"));
                AddInput(new Button("OEWiiClassicButtonStart")); // + button
                AddInput(new Button("OEWiiClassicButtonSelect")); // - button
                AddInput(new Button("OEWiiClassicButtonHome"));
            }
            else
            {
                // Buttons

                //  OE GC DPAD buttons
                AddInput(new Button("OEGCButtonUp"));
                AddInput(new Button("OEGCButtonDown"));
                AddInput(new Button("OEGCButtonLeft"));
                AddInput(new Button("OEGCButtonRight"));

                //  OE Add our GC analog axes as buttons
                AddAnalogInputs(new Axis("OEGCAnalogDown",Axis::negative),
                                new Axis("OEGCAnalogUp",Axis::positive));
                AddAnalogInputs(new Axis("OEGCAnalogLeft",Axis::negative),
                                new Axis("OEGCAnalogRight",Axis::positive));
                AddAnalogInputs(new Axis("OEGCAnalogCDown",Axis::negative),
                                new Axis("OEGCAnalogCUp",Axis::positive));
                AddAnalogInputs(new Axis("OEGCAnalogCLeft",Axis::negative),
                                new Axis("OEGCAnalogCRight",Axis::positive));
                // GC Buttons
                AddInput(new Button("OEGCButtonA"));
                AddInput(new Button("OEGCButtonB"));
                AddInput(new Button("OEGCButtonX"));
                AddInput(new Button("OEGCButtonY"));
                AddInput(new Button("OEGCButtonL"));
                AddInput(new Button("OEGCButtonR"));
                AddInput(new Button("OEGCButtonZ"));

                // OE GC StartButton
                AddInput(new Button("OEGCButtonStart"));

                //Maybe someday this can be added
                //	// Force Feedback
                //	FFCAPABILITIES ff_caps;
                //	if (SUCCEEDED(ForceFeedback::FFDeviceAdapter::Create(IOHIDDeviceGetService(m_device), &m_ff_device)) &&
                //		SUCCEEDED(FFDeviceGetForceFeedbackCapabilities(m_ff_device->m_device, &ff_caps)))
                //	{
                //		InitForceFeedback(m_ff_device, ff_caps.numFfAxes);
                //	}
            }
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
            m_neutral = 1/2;
            m_scale = 1;
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
