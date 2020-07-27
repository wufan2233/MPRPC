#include <iostream>
#include <string>

#include "user.pb.h"
#include "mprpcapplication.h"

/*
 * 在业务层将本地服务变为RPC服务
 * 1. 定义proto文件（Request, Response, Service）
 * 2. 继承rpc方法类（提供方：UserServiceRpc，调用方：UserServiceRpc_Stub），重写Login方法
 */

/*
 * UserService是一个本地服务，提供了两个进程内的本地方法Login和GetFriendLists
 */
class UserService : public fixbug::UserServiceRpc // rpc服务发布端
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "Loding ..." << std::endl;
        std::cout << "name: " << name << "pwd: " << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "Register ..." << std::endl;
        std::cout << "id: " << id << "name: " << name << "pwd: " << pwd << std::endl;
        return true;
    }

    /*
     * 写基类UserServiceRpc的虚函数，下面这些方法都是框架直接调用的
     * 1. caller发送rpc请求，rpc框架接收到rpc请求
     * 2. rpc框架匹配到Login方法
     * 3. 接收到请求后，从request拿出相应数据，做本地业务，填相应的响应response，再执行回调done
     */
    virtual void Login(::google::protobuf::RpcController *controller,
                       const ::fixbug::LoginRequest *request,
                       ::fixbug::LoginResponse *response,
                       ::google::protobuf::Closure *done)
    {
        /*
         * 1. 从 LoginRequest，获取参数的值
         * 2. 执行本地服务Login，并获取返回值
         * 3. 用上面的返回值填写 LoginResponse
         * 4. 一个回调，把LoginResponse发送给 rpc client
         */

        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 本地业务
        bool login_result = Login(name, pwd);

        // 把响应写入，包括错误码、错误消息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 执行回调操作，执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }

    virtual void Register(::google::protobuf::RpcController *controller,
                          const ::fixbug::RegisterRequest *request,
                          ::fixbug::RegisterResponse *response,
                          ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数RegisterRequest，应用获取相应数据做本地业务
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 本地业务
        bool register_result = Register(id, name, pwd);
        
        // 把响应写入，包括错误码、错误消息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(register_result);

        // 执行回调操作，执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }
};

// 框架的使用
int main(int argc, char **argv)
{
    // 调用框架的初始化操作
    // 用户传入的是命令行参数：provider -i config.conf
    // 从配置文件中读取相关网络服务器的IP和Port
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象
    // 把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点
    // Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}