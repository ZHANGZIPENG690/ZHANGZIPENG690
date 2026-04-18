// tcpclient_baidu.c - 连接百度服务器，发送 HTTP 请求并接收响应
// 编译：gcc tcpclient_baidu.c -o tcpclient_baidu.exe -lws2_32
// 运行：tcpclient_baidu.exe

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

int main()
{
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char send_buf[512];
    char recv_buf[4096];

    system("chcp 65001");

    // 初始化 Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 创建 socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // 配置百度服务器地址
    // 百度的 IP 地址是 110.242.68.66（可通过 ping www.baidu.com 获取）
    // HTTP 服务端口是 80
    // 用域名代替硬编码 IP
    struct hostent *host = gethostbyname("www.baidu.com");
    server_addr.sin_addr.s_addr = *(u_long *)host->h_addr_list[0];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80); // HTTP 标准端口 80
    // server_addr.sin_addr.s_addr = inet_addr("153.3.238.127"); // 百度服务器 IP（ping www.baidu.com 获取）
    printf("host->h_addr_list[0]: %s\n", inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));
    // 连接百度服务器
    // printf("Connecting to Baidu server (153.3.238.127:80)...\n");
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        printf("Connection failed! Error code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Connected to Baidu!\n\n");

    // 构造 HTTP GET 请求
    // HTTP 请求格式：
    //   请求行：GET / HTTP/1.1\r\n
    //   请求头：Host: www.baidu.com\r\n
    //           Connection: close\r\n     （告诉服务器发完响应就关闭连接）
    //   空行：  \r\n                      （空行表示请求头结束）
    sprintf(send_buf,
            "GET / HTTP/1.1\r\n"      // 请求方法 GET，路径 /，协议版本 HTTP/1.1
            "Host: www.baidu.com\r\n" // 必须告诉服务器你要访问哪个域名
            "Connection: close\r\n"   // 短连接：服务器发完响应就断开
            "\r\n");                  // 空行，表示请求头结束，后面没有请求体

    // 发送 HTTP 请求
    int send_len = send(sock, send_buf, strlen(send_buf), 0);
    printf("Sent HTTP request (%d bytes):\n%s\n", send_len, send_buf);

    // 接收百度服务器的 HTTP 响应
    // HTTP 响应可能很大（HTML 页面），所以需要循环接收
    printf("=== Response Start ===\n");
    int total_recv = 0;
    while (1)
    {
        memset(recv_buf, 0, sizeof(recv_buf));
        int len = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);

        if (len > 0)
        {
            recv_buf[len] = '\0';
            printf("%s", recv_buf); // 打印收到的内容
            total_recv += len;
        }
        else if (len == 0)
        {
            // 服务器主动关闭连接，说明响应发完了（因为我们设置了 Connection: close）
            break;
        }
        else
        {
            printf("\nRecv error!\n");
            break;
        }
    }
    printf("\n=== Response End (total %d bytes) ===\n", total_recv);

    // 关闭连接
    closesocket(sock);
    WSACleanup();

    return 0;
}
