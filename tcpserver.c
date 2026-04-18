// tcpserver.c - 在 Windows 上用 MinGW 编译运行
// 编译命令：gcc tcpserver.c -o tcpserver.exe -lws2_32
// 运行：tcpserver.exe

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
    system("chcp 65001");
    WSADATA wsaData;                      // Winsock 初始化信息结构体，存放 Winsock 库的版本、状态等信息，WSAStartup 需要它
    SOCKET server_fd, client_fd;           // server_fd: 服务器监听 socket，负责监听和接受新连接; client_fd: 客户端通信 socket，负责和已连接的客户端收发数据
    struct sockaddr_in address;            // IPv4 地址结构体，用于指定服务器的 IP 地址和端口号（sin_family, sin_port, sin_addr）
    int addrlen = sizeof(address);         // 地址结构体的长度，accept() 需要知道地址结构体有多大
    char buffer[1024] = {0};               // 接收数据的缓冲区，初始化为全0，用于存放客户端发来的数据

    // 初始化 Winsock
    // MAKEWORD(2,2) 请求 Winsock 2.2 版本，&wsaData 接收库的详细信息
    // Windows 下使用 socket 前必须调用，否则所有 socket 函数都会失败
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 1. 创建 socket
    // AF_INET: IPv4 协议族
    // SOCK_STREAM: TCP 流式套接字（可靠、有序、面向连接）
    // 0: 自动选择对应协议（TCP）
    // 返回一个 socket 描述符，后续所有操作都通过这个描述符
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 绑定地址和端口
    address.sin_family = AF_INET;           // 地址族：IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // 监听所有网卡（0.0.0.0），即本机所有 IP 地址都可以连接
    address.sin_port = htons(8888);         // 端口 8888，htons() 将主机字节序(小端)转为网络字节序(大端)
    // bind() 将 socket 和指定的地址+端口绑定，告诉系统"这个 socket 监听 8888 端口"
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // 3. 开始监听
    // 第二个参数 3 是"等待队列"的最大长度：
    // 当服务器正在处理一个客户端时，新来的连接会排进这个队列
    // 队列满了(3个)后，再来的连接会被拒绝
    // 注意：3 不是"最多连接3个客户端"，而是"最多3个连接排队等待 accept"
    listen(server_fd, 3);
    printf("Server listening on port 8888...\n");

    // 4. 接受客户端连接
    // accept() 是阻塞函数：没有客户端连接时会一直等待（程序停在这一行）
    // 当有客户端连接时，返回一个新的 client_fd 专门用于和这个客户端通信
    // server_fd 继续负责监听，client_fd 负责数据收发
    // &address 和 &addrlen 会填入客户端的地址信息（IP、端口）
    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    printf("Client connected!\n");

    // 5. 循环收发数据（Echo 服务）
    while (1)
    {
        memset(buffer, 0, sizeof(buffer)); // 每次接收前清空缓冲区，避免上次的数据残留
        // recv() 从 client_fd 接收数据，存入 buffer
        // 返回值 valread: >0 表示收到的字节数, =0 表示客户端主动断开, <0 表示出错
        int valread = recv(client_fd, buffer, sizeof(buffer), 0);

        if (valread <= 0)
        { // valread<=0 表示客户端断开连接或发生错误
            printf("Client disconnected\n");
            break;
        }

        printf("Received: %s", buffer); // 注意:%s不自带换行，数据以\0截断

        // 把收到的数据原样发回去（Echo 服务）
        // send() 的第三个参数用 valread 而不是 strlen(buffer)
        // 因为收到的数据可能包含非字符串内容，用实际接收长度更准确
        send(client_fd, buffer, valread, 0);
        printf("Echoed back\n");
    }

    // 6. 关闭连接
    closesocket(client_fd);  // 先关闭客户端通信 socket
    closesocket(server_fd);  // 再关闭服务器监听 socket
    WSACleanup();            // 清理 Winsock 资源，释放库占用的内存

    return 0;
}