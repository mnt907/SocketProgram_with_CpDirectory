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

    bool IsExistFiles(const std::string& dir_path)
    {
        bool is_exist_file = false;

#ifdef _WIN32
        auto dir = fs::recursive_directory_iterator(fs::path(dir_path));

        for (auto& p : dir)
        {
            is_exist_file = true;
            break;
        }
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

        return is_exist_file;
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

    bool CopyFiles(const std::string& src_path, const std::string& dst_path)
    {
        FILE* src_fp = NULL;
        src_fp = fopen(src_path.c_str(), "rb");
        if (src_fp == NULL)
        {
            std::cout << "can't create read_filepointer" << std::endl;
            return false;
        }

        FILE* dst_fp = NULL;
        if ((dst_fp = fopen(dst_path.c_str(), "wb")) == NULL)
        {
            std::cout << "can't create write_filepointer" << std::endl;
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

        int file_buf_size = 256;

        char* file_buf = (char*)malloc(file_buf_size);
        memset(file_buf, 0, file_buf_size);

        int copy_number = file_size / file_buf_size;
        int last_file_size = file_size % file_buf_size;

        for (int i = 0; i < copy_number; i++)
        {
            memset(file_buf, 0, file_buf_size);
            fread(file_buf, file_buf_size, 1, src_fp);
            fwrite(file_buf, file_buf_size, 1, dst_fp);
            memset(file_buf, 0, file_buf_size);
        }

        if (last_file_size > 0)
        {
            free(file_buf);
            file_buf = (char*)malloc(last_file_size);
            memset(file_buf, 0, last_file_size);
            fread(file_buf, last_file_size, 1, src_fp);
            fwrite(file_buf, last_file_size, 1, dst_fp);
            memset(file_buf, 0, last_file_size);
        }

        free(file_buf);
        fclose(src_fp);
        fclose(dst_fp);

        return true;
    }

    bool CopyDirectory(const std::string& src_path, const std::string& dst_path)
    {
#ifdef _WIN32
        using namespace std;
        auto dir = fs::recursive_directory_iterator(fs::path(src_path));

        int src_path_length = src_path.length();
        cout << src_path_length << endl;

        for (const auto& file : dir)
        {
            string file_path = file.path().generic_string();
            file_path.assign(file_path.c_str(), src_path.length(), file_path.length());

            const string dst_file_path = dst_path + file_path;
            const string src_file_path = src_path + file_path;

            if (IsExistDiretory(src_file_path))
            {
                fs::create_directory(dst_file_path);
            }
            else
            {
                CopyFiles(src_file_path, dst_file_path);
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

    //------------------get file data --------------------------
    const std::string src_path = "C:/Users/mnt/Desktop/empty_directory/setup_v4-190918 (2).zip";

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
    bool check_equal_data = false;
    //std::cout << "send file_size : " << file_size << std::endl;
    //file_size = 23;
    //sprintf(send_buf, "%d", file_size);
    //std::cout << "send file_size : " << send_buf << std::endl;
  
    //send(client_socket, send_buf, PACKET_SIZE, 0);

    int copy_number = file_size / PACKET_SIZE;
    int last_file_size = file_size % PACKET_SIZE;

    for (int i = 0; i < copy_number; i++)
    {
        memset(file_buf, 0, PACKET_SIZE);
        fread(file_buf, PACKET_SIZE, 1, src_fp);
        send(client_socket, file_buf, PACKET_SIZE, 0);
        memset(file_buf, 0, PACKET_SIZE);
        //std::cout << "copy : " << i << std::endl;
    }

    if (last_file_size > 0)
    {
        free(file_buf);
        file_buf = (char*)malloc(last_file_size);
        memset(file_buf, 0, last_file_size);
        fread(file_buf, last_file_size, 1, src_fp);
        send(client_socket, file_buf, last_file_size, 0);
        memset(file_buf, 0, last_file_size);
        std::cout << "last copy"<< std::endl;
    }

    free(file_buf);
    fclose(src_fp);

    //----------------------end---------------------------------
    send(client_socket, send_buf, PACKET_SIZE, 0);

    recv(client_socket, recv_buf, PACKET_SIZE, 0);
    std::cout << recv_buf << std::endl;

    closesocket(client_socket);

    WSACleanup();
}