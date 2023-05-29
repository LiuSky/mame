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
#include "main.h"
#include "fileio.h"
#include "gamedrv.h"
#include "drivenum.h"
#include "romload.h"
#include "screen.h"
#include "softlist_dev.h"
#include "strconv.h"
#include "corestr.h"
#include "mame.h"
#include "cheat.h"
#include "ui.h"

// OSD headers
#include "video.h"
#include "iososd.h"

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)<(b) ? (b) : (a))
//============================================================
//  常用打印提示语
//============================================================
#define IOS_UNINITIALIEED_MESSAGE "模拟器未出初始化"
#define IOS_INVALID_MESSAGE "无效的枚举类型"

//============================================================
// MYOSD globals
//============================================================
ios_osd_interface * osd_shared = NULL;
int myosd_display_width;
int myosd_display_height;

#define OSD_MACHINE osd_shared->machine()
#define OSD_OPTIONS osd_shared->options()
#define OSD_MANAGER mame_machine_manager::instance()


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
    emu_options options;
    options.add_entries(s_option_entries);
    osd_shared = new ios_osd_interface(options, host_callbacks);
    return emulator_info::start_frontend(options, *osd_shared, args);
}

//============================================================
//  myosd_get
//============================================================
extern "C" int myosd_get(int var)
{
    switch (var)
    {       
        case MYOSD_SPEED: {
            int speed = 1;
            speed = int(OSD_OPTIONS.speed() * 100.0);
            return speed;
        }
        break;
        case MYOSD_SOUND: {
 
            int sound = 1;
            sound = (osd_shared->m_isSound == true ? 1 : 0);
            return sound;
        }
        break;
        case MYOSD_ICON: {

           return  OSD_MACHINE.bookkeeping().coin_counter_get_count(0);
        }
        break;
        default:
            break;
    }
    return 0;
}
 
//============================================================
//  myosd_set
//============================================================
extern "C" void myosd_set(int var, int value)
{
    switch (var)
    {
        case MYOSD_DISPLAY_WIDTH:
            myosd_display_width = value;
            break;
        case MYOSD_DISPLAY_HEIGHT:
            myosd_display_height = value;
            break;
        case MYOSD_SOUND:
            if (osd_shared!=NULL)
            {
                osd_shared->m_isSound = (value == 0 ? false : true);
            }
            break;
        case MYOSD_SPEED:
            
            if (osd_shared!=NULL)
            {
                float speed = value / 100.0;
                OSD_OPTIONS.set_value(OPTION_SPEED, speed, OPTION_PRIORITY_NORMAL);
            }
            break;
        default:
            break;
    }
}


//============================================================
//  获取金手指
//============================================================
extern "C" const char *myosd_cheat()
{
    std::string result = "";
    int index = 0;
    for (auto &scannode : OSD_MANAGER->cheat().entries())
    {
        result.append(std::to_string(index));
        result.append("==");
        result.append(scannode->description());
        result.append("==");
        result.append(std::to_string(scannode->state()));
        result.append("==");
        if (scannode->is_onoff())
        {
            result.append(std::to_string(1));
            result.append("开关类型");
        }
        else
        {
            result.append(std::to_string(1));
            result.append("其他类型");
        }
        result.append("|");
        index += 1;
    }

    return strdup(result.c_str());
}


//============================================================
//  执行金手指开关脚本
//============================================================
extern "C" bool myosd_cheatSwitchScript(int index, bool isOpen)
{
    cheat_entry *scannode = OSD_MANAGER->cheat().entries()[index].get();
    if (scannode->is_onoff())
    {
        if (isOpen)
            scannode->set_state(SCRIPT_STATE_RUN);
        else
            scannode->set_state(SCRIPT_STATE_OFF);
            
        return true;
    }
    else
    {
        osd_printf_debug("此金手指类型不支持开关模式");
        return false;
    }
}

//============================================================
//  显示或者关闭菜单栏
//============================================================
extern "C" void myosd_ShowMenu() {

    OSD_MANAGER->ui().show_menu();
}

//============================================================
//  myosd_pause
//============================================================
extern "C" void myosd_pause() {

    OSD_MACHINE.pause();
}

//============================================================
//  myosd_resume
//============================================================
extern "C" void myosd_resume() {
    OSD_MACHINE.resume();
}

//============================================================
//  myosd_paused
//============================================================
extern "C" bool myosd_paused() {
    return OSD_MACHINE.paused();
}


//============================================================
//  myosd_save
//============================================================
extern "C" void myosd_save(const char *fileName) {

    // 图片保存
    std::string path = string_format("%s%s%s.png", OSD_MACHINE.basename(), PATH_SEPARATOR, fileName);
    emu_file file(OSD_OPTIONS.state_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
    file.open(path);
    OSD_MACHINE.video().save_snapshot(nullptr, file);

    // 存档保存
    OSD_MACHINE.schedule_save(fileName, [&](bool result, const std::string& msg) {
        osd_shared->callbacks().result_callback(MYOSD_CALLBACK_SAVE, result, strdup(msg.c_str()));
    });
}

//============================================================
//  myosd_load
//============================================================
extern "C" void myosd_load(const char *fileName) {
    
    OSD_MACHINE.schedule_load(fileName, [&](bool result, const std::string& msg) {
        osd_shared->callbacks().result_callback(MYOSD_CALLBACK_LOAD, result, strdup(msg.c_str()));
    });
}


//============================================================
//  myosd_exit
//============================================================
extern "C" void myosd_exit()
{
    OSD_MACHINE.schedule_exit();
}

//============================================================
//  刷新宏
//============================================================
extern "C" void myosd_macro_reload()
{
    osd_shared->inputmacro().reload();
}

//============================================================
//  执行输入宏
//============================================================
extern "C" void myosd_inputmacro_execute(int player, const char *key, bool* release) 
{
    osd_shared->inputmacro().execute(player, key, release, [&](bool result, const std::string& msg)
    {
        osd_shared->callbacks().result_callback(MYOSD_CALLBACK_SKILLE, result, strdup(msg.c_str()));
    });
}