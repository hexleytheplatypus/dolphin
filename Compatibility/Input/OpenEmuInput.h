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

typedef enum _OEDolJoyControls
{
    OEDolJoypadB,
    OEDolJoypadY,
    OEDolJoypadSelect,
    OEDolJoypadStart,
    OEDolJoypadUp,
    OEDolJoypadDown,
    OEDolJoypadLeft,
    OEDolJoypadRight,
    OEDolJoypadA,
    OEDolJoypadX,
    OEDolJoypadL,
    OEDolJoypadR,
    OEDolJoypadL2,
    OEDolJoypadR2,
    OEDolJoypadL3,
    OEDolJoypadR3
} OEDolJoyControls;

typedef enum _OEGCAnalogControls
{
    OEGCAnalog,
    OEGCAnalogC,
    OEGCAnalogTrigger
} OEGCAnalogContols;

typedef enum _OEDolAnalogAxis
{
    OEDolAnalogX,
    OEDolAnalogY
} OEDolAnalogAxis;

typedef enum _OEGCAxis
{
    OEGCAnalogX,
    OEGCAnalogY,
    OEGCAnalogCX,
    OEGCAnalogCY,
    OEGCAxisCount
} OEGCAxis;

typedef enum _OEWiimote
{
    OEWiimote = 1,
    OEWiimoteSW,
    OEWiimoteNC,
    OEWiimoteCC,
    OEWiiMoteReal
} OEWiiController;

namespace Input
{
    struct openemu_controller_description
    {
        const char *desc;
        unsigned id;
    };
    
    struct openemu_controller_info
    {
        const struct openemu_controller_description *types;
        unsigned num_types;
    };
    
    struct openemu_input_descriptor
    {
        unsigned port;
        unsigned device;
        unsigned index;
        unsigned id;
        const char *description;
    };
    
    typedef int16_t (*openemu_input_state_t)(unsigned port, unsigned device, unsigned index, unsigned id);
    typedef void (*openemu_input_poll_t)();
    
    void openemu_set_controller_port_device(unsigned port, unsigned device);
    void openemu_set_input_state(openemu_input_state_t);
    void openemu_set_input_poll(openemu_input_poll_t);
    
    void Openemu_Input_Init();
    void OpenEmu_Input_Update();
    void ResetControllers();

}
