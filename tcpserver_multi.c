// tcpserver_multi.c - 多线程 TCP Echo 服务器，支持同时服务多个客户端
// 编译命令：gcc tcpserver_multi.c -o tcpserver_multi.exe -lws2_32
// 运行：tcpserver_multi.exe
// 测试：开多个终端同时运行 tcpclient.exe，服务器可以同时服务所有客户端

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h> // CreateThread 等线程函数需要

#pragma comment(lib, "ws2_32.lib")

// 客户端编号计数器，用于区分不同的客户端
int client_count = 0;

// ============================================================
// 线程函数：专门处理一个客户端的收发数据
// 每当 accept 接受一个新连接，就创建一个线程运行这个函数
// 这样主线程可以继续 accept，不会被某个客户端的 recv 阻塞
// ============================================================
DWORD WINAPI handle_client(LPVOID arg)
{
    // 把传入的参数转回 SOCKET 类型
    // 因为线程是异步的，必须用 malloc 分配的内存传参，不能用局部变量
    SOCKET client_fd = *(SOCKET *)arg;
    free(arg); // 用完释放 malloc 分配的内存

    int id = ++client_count; // 给这个客户端分配一个编号
    printf("[Client %d] connected.\n", id);

    char buffer[1024];

    // 循环收发数据（Echo 服务）
    while (1)
    {
        memset(buffer, 0, sizeof(buffer)); // 每次接收前清空缓冲区
        int valread = recv(client_fd, buffer, sizeof(buffer), 0);

        if (valread <= 0)
        {
            // valread <= 0 表示客户端断开连接或出错
            printf("[Client %d] disconnected.\n", id);
            break;
        }

        printf("[Client %d] Received: %s", id, buffer);

        // 把收到的数据原样回显
        send(client_fd, buffer, valread, 0);
        printf("[Client %d] Echoed back.\n", id);
    }

    // 关闭这个客户端的 socket
    closesocket(client_fd);
    printf("[Client %d] socket closed.\n", id);

    return 0;
}

int main()
{
    system("chcp 65001");

    WSADATA wsaData;               // Winsock 初始化信息结构体
    SOCKET server_fd;              // 服务器监听 socket，只负责监听和接受新连接
    struct sockaddr_in address;    // IPv4 地址结构体
    int addrlen = sizeof(address); // 地址结构体长度

    // 初始化 Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 1. 创建 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 绑定地址和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡
    address.sin_port = htons(8888);       // 端口 8888
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // 3. 开始监听
    // 队列长度改为 5，允许更多客户端同时排队等待 accept
    listen(server_fd, 5);
    printf("Multi-thread Server listening on port 8888...\n");
    printf("Supports multiple clients simultaneously.\n");
    printf("Press Ctrl+C to stop.\n\n");

    // 4. 循环接受客户端连接（核心！）
    // 和单线程版本的区别：
    //   单线程：accept 一次 → while(recv/send) → 卡住，无法再 accept
    //   多线程：accept → CreateThread → 立刻回去 accept 下一个
    //           每个客户端在独立的线程中处理，互不阻塞
    while (1)
    {
        // accept() 阻塞等待新客户端连接
        // 返回一个独立的 client_fd，专门用于和这个客户端通信
        SOCKET *client_fd = malloc(sizeof(SOCKET));
        printf("开始等待接受\n");
        *client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        printf("等待接受结束\n");
        if (*client_fd == INVALID_SOCKET)
        {
            printf("Accept failed!\n");
            free(client_fd);
            continue;
        }

        // 创建新线程来处理这个客户端
        // CreateThread 参数说明：
        //   NULL:           默认安全属性
        //   0:              默认栈大小（1MB）
        //   handle_client:  线程运行的函数
        //   client_fd:      传给线程的参数（客户端 socket 指针）
        //   0:              默认创建标志（线程立即运行）
        //   NULL:           不需要获取线程 ID
        HANDLE thread = CreateThread(NULL, 0, handle_client, client_fd, 0, NULL);

        if (thread == NULL)
        {
            printf("Failed to create thread!\n");
            closesocket(*client_fd);
            free(client_fd);
            continue;
        }

        // 关闭线程句柄（不是关闭线程！线程会继续独立运行）
        // 只是主线程不需要再管理这个句柄了
        CloseHandle(thread);
    }

    // 以下代码实际上不会执行（while 是无限循环，Ctrl+C 退出程序）
    closesocket(server_fd);
    WSACleanup();

    return 0;
}
