## 网络工具库
**包含Linux服务器、跨平台客户端、序列化/反序列化工具、安全加密等丰富功能的网络工具库**

### 特性

- Linux下使用C++11实现的多线程Reactor模型TCP服务器（参考[muduo](http://github.com/chenshuo/muduo)实现）
- 支持Linux及Windows平台、线程安全的TCP客户端（包括C++、Java的实现）
- 自动生成序列化/反序列化代码的消息编译器（参考protobuf部分设计），以及消息分发器
- 通过以上序列化工具，实现轻量级RPC框架
- 简易的RTSP流媒体服务器、HTTP服务器
- 基于OpenSSL实现RSA+AES混合加密传输

### Linux下编译安装
```shell
make -j
make install [PREFIX=<install_dir>]
```

### TCP客户端
Linux平台或Windows的MinGW下编译安装：
```shell
cd client/cpp
mkdir build & cd build
cmake ..
make
make install
```
### message编译器
自动生成序列化及反序列的代码

构建：
```shell
cd tools/message_compiler
g++ -O2 -Wall -o msgc *.cpp
```
使用：

` ./msgc <xxx.msg>`