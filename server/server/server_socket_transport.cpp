#include <iostream>

#include <sys/types.h>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>

#endif

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555
#define PACKET_SIZE 1024
#define LISTEN_BACKLOG 50

namespace
{

}

bool main()
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == -1)
    {
        std::cout << "WSAStartup errno : " << errno << std::endl;
    }

    SOCKET server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1)
        std::cout << "socket errno : " << errno << std::endl;

    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == -1)
        std::cout << "bind errno : " << errno << std::endl;

    std::cout << "1" << std::endl;

    if (listen(server_socket, LISTEN_BACKLOG) == -1)
        std::cout << "listen errno : " << errno << std::endl;

    std::cout << "1" << std::endl;

    int sockaddr_size = sizeof(SOCKADDR_IN);
    SOCKADDR_IN client_addr;
    SOCKET client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &sockaddr_size);
    if (client_socket == -1)
        std::cout << "accept errno : " << errno << std::endl;

    std::cout << "1" << std::endl;

    //----------------------------------------------------------
    char recv_buf[PACKET_SIZE] = { 0 };
    bool check_equal_data = false;

    //recv(client_socket, recv_buf, PACKET_SIZE, 0);
    //
    //std::cout << recv_buf << std::endl;
    //std::cout << (int)recv_buf << std::endl;

    FILE* dst_fp = NULL;
    const std::string dst_path = "C:/Users/mnt/Desktop/dd/setup_v4-190918 (2).zip";
    dst_fp = fopen(dst_path.c_str(), "wb");
    if (dst_fp == NULL)
    {
        std::cout << "can't create write_filepointer" << std::endl;
        return false;
    }

    __int64 file_size = 3490503494;

    int copy_number = file_size / PACKET_SIZE;
    int last_file_size = file_size % PACKET_SIZE;
    char* file_buf = (char*)malloc(PACKET_SIZE);
    memset(file_buf, 0, PACKET_SIZE);

    for (int i = 0; i < copy_number; i++)
    {
        memset(file_buf, 0, PACKET_SIZE);
        recv(client_socket, recv_buf, PACKET_SIZE, 0);
        fwrite(file_buf, PACKET_SIZE, 1, dst_fp);
        memset(file_buf, 0, PACKET_SIZE);
        //std::cout << "copy : " << i << std::endl;
    }

    if (last_file_size > 0)
    {
        free(file_buf);
        file_buf = (char*)malloc(last_file_size);
        memset(file_buf, 0, last_file_size);
        recv(client_socket, recv_buf, last_file_size, 0);
        fwrite(file_buf, last_file_size, 1, dst_fp);
        memset(file_buf, 0, last_file_size);
        std::cout << "last copy" << std::endl;
    }

    free(file_buf);
    fclose(dst_fp);

    //-----------------------------------------------------------
    //char recv_buf[PACKET_SIZE] = { 0 };
    //recv(client_socket, recv_buf, PACKET_SIZE, 0);
    //std::cout << recv_buf << std::endl;



    //char send_buf[] = "server send";
    //send(client_socket, send_buf, sizeof(send_buf), 0);

    closesocket(server_socket);
    closesocket(client_socket);

    WSACleanup();
}