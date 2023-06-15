
//============================================================
//
//  kof系列刷新角色
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
void inputmacro_manager::kof_role_update(std::string &game_name)
{
    if (game_name == "kof97" || game_name == "kof97pls")
    {
      kof97_role();
    }
    else if (game_name == "kof98")
    {
      kof98_role();
    }
    else if (game_name == "kof99" || game_name == "kof2000")
    {
      kof99And2k_role();
    }
    else if (game_name == "kof2001")
    {
      kof2k1_role();
    }
    else if (game_name == "kof2002") 
    {
      kof2k2_role();
    }
    else if (game_name == "kof2003")
    {
      kof2k3_role();
    }
    else if (game_name == "kof2k4se")
    {
      kof2k4se_role();
    }
}

//============================================================
//  kof97角色刷新(还没有解决机器打的时候不显示一键技能。内存地址需要找到)
//============================================================
void inputmacro_manager::kof97_role()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);
        u8 time_data = space.read_byte(0x10A83A);
        u8 p1_death = space.read_byte(0x10A855);
        u8 p2_death = space.read_byte(0x10A866);
        u8 first_role_index = space.read_byte(0x10A851);
        u8 second_role_index = space.read_byte(0x10A852);
        u8 third_role_index = space.read_byte(0x10A853);
        u8 start = space.read_byte(0x1081E2);

        // 主要用来判断大蛇里面的团队人物
        u16 act = space.read_word(0x108172);

        /*
        判断p1_death是否死了3次 或者 p2_death是否也死了3次
        */
        if ((p1_death == 3 || p2_death == 3))
        {
            if (m_p1_role != -1)
            {
                m_p1_role = -1;
                m_refreshrole_callback("-1");
            }
        }
        /*
        1.判断p1死的人数是否是大于0并且小于3
        2.判断游戏时间大于0
        3.判断start等于8(代表下一局开始)并且m_p1_role等于-1，主要是用来解决读档的时候，这时候start已经开始了，所以并不会进入
        4.判断p1是否选择了人物顺序
        5.判断是否是大蛇人物(21,22,23)
        */
        else if (p1_death >= 0 && p1_death < 3 && time_data > 0 && (start == 8 || m_p1_role == -1) && !(first_role_index == 0 && second_role_index == 0 && third_role_index == 0)) 
        {

            u8 first_role_key = space.read_byte(0x10A84B);
            u8 second_role_key = space.read_byte(0x10A84C);
            u8 third_role_key = space.read_byte(0x10A84D);
            std::vector<u8> roles = {first_role_key, second_role_key, third_role_key};
            std::vector<u8> ooa = {first_role_index, second_role_index, third_role_index};
            u8 role = roles[ooa[p1_death]];

            if (role == 21 || role == 22 || role == 23) 
            {
                m_count_frame ++;
                if ((act == 0x15f || act == 0x0 || act == 0x15e) && m_count_frame >= 60)
                {
                    m_count_frame = 0;
                    if (role == 21 && m_p1_role != 21)
                    {
                        m_p1_role = role;
                        m_refreshrole_callback(std::to_string(m_p1_role));
                    }
                    else if (role == 22 && m_p1_role != 22)
                    {
                        m_p1_role = role;
                        m_refreshrole_callback(std::to_string(m_p1_role));
                    }
                    else if (role == 23 && m_p1_role != 23)
                    {
                        m_p1_role = role;
                        m_refreshrole_callback(std::to_string(m_p1_role));
                    }
                }
                else if ((act == 0x160 || act == 0x4a || act == 0x4f) && m_count_frame >= 30)
                {
                    m_count_frame = 0;
                    if (role == 21 && m_p1_role != 35)
                    {
                        m_p1_role = 35;
                        m_refreshrole_callback(std::to_string(m_p1_role));
                    }
                    else if (role == 22 && m_p1_role != 34)
                    {
                        m_p1_role = 34;
                        m_refreshrole_callback(std::to_string(m_p1_role));
                    }
                    else if (role == 23 && m_p1_role != 33)
                    {
                        m_p1_role = 33;
                        m_refreshrole_callback(std::to_string(m_p1_role));
                    }
                }
            }
            else
            {
                if (m_p1_role != role)
                {
                    m_count_frame = 0;
                    m_p1_role = role;
                    m_refreshrole_callback(std::to_string(role));
                }
            }
        }
    }  
}

//============================================================
//  kof98角色刷新
//============================================================
void inputmacro_manager::kof98_role()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);

        u8 time_data = space.read_byte(0x10A83A);
        u8 p1_death = space.read_byte(0x10A858);
        u8 p2_death = space.read_byte(0x10A869);
        u8 first_role_index = space.read_byte(0x10A854);
        u8 second_role_index = space.read_byte(0x10A855);
        u8 third_role_index = space.read_byte(0x10A856);
        u8 start = space.read_byte(0x1081E2);
        if (p1_death == 3 || p2_death == 3)
        {
            m_kof_p1death = -1;
            m_refreshrole_callback("-1");
        }
        else if (p1_death >= 0 && p1_death < 3 && m_kof_p1death != p1_death && time_data > 0 && (start == 8 || m_kof_p1death == -1) && !(first_role_index == 0 && second_role_index == 0 && third_role_index == 0)) 
        {
            m_kof_p1death = p1_death;
            u8 first_role_key = space.read_byte(0x10A84E);
            u8 second_role_key = space.read_byte(0x10A84F);
            u8 third_role_key = space.read_byte(0x10A850);
            std::vector<u8> roles = {first_role_key, second_role_key, third_role_key};
            std::vector<u8> ooa = {first_role_index, second_role_index, third_role_index};
            m_refreshrole_callback(std::to_string((roles[ooa[m_kof_p1death]])));
        }
    }  
}

//============================================================
//  kof99和2000角色刷新
//============================================================
void inputmacro_manager::kof99And2k_role()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);
        u8 time_data = space.read_byte(0x10A7E6);
        u8 p1_death = space.read_byte(0x10A807);
        u8 p2_death = space.read_byte(0x10A806);
        u8 first_role_index = space.read_byte(0x10A802);
        u8 second_role_index = space.read_byte(0x10A803);
        u8 third_role_index = space.read_byte(0x10A804);
        u8 start = space.read_byte(0x1081E2);
        if (p1_death == 3 || p2_death == 3)
        {
            m_kof_p1death = -1;
            m_refreshrole_callback("-1");
        }
        else if (p1_death >= 0 && p1_death < 3 && m_kof_p1death != p1_death && time_data > 0 && (start == 8 || m_kof_p1death == -1) && !(first_role_index == 0 && second_role_index == 0 && third_role_index == 0)) 
        {
            m_kof_p1death = p1_death;
            u8 first_role_key = space.read_byte(0x10A7FA);
            u8 second_role_key = space.read_byte(0x10A7FB);
            u8 third_role_key = space.read_byte(0x10A7FC);
            u8 fourth_role_key = space.read_byte(0x10A7FD);
            std::vector<u8> roles = {first_role_key, second_role_key, third_role_key, fourth_role_key};
            std::vector<u8> ooa = {first_role_index, second_role_index, third_role_index};
            m_refreshrole_callback(std::to_string((roles[ooa[m_kof_p1death]])));
        }
    }  
}

//============================================================
//  kof2k1角色刷新
//============================================================
void inputmacro_manager::kof2k1_role()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);
        u8 time = space.read_byte(0x10A7D3);
        u8 role = space.read_byte(0x118259);
        u8 start = space.read_byte(0x1081E2);
        if (start > 0 && m_p1_role != role && role >=0 && role <=60)
        {
            m_p1_role = role;
            m_refreshrole_callback(std::to_string(m_p1_role));
        }
        else if (start == 0 && time <= 0 && m_p1_role != -1)
        {
            m_p1_role = -1;
            m_refreshrole_callback("-1");
        }
    }
}

//============================================================
//  kof2k2角色刷新
//============================================================
void inputmacro_manager::kof2k2_role()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);
        u8 time = space.read_byte(0x10A7D2);
        u8 role = space.read_byte(0x118259);
        u8 start = space.read_byte(0x1081E2);
        if (start > 0 && m_p1_role != role && role >=0 && role <=60)
        {
            m_p1_role = role;
            m_refreshrole_callback(std::to_string(m_p1_role));
        }
        else if (start == 0 && time <= 0 && m_p1_role != -1)
        {
            m_p1_role = -1;
            m_refreshrole_callback("-1");
        }
    }
}

//============================================================
//  kof2k3角色刷新
//============================================================
void inputmacro_manager::kof2k3_role()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);
        // p1角色地址
        u8 roleA = space.read_byte(0x107D12);
        u8 roleB = space.read_byte(0x107D13);
        u8 roleC = space.read_byte(0x107D14);
        // p1角色出场索引地址
        u8 index = space.read_byte(0x107D1A);
        
        if ((roleA == 125 && roleB == 18 && roleC == 125) || (roleA == 0 && roleB == 0 && roleC == 0) || (roleA > 35 || roleB > 35 || roleC > 35) || (roleA < 0 || roleB < 0 || roleC < 0))
        {
            if (m_p1_role != -1)
            {
                m_p1_role = -1;
                m_refreshrole_callback("-1");
            }
        }
        else
        {
            std::vector<u8> roles = {roleA, roleB, roleC};
            u8 curr_role = roles[index];
            if (curr_role != m_p1_role)
            {
                m_p1_role = curr_role;
                m_refreshrole_callback(std::to_string((roles[index])));
            }
        }
    }
}

//============================================================
//  kof2k4se角色刷新
//============================================================
void inputmacro_manager::kof2k4se_role()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);
        /*
        1.找到p1选择的3个角色地址
        2.找到p1选择的出场顺序地址
        3.找到p1，p2战胜的场数地址
        4.找到开始的内存地址
        */
        u8 p1_role1 = space.read_byte(0x10A804);
        u8 p1_role2 = space.read_byte(0x10A805);
        u8 p1_role3 = space.read_byte(0x10A806);
        u8 index1 = space.read_byte(0x10A7F8);
        u8 index2 = space.read_byte(0x10A7F9);
        u8 index3 = space.read_byte(0x10A7FA);
        u8 p1_win = space.read_byte(0x10A7F2);
        u8 p2_win = space.read_byte(0x10A7F3);
        u8 start = space.read_byte(0x1081E2);

        if ((index1 == 0 && index2 == 0 && index3 == 0) || (index1 < 0 || index2 < 0 || index3 < 0) || (index1 > 2 || index2 > 2 || index3 > 2)) 
        {
            if (m_kof_p1death != -1)
            {
                m_kof_p1death = -1;
                m_refreshrole_callback("-1");
            }
        }
        else if (p1_win == 3 || p2_win == 3)
        {
            if (m_kof_p1death != -1)
            {
                m_kof_p1death = -1;
                m_refreshrole_callback("-1");
            }
        }
        else
        {
            if ((start == 10 || m_kof_p1death == -1) && m_kof_p1death != p2_win)
            {
                m_kof_p1death = p2_win;
                std::vector<u8> roles = {p1_role1, p1_role2, p1_role3};
                std::vector<u8> ooa = {index1, index2, index3};
                m_refreshrole_callback(std::to_string((roles[ooa[m_kof_p1death]])));
            }
        }
    }
}