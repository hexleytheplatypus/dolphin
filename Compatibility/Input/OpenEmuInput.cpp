#include <algorithm>
#include <array>
#include <cassert>

#include "Common/Common.h"
#include "Common/CommonTypes.h"
#include "Common/IniFile.h"
#include "Core/ConfigManager.h"
#include "Core/HW/GCKeyboard.h"
#include "Core/HW/GCPad.h"
#include "Core/HW/GCPadEmu.h"
#include "Core/HW/Wiimote.h"
#include "Core/HW/WiimoteEmu/Extension/Classic.h"
#include "Core/HW/WiimoteEmu/Extension/Nunchuk.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "Core/HW/WiimoteReal/WiimoteReal.h"
#include "Core/Host.h"
#include "OpenEmuInput.h"

#include "InputCommon/ControlReference/ControlReference.h"
#include "InputCommon/ControlReference/ExpressionParser.h"
#include "InputCommon/ControllerEmu/Control/Control.h"
#include "InputCommon/ControllerEmu/ControlGroup/Attachments.h"
#include "InputCommon/ControllerEmu/Setting/NumericSetting.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/GCPadStatus.h"
#include "InputCommon/InputConfig.h"

#include "DolHost.h"

extern WindowSystemInfo wsi;

static Input::openemu_input_state_t input_cb;
static Input::openemu_input_poll_t poll_cb;
static const std::string source = "OpenEmu";
static unsigned input_types[4];


    static std::string GetDeviceName(unsigned device)
    {
        switch (device)
        {
            case OEDolDevJoy:
                return "Joypad";
            case OEDolDevAnalog:
                return "Analog";
            case OEDolDevPointer:
                return "Pointer";
        }
        return "Unknown";
    }

    static std::string GetQualifiedName(unsigned port, unsigned device)
    {
        return ciface::Core::DeviceQualifier(source, port, GetDeviceName(device)).ToString();
    }


    class OEDevice : public ciface::Core::Device
    {
    private:
        class Button : public ciface::Core::Device::Input
        {
        public:
            Button(unsigned port, unsigned device, unsigned index, unsigned id, const char* name)
            : m_port(port), m_device(device), m_index(index), m_id(id), m_name(name)
            {
            }
            std::string GetName() const override { return m_name; }
            ControlState GetState() const override {
                return input_cb(m_port, m_device, m_index, m_id);
            }

        private:
            const unsigned m_port;
            const unsigned m_device;
            const unsigned m_index;
            const unsigned m_id;
            const char* m_name;
        };

        class Axis : public ciface::Core::Device::Input
        {
        public:
            Axis(unsigned port, unsigned device, unsigned index, unsigned id, s16 range, const char* name)
            : m_port(port), m_device(device), m_index(index), m_id(id), m_range(range), m_name(name)
            {
            }
            std::string GetName() const override { return m_name; }
            ControlState GetState() const override
            {
                return std::max(0.0, input_cb(m_port, m_device, m_index, m_id) / m_range);
            }

        private:
            const unsigned m_port;
            const unsigned m_device;
            const unsigned m_index;
            const unsigned m_id;
            const ControlState m_range;
            const char* m_name;
        };

        class Motor : public ciface::Core::Device::Output
        {
        public:
            Motor(u8 port) : m_port(port) {}
            std::string GetName() const override { return "Rumble"; }
            void SetState(ControlState state) override
            {
                uint16_t str = std::min(std::max(0.0, state), 1.0) * 0xFFFF;

            }

        private:
            const u8 m_port;
        };
        
        void AddButton(unsigned id, const char* name, unsigned index = 0)
        {
            AddInput(new Button(m_port, m_device, index, id, name));
        }
        void AddAxis(unsigned id, s16 range, const char* name, unsigned index = 0)
        {
            AddInput(new Axis(m_port, m_device, index, id, range, name));
        }
        void AddMotor() { AddOutput(new Motor(m_port)); }

    public:
        OEDevice(unsigned device, unsigned port);
        void UpdateInput() override
        {
            poll_cb();
        }
        std::string GetName() const override { return GetDeviceName(m_device); }
        std::string GetSource() const override { return source; }
        unsigned GetPort() const { return m_port; }

    private:
        unsigned m_device;
        unsigned m_port;
    };

    OEDevice::OEDevice(unsigned device, unsigned p) : m_device(device), m_port(p)
    {
        switch (device)
        {
            case OEDolDevJoy:
                AddButton(OEGCButtonB, "B");
                AddButton(OEGCButtonY, "Y");
                AddButton(OEGCButtonStart, "Start");
                AddButton(OEGCButtonUp, "Up");
                AddButton(OEGCButtonDown, "Down");
                AddButton(OEGCButtonLeft, "Left");
                AddButton(OEGCButtonRight, "Right");
                AddButton(OEGCButtonA, "A");
                AddButton(OEGCButtonX, "X");
                AddButton(OEGCDigitalL, "L");
                AddButton(OEGCDigitalR, "R");
                AddButton(OEGCButtonZ, "Z");
                return;
                
            case OEDolDevAnalog:
                AddAxis(OEGCAnalogLeft, -0x8000, "X0-");
                AddAxis(OEGCAnalogRight, 0x7FFF, "X0+");
                AddAxis(OEGCAnalogUp, -0x8000, "Y0-");
                AddAxis(OEGCAnalogDown, 0x7FFF, "Y0+");
                AddAxis(OEGCAnalogCLeft, -0x8000, "X1-");
                AddAxis(OEGCAnalogCRight, 0x7FFF, "X1+");
                AddAxis(OEGCAnalogCUp, -0x8000, "Y1-");
                AddAxis(OEGCAnalogCDown, 0x7FFF, "Y1+");
                AddAxis(OEGCButtonL, 0x7FFF, "Trigger0+");
                AddAxis(OEGCButtonR, 0x7FFF, "Trigger1+");
                return;
        }
    }

    static void AddDevicesForPort(unsigned port)
    {
        g_controller_interface.AddDevice(std::make_shared<OEDevice>(OEDolDevJoy, port));
        g_controller_interface.AddDevice(std::make_shared<OEDevice>(OEDolDevAnalog, port));
        g_controller_interface.AddDevice(std::make_shared<OEDevice>(OEDolDevPointer, port));
    }

    static void RemoveDevicesForPort(unsigned port)
    {
        g_controller_interface.RemoveDevice([&port](const auto& device) {
            return device->GetSource() == source
            && (device->GetName() == GetDeviceName(OEDolDevAnalog)
                || device->GetName() == GetDeviceName(OEDolDevJoy)
                || device->GetName() == GetDeviceName(OEDolDevPointer))
            && dynamic_cast<const OEDevice *>(device)->GetPort() == port;
        });
    }

    void Input::Openemu_Input_Init()
    {
        g_controller_interface.Initialize(DolHost::GetInstance()->GetWSI());

        g_controller_interface.AddDevice(std::make_shared<OEDevice>(OEDolDevKeyboard, 0));

        Pad::Initialize();
        Keyboard::Initialize();
        
    }

    void Shutdown()
    {
        Wiimote::Shutdown();
        Keyboard::Shutdown();
        Pad::Shutdown();
        g_controller_interface.Shutdown();
    }

    void OpenEmu_Input_Update()
    {
    }

    void Input::ResetControllers()
    {
        for (int port = 0; port < 4; port++)
            Input::openemu_set_controller_port_device(port, input_types[port]);
    }

void Input::openemu_set_input_state(Input::openemu_input_state_t cb)
    {
        input_cb = cb;
    }

void Input::openemu_set_input_poll(Input::openemu_input_poll_t cb)
    {
        poll_cb = cb;
    }

    void Input::openemu_set_controller_port_device(unsigned port, unsigned device)
    {
        if (port > 4)
            return;

        input_types[port] = device;

        std::string devJoypad = GetQualifiedName(port, OEDolDevJoy);
        std::string devAnalog = GetQualifiedName(port, OEDolDevAnalog);
        std::string devPointer = GetQualifiedName(port, OEDolDevPointer);

        RemoveDevicesForPort(port);
        if ((device & 0xff) != OEDolDevNone)
            AddDevicesForPort(port);


            GCPad* gcPad = (GCPad*)Pad::GetConfig()->GetController(port);
            // load an empty inifile section, clears everything
            IniFile::Section sec;
            gcPad->LoadConfig(&sec);
            gcPad->SetDefaultDevice(devJoypad);

            ControllerEmu::ControlGroup* gcButtons = gcPad->GetGroup(PadGroup::Buttons);
            ControllerEmu::ControlGroup* gcMainStick = gcPad->GetGroup(PadGroup::MainStick);
            ControllerEmu::ControlGroup* gcCStick = gcPad->GetGroup(PadGroup::CStick);
            ControllerEmu::ControlGroup* gcDPad = gcPad->GetGroup(PadGroup::DPad);
            ControllerEmu::ControlGroup* gcTriggers = gcPad->GetGroup(PadGroup::Triggers);
            ControllerEmu::ControlGroup* gcRumble = gcPad->GetGroup(PadGroup::Rumble);
//#if 0
//            ControllerEmu::ControlGroup* gcMic = gcPad->GetGroup(PadGroup::Mic);
//            ControllerEmu::ControlGroup* gcOptions = gcPad->GetGroup(PadGroup::Options);
//#endif

            gcButtons->SetControlExpression(0, "A");                              // A
            gcButtons->SetControlExpression(1, "B");                              // B
            gcButtons->SetControlExpression(2, "X");                              // X
            gcButtons->SetControlExpression(3, "Y");                              // Y
            gcButtons->SetControlExpression(4, "Z");                              // Z
            gcButtons->SetControlExpression(5, "Start");                          // Start
            gcMainStick->SetControlExpression(0, "`" + devAnalog + ":Y0-`");      // Up
            gcMainStick->SetControlExpression(1, "`" + devAnalog + ":Y0+`");      // Down
            gcMainStick->SetControlExpression(2, "`" + devAnalog + ":X0-`");      // Left
            gcMainStick->SetControlExpression(3, "`" + devAnalog + ":X0+`");      // Right
            gcCStick->SetControlExpression(0, "`" + devAnalog + ":Y1-`");         // Up
            gcCStick->SetControlExpression(1, "`" + devAnalog + ":Y1+`");         // Down
            gcCStick->SetControlExpression(2, "`" + devAnalog + ":X1-`");         // Left
            gcCStick->SetControlExpression(3, "`" + devAnalog + ":X1+`");         // Right
            gcDPad->SetControlExpression(0, "Up");                                // Up
            gcDPad->SetControlExpression(1, "Down");                              // Down
            gcDPad->SetControlExpression(2, "Left");                              // Left
            gcDPad->SetControlExpression(3, "Right");                             // Right
            gcTriggers->SetControlExpression(0, "L");                            // L-trigger
            gcTriggers->SetControlExpression(1, "R");                            // R-trigger
            gcTriggers->SetControlExpression(2, "`" + devAnalog + ":Trigger0+`"); // L-trigger Analog
            gcTriggers->SetControlExpression(3, "`" + devAnalog + ":Trigger1+`"); // R-trigger Analog
            gcRumble->SetControlExpression(0, "Rumble");

            gcPad->UpdateReferences(g_controller_interface);
            Pad::GetConfig()->SaveConfig();

//        all_descs.push_back({ 0 });
    }
