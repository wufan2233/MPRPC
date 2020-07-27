# MPRPC
该项目RPC分布式网络通信框架项目，基于muduo库以及protobuf开发
|目录名称| 功能 |
|--|--|
| bin | 可执行文件 |
| build |项目编译文件  |
| example |框架代码使用范例  |
| lib | 项目库文件 |
| src | 源文件 |
| test/protobuf | protobuf 测试代码 |
| CMakeLists.txt | 顶层的cmake文件 |
| autobuild.sh| 一键编译脚本 |

在执行编译脚本之前，需要先启动zookeeper服务器：./zkServer.sh start
