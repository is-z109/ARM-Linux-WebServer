# ARM Linux Web Server

一个基于 C++ 实现的 Linux / ARM Linux Web 服务器项目，使用 `epoll` 实现 I/O 多路复用，并结合线程池、HTTP 请求解析、用户登录认证、静态文件服务、GPIO 控制、UART 串口通信和 JSON 数据接口，实现 PC / 平板浏览器与嵌入式 Linux 设备之间的 Web 交互。

## 项目功能

- 基于 Linux Socket API 实现 TCP 服务器
- 使用 `epoll` + `EPOLLET`（边缘触发）处理网络连接
- 非阻塞监听 Socket
- 固定大小线程池处理客户端请求
- HTTP/1.1 请求解析
- 支持 GET / POST 请求
- 支持 `application/x-www-form-urlencoded` 表单解析
- 支持 URL 百分号编码和 `+` 空格解析
- 静态文件服务
- HTTP 400 / 401 / 404 错误响应
- 基于 Cookie 的登录状态管理
- 用户注册与登录
- 基于阻塞队列的异步日志
- 通过 Linux GPIO sysfs 接口控制 GPIO
- 通过 UART 与 STM32 通信
- 基于 `nlohmann/json` 提供 JSON 数据接口
- 硬件任务队列，串行执行 GPIO 控制任务

## 系统架构

```text
                 客户端
          PC / 平板 / 浏览器
                    |
                   HTTP
                    |
                    v
             +-------------+
             |   Socket    |
             +-------------+
                    |
                    v
             +-------------+
             |    epoll    |
             |   EPOLLET   |
             +-------------+
                    |
                    v
             +-------------+
             |    线程池    |
             +-------------+
                    |
                    v
             +-------------+
             |  HTTP解析器  |
             +-------------+
                    |
          +---------+---------+
          |         |         |
          v         v         v
        登录      静态文件     硬件
                              |
                       +------+------+
                       |             |
                      GPIO          UART
                                     |
                                     v
                                   STM32
```

## HTTP 请求处理

HTTP 请求使用状态机进行解析：

```text
请求行
  |
  v
请求头
  |
  v
请求正文
```

解析器主要处理：

- HTTP 请求行
- HTTP 请求头
- `Content-Length`
- POST 请求正文
- `application/x-www-form-urlencoded`
- URL 百分号编码
- `+` 转空格
- Cookie 请求头

服务器当前支持 HTTP/1.1 请求。

## 用户认证

服务器提供用户注册和登录功能。

### 用户登录

```text
POST /login
```

登录成功后服务器返回 Cookie：

```text
is_login=true
```

后续请求通过该 Cookie 判断用户是否处于登录状态。

### 用户注册

```text
POST /register
```

用户信息保存于：

```text
u_name.txt
```

当前认证模块主要用于项目学习和功能演示，用户名和密码采用明文保存，不具备生产环境所需的密码加密、会话管理等安全机制。

## 静态文件服务

服务器从指定 Web 根目录读取并发送文件：

```cpp
string webroot = "/home/lyra/novel/";
```

目前支持：

- HTML
- CSS
- JPEG
- 其他文件（二进制方式发送）

首页会根据 Web 根目录中的文件动态生成文件列表。

## GPIO 控制

GPIO 模块通过 Linux GPIO sysfs 接口实现 GPIO 初始化、输入输出方向设置以及电平读写。

当前代码使用的 GPIO：

```text
GPIO 0
GPIO 1
GPIO 41
```

Web 服务器提供以下硬件控制接口：

```text
GET /on
GET /off
```

GPIO 操作不会直接在 HTTP 工作线程中执行，而是提交到硬件任务队列，由独立工作线程串行处理。

## UART 串口通信

项目通过 UART 与 STM32 进行通信。

默认串口设备：

```text
/dev/ttyS1
```

串口参数：

```text
波特率：9600
数据位：8
校验位：无
停止位：1
流控：无
```

串口模块使用 128 字节接收缓冲区接收 STM32 数据。

## STM32 通信协议

项目定义了固定长度的 STM32 数据帧：

```text
+----------+----------+----------+-------------+----------+-------------+---------------+----------+
| 帧头1    | 帧头2    | 消息类型 | 温度        | 湿度     | 电机转速    | 开关状态      | 校验和   |
+----------+----------+----------+-------------+----------+-------------+---------------+----------+
|  0xAA    |  0x55    |  1字节   |   4字节     |  4字节   |   4字节     |    1字节      |  1字节   |
+----------+----------+----------+-------------+----------+-------------+---------------+----------+
```

完整数据帧长度为 17 字节。

接收模块首先搜索：

```text
0xAA 0x55
```

找到帧头后读取完整数据帧，并将接收到的 STM32 数据更新到共享数据结构中。

共享数据在访问时使用互斥锁进行保护。

## JSON 数据接口

服务器提供：

```text
GET /message
```

用于获取 STM32 采集的数据。

返回数据示例：

```json
{
  "tempre": 25.6,
  "lig": 58.3
}
```

JSON 数据使用 `nlohmann/json` 进行序列化。

## 异步日志

项目实现了基于阻塞队列的异步日志系统：

```text
HTTP 工作线程
      |
      v
   阻塞队列
      |
      v
   日志线程
      |
      v
    log.txt
```

日志主要记录：

- 请求路径
- 客户端 IP
- 请求时间
- 登录请求对应的用户名

HTTP 工作线程只负责将日志任务提交到队列，日志线程负责实际文件写入。

## 线程池

服务器启动时创建 4 个工作线程：

```cpp
threadpool pool(4);
```

客户端连接建立后，将客户端处理任务提交到线程池：

```text
accept()
   |
   v
线程池任务队列
   |
   v
工作线程
   |
   v
handleclient()
```

线程池主要使用：

- `std::thread`
- `std::queue`
- `std::mutex`
- `std::condition_variable`

实现任务调度和线程同步。

## HTTP 接口

| 方法 | 路径 | 功能 |
|---|---|---|
| `GET` | `/` | 生成并返回首页 |
| `GET` | `/login.html` | 返回登录页面 |
| `GET` | `/register.html` | 返回注册页面 |
| `POST` | `/login` | 用户登录 |
| `POST` | `/register` | 用户注册 |
| `GET` | `/on` | 打开 GPIO |
| `GET` | `/off` | 关闭 GPIO |
| `GET` | `/message` | 获取 STM32 数据 |
| `GET` | `/<file>` | 返回 Web 根目录下的文件 |

## 项目结构

```text
webserve/
├── CMakeLists.txt
├── README.md
├── .gitignore
│
├── src/
│   ├── main.cpp
│   ├── GPIO.cpp
│   ├── log.cpp
│   └── tool.cpp
│
├── include/
│   ├── GPIO.h
│   ├── Login.h
│   ├── USART.h
│   ├── httprequest.h
│   ├── log.h
│   ├── mqueue.h
│   ├── recv.h
│   ├── theardpool.h
│   ├── tool.h
│   ├── updata.h
│   └── wheard.h
│
└── third_party/
    └── json.hpp
```

## 编译

项目使用 CMake 构建，C++ 标准为 C++17。

### Linux 本机编译

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### ARM Linux 交叉编译

可以使用 ARM Linux 交叉编译工具链进行编译：

```bash
cmake .. \
    -DCMAKE_C_COMPILER=arm-none-linux-gnueabihf-gcc \
    -DCMAKE_CXX_COMPILER=arm-none-linux-gnueabihf-g++
```

具体交叉编译器需要根据目标 ARM 平台进行调整。

## 运行环境

运行服务器需要：

- Linux / ARM Linux
- 支持 C++17 的编译器
- POSIX 线程
- Linux `epoll`
- Linux GPIO 接口
- UART 串口设备
- Web 根目录
- 使用 UART 功能时需要连接 STM32

GPIO 编号、UART 设备和 Web 根目录目前直接配置在源代码中，在不同硬件平台上运行时需要根据实际环境进行修改。

## 依赖

- C++17
- CMake 3.15+
- POSIX Threads
- Linux Socket API
- `nlohmann/json`

## 许可证

本项目主要用于个人学习、课程实践以及项目展示。
