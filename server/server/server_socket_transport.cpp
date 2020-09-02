#include <iostream>
#include <sys/types.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <filesystem>
#include <conio.h>
#include <conio.h>
#include <cassert>
#include <conio.h>
#include <stdio.h>
#else
#include <sys/socket.h>

#endif

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555
#define PACKET_SIZE 1400
#define LISTEN_BACKLOG 50
#define COUNT 1

struct FileInfomation
{
    FileInfomation()
        : file_size(0)
        , is_directory(false)
    {
        memset(file_path, 0, sizeof(file_path));
    }

    char file_path[256];
    __int64 file_size;
    bool is_directory;
};

namespace
{
    bool RecvFileData(const std::string& dst_path, SOCKET& socket, FileInfomation& file_info)
    {
        errno_t write_fp_err;
        FILE* dst_fp = NULL;
        write_fp_err = fopen_s(&dst_fp, dst_path.c_str(), "wb");
        if (write_fp_err != 0)
        {
            std::cout << "can't create write_filepointer" << std::endl;
            return false;
        }

        const __int64 FILE_SIZE = file_info.file_size;
        std::cout << FILE_SIZE << std::endl;

        const __int64 COPY_NUMBER = FILE_SIZE / PACKET_SIZE;
        const int LAST_FILE_SIZE = FILE_SIZE % PACKET_SIZE;

        char recv_buf[PACKET_SIZE] = { 0 };

        for (int i = 0; i < COPY_NUMBER; i++)
        {
            memset(recv_buf, 0, PACKET_SIZE);
            int recvlen = recv(socket, recv_buf, PACKET_SIZE, 0);
            if (recvlen != PACKET_SIZE)
            {
                std::cout << "can't recv file_data" << std::endl;
                return false;
            }
            int writelen = fwrite(recv_buf, PACKET_SIZE, COUNT, dst_fp);
            if (writelen != COUNT)
            {
                std::cout << "can't write file_data" << std::endl;
                return false;
            }
            memset(recv_buf, 0, PACKET_SIZE);
        }

        if (LAST_FILE_SIZE > 0)
        {
            memset(recv_buf, 0, LAST_FILE_SIZE);
            int recvlen = recv(socket, recv_buf, LAST_FILE_SIZE, 0);
            if (recvlen != LAST_FILE_SIZE)
            {
                std::cout << "can't recv file_data" << std::endl;
                return false;
            }
            int writelen = fwrite(recv_buf, LAST_FILE_SIZE, COUNT, dst_fp);
            if (writelen != COUNT)
            {
                std::cout << "can't write file_data" << std::endl;
                return false;
            }
            memset(recv_buf, 0, LAST_FILE_SIZE);
        }

        fclose(dst_fp);
        return true;
    }

    bool DirectoryCopy(const std::string& dst_path, SOCKET& socket)
    {
        char recv_buf[PACKET_SIZE] = { 0 };
        int recvlen = recv(socket, recv_buf, PACKET_SIZE, 0);
        if (recvlen != PACKET_SIZE)
        {
            std::cout << "can't recv file_data" << std::endl;
            return false;
        }

        const int FILE_NUM = atoi(recv_buf);
        struct FileInfomation file_info;

        for (int i = 0; i < FILE_NUM; i++)
        {
            recvlen = recv(socket, (char*)&file_info, sizeof(file_info), 0);
            if (recvlen != sizeof(file_info))
            {
                std::cout << "can't recv file_info" << std::endl;
                return false;
            }
            std::cout << "file_path : " << file_info.file_path << std::endl;
            std::cout << "file_size : " << file_info.file_size << std::endl;
            std::cout << "is_directory : " << file_info.is_directory << std::endl;

            const std::string DST_FILE_PATH = dst_path + file_info.file_path;
            std::cout << "dst_file_path : " << DST_FILE_PATH << std::endl;

            if (file_info.is_directory == true)
            {
                std::cout << "make directory" << std::endl;
                namespace fs = std::experimental::filesystem;
                fs::create_directory(DST_FILE_PATH);
            }
            else
            {
                if(RecvFileData(DST_FILE_PATH, socket, file_info) == false)
                {
                    std::cout << "can't RecvFileData" << std::endl;
                    return false;
                }
            }
        }
        return true;
    }

    bool IsExistDirectory(const std::string& dir_path)
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
}

int main()
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == -1)
    {
        std::cout << "WSAStartup errno : " << errno << std::endl;
        return false;
    }

    SOCKET server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1)
    {
        std::cout << "socket errno : " << errno << std::endl;
        return false;
    }
    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cout << "bind errno : " << errno << std::endl;
        return false;
    }

    if (listen(server_socket, LISTEN_BACKLOG) == -1)
    {
        std::cout << "listen errno : " << errno << std::endl;
        return false;
    }

    FD_SET read_set;
    FD_SET tmp;
    TIMEVAL time;

    FD_ZERO(&read_set);
    FD_SET(server_socket, &read_set);
    while (!_kbhit())
    {
        tmp = read_set;

        time.tv_sec = 1;
        time.tv_usec = 0;
        unsigned int fd_max = 0;
        for (unsigned int i = 0; i < read_set.fd_count; i++)
        {
            if (fd_max < read_set.fd_array[i])
                fd_max = read_set.fd_array[i];
            std::cout << "  " << read_set.fd_array[i] << std::endl;
        }

        int req_count = select(fd_max + 1, &tmp, NULL, NULL, &time);

        if (req_count == -1)
        {
            std::cout << "req_count error : " << errno <<std::endl;
        }

        if (req_count == 0)
            continue;

        for (unsigned int i = 0; i < read_set.fd_count; i++)
        {
            if (FD_ISSET(read_set.fd_array[i], &tmp))
            {
                SOCKADDR_IN client_addr;
                int sockaddr_size = sizeof(SOCKADDR_IN);

                if (read_set.fd_array[i] == server_socket)
                {
                    SOCKET client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &sockaddr_size);
                    if (client_socket == -1)
                    {
                        std::cout << "can't accept client_socket : " << std::endl;
                        continue;
                    }
                    FD_SET(client_socket, &read_set);
                }
                else
                {
                    char dst_path[PACKET_SIZE] = { 0 };
                    int recvlen = recv(read_set.fd_array[i], dst_path, PACKET_SIZE, 0);
                    if (recvlen != PACKET_SIZE)
                    {
                        std::cout << "can't recv file_data" << std::endl;
                        FD_CLR(read_set.fd_array[i], &read_set);
                        std::cout << "--------------------------finish----------------------------" << std::endl;
                        closesocket(tmp.fd_array[i]);
                        continue;
                    }
                    std::cout << "dst_path : " << dst_path << std::endl;

                    bool input_check = true;
                    if (IsExistDirectory(dst_path) == false)
                    {
                        std::cout << "can't check Directory" << std::endl;
                        input_check = false; 

                        int sendlen = send(read_set.fd_array[i], (char*)&input_check, sizeof(dst_path), 0);
                        
                        if (sendlen != sizeof(dst_path))
                        {
                            std::cout << "can't send dst_path" << std::endl;
                            continue;
                        }
                        continue;
                    }
                    int sendlen = send(read_set.fd_array[i], (char*)&input_check, sizeof(dst_path), 0);
                    if (sendlen != sizeof(dst_path))
                    {
                        std::cout << "can't send input_check" << std::endl;
                        continue;
                    }

                    if (DirectoryCopy(dst_path, read_set.fd_array[i]) == false)
                    {
                        std::cout << "fail to copy directory" << std::endl;
                        FD_CLR(read_set.fd_array[i], &read_set);
                        std::cout << "--------------------------finish----------------------------" << std::endl;
                        closesocket(tmp.fd_array[i]);
                        continue;
                    }

                    closesocket(read_set.fd_array[i]);

                    std::cout << "end file copy" << std::endl;

                    FD_CLR(read_set.fd_array[i], &read_set);
                    std::cout << "--------------------------finish----------------------------" << std::endl;
                    closesocket(tmp.fd_array[i]);
                    continue;
                }
            }
        }
    }

    closesocket(server_socket);

    WSACleanup();
    return true;
}