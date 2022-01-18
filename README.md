
C++11实现的Reactor模型网络库（参考[muduo](http://github.com/chenshuo/muduo)实现）

服务端为Linux C++

客户端支持Linux及Windows（包括C++，Java的实现）

特性：
- 消息编解码器，序列化及反序列化（参考protobuf设计）
- 轻量级RPC框架
- 简易的RTSP流媒体服务器、HTTP服务器


### Linux下编译安装
```shell
make -j
make install [PREFIX=<install_dir>]
```

### 跨平台的网络客户端
编译安装：
```shell
cd client/cpp
mkdir build & cd build
cmake ..
```
若为Linux平台或Windows下MinGW：
```shell
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