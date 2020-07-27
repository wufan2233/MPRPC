#include "mprpcapplication.h"

/*
 * header_size + header_str(service_name method_name args_size) + args_str
 */
// 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    // 获取service对象名和method方法名
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // 获取参数的序列化字符串和长度
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("serialize request error!");
        LOG_ERR("(mprpcchannel.cc) serialize request error!");
        return;
    }

    // 定义rpc请求头
    mprpc::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);

    // 获取rpc请求头的序列化字符串和长度
    uint32_t header_size;
    std::string rpc_header_str;
    if (rpcheader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("serialize rpc header error!");
        LOG_ERR("(mprpcchannel.cc) serialize rpc header error!");
        return;
    }

    // 组织发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    // std::cout << "*****************************************" << std::endl;
    // std::cout << "header_size: " << header_size << std::endl;
    // std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    // std::cout << "service_name: " << service_name << std::endl;
    // std::cout << "method_name: " << method_name << std::endl;
    // std::cout << "args_size: " << args_size << std::endl;
    // std::cout << "args_str: " << args_str << std::endl;
    // std::cout << "*****************************************" << std::endl;

    // 使用TCP编程，完成rpc方法的远程调用（客户端，不需要高并发）
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno:%d", errno);
        controller->SetFailed(errtxt);
        LOG_ERR("(mprpcchannel.cc) create socket error! errno:%d", errno);
        return;
    }

    // 创建并初始化InetAddress对象
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    // rpc调用方需要调用service_name的method_name方法，需要查询zk上该服务的host信息
    ZkClient zkCli;
    zkCli.Start();
    // zk中method_name的路径
    std::string method_path = "/" + service_name + "/" + method_name;
    // 获取zk中method_name的znode结点的内容
    std::string host_data = zkCli.GetData(method_path.c_str());
    if ("" == host_data)
    {
        controller->SetFailed(method_path + " is not exist!");
        LOG_ERR("(mprpcchannel.cc) %s is not exist!", method_path.c_str());
        return;
    }
    // 解析结点内容
    int idx = host_data.find(':');
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        LOG_ERR("(mprpcchannel.cc) %s address is invalid!", method_path.c_str());
        return;
    }
    // 获取解析后的内容
    std::string ip = host_data.substr(0, idx);
    uint32_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务结点
    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);

        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno:%d", errno);
        controller->SetFailed(errtxt);
        LOG_ERR("(mprpcchannel.cc) connect error! errno:%d", errno);

        return;
    }

    // 向服务端发送rpc请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(clientfd);

        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno:%d", errno);
        controller->SetFailed(errtxt);
        LOG_ERR("(mprpcchannel.cc) send error! errno:%d", errno);

        return;
    }

    // 接收rpc请求响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1023, 0)))
    {
        close(clientfd);

        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        LOG_ERR("(mprpcchannel.cc) recv error! errno:%d", errno);

        return;
    }

    // 反序列化rpc调用的响应数据
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        close(clientfd);

        char errtxt[512] = {0};
        sprintf(errtxt, "response parse error! recv_buf:%s", recv_buf);
        controller->SetFailed(errtxt);
        LOG_ERR("(mprpcchannel.cc) response parse error! recv_buf:%s", recv_buf);

        return;
    }

    close(clientfd);
}