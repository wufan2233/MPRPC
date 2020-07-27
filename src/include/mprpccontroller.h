#pragma once

#include <string>
#include <google/protobuf/service.h>


class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();
    void Reset();                               // 重置函数
    bool Failed() const;                        // 执行出现问题
    std::string ErrorText() const;              // 错误信息
    void SetFailed(const std::string& reason);  // 执行发生错误

    // 未实现功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool m_failed;          // RPC方法执行过程中的状态
    std::string m_errText;  // RPC方法执行过程中的状态信息
};