#include "mprpcapplication.h"

// 对解析加载文件的类进行初始化
MprpcConfig MprpcApplication::m_config;

// 给用户提示输入格式
void ShowArgsHelp()
{
    std::cout << "format: command -i <configfile>" << std::endl;
}

// 框架初始化
void MprpcApplication::Init(int argc, char **argv)
{
    // 没有传参，报错
    if (argc < 2)
    {
        ShowArgsHelp(); // 给用户提示输入格式
        exit(EXIT_FAILURE);
    }

    // 通过getopt来处理参数
    // getopt详解见 https://www.cnblogs.com/qingergege/p/5914218.html
    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        // 匹配到选项i
        case 'i':
            config_file = optarg;   // 记录配置文件名称
            break;
        // 没有选项i
        case '?':
            // std::cout<<"invalid args"<<std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        // 选项i后面没有参数
        case ':':
            // std::cout<<"need <configfile>"<<std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 加载配置文件
    // 配置文件内容：rpcserver_ip=  rpcserver_port=  zookeeper_ip=  zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());
    
    // 测试代码
    // std::cout << "rpcserverip: " << m_config.Load("rpccserverip") << std::endl;
    // std::cout << "rpcserverport: " << m_config.Load("rpcserverport") << std::endl;
    // std::cout << "zookeeperip: " << m_config.Load("zookeeperip") << std::endl;
    // std::cout << "zookeeperport: " << m_config.Load("zookeeperport") << std::endl;
}

// 返回一个静态对象
MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

// 返回解析加载文件的类对象m_config
MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}