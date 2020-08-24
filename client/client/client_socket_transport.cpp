#include <iostream>
#include <string>
#include <sys/types.h>
#include <cstring>
#ifdef _WIN32
#include <WinSock2.h>
#include <filesystem>
#include <Windows.h>
#include <fstream>
#else
#include <sys/socket.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#endif

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555
#define PACKET_SIZE 1024
#define SERVER_IP "192.168.12.112"

namespace
{
#ifdef _WIN32
    namespace fs = std::experimental::filesystem;
#endif //WIN32

    std::string InputPath()
    {
        std::cout << "Enter the PATH" << std::endl;
        std::string user_input;
        std::cin >> user_input;
        std::cout << user_input << std::endl;
        return user_input;
    }

    int CountFiles(const std::string& dir_path)
    {
#ifdef _WIN32
        int file_count = 0;
        auto dir = fs::recursive_directory_iterator(fs::path(dir_path));

        for (auto& p : dir)
        {
            file_count++;
        }
        return file_count;
#else
        DIR *dir_dirp = NULL;
        if ((dir_dirp = opendir(dir_path.c_str())) == NULL)
        {
            std::cout << "can't open directory" << dir_path << std::endl;
            return false;
        }
        const struct dirent *dp;
        if ((dp = readdir(dir_dirp)) == NULL)
        {
            std::cout << "can't read directory" << dir_path << std::endl;
            return false;
        }

        while ((dp = readdir(dir_dirp)) != NULL)
        {
            is_exist_file = true;
            break;
        }

        closedir(dir_dirp);

#endif //_WIN32


    }

    bool IsExistDiretory(const std::string& dir_path)
    {
#ifdef _WIN32
        if (fs::is_directory(fs::path(dir_path)))
#else
        DIR *src_dirp = NULL;
        if ((src_dirp = opendir(dir_path.c_str())) != NULL)
#endif
        {
            std::cout << "exists Directory" << std::endl;
            return true;
        }

        std::cout << "not exists Directory" << std::endl;
#ifndef _WIN32
        closedir(src_dirp);
#endif
        return false;
    }
#include <errno.h>

    bool SendFileData(const std::string& src_path, SOCKET& socket)
    {
  
        send(socket, src_path.c_str(), PACKET_SIZE, 0);

        FILE* src_fp = NULL;
        src_fp = fopen(src_path.c_str(), "rb");
        if (src_fp == NULL)
        {
            std::cout << "can't create read_filepointer" << std::endl;
            return false;
        }

        _fseeki64(src_fp, (__int64)0, SEEK_END);
        __int64 file_size = _ftelli64(src_fp);
        if (file_size < 0)
        {
            printf("can't get a file's offset. errno(%d, %s)\r\n", errno, strerror(errno));
            return false;
        }
        rewind(src_fp);

        char* file_buf = (char*)malloc(PACKET_SIZE);
        memset(file_buf, 0, PACKET_SIZE);

        char send_buf[PACKET_SIZE] = { 0 };
        char recv_buf[PACKET_SIZE] = { 0 };

        sprintf(send_buf, "%I64u", file_size);
        send(socket, send_buf, PACKET_SIZE, 0);
        std::cout << send_buf << std::endl;

        int copy_number = file_size / PACKET_SIZE;
        int last_file_size = file_size % PACKET_SIZE;

        for (int i = 0; i < copy_number; i++)
        {
            memset(file_buf, 0, PACKET_SIZE);
            fread(file_buf, PACKET_SIZE, 1, src_fp);
            send(socket, file_buf, PACKET_SIZE, 0);
            memset(file_buf, 0, PACKET_SIZE);
        }

        if (last_file_size > 0)
        {
            free(file_buf);
            file_buf = (char*)malloc(last_file_size);
            memset(file_buf, 0, last_file_size);
            fread(file_buf, last_file_size, 1, src_fp);
            send(socket, file_buf, last_file_size, 0);
            memset(file_buf, 0, last_file_size);
        }

        free(file_buf);
        fclose(src_fp);
    }

    bool DirectoryCopy(const std::string& src_path, SOCKET& socket)
    {
#ifdef _WIN32
        int file_count = CountFiles(src_path);
        char send_buf[PACKET_SIZE] = { 0 };
        sprintf(send_buf, "%d", file_count);
        send(socket, send_buf, sizeof(send_buf), 0);

        auto dir = fs::recursive_directory_iterator(fs::path(src_path));

        for (const auto& file : dir)
        {
            std::string file_path = file.path().generic_string();
            file_path.assign(file_path.c_str(), src_path.length(), file_path.length());
            std::cout << "file_path : " << file_path << std::endl;

            const std::string src_file_path = src_path + file_path;

            if (IsExistDiretory(src_file_path))
            {
                //make directory!!!!!!!!!!!!!
                send(socket, file_path.c_str(), file_path.length(), 0);
            }
            else
            {
                strcpy(send_buf, "true");
                send(socket, send_buf, PACKET_SIZE, 0);
                SendFileData(src_file_path, socket);
            }
        }
        return true;
#else
        DIR *src_dirp = NULL;
        if ((src_dirp = opendir(src_path.c_str())) == NULL)
        {
            std::cout << "can't open directory" << src_path << std::endl;
            return false;
        }

        DIR *dst_dirp = NULL;
        if ((dst_dirp = opendir(dst_path.c_str())) == NULL)
        {
            std::cout << "can't open directory " << dst_path << std::endl;
            return false;
        }

        const struct dirent *dp;
        if ((dp = readdir(src_dirp)) == NULL)
        {
            std::cout << "can't read directory" << src_path << std::endl;
            return false;
        }

        struct stat file_info;
        while ((dp = readdir(src_dirp)) != NULL)
        {
            const std::string filename = dp->d_name;
            const std::string new_src_path = src_path + "/" + filename;
            const std::string new_dst_path = dst_path + "/" + filename;

            if ((lstat(new_src_path.c_str(), &file_info)) == -1)
            {
                std::cout << "can't read file info " << new_src_path << std::endl;
                continue;
            }

            if (S_ISDIR(file_info.st_mode))
            {
                if (filename == "." || filename == "..")
                    continue;
                else
                {
                    std::cout << "[directory name] " << new_src_path << std::endl;

                    mkdir(new_dst_path.c_str(), file_info.st_mode);
                    CopyDirectory(new_src_path, new_dst_path);
                }
            }
            else
            {
                std::cout << "[file name] " << new_src_path << std::endl;
                CopyFiles(new_src_path, new_dst_path);
                chmod(new_dst_path.c_str(), file_info.st_mode);
                chown(new_dst_path.c_str(), file_info.st_uid, file_info.st_gid);
            }
        }

        closedir(src_dirp);
        closedir(dst_dirp);

        return true;

#endif //_WIN32
    }
}

int main()
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == -1)
    {
        std::cout << "WSAStartup errno : " << errno << std::endl;
    }

    SOCKET client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1)
        std::cout << "socket errno : " << errno << std::endl;

    SOCKADDR_IN client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    client_addr.sin_port = htons(PORT);

    if (connect(client_socket, (SOCKADDR*)&client_addr, sizeof(client_addr)) == -1)
    {
        std::cout << "connect errno : " << errno << std::endl;
    }

    const std::string src_path = "C:/Users/mnt/Desktop/empty_directory";
    DirectoryCopy(src_path, client_socket);

    closesocket(client_socket);

    WSACleanup();
}