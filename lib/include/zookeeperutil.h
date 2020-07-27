#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>
#include <iostream>


// 封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient启动连接zkserver
    void Start();
    // 在zkserver上根据path创建znode结点，data为结点数据，state是结点类型（0为永久性结点）
    void Create(const char *path, const char *data, int datalen, int state = 0);
    // 根据参数指定的znode结点路径，获取结点的值
    std::string GetData(const char *path);

private:
    // zk的客户端句柄
    zhandle_t *m_zhandle;
};