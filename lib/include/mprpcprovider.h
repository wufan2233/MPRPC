#pragma once
#include <string>
#include <functional>
#include <unordered_map>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>


// 框架提供的专门发布rpc服务的网络对象类
class RpcProvider
{
public:
    // 框架给外部提供的可以发布rpc方法的类
    void NotifyService(::google::protobuf::Service *service);

    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
    // 组合了EventLoop
    muduo::net::EventLoop m_eventLoop;

    // 服务类型信息
    struct ServiceInfo
    {
        // 保存服务对象
        google::protobuf::Service *m_service;
        // 保存服务方法信息(方法名, 方法描述)
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    // 存储注册成功的服务对象和其服务方法的所有信息(对象名, 对象描述)
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // 新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    // 已建立连接用户的读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr &, // 连接
                   muduo::net::Buffer *,                 // 缓冲区
                   muduo::Timestamp );                   // 接收到数据的时间信息
    // Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};