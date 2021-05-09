// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  iosmain.c - main file for ios OSD
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include <functional>   // only for oslog callback

// standard includes
#include <unistd.h>

// MAME headers
#include "osdepend.h"
#include "emu.h"
#include "emuopts.h"
#include "gamedrv.h"
#include "drivenum.h"
#include "screen.h"
#include "strconv.h"
#include "corestr.h"

// OSD headers
#include "video.h"
#include "iososd.h"

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)<(b) ? (b) : (a))

//============================================================
// MYOSD globals
//============================================================
int myosd_display_width;
int myosd_display_height;

//============================================================
//  myosd_main
//============================================================

extern "C" int myosd_main(int argc, char** argv, myosd_callbacks* callbacks, size_t callbacks_size)
{
    myosd_callbacks host_callbacks;
    memset(&host_callbacks, 0, sizeof(host_callbacks));
    memcpy(&host_callbacks, callbacks, MIN(sizeof(host_callbacks), sizeof(myosd_callbacks)));
    
    if (argc == 0) {
        static const char* args[] = {"myosd"};
        argc = 1;
        argv = (char**)args;
    }

    std::vector<std::string> args = osd_get_command_line(argc, argv);

    osd_options options;
    ios_osd_interface osd(options, host_callbacks);

    return emulator_info::start_frontend(options, osd, args);
}

//============================================================
//  myosd_get
//============================================================
extern "C" intptr_t myosd_get(int var)
{
    switch (var)
    {
        case MYOSD_VERSION:
            return (int)(atof(emulator_info::get_bare_build_version()) * 1000.0);

        case MYOSD_VERSION_STRING:
            return (intptr_t)(void*)emulator_info::get_build_version();
            
        case MYOSD_FPS:
            return 0;
            
        case MYOSD_SPEED:
            return 0;
    }
    return 0;
}

//============================================================
//  myosd_set
//============================================================
extern "C" void myosd_set(int var, intptr_t value)
{
    switch (var)
    {
        case MYOSD_DISPLAY_WIDTH:
            myosd_display_width = value;
            break;
        case MYOSD_DISPLAY_HEIGHT:
            myosd_display_height = value;
            break;
        case MYOSD_FPS:
            //myosd_fps = value;
            break;
        case MYOSD_SPEED:
            //myosd_speed = value;
            break;
    }
}

//============================================================
//  constructor
//============================================================

ios_osd_interface::ios_osd_interface(osd_options &options, myosd_callbacks &callbacks)
: m_options(options), m_verbose(false), m_callbacks(callbacks)
{
    osd_output::push(this);
    
    if (m_callbacks.output_init != NULL)
        m_callbacks.output_init();
}


//============================================================
//  destructor
//============================================================

ios_osd_interface::~ios_osd_interface()
{
    if (m_callbacks.output_exit != NULL)
        m_callbacks.output_exit();
    
    osd_output::pop(this);
}

//-------------------------------------------------
//  output_callback  - callback for osd_printf_...
//-------------------------------------------------

void ios_osd_interface::output_callback(osd_output_channel channel, const util::format_argument_pack<std::ostream> &args)
{
    if (channel == OSD_OUTPUT_CHANNEL_VERBOSE && !m_verbose)
        return;

    std::ostringstream buffer;
    util::stream_format(buffer, args);
    
    if (m_callbacks.output_text != NULL)
    {
        _Static_assert((int)MYOSD_OUTPUT_ERROR == (int)OSD_OUTPUT_CHANNEL_ERROR);
        _Static_assert((int)MYOSD_OUTPUT_WARNING == (int)OSD_OUTPUT_CHANNEL_WARNING);
        _Static_assert((int)MYOSD_OUTPUT_INFO == (int)OSD_OUTPUT_CHANNEL_INFO);
        _Static_assert((int)MYOSD_OUTPUT_DEBUG == (int)OSD_OUTPUT_CHANNEL_DEBUG);
        _Static_assert((int)MYOSD_OUTPUT_VERBOSE == (int)OSD_OUTPUT_CHANNEL_VERBOSE);

        m_callbacks.output_text(channel, buffer.str().c_str());
    }
    else
    {
        static const char* channel_str[] = {"[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[VERBOSE]: ", "[LOG]: "};
        fputs(channel_str[channel], stderr);
        fputs(buffer.str().c_str(), stderr);
    }
}

//============================================================
// get_game_info - convert game_driver to a myosd_game_info
//============================================================
static void get_game_info(myosd_game_info* info, const game_driver *driver, emu_options *options)
{
    memset(info, 0, sizeof(myosd_game_info));
    
    /*
    device_type                 type;               // static type info for driver class
    const char *                parent;             // if this is a clone, the name of the parent
    const char *                year;               // year the game was released
    const char *                manufacturer;       // manufacturer of the game
    machine_creator_wrapper     machine_creator;    // machine driver tokens
    ioport_constructor          ipt;                // pointer to constructor for input ports
    driver_init_wrapper         driver_init;        // DRIVER_INIT callback
    const tiny_rom_entry *      rom;                // pointer to list of ROMs for the game
    const char *                compatible_with;
    const internal_layout *     default_layout;     // default internally defined layout
    machine_flags::type         flags;              // orientation and other flags; see defines above
    char                        name[MAX_DRIVER_NAME_CHARS + 1]; // short name of the game
    */
    
    int type = (driver->flags & machine_flags::MASK_TYPE);
    
    if (type == MACHINE_TYPE_ARCADE)
        info->type = MYOSD_GAME_TYPE_ARCADE;
    else if (type == MACHINE_TYPE_CONSOLE)
        info->type = MYOSD_GAME_TYPE_CONSOLE;
    else if (type == MACHINE_TYPE_COMPUTER)
        info->type = MYOSD_GAME_TYPE_COMPUTER;
    else
        info->type = MYOSD_GAME_TYPE_OTHER;

    info->source_file  = driver->type.source();
    info->parent       = driver->parent;
    info->name         = driver->name;
    info->description  = driver->type.fullname();
    info->year         = driver->year;
    info->manufacturer = driver->manufacturer;
    
    if (info->parent != NULL && info->parent[0] == '0' && info->parent[1] == 0)
        info->parent = "";
    
    if (driver->flags & (MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION))
        info->flags |= MYOSD_GAME_INFO_NOT_WORKING;

    if ((driver->flags & machine_flags::MASK_ORIENTATION) == machine_flags::ROT90 || (driver->flags & machine_flags::MASK_ORIENTATION) == machine_flags::ROT270)
        info->flags |= MYOSD_GAME_INFO_VERTICAL;
    
    if (driver->flags & MACHINE_IS_BIOS_ROOT)
        info->flags |= MYOSD_GAME_INFO_BIOS;
    
    if (driver->flags & (MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL))
        info->flags |= MYOSD_GAME_INFO_IMPERFECT_GRAPHICS;

    if (driver->flags & (MACHINE_NO_SOUND | MACHINE_IMPERFECT_SOUND | MACHINE_NO_SOUND_HW))
        info->flags |= MYOSD_GAME_INFO_IMPERFECT_SOUND;

    if (driver->flags & MACHINE_SUPPORTS_SAVE)
        info->flags |= MYOSD_GAME_INFO_SUPPORTS_SAVE;

    // check for a vector game
    machine_config config(*driver, *options);
    for (const screen_device &device : screen_device_enumerator(config.root_device()))
    {
        if (device.screen_type() == SCREEN_TYPE_VECTOR)
            info->flags |= MYOSD_GAME_INFO_VECTOR;
    }
    
    // TODO: get software list(s)
}

//============================================================
//  init
//============================================================

void ios_osd_interface::init(running_machine &machine)
{
    // This function is responsible for initializing the OSD-specific
    // video and input functionality, and registering that functionality
    // with the MAME core.
    //
    // In terms of video, this function is expected to create one or more
    // render_targets that will be used by the MAME core to provide graphics
    // data to the system. Although it is possible to do this later, the
    // assumption in the MAME core is that the user interface will be
    // visible starting at init() time, so you will have some work to
    // do to avoid these assumptions.
    //
    // In terms of input, this function is expected to enumerate all input
    // devices available and describe them to the MAME core by adding
    // input devices and their attached items (buttons/axes) via the input
    // system.
    //
    // Beyond these core responsibilities, init() should also initialize
    // any other OSD systems that require information about the current
    // running_machine.
    //
    // This callback is also the last opportunity to adjust the options
    // before they are consumed by the rest of the core.
    //

    m_machine = &machine;
    
    // ensure we get called on the way out
    machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&ios_osd_interface::machine_exit, this));
    
    auto &options = machine.options();
    
    // extract the verbose printing option
    if (options.verbose() || DebugLog > 1)
        set_verbose(true);
    
    bool in_game = (&m_machine->system() != &GAME_NAME(___empty));
        
    if (m_callbacks.game_init != NULL && in_game)
    {
        myosd_game_info info;
        get_game_info(&info, &m_machine->system(), &m_machine->options());
        m_callbacks.game_init(&info);
    }
    
    video_init();
    input_init();
    sound_init();
}

//============================================================
//  machine_exit
//============================================================

void ios_osd_interface::machine_exit()
{
    bool in_game = (&m_machine->system() != &GAME_NAME(___empty));

    if (m_callbacks.game_exit != NULL && in_game)
        m_callbacks.game_exit();
    
    video_exit();
    input_exit();
    sound_exit();
}

//============================================================
//  osd_setup_osd_specific_emu_options
//============================================================

// struct definitions
static const options_entry myosd_option_entries[] =
{
    // MYOSD options
    { nullptr,              nullptr,    OPTION_HEADER,     "MYOSD OPTIONS" },
    { OPTION_HISCORE,       "0",        OPTION_BOOLEAN,    "enable hiscore system" },

    { nullptr }
};

void osd_setup_osd_specific_emu_options(emu_options &opts)
{
    opts.add_entries(myosd_option_entries);
}
