typedef enum _OEDolDev
{
    OEDolDevNone,
    OEDolDevJoy,
    OEDolDevMouse,
    OEDolDevKeyboard,
    OEDolDevLightGun,
    OEDolDevAnalog,
    OEDolDevPointer
} OEDolDevs;

typedef enum _OEGCDigital
{
    OEGCDigitalL = 21,
    OEGCDigitalR
} OEGCDigital;

typedef enum _OEWiiConYype
{
    OEWiimote = 1,
    OEWiimoteSW,
    OEWiimoteNC,
    OEWiimoteCC,
    OEWiiMoteReal
} OEWiiConType;

namespace Input
{
    typedef int16_t (*openemu_input_state_t)(unsigned port, unsigned device, unsigned index, unsigned id);
    typedef void (*openemu_input_poll_t)();
    
    void openemu_set_controller_port_device(unsigned port, unsigned device);
    void openemu_set_input_state(openemu_input_state_t);
    void openemu_set_input_poll(openemu_input_poll_t);
    
    void Openemu_Input_Init();
    void OpenEmu_Input_Update();
    void ResetControllers();

}
