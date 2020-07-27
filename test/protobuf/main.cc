#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

/*
 * 数据类型示例
 */
#if 0
int main()
{
    // 封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zhang san");      // 设置用户名
    req.set_pwd("123456");          // 设置密码

    // 数据对象序列化  ->  char *
    std::string send_str;
    if (req.SerializeToString(&send_str))   // 将数据转化为字符串
    {
        std::cout << send_str << std::endl;
    }

    // 从send_str反序列化一个login请求对象
    LoginRequest reqB;
    if (reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}
#endif

/*
 * 对象类型使用示例
 */
#if 0
/*
 * 对LoginResponse重新进行了定义，对errcode和errmsg进行了封装
 * 封装后LoginResponse的成员由 errcode和errmsg 变为 ResultCode类型的result
 * 改变后的使用方法如下所示
 */
int main()
{
    LoginResponse rsp;
    ResultCode *rc = rsp.mutable_result(); // 得到rsp的ResultCode类，进而进行初始化
    rc->set_errcode(1);
    rc->set_errmsg("登录失败");

    ResultCode sp = rsp.result();
    std::cout << sp.errcode() << std::endl;

    return 0;
}
#endif

/*
 * 列表类型示例
 */
#if 0
int main()
{
    GetFriendListsResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);

    // 对列表类型的使用
    User *user1 = rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(21);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("li si");
    user2->set_age(21);
    user2->set_sex(User::MAN);

    // 显示列表大小
    int size = rsp.friend_list_size();
    std::cout << size << std::endl;

    User user_tmp;
    // 显示列表内容
    for (int i = 0; i < size; i++)
    {
        user_tmp = rsp.friend_list(i);
        std::cout << user_tmp.name() << std::endl;
    }
    
    return 0;
}
#endif