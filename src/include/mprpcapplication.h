#pragma once
#include <iostream>
#include <unistd.h>

#include "mprpcconfig.h"
#include "mprpccontroller.h"
#include "mprpcchannel.h"
#include "mprpcprovider.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"


// mprpc框架的基础类（单例）负责框架的一些初始化操作
class MprpcApplication
{
public:
    // 初始化框架
    static void Init(int argc, char **argv);
    // 返回一个静态对象
    static MprpcApplication& GetInstance();
    // 返回解析加载文件的类对象m_config
    static MprpcConfig& GetConfig();

private:
    static MprpcConfig m_config;    // 解析加载文件的类

    // 单例模式
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(MprpcApplication &&) = delete;
};