#pragma once
#include <string>
#include <iostream>
#include <unordered_map>


class MprpcConfig
{
public:
    // 负责解析加载配置文件
    void LoadConfigFile(const char *config_file);

    // 查询配置项信息
    std::string Load(const std::string &key);

private:
    /*
     * 不需要考虑线程安全，因为框架只需要初始化一次，为防止框架多次初始化
     * 可以在MprpcApplication类添加一个静态成员记录初始化次数
     */
    std::unordered_map<std::string, std::string> m_configMap;

    // 去掉字符串前后的空格
    void Trim(std::string &src_buf);
};