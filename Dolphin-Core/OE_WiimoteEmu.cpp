// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cmath>
#include <cstring>

#include "Common/ChunkFile.h"
#include "Common/CommonTypes.h"
#include "Common/MathUtil.h"
#include "Common/MsgHandler.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/Host.h"
#include "Core/Movie.h"
#include "Core/NetPlayClient.h"
#include "Core/HW/WiimoteEmu/MatrixMath.h"
#include "OE_WiimoteEmu.h"
#include "Core/HW/WiimoteEmu/WiimoteHid.h"
#include "OE_Classic.h"
#include "OE_Drums.h"
#include "OE_Guitar.h"
#include "OE_Nunchuk.h"
#include "OE_Turntable.h"
#include "Core/HW/WiimoteReal/WiimoteReal.h"

namespace
{
    // :)
    auto const TAU = 6.28318530717958647692;
    auto const PI = TAU / 2.0;
}

namespace WiimoteEmu
{

    static const u8 eeprom_data_0[] = {
        // IR, maybe more
        // assuming last 2 bytes are checksum
        0xA1, 0xAA, 0x8B, 0x99, 0xAE, 0x9E, 0x78, 0x30, 0xA7, /*0x74, 0xD3,*/ 0x00, 0x00, // messing up the checksum on purpose
        0xA1, 0xAA, 0x8B, 0x99, 0xAE, 0x9E, 0x78, 0x30, 0xA7, /*0x74, 0xD3,*/ 0x00, 0x00,
        // Accelerometer
        // Important: checksum is required for tilt games
        ACCEL_ZERO_G, ACCEL_ZERO_G, ACCEL_ZERO_G, 0, ACCEL_ONE_G, ACCEL_ONE_G, ACCEL_ONE_G, 0, 0, 0xA3,
        ACCEL_ZERO_G, ACCEL_ZERO_G, ACCEL_ZERO_G, 0, ACCEL_ONE_G, ACCEL_ONE_G, ACCEL_ONE_G, 0, 0, 0xA3,
    };

    static const u8 motion_plus_id[] = { 0x00, 0x00, 0xA6, 0x20, 0x00, 0x05 };

    static const u8 eeprom_data_16D0[] = {
        0x00, 0x00, 0x00, 0xFF, 0x11, 0xEE, 0x00, 0x00,
        0x33, 0xCC, 0x44, 0xBB, 0x00, 0x00, 0x66, 0x99,
        0x77, 0x88, 0x00, 0x00, 0x2B, 0x01, 0xE8, 0x13
    };

    static const ReportFeatures reporting_mode_features[] =
    {
        //0x30: Core Buttons
        { 2, 0, 0, 0, 4 },
        //0x31: Core Buttons and Accelerometer
        { 2, 4, 0, 0, 7 },
        //0x32: Core Buttons with 8 Extension bytes
        { 2, 0, 0, 4, 12 },
        //0x33: Core Buttons and Accelerometer with 12 IR bytes
        { 2, 4, 7, 0, 19 },
        //0x34: Core Buttons with 19 Extension bytes
        { 2, 0, 0, 4, 23 },
        //0x35: Core Buttons and Accelerometer with 16 Extension Bytes
        { 2, 4, 0, 7, 23 },
        //0x36: Core Buttons with 10 IR bytes and 9 Extension Bytes
        { 2, 0, 4, 14, 23 },
        //0x37: Core Buttons and Accelerometer with 10 IR bytes and 6 Extension Bytes
        { 2, 4, 7, 17, 23 },

        // UNSUPPORTED:
        //0x3d: 21 Extension Bytes
        { 0, 0, 0, 2, 23 },
        //0x3e / 0x3f: Interleaved Core Buttons and Accelerometer with 36 IR bytes
        { 0, 0, 0, 0, 23 },
    };

    static const u16 button_bitmasks[] =
    {
        Wiimote::BUTTON_A,
        Wiimote::BUTTON_B,
        Wiimote::BUTTON_ONE,
        Wiimote::BUTTON_TWO,
        Wiimote::BUTTON_MINUS,
        Wiimote::BUTTON_PLUS,
        Wiimote::BUTTON_HOME
    };

    static const u16 dpad_bitmasks[] =
    {
        Wiimote::PAD_UP, Wiimote::PAD_DOWN, Wiimote::PAD_LEFT, Wiimote::PAD_RIGHT
    };
    static const u16 dpad_sideways_bitmasks[] =
    {
        Wiimote::PAD_RIGHT, Wiimote::PAD_LEFT, Wiimote::PAD_UP, Wiimote::PAD_DOWN
    };

    static const char* const named_buttons[] =
    {
        "A", "B", "1", "2", "-", "+", "Home",
    };

    void Wiimote::Reset()
    {
        m_reporting_mode =  WM_REPORT_CORE;
        // i think these two are good
        m_reporting_channel = 0;
        m_reporting_auto = false;

        m_rumble_on = false;
        m_speaker_mute = false;
        m_motion_plus_present = false;
        m_motion_plus_active = false;

        // will make the first Update() call send a status request
        // the first call to RequestStatus() will then set up the status struct extension bit
        m_extension->active_extension = -1;

        // eeprom
        memset(m_eeprom, 0, sizeof(m_eeprom));
        // calibration data
        memcpy(m_eeprom, eeprom_data_0, sizeof(eeprom_data_0));
        // dunno what this is for, copied from old plugin
        memcpy(m_eeprom + 0x16D0, eeprom_data_16D0, sizeof(eeprom_data_16D0));

        // set up the register
        memset(&m_reg_speaker, 0, sizeof(m_reg_speaker));
        memset(&m_reg_ir, 0, sizeof(m_reg_ir));
        memset(&m_reg_ext, 0, sizeof(m_reg_ext));
        memset(&m_reg_motion_plus, 0, sizeof(m_reg_motion_plus));

        memcpy(&m_reg_motion_plus.ext_identifier, motion_plus_id, sizeof(motion_plus_id));

        // status
        memset(&m_status, 0, sizeof(m_status));
        // Battery levels in voltage
        //   0x00 - 0x32: level 1
        //   0x33 - 0x43: level 2
        //   0x33 - 0x54: level 3
        //   0x55 - 0xff: level 4
        m_status.battery = (u8)(m_options->settings[5]->GetValue() * 100);

        memset(m_shake_step, 0, sizeof(m_shake_step));

        // clear read request queue
        while (!m_read_requests.empty())
        {
            delete[] m_read_requests.front().data;
            m_read_requests.pop();
        }

        // Yamaha ADPCM state initialize
        m_adpcm_state.predictor = 0;
        m_adpcm_state.step = 127;
    }

    Wiimote::Wiimote(const unsigned int index)
    : m_index(index)
    , ir_sin(0)
    , ir_cos(1)
    , m_last_connect_request_counter(0)
    {
        // ---- set up all the controls ----

        // buttons
        groups.emplace_back(m_buttons = new Buttons("Buttons"));
        for (auto& named_button : named_buttons)
            m_buttons->controls.emplace_back(new ControlGroup::Input(named_button));

        // ir
        groups.emplace_back(m_ir = new Cursor(_trans("IR")));

        // swing
        groups.emplace_back(m_swing = new Force(_trans("Swing")));

        // tilt
        groups.emplace_back(m_tilt = new Tilt(_trans("Tilt")));

        // shake
        groups.emplace_back(m_shake = new Buttons(_trans("Shake")));
        m_shake->controls.emplace_back(new ControlGroup::Input("X"));
        m_shake->controls.emplace_back(new ControlGroup::Input("Y"));
        m_shake->controls.emplace_back(new ControlGroup::Input("Z"));

        // extension
        groups.emplace_back(m_extension = new Extension(_trans("Extension")));
        m_extension->attachments.emplace_back(new WiimoteEmu::None(m_reg_ext));
        m_extension->attachments.emplace_back(new WiimoteEmu::Nunchuk(m_reg_ext));
        m_extension->attachments.emplace_back(new WiimoteEmu::Classic(m_reg_ext));
        m_extension->attachments.emplace_back(new WiimoteEmu::Guitar(m_reg_ext));
        m_extension->attachments.emplace_back(new WiimoteEmu::Drums(m_reg_ext));
        m_extension->attachments.emplace_back(new WiimoteEmu::Turntable(m_reg_ext));

        m_extension->settings.emplace_back(new ControlGroup::Setting(_trans("Motion Plus"), 0, 0, 1));

        // rumble
        groups.emplace_back(m_rumble = new ControlGroup(_trans("Rumble")));
        m_rumble->controls.emplace_back(new ControlGroup::Output(_trans("Motor")));

        // dpad
        groups.emplace_back(m_dpad = new Buttons("D-Pad"));
        for (auto& named_direction : named_directions)
            m_dpad->controls.emplace_back(new ControlGroup::Input(named_direction));

        // options
        groups.emplace_back(m_options = new ControlGroup(_trans("Options")));
        m_options->settings.emplace_back(new ControlGroup::BackgroundInputSetting(_trans("Background Input")));
        m_options->settings.emplace_back(new ControlGroup::Setting(_trans("Sideways Wiimote"), false));
        m_options->settings.emplace_back(new ControlGroup::Setting(_trans("Upright Wiimote"), false));
        m_options->settings.emplace_back(new ControlGroup::IterateUI(_trans("Iterative Input")));
        m_options->settings.emplace_back(new ControlGroup::Setting(_trans("Speaker Pan"), 0, -127, 127));
        m_options->settings.emplace_back(new ControlGroup::Setting(_trans("Battery"), 95.0 / 100, 0, 255));

        // TODO: This value should probably be re-read if SYSCONF gets changed
        m_sensor_bar_on_top = SConfig::GetInstance().m_SYSCONF->GetData<u8>("BT.BAR") != 0;

        // --- reset eeprom/register/values to default ---
        Reset();
    }

    std::string Wiimote::GetName() const
    {
        return std::string("Wiimote") + char('1'+m_index);
    }

    void Wiimote::UpdateAccelData (float X, float Y, float Z)
    {
        m_accel.x = X;
        m_accel.y = Y;
        m_accel.z = Z;

    }

    void Wiimote::UpdateNunchukAccelData (float X, float Y, float Z)
    {
        m_extension->UpdateAccelData (X, Y, Z);
    }

    void Wiimote::UpdateIRdata (double* dX ,double* dY,double* dSize){
        for ( int i = 0; i < 4; i++)
        {
           //  Make sure the dots are visible
           if (dX[i] < 1024 && dY[i] < 768){
                m_IR.x[i] = dX[i];
                m_IR.y[i] = dY[i];
                m_IR.s[i] = dSize[i];
           }
        }
    }

    bool Wiimote::Step()
    {
        // TODO: change this a bit
        m_motion_plus_present = m_extension->settings[0]->value != 0;

        m_rumble->controls[0]->control_ref->State(m_rumble_on);

        // when a movie is active, this button status update is disabled (moved), because movies only record data reports.
        if (!Core::g_want_determinism)
        {
            UpdateButtonsStatus();
        }

        // check if there is a read data request
        if (!m_read_requests.empty())
        {
            ReadRequest& rr = m_read_requests.front();
            // send up to 16 bytes to the Wii
            SendReadDataReply(rr);
            //SendReadDataReply(rr.channel, rr);

            // if there is no more data, remove from queue
            if (0 == rr.size)
            {
                delete[] rr.data;
                m_read_requests.pop();
            }

            // don't send any other reports
            return true;
        }

        // check if a status report needs to be sent
        // this happens on Wiimote sync and when extensions are switched
        if (m_extension->active_extension != m_extension->switch_extension)
        {
            RequestStatus();

            // WiiBrew: Following a connection or disconnection event on the Extension Port,
            // data reporting is disabled and the Data Reporting Mode must be reset before new data can arrive.
            // after a game receives an unrequested status report,
            // it expects data reports to stop until it sets the reporting mode again
            m_reporting_auto = false;

            return true;
        }

        return false;
    }

    void Wiimote::UpdateButtonsStatus()
    {
        // update buttons in status struct
        m_status.buttons.hex = 0;
        const bool is_sideways = m_options->settings[1]->value != 0;
        m_buttons->GetState(&m_status.buttons.hex, button_bitmasks);
        m_dpad->GetState(&m_status.buttons.hex, is_sideways ? dpad_sideways_bitmasks : dpad_bitmasks);
    }

    void Wiimote::GetButtonData(u8* const data)
    {
        // when a movie is active, the button update happens here instead of Wiimote::Step, to avoid potential desync issues.
        if (Core::g_want_determinism)
        {
            UpdateButtonsStatus();
        }

        ((wm_buttons*)data)->hex |= m_status.buttons.hex;
    }

    void Wiimote::GetAccelData(u8* const data, const ReportFeatures& rptf)
    {
        wm_accel& accel = *(wm_accel*)(data + rptf.accel);
        wm_buttons& core = *(wm_buttons*)(data + rptf.core);

        accel.x =  m_accel.x ;
        accel.y =  m_accel.y;
        accel.z =  m_accel.z;

        core.acc_x_lsb = m_accel.x;
        core.acc_y_lsb = m_accel.y;
        core.acc_z_lsb = m_accel.z;
    }

    inline void LowPassFilter(double& var, double newval, double period)
    {
        static const double CUTOFF_FREQUENCY = 5.0;

        double RC = 1.0 / CUTOFF_FREQUENCY;
        double alpha = period / (period + RC);
        var = newval * alpha + var * (1.0 - alpha);
    }

    void Wiimote::GetIRData(u8* const data, bool use_accel)
    {

        if (m_reg_ir.data[0x30] ) // && _needIRupdate )
        {
            switch (m_reg_ir.mode)
            {
                case 1:
                {
                    memset(data, 0xFF, 10);
                    wm_ir_basic* const irdata = (wm_ir_basic*)data;

                    if (m_IR.x[0] < 1024 && m_IR.y[0] < 768)
                    {
                        irdata[0].x1 = static_cast<u8>(m_IR.x[1]);
                        irdata[0].x1hi = m_IR.x[0] >> 8;

                        irdata[0].y1 = static_cast<u8>(m_IR.y[0]);
                        irdata[0].y1hi = m_IR.y[0] >> 8;
                    }

                    if (m_IR.x[1] < 1024 && m_IR.y[1] < 768)
                    {
                        irdata[0].x2 = static_cast<u8>(m_IR.x[0]);
                        irdata[0].x2hi = m_IR.x[1] >> 8;

                        irdata[0].y2 = static_cast<u8>(m_IR.y[1]);
                        irdata[0].y2hi = m_IR.y[1] >> 8;
                    }

                    if (m_IR.x[2] < 1024 && m_IR.y[2] < 768)
                    {
                        irdata[1].x1 = static_cast<u8>(m_IR.x[3]);
                        irdata[1].x1hi = m_IR.x[2] >> 8;

                        irdata[1].y1 = static_cast<u8>(m_IR.y[2]);
                        irdata[1].y1hi = m_IR.y[2] >> 8;
                    }

                    if (m_IR.x[3] < 1024 && m_IR.y[3] < 768)
                    {
                        irdata[1].x2 = static_cast<u8>(m_IR.x[2]);
                        irdata[1].x2hi = m_IR.x[3] >> 8;

                        irdata[1].y2 = static_cast<u8>(m_IR.y[3]);
                        irdata[1].y2hi = m_IR.y[3] >> 8;

                    }
                }
                    break;
                case 3:
                {
                    memset(data, 0xFF, 12);
                    wm_ir_extended* const irdata = (wm_ir_extended*)data;

                    for (unsigned int i = 0; i < 4; ++i)
                        if (m_IR.x[i] < 1024 && m_IR.y[i] < 768)
                        {
                            irdata[i].x = static_cast<u8>(m_IR.x[i]);
                            irdata[i].xhi = m_IR.x[i] >> 8;

                            irdata[i].y = static_cast<u8>(m_IR.y[i]);
                            irdata[i].yhi = m_IR.y[i] >> 8;

                            irdata[i].size = m_IR.s[i];
                        }
                }
                    break;
                    
            }
           // _needIRupdate = false;
        }
    }

    void Wiimote::GetExtData(u8* const data)
    {
        m_extension->GetState(data);

        // i dont think anything accesses the extension data like this, but ill support it. Indeed, commercial games don't do this.
        // i think it should be unencrpyted in the register, encrypted when read.
        memcpy(m_reg_ext.controller_data, data, sizeof(wm_nc)); // TODO: Should it be nc specific?

        // motionplus pass-through modes
        if (m_motion_plus_active)
        {
            switch (m_reg_motion_plus.ext_identifier[0x4])
            {
                    // nunchuk pass-through mode
                    // Bit 7 of byte 5 is moved to bit 6 of byte 5, overwriting it
                    // Bit 0 of byte 4 is moved to bit 7 of byte 5
                    // Bit 3 of byte 5 is moved to bit 4 of byte 5, overwriting it
                    // Bit 1 of byte 5 is moved to bit 3 of byte 5
                    // Bit 0 of byte 5 is moved to bit 2 of byte 5, overwriting it
                case 0x5:
                    //data[5] & (1 << 7)
                    //data[4] & (1 << 0)
                    //data[5] & (1 << 3)
                    //data[5] & (1 << 1)
                    //data[5] & (1 << 0)
                    break;

                    // classic controller/musical instrument pass-through mode
                    // Bit 0 of Byte 4 is overwritten
                    // Bits 0 and 1 of Byte 5 are moved to bit 0 of Bytes 0 and 1, overwriting
                case 0x7:
                    //data[4] & (1 << 0)
                    //data[5] & (1 << 0)
                    //data[5] & (1 << 1)
                    break;

                    // unknown pass-through mode
                default:
                    break;
            }

            ((wm_motionplus_data*)data)->is_mp_data = 0;
            ((wm_motionplus_data*)data)->extension_connected = m_extension->active_extension;
        }

        if (0xAA == m_reg_ext.encryption)
            WiimoteEncrypt(&m_ext_key, data, 0x00, sizeof(wm_nc));
    }

    void Wiimote::Update()
    {
        // no channel == not connected i guess
        if (0 == m_reporting_channel)
            return;

        // returns true if a report was sent
        if (Step())
            return;

        u8 data[MAX_PAYLOAD];
        memset(data, 0, sizeof(data));

        Movie::SetPolledDevice();

        m_status.battery = (u8)(m_options->settings[5]->GetValue() * 100);

        const ReportFeatures& rptf = reporting_mode_features[m_reporting_mode - WM_REPORT_CORE];
        s8 rptf_size = rptf.size;
        if (Movie::IsPlayingInput() && Movie::PlayWiimote(m_index, data, rptf, m_extension->active_extension, m_ext_key))
        {
            if (rptf.core)
                m_status.buttons = *(wm_buttons*)(data + rptf.core);
        }
        else
        {
            data[0] = 0xA1;
            data[1] = m_reporting_mode;

            // core buttons
            if (rptf.core)
                GetButtonData(data + rptf.core);

            // acceleration
            if (rptf.accel)
                GetAccelData(data, rptf);

            // IR
            if (rptf.ir)
                GetIRData(data + rptf.ir, (rptf.accel != 0));

            // extension
            if (rptf.ext)
                GetExtData(data + rptf.ext);

            // hybrid Wiimote stuff (for now, it's not supported while recording)
            if (WIIMOTE_SRC_HYBRID == g_wiimote_sources[m_index] && !Movie::IsRecordingInput())
            {
                using namespace WiimoteReal;

                std::lock_guard<std::recursive_mutex> lk(g_refresh_lock);
                if (g_wiimotes[m_index])
                {
                    const Report& rpt = g_wiimotes[m_index]->ProcessReadQueue();
                    if (!rpt.empty())
                    {
                        const u8 *real_data = rpt.data();
                        switch (real_data[1])
                        {
                                // use data reports
                            default:
                                if (real_data[1] >= WM_REPORT_CORE)
                                {
                                    const ReportFeatures& real_rptf = reporting_mode_features[real_data[1] - WM_REPORT_CORE];

                                    // force same report type from real-Wiimote
                                    if (&real_rptf != &rptf)
                                        rptf_size = 0;

                                    // core
                                    // mix real-buttons with emu-buttons in the status struct, and in the report
                                    if (real_rptf.core && rptf.core)
                                    {
                                        m_status.buttons.hex |= ((wm_buttons*)(real_data + real_rptf.core))->hex;
                                        *(wm_buttons*)(data + rptf.core) = m_status.buttons;
                                    }

                                    // accel
                                    // use real-accel data always i guess
                                    if (real_rptf.accel && rptf.accel)
                                        memcpy(data + rptf.accel, real_data + real_rptf.accel, sizeof(wm_accel));

                                    // ir
                                    // TODO
                                    if (real_rptf.ir && rptf.ir)
                                        memcpy(data + rptf.ir, real_data + real_rptf.ir, sizeof(wm_ir_basic));



                                    // ext
                                    // use real-ext data if an emu-extention isn't chosen
                                    if (real_rptf.ext && rptf.ext && (0 == m_extension->switch_extension))
                                        memcpy(data + rptf.ext, real_data + real_rptf.ext, sizeof(wm_nc));  // TODO: Why NC specific?
                                }
                                else if (WM_ACK_DATA != real_data[1] || m_extension->active_extension > 0)
                                    rptf_size = 0;
                                else
                                    // use real-acks if an emu-extension isn't chosen
                                    rptf_size = -1;
                                break;

                                // use all status reports, after modification of the extension bit
                            case WM_STATUS_REPORT :
                                //if (m_extension->switch_extension)
                                //((wm_status_report*)(real_data + 2))->extension = (m_extension->active_extension > 0);
                                if (m_extension->active_extension)
                                    ((wm_status_report*)(real_data + 2))->extension = 1;
                                rptf_size = -1;
                                break;

                                // use all read-data replies
                            case WM_READ_DATA_REPLY:
                                rptf_size = -1;
                                break;

                        }

                        // copy over report from real-Wiimote
                        if (-1 == rptf_size)
                        {
                            std::copy(rpt.begin(), rpt.end(), data);
                            rptf_size = (s8)(rpt.size());
                        }
                    }
                }
            }

            Movie::CallWiiInputManip(data, rptf, m_index, m_extension->active_extension, m_ext_key);
        }
        if (NetPlay::IsNetPlayRunning())
        {
            NetPlay_GetWiimoteData(m_index, data, rptf.size);
            if (rptf.core)
                m_status.buttons = *(wm_buttons*)(data + rptf.core);
        }

        Movie::CheckWiimoteStatus(m_index, data, rptf, m_extension->active_extension, m_ext_key);

        // don't send a data report if auto reporting is off
        if (false == m_reporting_auto && data[1] >= WM_REPORT_CORE)
            return;

        // send data report
        if (rptf_size)
        {
            Core::Callback_WiimoteInterruptChannel(m_index, m_reporting_channel, data, rptf_size);
        }
    }

    void Wiimote::ControlChannel(const u16 _channelID, const void* _pData, u32 _Size)
    {
        // Check for custom communication
        if (99 == _channelID)
        {
            // Wiimote disconnected
            // reset eeprom/register/reporting mode
            Reset();
            return;
        }

        // this all good?
        m_reporting_channel = _channelID;

        const hid_packet* const hidp = (hid_packet*)_pData;

        INFO_LOG(WIIMOTE, "Emu ControlChannel (page: %i, type: 0x%02x, param: 0x%02x)", m_index, hidp->type, hidp->param);

        switch (hidp->type)
        {
            case HID_TYPE_HANDSHAKE :
                PanicAlert("HID_TYPE_HANDSHAKE - %s", (hidp->param == HID_PARAM_INPUT) ? "INPUT" : "OUPUT");
                break;

            case HID_TYPE_SET_REPORT :
                if (HID_PARAM_INPUT == hidp->param)
                {
                    PanicAlert("HID_TYPE_SET_REPORT - INPUT");
                }
                else
                {
                    // AyuanX: My experiment shows Control Channel is never used
                    // shuffle2: but lwbt uses this, so we'll do what we must :)
                    HidOutputReport((wm_report*)hidp->data);

                    u8 handshake = HID_HANDSHAKE_SUCCESS;
                    Core::Callback_WiimoteInterruptChannel(m_index, _channelID, &handshake, 1);
                }
                break;

            case HID_TYPE_DATA :
                PanicAlert("HID_TYPE_DATA - %s", (hidp->param == HID_PARAM_INPUT) ? "INPUT" : "OUTPUT");
                break;

            default :
                PanicAlert("HidControlChannel: Unknown type %x and param %x", hidp->type, hidp->param);
                break;
        }

    }

    void Wiimote::InterruptChannel(const u16 _channelID, const void* _pData, u32 _Size)
    {
        // this all good?
        m_reporting_channel = _channelID;

        const hid_packet* const hidp = (hid_packet*)_pData;

        switch (hidp->type)
        {
            case HID_TYPE_DATA:
                switch (hidp->param)
            {
                case HID_PARAM_OUTPUT :
                {
                    const wm_report* const sr = (wm_report*)hidp->data;

                    if (WIIMOTE_SRC_REAL & g_wiimote_sources[m_index])
                    {
                        switch (sr->wm)
                        {
                                // these two types are handled in RequestStatus() & ReadData()
                            case WM_REQUEST_STATUS :
                            case WM_READ_DATA :
                                if (WIIMOTE_SRC_REAL == g_wiimote_sources[m_index])
                                    WiimoteReal::InterruptChannel(m_index, _channelID, _pData, _Size);
                                break;

                            default :
                                WiimoteReal::InterruptChannel(m_index, _channelID, _pData, _Size);
                                break;
                        }

                        HidOutputReport(sr, m_extension->switch_extension > 0);
                    }
                    else
                        HidOutputReport(sr);
                }
                    break;

                default :
                    PanicAlert("HidInput: HID_TYPE_DATA - param 0x%02x", hidp->param);
                    break;
            }
                break;

            default:
                PanicAlert("HidInput: Unknown type 0x%02x and param 0x%02x", hidp->type, hidp->param);
                break;
        }
    }

    void Wiimote::ConnectOnInput()
    {
        if (m_last_connect_request_counter > 0)
        {
            --m_last_connect_request_counter;
            return;
        }

        u16 buttons = 0;
        m_buttons->GetState(&buttons, button_bitmasks);
        m_dpad->GetState(&buttons, dpad_bitmasks);

        if (buttons != 0 || m_extension->IsButtonPressed())
        {
            Host_ConnectWiimote(m_index, true);
            // arbitrary value so it doesn't try to send multiple requests before Dolphin can react
            // if Wiimotes are polled at 200Hz then this results in one request being sent per 500ms
            m_last_connect_request_counter = 100;
        }
    }

    void Wiimote::LoadDefaults(const ControllerInterface& ciface)
    {
        ControllerEmu::LoadDefaults(ciface);

        // Buttons

        m_buttons->SetControlExpression(0, "OEWiiMoteButtonA"); // A
        m_buttons->SetControlExpression(1, "OEWiiMoteButtonB"); // B
        
        m_buttons->SetControlExpression(2, "OEWiiMoteButton1"); // 1
        m_buttons->SetControlExpression(3, "OEWiiMoteButton2"); // 2
        m_buttons->SetControlExpression(4, "OEWiiMoteButtonMinus"); // -
        m_buttons->SetControlExpression(5, "OEWiiMoteButtonPlus"); // +

        m_buttons->SetControlExpression(6, "OEWiiMoteButtonHome"); // Home
        
        // Shake
        for (int i = 0; i < 3; ++i)
            m_shake->SetControlExpression(i, "Click 2");
        
        // IR
        m_ir->SetControlExpression(0, "Cursor Y-");
        m_ir->SetControlExpression(1, "Cursor Y+");
        m_ir->SetControlExpression(2, "Cursor X-");
        m_ir->SetControlExpression(3, "Cursor X+");
        
        // DPad
        m_dpad->SetControlExpression(0, "OEWiiMoteButtonUp");    // Up
        m_dpad->SetControlExpression(1, "OEWiiMoteButtonDown");  // Down
        m_dpad->SetControlExpression(2, "OEWiiMoteButtonLeft");  // Left
        m_dpad->SetControlExpression(3, "OEWiiMoteButtonRight"); // Right
        
        
        // ugly stuff
        // enable nunchuk
        m_extension->switch_extension = 0;
        
        // set nunchuk defaults
        m_extension->attachments[1]->LoadDefaults(ciface);
        
        // set Classic controller defaults
        m_extension->attachments[2]->LoadDefaults(ciface);
    }
    
}
