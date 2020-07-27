#include "mprpcapplication.h"

/* 
 * 框架给外部提供的可以发布rpc方法的类
 * 需要生成一张表，记录服务对象和其发布的所有的服务方法，这样当接收到rpc调用请求时，rpc才会
 * 知道需要调用哪个服务对象的什么方法
 * 参数Service类是protobuf中service的基类
 * 
 * 实现：Protobuf ==> Service类描述对象，Method类描述方法
 * 
 * service_name => service描述
 *                      => service* 记录服务对象
 *                              method_name => method方法对象
 */
void RpcProvider::NotifyService(::google::protobuf::Service *service)
{
    ServiceInfo service_info;

    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象service的方法数量
    int methodCnt = pserviceDesc->method_count();

    // std::cout << "service name: " << service_name << std::endl;
    LOG_INFO("service name: %s", service_name.c_str());

    for (int i = 0; i < methodCnt; i++)
    {
        // 获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        // std::cout << "method name: " << method_name << std::endl;
        LOG_INFO("method name: %s", method_name.c_str());
    }
    service_info.m_service = service;

    m_serviceMap.insert({service_name, service_info});
}

// muduo库的好处：分离了网络代码和业务代码
// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    // 创建并初始化InetAddress对象
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    // 绑定用户连接的创建和断开回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    // 绑定消息读写回调
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));

    // 设置muduo库的线程数量（做好和内核数量保持一致）
    server.setThreadNum(4);

    // 将当前rpc结点上要发布的服务都注册到zk上，使得zkclient可以从zk上发现服务
    // session timeout: 30s  zkclient的网络I/O线程每隔1/3session timeout发生ping消息（心跳消息）
    ZkClient zkCli;
    zkCli.Start();  // 启动zk客户端
    // 将rpc结点上发布的服务类创建为zk的永久性结点
    for (auto &sp : m_serviceMap)
    {
        // /service_name：创建结点的路径
        std::string service_path = "/"+sp.first;
        // 创建的是永久性结点
        zkCli.Create(service_path.c_str(), nullptr, 0);
        // 将rpc结点上发布的服务类的方法创建为zk的临时性结点
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name：创建结点的路径
            std::string method_path = service_path + "/" + mp.first;
            // 结点中存放的数据：ip:port
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性结点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // rpc服务端启动，打印信息
    std::cout << "RpcProvider start server at ip " << ip << " port " << port << std::endl;

    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 和rpc client的连接断开了
        conn->shutdown();
    }
}

/*
 * 已建立连接用户的读写事件回调
 * 如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
 * 
 * 在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
 * service_name method_name args
 *      定义proto的message类型，进行数据头的序列化和反序列化
 *          service_name method_name args_size(防止tcp数据粘包)
 * 
 *  header_size + header_str(service_name method_name args_size) + args_str 
 *      (eg: 16UserServiceLoginzhang san123456)
 * 
 * 关于header_size：不知道读取多少个字节（如header_size分别为 "10"  "100"  "1000"）
 *      将header_size按照二进制（内存）的方式存储在字符串固定的前四个字节
 *      string的insert和copy方法
 */

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, // 连接
                            muduo::net::Buffer *buffer,               // 缓冲区
                            muduo::Timestamp time)                    // 接收到数据的时间信息
{
    // 网络上接收的远程rpc调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前四个字节的内容（header_size）
    uint32_t header_size;
    recv_buf.copy((char *)&header_size, 4, 0); // 从0开始读取前4个字节写入header_size

    // 根据header_size读取数据头的原始字符流
    std::string rpc_header_str = recv_buf.substr(4, header_size);

    // 反序列化数据，得到rpc请求的详细信息
    mprpc::RpcHeader rpcheader; // 定义消息对象
    std::string service_name;   // 服务对象名称
    std::string method_name;    // 方法名称
    uint32_t args_size;         // 参数的长度，防止tcp数据粘包

    if (rpcheader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcheader.service_name();
        method_name = rpcheader.method_name();
        args_size = rpcheader.args_size();
    }
    else
    {
        //  数据头反序列化失败
        std::cout << "rpc_header_str: " << rpc_header_str << " parse error" << std::endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "=========================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "=========================================" << std::endl;

    // 获取service对象和method对象
    auto sit = m_serviceMap.find(service_name);
    if (sit == m_serviceMap.end())
    {
        // 没有找到service对象
        std::cout << service_name << " is not exits!" << std::endl;
        return;
    }

    ServiceInfo sinfo = sit->second;
    auto mit = sinfo.m_methodMap.find(method_name);
    if (mit == sinfo.m_methodMap.end())
    {
        // 没有找到method对象
        std::cout << service_name << ":" << method_name << " is not exits!" << std::endl;
        return;
    }

    google::protobuf::Service *service = sinfo.m_service;           // service对象
    const google::protobuf::MethodDescriptor *method = mit->second; // method对象

    // 生成rpc方法调用的请求requset和相应response参数
    google::protobuf::Message *requset = service->GetRequestPrototype(method).New();
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 对请求参数进行反序列化
    if (!requset->ParseFromString(args_str))
    {
        // 反序列化失败
        std::cout << "requset parse error, content: " << args_str << std::endl;
        return;
    }

    // 给下面method方法的调用，绑定一个Closure的回调函数
    // 会重写done->Run()，在Run函数内调用绑定的回调函数
    google::protobuf::Closure *done =
        google::protobuf::NewCallback<RpcProvider,
                                      const muduo::net::TcpConnectionPtr &,
                                      google::protobuf::Message *>(this,
                                                                   &RpcProvider::SendRpcResponse,
                                                                   conn,
                                                                   response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    service->CallMethod(method, nullptr, requset, response, done);
}

// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn,
                                  google::protobuf::Message *response)
{
    std::string response_str;
    // response的序列化
    if (response->SerializeToString(&response_str))
    {
        // 序列化成功后，将rpc方法的调用结果通过网络发送给rpc调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "response serialize error!" << std::endl;
    }

    // 模拟http短链接服务，由服务提供方主动断开
    conn->shutdown();
}