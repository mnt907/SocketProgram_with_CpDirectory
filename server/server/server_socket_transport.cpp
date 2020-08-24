#include <iostream>

#include <sys/types.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <filesystem>
#else
#include <sys/socket.h>

#endif

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555
#define PACKET_SIZE 1024
#define LISTEN_BACKLOG 50

namespace
{
    bool SendFileData(const std::string& dst_path, SOCKET& socket)
    {
        char recv_buf[PACKET_SIZE] = { 0 };
        recv(socket, recv_buf, PACKET_SIZE, 0);
        const std::string dst_file_path = dst_path + recv_buf;

        FILE* dst_fp = NULL;
        dst_fp = fopen(dst_file_path.c_str(), "wb");
        if (dst_fp == NULL)
        {
            std::cout << "can't create write_filepointer" << std::endl;
            return false;
        }

        recv(socket, recv_buf, PACKET_SIZE, 0);
        std::cout << recv_buf << std::endl;

        __int64 file_size = _atoi64(recv_buf);
        std::cout << file_size << std::endl;

        int copy_number = file_size / PACKET_SIZE;
        int last_file_size = file_size % PACKET_SIZE;
        char* file_buf = (char*)malloc(PACKET_SIZE);
        memset(file_buf, 0, PACKET_SIZE);

        for (int i = 0; i < copy_number; i++)
        {
            memset(recv_buf, 0, PACKET_SIZE);
            recv(socket, recv_buf, PACKET_SIZE, 0);
            fwrite(recv_buf, PACKET_SIZE, 1, dst_fp);
            memset(recv_buf, 0, PACKET_SIZE);
        }

        if (last_file_size > 0)
        {
            memset(recv_buf, 0, last_file_size);
            recv(socket, recv_buf, last_file_size, 0);
            fwrite(recv_buf, last_file_size, 1, dst_fp);
            memset(recv_buf, 0, last_file_size);
        }

        free(file_buf);
        fclose(dst_fp);
    }
    
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

    if (listen(server_socket, LISTEN_BACKLOG) == -1)
        std::cout << "listen errno : " << errno << std::endl;

    int sockaddr_size = sizeof(SOCKADDR_IN);
    SOCKADDR_IN client_addr;
    SOCKET client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &sockaddr_size);
    if (client_socket == -1)
        std::cout << "accept errno : " << errno << std::endl;

    const std::string dst_path = "C:/Users/mnt/Desktop/dd";

    char recv_buf[PACKET_SIZE] = { 0 };
    recv(client_socket, recv_buf, PACKET_SIZE, 0);
    int file_count = atoi(recv_buf);

    for (int i = 0; i < file_count; i++) {
        recv(client_socket, recv_buf, PACKET_SIZE, 0);

        if (!strcmp(recv_buf, "true"))
        {
            std::string dst_file_path = dst_path + recv_buf;
            std::cout << "dst_file_path : " << dst_file_path << std::endl;
            SendFileData(dst_file_path, client_socket);
        }
        else
        {
            std::string dst_file_path = dst_path + recv_buf;
            std::cout << "dst_file_path : " << dst_file_path << std::endl;
            namespace fs = std::experimental::filesystem;
            fs::create_directory(dst_file_path);
        }
    }

    closesocket(server_socket);
    closesocket(client_socket);

    WSACleanup();
}