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
#define PACKET_SIZE 1400
#define LISTEN_BACKLOG 50

struct FileInfomation
{
    char file_path[256];
    __int64 file_size;
    bool is_directory;
};

namespace
{
    bool RecvFileData(const std::string& dst_path, SOCKET& socket, struct FileInfomation file_info)
    {
        FILE* dst_fp = NULL;
        dst_fp = fopen(dst_path.c_str(), "wb");
        if (dst_fp == NULL)
        {
            std::cout << "can't create write_filepointer" << std::endl;
            return false;
        }

        const __int64 file_size = file_info.file_size;
        std::cout << file_size << std::endl;

        const int copy_number = file_size / PACKET_SIZE;
        const int last_file_size = file_size % PACKET_SIZE;

        char recv_buf[PACKET_SIZE] = { 0 };

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

        fclose(dst_fp);
        return true;
    }

    bool DirectoryCopy(const std::string& dst_path, SOCKET& socket)
    {
        struct FileInfomation file_info;
        char recv_buf[PACKET_SIZE] = { 0 };
        int recvlen = recv(socket, recv_buf, PACKET_SIZE, 0);
        const int file_num = atoi(recv_buf);

        for (int i = 0; i < file_num; i++) 
        {
            //memset(&file_info, 0, sizeof(file_info));
            recvlen = recv(socket, (char*)&file_info, sizeof(file_info), 0);
            std::cout << "file_path : " << file_info.file_path << std::endl;
            std::cout << "file_size : " << file_info.file_size << std::endl;
            std::cout << "is_directory : " << file_info.is_directory << std::endl;

            const std::string dst_file_path = dst_path + file_info.file_path;
            std::cout << "dst_file_path : " << dst_file_path << std::endl;

            if (file_info.is_directory == true)
            {
                std::cout << "make directory" << std::endl;
                namespace fs = std::experimental::filesystem;
                fs::create_directory(dst_file_path);
            }
            else
            {
                RecvFileData(dst_file_path, socket, file_info);
            }
        }
        return true;
    }

    bool IsExistDiretory(const std::string& dir_path)
    {
        namespace fs = std::experimental::filesystem;
        if (fs::is_directory(fs::path(dir_path)))
        {
            std::cout << "exists Directory" << std::endl;
            return true;
        }

        std::cout << "not exists Directory" << std::endl;
        return false;
    }

    std::string InputPath()
    {
        std::cout << "Enter the PATH" << std::endl;
        std::string user_input;
        std::cin >> user_input;
        std::cout << user_input << std::endl;
        return user_input;
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

    if (IsExistDiretory(dst_path) == false)
        return false;

    DirectoryCopy(dst_path, client_socket);
    
    closesocket(server_socket);
    closesocket(client_socket);

    WSACleanup();
    return true;
}