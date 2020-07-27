#include <iostream>

#include "mprpcapplication.h"
#include "friend.pb.h"

int main(int argc, char **argv)
{
    // 框架的初始化
    MprpcApplication::Init(argc, argv);

    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    // rpc方法的请求参数和响应参数
    fixbug::GetFriendsListRequest request;
    request.set_userid(1234);
    fixbug::GetFriendsListResponse response;

    // 定义一个controller对象，获取rpc方法调用信息
    MprpcController controller;

    // 调用远程rpc方法GetFriendsList
    stub.GetFriendsList(&controller, &request, &response, nullptr);
    // 一次rpc调用完成，读调用的结果
    if (controller.Failed())
    {
        // rpc调用失败
        // std::cout << controller.ErrorText() << std::endl;
        LOG_ERR("(callfriendservice.cc) %s", controller.ErrorText().c_str());
    }
    else
    {
        if (response.result().errcode() == 0)
        {
            // 调用成功
            std::cout << "rpc GetFriendsList response friends: " << std::endl;
            // auto it = response.friends().begin();
            // for (; it != response.friends().end(); ++it)
            // {
            //     std::cout << it->c_str() << std::endl;
            // }

            int friend_size = response.friends_size();
            for (int i = 0; i < friend_size; i++)
            {
                std::cout << "index: " << i << "friend name: " << response.friends(i) << std::endl;
            }
        }
        else
        {
            // 调用失败
            // std::cout << "rpc GetFriendsList response error: " << response.result().errmsg() << std::endl;
            LOG_ERR("(callfriendservice.cc) rpc GetFriendsList response error: %s", response.result().errmsg().c_str());
        }
    }

    return 0;
}