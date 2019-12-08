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

namespace Input
{
    static struct openemu_input_descriptor descGC[] = {
        {0, OEDolDevJoy, 0, OEGCButtonLeft , "Left"},
        {0, OEDolDevJoy, 0, OEGCButtonUp, "Up"},
        {0, OEDolDevJoy, 0, OEGCButtonDown, "Down"},
        {0, OEDolDevJoy, 0, OEGCButtonRight, "Right"},
        {0, OEDolDevJoy, 0, OEGCButtonB, "B"},
        {0, OEDolDevJoy, 0, OEGCButtonA, "A"},
        {0, OEDolDevJoy, 0, OEGCButtonX, "X"},
        {0, OEDolDevJoy, 0, OEGCButtonY, "Y"},
        {0, OEDolDevJoy, 0, OEGCButtonL, "L"},
        {0, OEDolDevJoy, 0, OEGCButtonR, "R"},
        {0, OEDolDevJoy, 0, OEGCButtonZ, "Z"},
        {0, OEDolDevJoy, 0, OEGCButtonStart, "Start"},
        {0, OEDolDevAnalog, OEGCAnalog, OEGCAnalogX  ,
            "Control Stick X"},
        {0, OEDolDevAnalog, OEGCAnalog, OEGCAnalogY,
            "Control Stick Y"},
        {0, OEDolDevAnalog, OEGCAnalogC, OEGCAnalogCX,
            "C Buttons X"},
        {0, OEDolDevAnalog, OEGCAnalogC, OEGCAnalogCY,
            "C Buttons Y"},
        {0},
    };
//
//    static struct openemu_input_descriptor descWiimoteCC[] = {
//        {0, OEDolDevJoy, 0, OEDolJoypadLeft, "Left"},
//        {0, OEDolDevJoy, 0, OEDolJoypadUp, "Up"},
//        {0, OEDolDevJoy, 0, OEDolJoypadDown, "Down"},
//        {0, OEDolDevJoy, 0, OEDolJoypadRight, "Right"},
//        {0, OEDolDevJoy, 0, OEDolJoypadB, "B"},
//        {0, OEDolDevJoy, 0, OEDolJoypadA, "A"},
//        {0, OEDolDevJoy, 0, OEDolJoypadX, "X"},
//        {0, OEDolDevJoy, 0, OEDolJoypadY, "Y"},
//        {0, OEDolDevJoy, 0, OEDolJoypadL2, "L"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR2, "R"},
//        {0, OEDolDevJoy, 0, OEDolJoypadL, "ZL"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR, "ZR"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR3, "Home"},
//        {0, OEDolDevJoy, 0, OEDolJoypadStart, "+"},
//        {0, OEDolDevJoy, 0, OEDolJoypadSelect, "-"},
//        {0, OEDolDevAnalog, OEGCAnalog, OEDolAnalogX,
//            "Left Stick X"},
//        {0, OEDolDevAnalog, OEGCAnalog, OEDolAnalogY,
//            "Left Stick Y"},
//        {0, OEDolDevAnalog, OEGCAnalogC, OEDolAnalogX,
//            "Right Stick X"},
//        {0, OEDolDevAnalog, OEGCAnalogC, OEDolAnalogY,
//            "Right Stick Y"},
//        {0},
//    };
//
//    static struct openemu_input_descriptor descWiimote[] = {
//        {0, OEDolDevJoy, 0, OEDolJoypadLeft, "Left"},
//        {0, OEDolDevJoy, 0, OEDolJoypadUp, "Up"},
//        {0, OEDolDevJoy, 0, OEDolJoypadDown, "Down"},
//        {0, OEDolDevJoy, 0, OEDolJoypadRight, "Right"},
//        {0, OEDolDevJoy, 0, OEDolJoypadB, "B"},
//        {0, OEDolDevJoy, 0, OEDolJoypadA, "A"},
//        {0, OEDolDevJoy, 0, OEDolJoypadX, "1"},
//        {0, OEDolDevJoy, 0, OEDolJoypadY, "2"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR2, "Shake Wiimote"},
//        {0, OEDolDevJoy, 0, OEDolJoypadStart, "+"},
//        {0, OEDolDevJoy, 0, OEDolJoypadSelect, "-"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR3, "Home"},
//        {0, OEDolDevAnalog, OEGCAnalog, OEDolAnalogX,
//            "Tilt Left/Right"},
//        {0, OEDolDevAnalog, OEGCAnalog, OEDolAnalogY,
//            "Tilt Forward/Backward"},
//        {0},
//    };
//
//    static struct openemu_input_descriptor descWiimoteSideways[] = {
//        {0, OEDolDevJoy, 0, OEDolJoypadLeft, "Up"},
//        {0, OEDolDevJoy, 0, OEDolJoypadUp, "Right"},
//        {0, OEDolDevJoy, 0, OEDolJoypadDown, "Left"},
//        {0, OEDolDevJoy, 0, OEDolJoypadRight, "Down"},
//        {0, OEDolDevJoy, 0, OEDolJoypadB, "1"},
//        {0, OEDolDevJoy, 0, OEDolJoypadA, "2"},
//        {0, OEDolDevJoy, 0, OEDolJoypadX, "A"},
//        {0, OEDolDevJoy, 0, OEDolJoypadY, "B"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR2, "Shake Wiimote"},
//        {0, OEDolDevJoy, 0, OEDolJoypadStart, "+"},
//        {0, OEDolDevJoy, 0, OEDolJoypadSelect, "-"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR3, "Home"},
//        {0, OEDolDevAnalog, OEGCAnalog, OEDolAnalogX,
//            "Tilt Left/Right"},
//        {0, OEDolDevAnalog, OEGCAnalog, OEDolAnalogY,
//            "Tilt Forward/Backward"},
//        {0},
//    };
//
//    static struct openemu_input_descriptor descWiimoteNunchuk[] = {
//        {0, OEDolDevJoy, 0, OEDolJoypadLeft, "Left"},
//        {0, OEDolDevJoy, 0, OEDolJoypadUp, "Up"},
//        {0, OEDolDevJoy, 0, OEDolJoypadDown, "Down"},
//        {0, OEDolDevJoy, 0, OEDolJoypadRight, "Right"},
//        {0, OEDolDevJoy, 0, OEDolJoypadB, "B"},
//        {0, OEDolDevJoy, 0, OEDolJoypadA, "A"},
//        {0, OEDolDevJoy, 0, OEDolJoypadL, "C"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR, "Z"},
//        {0, OEDolDevJoy, 0, OEDolJoypadSelect, "-"},
//        {0, OEDolDevJoy, 0, OEDolJoypadStart, "+"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR2, "Shake Wiimote"},
//        {0, OEDolDevJoy, 0, OEDolJoypadL2, "Shake Nunchuk"},
//        {0, OEDolDevJoy, 0, OEDolJoypadX, "1"},
//        {0, OEDolDevJoy, 0, OEDolJoypadY, "2"},
//        {0, OEDolDevJoy, 0, OEDolJoypadR3, "Home"},
//        {0, OEDolDevAnalog, OEGCAnalog, OEDolAnalogX,
//            "Nunchuk Stick X"},
//        {0, OEDolDevAnalog, OEGCAnalog, OEDolAnalogY,
//            "Nunchuk Stick Y"},
//        {0, OEDolDevAnalog, OEGCAnalogC, OEDolAnalogX,
//            "Tilt Left/Right"},
//        {0, OEDolDevAnalog, OEGCAnalogC, OEDolAnalogY,
//            "Tilt Forward/Backward"},
//        {0},
//    };

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

                //rumble.set_rumble_state(m_port, OPENEMU_RUMBLE_WEAK, str);
                //rumble.set_rumble_state(m_port, OPENEMU_RUMBLE_STRONG, str);
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
                AddButton(OEGCButtonL, "L");
                AddButton(OEGCButtonR, "R");
                AddButton(OEGCButtonZ, "R2");
                return;
                
            case OEDolDevAnalog:
                AddAxis(OEGCAnalogLeft, -0x8000, "X0-", OEGCAnalog);
                AddAxis(OEGCAnalogRight, 0x7FFF, "X0+", OEGCAnalog);
                AddAxis(OEGCAnalogUp, -0x8000, "Y0-", OEGCAnalog);
                AddAxis(OEGCAnalogDown, 0x7FFF, "Y0+", OEGCAnalog);
                AddAxis(OEGCAnalogCLeft, -0x8000, "X1-", OEGCAnalogC);
                AddAxis(OEGCAnalogCRight, 0x7FFF, "X1+", OEGCAnalogC);
                AddAxis(OEGCAnalogCUp, -0x8000, "Y1-", OEGCAnalogC);
                AddAxis(OEGCAnalogCDown, 0x7FFF, "Y1+", OEGCAnalogC);
                AddAxis(OEDolJoypadL2, 0x7FFF, "Trigger0+", OEGCAnalogTrigger);
                AddAxis(OEDolJoypadR2, 0x7FFF, "Trigger1+", OEGCAnalogTrigger);
                return;

//            case OEDolDevPointer:
//                AddButton(OPENEMU_DEVICE_ID_POINTER_PRESSED, "Pressed0", 0);
//                AddAxis(OPENEMU_DEVICE_ID_POINTER_X, -0x8000, "X0-", 0);
//                AddAxis(OPENEMU_DEVICE_ID_POINTER_X, 0x7FFF, "X0+", 0);
//                AddAxis(OPENEMU_DEVICE_ID_POINTER_Y, -0x8000, "Y0-", 0);
//                AddAxis(OPENEMU_DEVICE_ID_POINTER_Y, 0x7FFF, "Y0+", 0);
//                return;
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

    void Openemu_Input_Init()
    {
        g_controller_interface.Initialize(DolHost::GetInstance()->GetWSI());

        g_controller_interface.AddDevice(std::make_shared<OEDevice>(OEDolDevKeyboard, 0));

        Pad::Initialize();
        Keyboard::Initialize();

//        if (SConfig::GetInstance().bWii && !SConfig::GetInstance().m_bt_passthrough_enabled)
//        {
//            Wiimote::Initialize(Wiimote::InitializeMode::DO_NOT_WAIT_FOR_WIIMOTES);
//
//            static const struct openemu_controller_description wiimote_desc[] = {
//                {"WiiMote", OEWiimote},
//                {"WiiMote (sideways)", OEWiimoteSW},
//                {"WiiMote + Nunchuk",  OEWiimoteNC},
//                {"WiiMote + Classic Controller",  OEWiimoteCC},
//                {"Real WiiMote",  OEWiiMoteReal},
//                {0},
//            };
//
//            static const struct openemu_controller_info ports[] = {
//                {wiimote_desc, sizeof(wiimote_desc) / sizeof(*wiimote_desc)},
//                {wiimote_desc, sizeof(wiimote_desc) / sizeof(*wiimote_desc)},
//                {wiimote_desc, sizeof(wiimote_desc) / sizeof(*wiimote_desc)},
//                {wiimote_desc, sizeof(wiimote_desc) / sizeof(*wiimote_desc)},
//                {0},
//            };
//        }
//        else
//        {
            static const struct openemu_controller_description gcpad_desc[] = {
                {"GameCube Controller", OEDolDevJoy},
            };

            static const struct openemu_controller_info ports[] = {
                {gcpad_desc, sizeof(gcpad_desc) / sizeof(*gcpad_desc)},
                {gcpad_desc, sizeof(gcpad_desc) / sizeof(*gcpad_desc)},
                {gcpad_desc, sizeof(gcpad_desc) / sizeof(*gcpad_desc)},
                {gcpad_desc, sizeof(gcpad_desc) / sizeof(*gcpad_desc)},
                {0},
            };
//        }
    }

    void Shutdown()
    {
        Wiimote::Shutdown();
        Keyboard::Shutdown();
        Pad::Shutdown();
        g_controller_interface.Shutdown();

        //rumble.set_rumble_state = nullptr;
    }

    void OpenEmu_Input_Update()
    {
    }

    void ResetControllers()
    {
        for (int port = 0; port < 4; port++)
            openemu_set_controller_port_device(port, input_types[port]);
    }

    void openemu_set_input_state(openemu_input_state_t cb)
    {
        input_cb = cb;
    }

    void openemu_set_input_poll(openemu_input_poll_t cb)
    {
        poll_cb = cb;
    }

    void openemu_set_controller_port_device(unsigned port, unsigned device)
    {
        if (port > 3)
            return;

        input_types[port] = device;

        std::string devJoypad = GetQualifiedName(port, OEDolDevJoy);
        std::string devAnalog = GetQualifiedName(port, OEDolDevAnalog);
        std::string devPointer = GetQualifiedName(port, OEDolDevPointer);

        RemoveDevicesForPort(port);
        if ((device & 0xff) != OEDolDevNone)
            AddDevicesForPort(port);

//        if (!SConfig::GetInstance().bWii)
//        {
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
            gcButtons->SetControlExpression(4, "R");                              // Z
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
            gcTriggers->SetControlExpression(0, "L2");                            // L-trigger
            gcTriggers->SetControlExpression(1, "R2");                            // R-trigger
            gcTriggers->SetControlExpression(2, "`" + devAnalog + ":Trigger0+`"); // L-trigger Analog
            gcTriggers->SetControlExpression(3, "`" + devAnalog + ":Trigger1+`"); // R-trigger Analog
            gcRumble->SetControlExpression(0, "Rumble");

            gcPad->UpdateReferences(g_controller_interface);
            Pad::GetConfig()->SaveConfig();
//        }
//        else if (!SConfig::GetInstance().m_bt_passthrough_enabled && (device & 0xff) != OEDolDevNone)
//        {
//            WiimoteEmu::Wiimote* wm = (WiimoteEmu::Wiimote*)Wiimote::GetConfig()->GetController(port);
//            // load an empty inifile section, clears everything
//            IniFile::Section sec;
//            wm->LoadConfig(&sec);
//            wm->SetDefaultDevice(devJoypad);
//
//            using namespace WiimoteEmu;
//            if (device == OEWiimoteCC)
//            {
//                ControllerEmu::ControlGroup* ccButtons = wm->GetClassicGroup(ClassicGroup::Buttons);
//                ControllerEmu::ControlGroup* ccTriggers = wm->GetClassicGroup(ClassicGroup::Triggers);
//                ControllerEmu::ControlGroup* ccDpad = wm->GetClassicGroup(ClassicGroup::DPad);
//                ControllerEmu::ControlGroup* ccLeftStick = wm->GetClassicGroup(ClassicGroup::LeftStick);
//                ControllerEmu::ControlGroup* ccRightStick = wm->GetClassicGroup(ClassicGroup::RightStick);
//
//                ccButtons->SetControlExpression(0, "A");                              // A
//                ccButtons->SetControlExpression(1, "B");                              // B
//                ccButtons->SetControlExpression(2, "X");                              // X
//                ccButtons->SetControlExpression(3, "Y");                              // Y
//                ccButtons->SetControlExpression(4, "L");                              // ZL
//                ccButtons->SetControlExpression(5, "R");                              // ZR
//                ccButtons->SetControlExpression(6, "Select");                         // -
//                ccButtons->SetControlExpression(7, "Start");                          // +
//                ccButtons->SetControlExpression(8, "R3");                             // Home
//                ccTriggers->SetControlExpression(0, "L2");                            // L-trigger
//                ccTriggers->SetControlExpression(1, "R2");                            // R-trigger
//                ccTriggers->SetControlExpression(2, "`" + devAnalog + ":Trigger0+`"); // L-trigger Analog
//                ccTriggers->SetControlExpression(3, "`" + devAnalog + ":Trigger1+`"); // R-trigger Analog
//                ccDpad->SetControlExpression(0, "Up");                                // Up
//                ccDpad->SetControlExpression(1, "Down");                              // Down
//                ccDpad->SetControlExpression(2, "Left");                              // Left
//                ccDpad->SetControlExpression(3, "Right");                             // Right
//                ccLeftStick->SetControlExpression(0, "`" + devAnalog + ":Y0-`");      // Up
//                ccLeftStick->SetControlExpression(1, "`" + devAnalog + ":Y0+`");      // Down
//                ccLeftStick->SetControlExpression(2, "`" + devAnalog + ":X0-`");      // Left
//                ccLeftStick->SetControlExpression(3, "`" + devAnalog + ":X0+`");      // Right
//
//                //            if (libOE::Options::irMode != 1 && libOE::Options::irMode != 2)
//                //            {
//                //                ccRightStick->SetControlExpression(0, "`" + devAnalog + ":Y1-`");     // Up
//                //                ccRightStick->SetControlExpression(1, "`" + devAnalog + ":Y1+`");     // Down
//                //                ccRightStick->SetControlExpression(2, "`" + devAnalog + ":X1-`");     // Left
//                //                ccRightStick->SetControlExpression(3, "`" + devAnalog + ":X1+`");     // Right
//                //            }
//            }
//            else
//            {
//                ControllerEmu::ControlGroup* wmButtons = wm->GetWiimoteGroup(WiimoteGroup::Buttons);
//                ControllerEmu::ControlGroup* wmDPad = wm->GetWiimoteGroup(WiimoteGroup::DPad);
//                //ControllerEmu::ControlGroup* wmIR = wm->GetWiimoteGroup(WiimoteGroup::IR);
//                ControllerEmu::ControlGroup* wmShake = wm->GetWiimoteGroup(WiimoteGroup::Shake);
//                ControllerEmu::ControlGroup* wmTilt = wm->GetWiimoteGroup(WiimoteGroup::Tilt);
//#if 0
//                ControllerEmu::ControlGroup* wmSwing = wm->GetWiimoteGroup(WiimoteGroup::Swing);
//                ControllerEmu::ControlGroup* wmHotkeys = wm->GetWiimoteGroup(WiimoteGroup::Hotkeys);
//#endif
//
//                wmButtons->SetControlExpression(0, "A | `" + devPointer + ":Pressed0`");  // A
//                wmButtons->SetControlExpression(1, "B");                                  // B
//                //            wmIR->numeric_settings[0]->SetValue(libOE::Options::irCenter / 100.0); // IR Center
//                //            wmIR->numeric_settings[1]->SetValue(libOE::Options::irWidth / 100.0);  // IR Width
//                //            wmIR->numeric_settings[2]->SetValue(libOE::Options::irHeight / 100.0); // IR Height
//
//                if (device == OEWiimoteNC)
//                {
//                    ControllerEmu::ControlGroup* ncButtons = wm->GetNunchukGroup(NunchukGroup::Buttons);
//                    ControllerEmu::ControlGroup* ncStick = wm->GetNunchukGroup(NunchukGroup::Stick);
//                    ControllerEmu::ControlGroup* ncShake = wm->GetNunchukGroup(NunchukGroup::Shake);
//#if 0
//                    ControllerEmu::ControlGroup* ncTilt = wm->GetNunchukGroup(NunchukGroup::Tilt);
//                    ControllerEmu::ControlGroup* ncSwing = wm->GetNunchukGroup(NunchukGroup::Swing);
//#endif
//
//                    ncButtons->SetControlExpression(0, "X");                      // C
//                    ncButtons->SetControlExpression(1, "Y");                      // Z
//                    ncStick->SetControlExpression(0, "`" + devAnalog + ":Y0-`");  // Up
//                    ncStick->SetControlExpression(1, "`" + devAnalog + ":Y0+`");  // Down
//                    ncStick->SetControlExpression(2, "`" + devAnalog + ":X0-`");  // Left
//                    ncStick->SetControlExpression(3, "`" + devAnalog + ":X0+`");  // Right
//                    ncShake->SetControlExpression(0, "L2");                       // X
//                    ncShake->SetControlExpression(1, "L2");                       // Y
//                    ncShake->SetControlExpression(2, "L2");                       // Z
//
//                    wmButtons->SetControlExpression(2, "Start");   // 1
//                    wmButtons->SetControlExpression(3, "Select");  // 2
//                    wmButtons->SetControlExpression(4, "L");       // -
//                    wmButtons->SetControlExpression(5, "R");       // +
//
//                    //                if (libOE::Options::irMode != 1 && libOE::Options::irMode != 2)
//                    //                {
//                    //                    wmTilt->SetControlExpression(0, "`" + devAnalog + ":Y1-`");  // Forward
//                    //                    wmTilt->SetControlExpression(1, "`" + devAnalog + ":Y1+`");  // Backward
//                    //                    wmTilt->SetControlExpression(2, "`" + devAnalog + ":X1-`");  // Left
//                    //                    wmTilt->SetControlExpression(3, "`" + devAnalog + ":X1+`");  // Right
//                    //                }
//                }
//                else
//                {
//                    wmButtons->SetControlExpression(2, "X");       // 1
//                    wmButtons->SetControlExpression(3, "Y");       // 2
//                    wmButtons->SetControlExpression(4, "Select");  // -
//                    wmButtons->SetControlExpression(5, "Start");   // +
//                    wmTilt->SetControlExpression(0, "`" + devAnalog + ":Y0-`");  // Forward
//                    wmTilt->SetControlExpression(1, "`" + devAnalog + ":Y0+`");  // Backward
//                    wmTilt->SetControlExpression(2, "`" + devAnalog + ":X0-`");  // Left
//                    wmTilt->SetControlExpression(3, "`" + devAnalog + ":X0+`");  // Right
//                }
//
//                wmButtons->SetControlExpression(6, "R3");                   // Home
//                wmDPad->SetControlExpression(0, "Up");                      // Up
//                wmDPad->SetControlExpression(1, "Down");                    // Down
//                wmDPad->SetControlExpression(2, "Left");                    // Left
//                wmDPad->SetControlExpression(3, "Right");                   // Right
//
//                //            if (libOE::Options::irMode == 1 || libOE::Options::irMode == 2)
//                //            {
//                //                // Set right stick to control the IR
//                //                wmIR->SetControlExpression(0, "`" + devAnalog + ":Y1-`");     // Up
//                //                wmIR->SetControlExpression(1, "`" + devAnalog + ":Y1+`");     // Down
//                //                wmIR->SetControlExpression(2, "`" + devAnalog + ":X1-`");     // Left
//                //                wmIR->SetControlExpression(3, "`" + devAnalog + ":X1+`");     // Right
//                //                wmIR->boolean_settings[0]->SetValue(libOE::Options::irMode == 1); // Relative input
//                //                wmIR->boolean_settings[1]->SetValue(true);                    // Auto hide
//                //            }
//                //            else
//                {
//                    // Mouse controls IR
//                    //                wmIR->SetControlExpression(0, "`" + devPointer + ":Y0-`");  // Up
//                    //                wmIR->SetControlExpression(1, "`" + devPointer + ":Y0+`");  // Down
//                    //                wmIR->SetControlExpression(2, "`" + devPointer + ":X0-`");  // Left
//                    //                wmIR->SetControlExpression(3, "`" + devPointer + ":X0+`");  // Right
//                    //                wmIR->boolean_settings[0]->SetValue(false);                 // Relative input
//                    //                wmIR->boolean_settings[1]->SetValue(false);                 // Auto hide
//                }
//                wmShake->SetControlExpression(0, "R2");                     // X
//                wmShake->SetControlExpression(1, "R2");                     // Y
//                wmShake->SetControlExpression(2, "R2");                     // Z
//#if 0
//                wmHotkeys->SetControlExpression(0, "");  // Sideways Toggle
//                wmHotkeys->SetControlExpression(1, "");  // Upright Toggle
//                wmHotkeys->SetControlExpression(2, "");  // Sideways Hold
//                wmHotkeys->SetControlExpression(3, "");  // Upright Hold
//                wmShake->SetControlExpression(0, "");  // X
//                wmShake->SetControlExpression(1, "");  // Y
//                wmShake->SetControlExpression(2, "");  // Z
//#endif
//            }
//
//            ControllerEmu::ControlGroup* wmRumble = wm->GetWiimoteGroup(WiimoteGroup::Rumble);
//            ControllerEmu::ControlGroup* wmOptions = wm->GetWiimoteGroup(WiimoteGroup::Options);
//            ControllerEmu::Extension* wmExtension = (ControllerEmu::Extension*)wm->GetWiimoteGroup(WiimoteGroup::Attachments);
//
////                    wmOptions->boolean_settings[0]->SetValue(true);        // Forward Wiimote
////                    wmOptions->boolean_settings[1]->SetValue(false);       // Upright Wiimote
////                    wmOptions->boolean_settings[2]->SetValue(false);       // Sideways Wiimote
////                    wmOptions->numeric_settings[0]->SetValue(0);           // Speaker Pan [-127, 127]
////                    wmOptions->numeric_settings[1]->SetValue(95.0 / 100);  // Battery
//            wmRumble->SetControlExpression(0, "Rumble");
//
//            switch (device)
//            {
//                case OEWiimote:
//                    //wmExtension->switch_extension = EXT_NONE;
//                    WiimoteReal::ChangeWiimoteSource(port, WIIMOTE_SRC_EMU);
//                    break;
//
//                case OEWiimoteSW:
//                    //wmExtension->switch_extension = EXT_NONE;
//                    //                wmOptions->boolean_settings[2]->SetValue(true);  // Sideways Wiimote
//                    WiimoteReal::ChangeWiimoteSource(port, WIIMOTE_SRC_EMU);
//                    break;
//
//                case OEWiimoteNC:
//                    //wmExtension->switch_extension = EXT_NUNCHUK;
//                    WiimoteReal::ChangeWiimoteSource(port, WIIMOTE_SRC_EMU);
//                    break;
//
//                case OEWiimoteCC:
//                    //wmExtension->switch_extension = EXT_CLASSIC;
//                    WiimoteReal::ChangeWiimoteSource(port, WIIMOTE_SRC_EMU);
//                    break;
//
//                default:
//                    WiimoteReal::ChangeWiimoteSource(port, WIIMOTE_SRC_NONE);
//                    break;
//            }
//
//            wm->UpdateReferences(g_controller_interface);
//            ::Wiimote::GetConfig()->SaveConfig();
//        }
//        else if (input_types[port] == OEWiiMoteReal)
//            WiimoteReal::ChangeWiimoteSource(port, WIIMOTE_SRC_REAL);

//        std::vector<openemu_input_descriptor> all_descs;
//
//        for (int i = 0; i < 4; i++)
//        {
//            openemu_input_descriptor* desc;
//
//            switch (input_types[i])
//            {
//                case OEWiimote:
//                    desc = descWiimote;
//                    break;
//
//                case OEWiimoteSW:
//                    desc = descWiimoteSideways;
//                    break;
//
//                case OEWiimoteNC:
//                    desc = descWiimoteNunchuk;
//                    break;
//
//                case OEWiimoteCC:
//                    desc = descWiimoteCC;
//                    break;
//
//                case OEWiiMoteReal:
//                case OEDolDevNone:
//                    continue;
//
//                default:
//                    desc = descGC;
//                    break;
//            }
//            for (int j = 0; desc[j].device != 0; j++)
//            {
//                openemu_input_descriptor new_desc = desc[j];
//                new_desc.port = i;
//                all_descs.push_back(new_desc);
//            }
//        }
//        all_descs.push_back({ 0 });
    }
} // namespace Input
