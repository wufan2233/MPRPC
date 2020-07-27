#include "mprpccontroller.h"

// 构造函数
MprpcController::MprpcController()
{
    m_failed = false;
    m_errText = "";
}

// 重置函数
void MprpcController::Reset()
{
    m_failed = false;
    m_errText = "";
}

// 执行出现问题
bool MprpcController::Failed() const
{
    return m_failed;
}

// 错误信息
std::string MprpcController::ErrorText() const
{
    return m_errText;
}

// 执行发生错误
void MprpcController::SetFailed(const std::string &reason)
{
    m_failed = true;
    m_errText = reason;
}

void MprpcController::StartCancel() {}
bool MprpcController::IsCanceled() const { return false; }
void MprpcController::NotifyOnCancel(google::protobuf::Closure *callback) {}