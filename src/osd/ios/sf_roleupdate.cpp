
//============================================================
//
//  街霸系列刷新角色
//
//============================================================

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include <string>

#include "inputmacro.h"

//============================================================
//  刷新用户角色
//============================================================
void inputmacro_manager::sf_role_update(std::string &game_name)
{
    if (game_name == "sf2")
    {
        sf2_role();
    }
    else if (game_name == "sf2ce")
    {
        sf2ce_role();
    }
}

//============================================================
//  街霸2用户角色
//============================================================
void inputmacro_manager::sf2_role()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);
        u8 role = space.read_byte(0xFF8657);
        u8 time = space.read_byte(0xFF8ACE);
        u8 p1_round_win = space.read_byte(0xFF8656);
        u8 p2_round_win = space.read_byte(0xFF8956);
        if ((time == 0 && p1_round_win == 0 && p2_round_win == 0) || p1_round_win == 2 || p2_round_win == 2)
        {
            if (m_p1_role != -1)
            {
                m_p1_role = -1;
                m_refreshrole_callback("-1");
            }
        }
        else
        {
            if (m_p1_role != role)
            {
                m_p1_role = role;
                m_refreshrole_callback(std::to_string(m_p1_role));
            }
        }
    }
}

//============================================================
//  街霸2冠军用户角色
//============================================================
void inputmacro_manager::sf2ce_role()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);
        u8 role = space.read_byte(0xFF864F);
        u8 time = space.read_byte(0xFF8ABE);
        u8 p1_round_win = space.read_byte(0xFF864E);
        u8 p2_round_win = space.read_byte(0xFF894E);
        if ((time == 0 && p1_round_win == 0 && p2_round_win == 0) || p1_round_win == 2 || p2_round_win == 2)
        {
            if (m_p1_role != -1)
            {
                m_p1_role = -1;
                m_refreshrole_callback("-1");
            }
        }
        else
        {
            if (m_p1_role != role)
            {
                m_p1_role = role;
                m_refreshrole_callback(std::to_string(m_p1_role));
            }
        }
    }
}