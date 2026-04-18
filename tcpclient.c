// tcpclient_loop.c - 定时发送，不自动退出
// 编译：gcc tcpclient_loop.c -o tcpclient_loop.exe -lws2_32
// 运行：tcpclient_loop.exe

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>  // for Sleep

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char send_buf[256];
    char recv_buf[128];
    int count = 0;
    
    // 初始化 Winsock
    WSAStartup(MAKEWORD(2,2), &wsaData);
    
    // 创建 socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8889);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.2");
    
    // 连接服务器
    printf("Connecting to server...\n");
    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        printf("Connection failed!\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Connected to server!\n");
    printf("Press Ctrl+C to exit\n");
    printf("-----------------------------------\n");
    
    // 循环定时发送
    while(1) {
        count++;
        
        // 构造发送内容
        sprintf(send_buf, "[%d] Hello from client!\n", count);
        
        // 发送数据
        send(sock, send_buf, strlen(send_buf), 0);
        printf("Sent: %s", send_buf);
        
        // 接收回显
        memset(recv_buf, 0, sizeof(recv_buf));
        int len = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
        
        if(len > 0) {
            recv_buf[len] = '\0';
            printf("Received: %s", recv_buf);
        } else if(len == 0) {
            printf("Server disconnected!\n");
            break;
        } else {
            printf("Recv error!\n");
            break;
        }
        
        printf("-----------------------------------\n");
        
        // 等待 3 秒再发下一次
        Sleep(3000);
    }
    
    // 关闭连接
    closesocket(sock);
    WSACleanup();
    
    return 0;
}