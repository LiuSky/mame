#include "emu.h"
#include "emuopts.h"
#include <string>
#include "inputdev.h"
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <algorithm>
#include "inputmacro.h"



//============================================================
//  初始化
//============================================================
inputmacro_manager::inputmacro_manager(running_machine &machine, refreshRole_callback callback)
	: m_machine(machine),
    m_refreshrole_callback(callback),
    m_inputmacro(std::make_unique<inputmacro>()),
    m_active_macro(),
    m_ioportList(),
    m_active_inputs(),
    m_whitelist(),
    m_p1death(-1)
{
    /// 加载数据
    load_macro();
    m_whitelist = {"kof97", "kof98", "kof99", "kof2000"};

    // 添加一个每帧执行一次的回调函数
    machine.add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(&inputmacro_manager::update, this));
}

//============================================================
//  释放
//============================================================
inputmacro_manager::~inputmacro_manager()
{
    //释放不需要的参数
}

//============================================================
//  执行一键宏
//============================================================
void inputmacro_manager::execute(int player, const char *key, bool* release, execute_callback callback)
{
    // 判断方向
    bool result = left_direction(player);
    bool isExist = false;
    for (auto& role : m_inputmacro->rolelist) 
    {
        for (auto & skill : role.skilllist) 
        {
            if (skill.key == key)
            {
                isExist = true;
                if (result)
                {
                    m_active_macro.push_back(inputmacro_active(player, skill, callback, release));
                }
                else
                {
                    inputmacro_skill newskill;
                    newskill.key = skill.key;
                    newskill.frame = skill.frame;
                    newskill.loop = skill.loop;
                    newskill.step = skill.step;
                    for (auto& step : skill.steps)
                    {
                        inputmacro_step newstep;
                        newstep.delay = step.delay;
                        newstep.duration = step.duration;
                        for (auto& input : step.inputs)
                        {
                            if (input.mask == 53)
                            {
                                newstep.inputs.push_back(inputmacro_mask(54));
                            }
                            else if (input.mask == 54)
                            {
                                newstep.inputs.push_back(inputmacro_mask(53));
                            }
                            else
                            {
                                newstep.inputs.push_back(input);
                            }
                        }
                        newskill.steps.push_back(newstep);
                    }
                    m_active_macro.push_back(inputmacro_active(player, newskill, callback, release));
                }
            }
        }
    }

    if (!isExist)
    {
        callback(false, "没有匹配到该一键技能Key");
    }
}

//============================================================
//  每一帧更新的函数
//============================================================
void inputmacro_manager::update()
{
    refresh_role();
    process_frame();
}

//============================================================
//  处理游戏帧的函数
//============================================================
void inputmacro_manager::process_frame()
{
    // 变量previous_inputs被赋值为m_active_inputs，这个变量用于记录前一帧被激活的输入
    std::vector<ioport_field*> previous_inputs = m_active_inputs;
    
    // 在处理新的一帧之前，清除之前所有被激活的输入
    m_active_inputs.clear();

    //记录要移除的索引
    std::vector<decltype(m_active_macro)::iterator> to_remove;

    // 循环遍历当前待处理的输入
    for (auto macro = m_active_macro.begin(); macro != m_active_macro.end(); ++macro)
    {
        // 如果当前宏指令还没有开始
        if (macro->value.step == 0)
        {
            // 将当前宏指令的步骤设置为1
            macro->value.step = 1;
            // 将当前宏指令的计时器重置为1
            macro->value.frame = 1;
            // 获取第一步的信息
            inputmacro_step step = macro->value.steps.front();
            // 如果第一步没有延迟，则立即激活指定的输入
            if (step.delay == 0) 
            {
                // 激活指定的输入
                active_field(macro->player, step);
            }
        }
        else
        {
            // 如何当前宏正在执行当中
            // 将宏指令的计时器加1，表示当前已经过去了多少帧
            macro->value.frame++;
            // 获取当前步骤
            inputmacro_step step = macro->value.steps[macro->value.step - 1];

            // 如果当前宏指令已经执行完毕，则判断后续需要进行的操作
            if (macro->value.frame > (step.delay + step.duration))
            {
                 // 如果当前宏指令不是最后一步，则继续执行下一步
                if (macro->value.step < macro->value.steps.size())
                {
                    // 将宏指令的步骤加1
                    macro->value.step++;
                    // 重新设置宏指令的计时器
                    macro->value.frame = 1;
                    // 获取下一步的信息
                    step = macro->value.steps[macro->value.step - 1];
                }
                // 当前宏指令已经执行完毕并且绑定的按键已经被释放，则结束该宏指令(只有配置loop为0的时候才有其到效果)
                else if (!(*macro->release) && macro->value.loop == 0)
                {
                    step.inputs = std::vector<inputmacro_mask>();
                    macro->value.step = 0;
                    macro->value.frame = 0;
                    to_remove.push_back(macro);
                }
                // 如果宏指令需要循环播放，则重新开始播放
                else if (macro->value.loop > 0)
                {
                    // 将宏指令的步骤设置为loop所指定的步骤
                    macro->value.step = macro->value.loop;
                    
                    // 将宏指令的计时器重置为1
                    macro->value.frame = 1;
                }
                // 如果宏指令需要一直执行直到按键释放，则在绑定的按键已经释放时结束该宏指令
                else if (macro->value.loop < 0)
                {
                    // 将下一步的信息设置为0
                    step.inputs = std::vector<inputmacro_mask>();
                    macro->value.step = 0;
                    macro->value.frame = 0;
                    to_remove.push_back(macro);
                }
            }

            // 如果当前宏指令需要执行当前步骤的指令，并且已经过了延迟时间
            if (!step.inputs.empty() && macro->value.frame > step.delay)
            {
                //激活指定的输入
                active_field(macro->player, step);
            }
        }
    }

    // 将当前帧的激活状态标记为已激活
    for (ioport_field *field : m_active_inputs)
    {   
        // 将对应的输入字段设置为1
        field->set_value(1);
    }

    // 将上一帧被激活的输入恢复为未激活状态
    for (ioport_field *field : previous_inputs)
    {
        auto iter = std::find(m_active_inputs.begin(), m_active_inputs.end(), field);
        if (iter == m_active_inputs.end())
        {
            // 将对应的输入字段设置为0
            field->clear_value();
        }
    }

    // 遍历完成后批量删除元素
    for (auto& macro : to_remove)
    {
        macro->callback(true, string_format("机位%d:技能%s:执行完成",macro->player, macro->value.key));
        m_active_macro.erase(macro);
    }
}

//============================================================
//  添加激活输入文本
//============================================================
void inputmacro_manager::active_field(int player, inputmacro_step step)
{
    std::string playerName = "P" + std::to_string(player);
    for (ioport_field *field : m_ioportList)
    {
        if (field->name().find(playerName) != std::string::npos)
        {
            for (auto & input : step.inputs)
            {
                if (field->type() == input.mask)
                {
                    m_active_inputs.push_back(field);
                }
            }
        }
    }
}


//============================================================
//  判断人物是否是在左边
//============================================================
bool inputmacro_manager::left_direction(int player)
{
   cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
   if (main_cpu != nullptr)
   {
        inputmacro_direction_address address;
        // 是否取反(有些游戏可能只要找一个方向就够了。比如kof 系列，比如P1 在左边，那P2肯定就在右边)
        bool is_invert = false;
        if (m_inputmacro->direction_address.size() == 1 && player == 2)
        {
            address = m_inputmacro->direction_address[0];
            is_invert = true;
        }
        else
        {
            address = m_inputmacro->direction_address[player-1];
            is_invert = false;
        }

        address_space &space = main_cpu->space(AS_PROGRAM);
        u8 direction_data = space.read_byte(address.address);

        if (direction_data == address.value)
        {
            return is_invert ? false : true;
        }
        else
        {
            return is_invert ? true : false;
        }
   }
   else 
   {
      osd_printf_verbose("没有匹配到cpu_device\n");
      return false;
   }
   return false;
}

//============================================================
//  刷新用户角色
//============================================================
void inputmacro_manager::refresh_role()
{
   // 判断是否存在在白名单之内
   bool ip_exist_in_whitelist = std::find(m_whitelist.begin(), m_whitelist.end(), m_machine.basename()) != m_whitelist.end();
   if (ip_exist_in_whitelist)
   {
      if (m_machine.basename() == "kof97")
      {
        role_kof97();
      }
      else if (m_machine.basename() == "kof98")
      {
        role_kof98();
      }
      else if (m_machine.basename() == "kof99" || m_machine.basename() == "kof2000")
      {
        role_kof99And2000();
      }
   }
}

//============================================================
//  kof97角色刷新
//============================================================
void inputmacro_manager::role_kof97()
{
    cpu_device *main_cpu = downcast<cpu_device*>(m_machine.root_device().subdevice("maincpu"));
    if (main_cpu != nullptr)
    {
        address_space &space = main_cpu->space(AS_PROGRAM);
        u8 time_data = space.read_byte(0x10A83A);
        u8 p1_death = space.read_byte(0x10A865);
        u8 p2_death = space.read_byte(0x10A866);
        u8 first_role_index = space.read_byte(0x10A851);
        u8 second_role_index = space.read_byte(0x10A852);
        u8 third_role_index = space.read_byte(0x10A853);
        if (p1_death == 3 || p2_death == 3)
        {
            m_p1death = -1;
            m_refreshrole_callback("-1");
        }
        else if (p1_death >= 0 && p1_death < 3 && m_p1death != p1_death && time_data > 0 && !(first_role_index == 0 && second_role_index == 0 && third_role_index == 0)) 
        {
            m_p1death = p1_death;
            u8 first_role_key = space.read_byte(0x10A84B);
            u8 second_role_key = space.read_byte(0x10A84C);
            u8 third_role_key = space.read_byte(0x10A84D);
            std::vector<u8> roles = {first_role_key, second_role_key, third_role_key};
            std::vector<u8> ooa = {first_role_index, second_role_index, third_role_index};
            m_refreshrole_callback(std::to_string((roles[ooa[m_p1death]])));
        }
    }  
}

//============================================================
//  kof98角色刷新
//============================================================
void inputmacro_manager::role_kof98()
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
        if (p1_death == 3 || p2_death == 3)
        {
            m_p1death = -1;
            m_refreshrole_callback("-1");
        }
        else if (p1_death >= 0 && p1_death < 3 && m_p1death != p1_death && time_data > 0 && !(first_role_index == 0 && second_role_index == 0 && third_role_index == 0)) 
        {
            m_p1death = p1_death;
            u8 first_role_key = space.read_byte(0x10A84E);
            u8 second_role_key = space.read_byte(0x10A84F);
            u8 third_role_key = space.read_byte(0x10A850);
            std::vector<u8> roles = {first_role_key, second_role_key, third_role_key};
            std::vector<u8> ooa = {first_role_index, second_role_index, third_role_index};
            m_refreshrole_callback(std::to_string((roles[ooa[m_p1death]])));
        }
    }  
}

//============================================================
//  kof99和2000角色刷新
//============================================================
void inputmacro_manager::role_kof99And2000()
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

        if (p1_death == 3 || p2_death == 3)
        {
            m_p1death = -1;
            m_refreshrole_callback("-1");
        }
        else if (p1_death >= 0 && p1_death < 3 && m_p1death != p1_death && time_data > 0 && !(first_role_index == 0 && second_role_index == 0 && third_role_index == 0)) 
        {
            m_p1death = p1_death;
            u8 first_role_key = space.read_byte(0x10A7FA);
            u8 second_role_key = space.read_byte(0x10A7FB);
            u8 third_role_key = space.read_byte(0x10A7FC);
            u8 fourth_role_key = space.read_byte(0x10A7FD);
            std::vector<u8> roles = {first_role_key, second_role_key, third_role_key, fourth_role_key};
            std::vector<u8> ooa = {first_role_index, second_role_index, third_role_index};
            m_refreshrole_callback(std::to_string((roles[ooa[m_p1death]])));
        }
    }  
}



//============================================================
//  加载所有的输入端口
//============================================================
void inputmacro_manager::load_ioportField()
{
    for (auto &port : m_machine.ioport().ports())
    {
        auto it = std::find(m_inputmacro->portKeylist.begin(), m_inputmacro->portKeylist.end(), port.first);
        if (it != m_inputmacro->portKeylist.end())
        {
            for (ioport_field &field : port.second->fields())
            {
                m_ioportList.push_back(&field);
            }
        }
    }
}

//============================================================
//  刷新数据
//============================================================
void inputmacro_manager::reload()
{
    m_inputmacro = std::make_unique<inputmacro>();
    m_active_macro.clear();
    m_ioportList.clear();
    m_active_inputs.clear();
    m_p1death = -1;
    load_macro();
}


//============================================================
//  加载输入宏数据
//============================================================
void inputmacro_manager::load_macro()
{
    std::string name = m_machine.basename();
    std::string path = string_format("%s%s%s.cfg", "macro", PATH_SEPARATOR, name);
	std::ifstream ifs(path);
    if (ifs.good())
    {
        rapidjson::IStreamWrapper isw(ifs);
	    rapidjson::Document document;
	    document.ParseStream<0>(isw);
        if (document.HasParseError())
        {
            osd_printf_verbose("解析%s输入宏JSON文件失败\n", name);
        }
        else
        {
            if (document.HasMember("direction") && document["direction"].IsArray())
            {
                const auto& directions = document["direction"].GetArray();
                for(auto &direction : directions)
                {
                    inputmacro_direction_address m_direction_address;
                    m_direction_address.address = direction["address"].GetUint();
                    m_direction_address.value = direction["value"].GetInt();
                    m_inputmacro->direction_address.push_back(m_direction_address);
                }
            }

            if (document.HasMember("portList") && document["portList"].IsArray())
            {
                const auto& ports = document["portList"].GetArray();
                for (auto& port : ports)
                {
                    m_inputmacro->portKeylist.push_back(port.GetString());
                }
            }

            if (document.HasMember("roles") && document["roles"].IsArray())
            {
                const auto& roles = document["roles"].GetArray();
                for (auto& role : roles)
                {
                    inputmacro_role m_role;
                    m_role.id = role["id"].GetString();
                    m_role.address = role["address"].GetString();
                    if (role.HasMember("skills") && role["skills"].IsArray())
                    {
                        const auto& skills = role["skills"].GetArray();
                        for (auto& skill : skills)
                        {
                            inputmacro_skill m_skill;
                            m_skill.key = skill["key"].GetString();
                            m_skill.loop = skill["loop"].GetInt();
                            m_skill.step = 0;
                            m_skill.frame = 0;
                            if (skill.HasMember("steps") && skill["steps"].IsArray())
                            {
                                const auto& steps = skill["steps"].GetArray();
                                for (auto& step: steps)
                                {
                                    inputmacro_step m_step;
                                    m_step.delay = step["delay"].GetInt();
                                    m_step.duration = step["duration"].GetInt();
                                    if (step.HasMember("inputs") && step["inputs"].IsArray())
                                    {
                                        const auto& inputs = step["inputs"].GetArray();
                                        for (auto& input: inputs)
                                        {
                                            m_step.inputs.push_back(inputmacro_mask(input["mask"].GetInt()));
                                        }
                                    }
                                    m_skill.steps.push_back(m_step);
                                }
                            }
                            m_role.skilllist.push_back(m_skill);
                        }
                    }
                    m_inputmacro->rolelist.push_back(m_role);
                }
            }
            load_ioportField();
        }
    }
    else
    {
        osd_printf_verbose("不存在%s游戏的输入宏\n", name);
    }
}

