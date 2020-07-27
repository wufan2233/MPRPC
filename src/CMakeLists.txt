# 如果cmake文件不做改动，则build文件夹里的Makefile也不会更新
# 当在src下添加新的文件后，若cmake没有改动，则新添加的文件不会参与编译链接
# 解决：将buile目录下的文件全部删除重新进行编译
# aux_source_directory(. SRC_LIST)
set(SRC_LIST mprpcapplication.cc 
             mprpcconfig.cc 
             mprpcprovider.cc
             rpcheader.pb.cc
             mprpcchannel.cc
             mprpccontroller.cc
             logger.cc
             zookeeperutil.cc)

# mprpc编译链接为共享库
# add_library(mprpc SHARED ${SRC_LIST})

# mprpc编译链接静态库（因为muduo库为静态库）
add_library(mprpc ${SRC_LIST})

# 为mprpc设置依赖，需要注意muduo库的连接顺序
# zookeeper_mt：多线程版本
# zookeeper_st：单线程版本
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)