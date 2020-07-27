#include <iostream>
#include <string>
#include <vector>

#include "friend.pb.h"
#include "mprpcapplication.h"


class FriendService : public fixbug::FriendServiceRpc
{
public:
    // 本地的GetFriendsList方法
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::vector<std::string> vec;
        vec.push_back("zhang san");
        vec.push_back("li si");
        vec.push_back("wang wu");

        std::cout << "FriendsList Service: " << std::endl;
        std::cout << "userid: " << userid << std::endl;

        return vec;
    }

    // 重写基类方法，由框架调用
    virtual void GetFriendsList(::google::protobuf::RpcController *controller,
                                const ::fixbug::GetFriendsListRequest *request,
                                ::fixbug::GetFriendsListResponse *response,
                                ::google::protobuf::Closure *done)
    {
        // 取出请求数据
        uint32_t userid = request->userid();

        // 执行本地的业务
        std::vector<std::string> friendlist = GetFriendsList(userid);

        // 向响应response中写入数据
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        // for (int i = 0; i < friendlist.size(); i++)
        // {
        //     // 向列表里面添加数据
        //     response->add_friends(friendlist[i].c_str());
        // }

        for (std::string &name : friendlist)
        {
            std::string *p = response->add_friends();
            *p = name;
        }
        
        // 执行回调
        done->Run();
    }
};


int main(int argc, char **argv)
{
    // 日志信息
    LOG_INFO("first log message!");
    // LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    // 初始化框架
    MprpcApplication::Init(argc, argv);

    // 把FriendService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点
    // Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}