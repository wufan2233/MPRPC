#include "mprpcconfig.h"


// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf) // 文件打开失败
    {
        std::cout << config_file << " is not exist." << std::endl;
        exit(EXIT_FAILURE);
    }

    // 解析文件
    while (!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf); // 从文件中读取一行

        std::string read_buf(buf); // 将字符数组类型转换为字符串

        Trim(read_buf);

        // 判断注释#和空行
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }

        // 解析注释项
        int idx = read_buf.find('=');
        if (idx == -1)
        {
            continue; // 配置不合法
        }
        
        std::string key;
        std::string value;
        // 截取key值，即 = 前面的值
        key = read_buf.substr(0, idx);
        Trim(key);
        // 截取value，并将最后的换行去掉
        // rpcserviceip=127.0.0.1\n
        int endidx = read_buf.find('\n', idx);
        value = read_buf.substr(idx + 1, endidx - idx - 1);
        Trim(value);

        // 将key-calue插入到unordered_map表中
        m_configMap.insert({key, value});
    }
}

// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    // 寻找key对应的value值
    auto it = m_configMap.find(key);
    // 没有找到返回空
    if (it == m_configMap.end())    
    {
        return "";
    }

    return it->second;
}


// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
    // 消除字符串前后的空格
    int idx = src_buf.find_first_not_of(' '); // 寻找第一个不是空格的字符下标
    if (idx != -1)
    {
        // 去掉字符串前面的空格
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    idx = src_buf.find_last_not_of(' '); // 寻找最后一个不是空格的字符下标
    if (idx != -1)
    {
        // 去掉字符串后面的空格
        src_buf = src_buf.substr(0, idx + 1);
    }
}