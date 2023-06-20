#ifndef __INPUTMACRO_H__
#define __INPUTMACRO_H__

#include <stdio.h>
#include <string>
#include <vector>
#include "machine.h"

/// @brief 执行一键技能结果返回
typedef std::function<void(bool, std::string)> execute_callback;
/// @brief 刷新角色回调
typedef std::function<void(std::string)> refreshRole_callback;

/// @brief 输入宏端口值
struct inputmacro_mask
{
    /// @brief 上下左右ABCDEF对应的端口的值(up:51,down:52,left:53,right:54,a:64,b:65,c:66,d:67,e:68,f:69)
    int mask;

    inputmacro_mask(int p)
        : mask(p)
    {
    }
};

/// @brief 输入宏步骤
struct inputmacro_step
{
    /// @brief 延迟(按照帧数来算)
    int delay;

    /// @brief 持续时长(按照帧数来算)
    int duration;

    /// @brief 输入端口值
    std::vector<inputmacro_mask> inputs;
};

/// @brief 输入宏技能结构体
struct inputmacro_skill
{
    /// @brief key
    std::string key;

    /// @brief 循环(1:马上释放技能。2:为以用户为基准，按住按键就不放，松开则释放，适用一些大招。 3:turbo半自动连发功能。4:autofire自动连发功能)
    int type;

    /// @brief 技能步骤
    std::vector<inputmacro_step> steps;

    /// @brief 当前执行到的步骤，如果宏指令没有正在执行，则为nil（整数或nil）
    int step;

    /// @brief 当前步骤执行的帧数，从1开始计数（整数）
    int frame;
};

/// @brief 输入宏角色结构体
struct inputmacro_role
{
    /// @brief id
    std::string id;

    /// @brief 地址 
    std::string address;

    /// @brief 技能列表
    std::vector<inputmacro_skill> skilllist;
};

/// @brief 输入宏方向地址
struct inputmacro_direction_address
{
    /// @brief 方向地址
    u32 address;

    /// @brief 具体的值(代码只从左边开始判断)
    int value;
};

/// @brief 输入宏对象
struct inputmacro
{
    /// @brief 方向地址
    std::vector<inputmacro_direction_address> direction_address;

    /// @brief 角色列表
    std::vector<inputmacro_role> rolelist;
};

/// @brief 输入宏活跃结构体
struct inputmacro_active
{
    /// @brief 机位
    int player;

    /// @brief 技能对象
    inputmacro_skill value;

    /// @brief 回调函数
    execute_callback callback;

    /// @brief 绑定的按键是否放开
    bool* release;

    inputmacro_active(int p, const inputmacro_skill& v, const execute_callback& cb, bool* r = nullptr)
        : player(p), value(v), callback(cb), release(r)
    {
        // 默认构造函数体
    }
};


/// @brief 输入宏管理类
class inputmacro_manager
{

public:
	/// @brief 构造函数
	/// @param machine 运行的machine
	/// @param callback 角色刷新回调
	inputmacro_manager(running_machine &machine, refreshRole_callback callback);
	
	/// @brief 析构函数
	~inputmacro_manager();

    /// @brief 重新刷新数据
    void reload();

   /// @brief 是否有宏正在执行当中
   /// @return bool (true: 正在执行，false: 没有正在执行)
   bool is_macro_executing() const { return !m_active_macro.empty(); }

   /// @brief 执行
   /// @param player 机位(P1, P2, P3, P4)
   /// @param key key
   /// @param release 按住按键是否可以释放
   /// @param callback (true: 匹配到一键技能的key，并且执行完成, false: 没有匹配到该技能)
   void execute(int player, const char *key, bool* release, execute_callback callback);

private:

   /// @brief 加载输入宏数据 
   void load_macro();

   /// @brief 每一帧更新
   void update();

   /// @brief 处理游戏帧的函数
   void process_frame();

   /// @brief 加载输入文本
   void load_ioportField();

   /// @brief 激活输入
   /// @param player 机位
   /// @param step   步骤
   void active_field(int player, inputmacro_step step);

   /// @brief 判断人物是否在左边
   /// @param player 机位
   /// @return 是否是在左边
   bool left_direction(int player);
   
   /// @brief 刷新角色
   void refresh_role();

   /// @brief kof角色刷新
   /// @param game_name 
   void kof_role_update(std::string &game_name);
   void kof97_role();
   void kof98_role();
   void kof99And2k_role();
   void kof2k1_role();
   void kof2k2_role();
   void kof2k3_role();
   void kof2k4se_role();

   /// @brief  街霸角色刷新
   /// @param game_name 
   void sf_role_update(std::string &game_name);
   void sf2_role();
   void sf2ce_role();


   /// @brief 运行的机器
   running_machine &                                m_machine;
   /// @brief 刷新角色callback
   refreshRole_callback                             m_refreshrole_callback;
   /// @brief 输入宏数据
   std::unique_ptr<inputmacro>                      m_inputmacro;
   /// @brief 活跃的宏(正准备执行的或者正在执行的宏)
   std::vector<inputmacro_active>                   m_active_macro;
   /// @brief 当前游戏输入端口列表
   std::vector<ioport_field *>                      m_ioportList;
   /// @brief 当前活跃的端口列表
   std::vector<ioport_field *>                      m_active_inputs;
   /// @brief 角色刷新白名单游戏
   std::vector<std::string>                         m_whitelist;
   /// @brief kof p1死亡人数
   int                                              m_kof_p1death;
   /// @brief p1角色
   int                                              m_p1_role;
   /// @brief 帧数
   int                                              m_count_frame;
};


#endif