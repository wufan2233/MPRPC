#include "mprpcapplication.h"


void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT) // 回调消息类型，和会话相关
    {
        if (state == ZOO_CONNECTED_STATE) // zkclient与zkserver连接成功
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        // 关闭句柄，释放资源
        zookeeper_close(m_zhandle);
    }
}

// zkclient启动连接zkserver
void ZkClient::Start()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string host = ip + ":" + port;

    /**
     * zookeeper_mt：多线程版本
     * zookeeper的API客户端程序提供了三个线程：
     *      API调用线程、网络I/O线程(pthreat_creat  poll)、watcher回调线程(pthreat_creat)
     * 
     * zookeeper_init方法创建一个新的句柄和与该句柄相对应的Zookeeper会话。会话建立是异步的，
     * 这意味着直到（并且除非）收到状态Z00_CONNECTED_STATE事件，才应将会话视为已建立
     * host的格式："127.0.0.1:3000"
     * global_watcher：回调函数，由watcher回调线程调用
     */
    m_zhandle = zookeeper_init(host.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle)
    {
        std::cout << "zookeeper_init error" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 创建一个信号量，初始为0
    sem_t sem;
    sem_init(&sem, 0, 0);
    // 给句柄设置一个信号量
    zoo_set_context(m_zhandle, &sem);

    // 阻塞，等待zkserver响应（回调函数被调用），当与zkserver连接成功后继续运行
    sem_wait(&sem);
    std::cout << "zookeeper_init success" << std::endl;
}

// 在zkserver上根据path创建znode结点，data为结点数据，state是结点类型（0为永久性结点）
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    // zoo_create要求的参数
    char path_buffer[128];
    int path_buffer_len = sizeof(path_buffer);

    // 判断path表示的结点是否存在，如果存在，则不再创建
    int flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (flag == ZNONODE) // znode结点不存在
    {
        // 创建znode结点
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, path_buffer_len);
        if (ZOK == flag) // 成功
        {
            std::cout << "znode create success, path: " << path << std::endl;
        }
        else
        {
            std::cout << "flag: " << flag << std::endl;
            std::cout << "znode create error, path: " << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据参数指定的znode结点路径，获取结点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int buffer_len = sizeof(buffer);

    // 获取path的值
    int flag = zoo_get(m_zhandle, path, 0, buffer, &buffer_len, nullptr);
    if (flag != ZOK)
    {
        // 获取失败
        std::cout << "get znode error, path: " << path << std::endl;
        return "";
    }
    else
    {
        // 获取成功
        return buffer;
    }
    
}