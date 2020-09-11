#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <sys/types.h>
#include <cstring>
#ifdef _WIN32
#include <WinSock2.h>
#include <filesystem>
#include <Windows.h>
#include <fstream>
#include <WS2tcpip.h>
#include <istream>
#include <conio.h>
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
#define PACKET_SIZE 1400
#define SERVER_IP "127.0.0.1"
#define COUNT 1

struct Header
{
    enum class TYPE
    {
        CONNECT,
        CHECK_DIR,
        FILE_SEND,
        ERROR_SIG
    };
    Header()
        : type(Header::TYPE::CONNECT)
        , length(0)
        , is_dir(false)
    {
        memset(file_path, 0, sizeof(file_path));
    }

    TYPE type;
    __int64 length;
    bool is_dir;
    char file_path[256];
};

namespace
{
#ifdef _WIN32
    namespace fs = std::experimental::filesystem;
#endif //WIN32

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

    bool IsExistDirectory(const std::string& dir_path)
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

    bool SendFileData(const std::string& src_path, const SOCKET& socket, Header& header)
    {
        if (header.is_dir)
        {
            int sendlen = send(socket, (char*)&header, sizeof(header), 0);
            if (sendlen != sizeof(header))
            {
                std::cout << "can't send header" << std::endl;
                return false;
            }
            return true;
        }

        errno_t err;
        FILE* src_fp = NULL;
        err = fopen_s(&src_fp, src_path.c_str(), "rb");
        if (err != 0)
        {
            std::cout << "can't create read_filepointer" << std::endl;
            return false;
        }

        _fseeki64(src_fp, (__int64)0, SEEK_END);
        const __int64 FILE_SIZE = _ftelli64(src_fp);
        if (FILE_SIZE < 0)
        {
            std::cout << "can't get a file's offset" << std::endl;
            return false;
        }
        rewind(src_fp);

        header.length = FILE_SIZE;

        char send_buf[PACKET_SIZE] = { 0 };
        int sendlen = send(socket, (char*)&header, sizeof(header), 0);
        if (sendlen != sizeof(header))
        {
            std::cout << "can't send header" << std::endl;
            return false;
        }
            
        std::cout << "file_path : " << header.file_path << std::endl;
        std::cout << "file_size : " << header.length << std::endl;
        std::cout << "is_directory : " << header.is_dir << std::endl;

        const __int64 COPY_NUMBER = FILE_SIZE / PACKET_SIZE;
        const int LAST_FILE_SIZE = FILE_SIZE % PACKET_SIZE;

        for (int i = 0; i < COPY_NUMBER; i++)
        {
            memset(send_buf, 0, PACKET_SIZE);
            int readlen = fread(send_buf, PACKET_SIZE, COUNT, src_fp);
            if (readlen != COUNT)
            {
                std::cout << "can't read file_data" << std::endl;
                return false;
            }
     
            int sendlen = send(socket, send_buf, PACKET_SIZE, 0);
            if (sendlen != PACKET_SIZE)
            {
                std::cout << "can't send file_data" << std::endl;
                return false;
            }
               
            memset(send_buf, 0, PACKET_SIZE);
        }
        Sleep(1);

        if (LAST_FILE_SIZE > 0)
        {
            memset(send_buf, 0, LAST_FILE_SIZE);
            int readlen = fread(send_buf, LAST_FILE_SIZE, COUNT, src_fp);
            if (readlen != COUNT)
            {
                std::cout << "can't read file_data" << std::endl;
                return false;
            }
                
            int sendlen = send(socket, send_buf, LAST_FILE_SIZE, 0);
            if (sendlen != LAST_FILE_SIZE)
            {
                std::cout << "can't send file_data" << std::endl;
                return false;
            }

            memset(send_buf, 0, LAST_FILE_SIZE);
        }
        fclose(src_fp);
        return true;
    }

    bool DirectoryCopy(const std::string& src_path, const std::string& dst_path, SOCKET& socket, Header& header)
    {
#ifdef _WIN32
        char send_buf[PACKET_SIZE] = { 0 };

        auto dir = fs::recursive_directory_iterator(fs::path(src_path));

        for (const auto& file : dir)
        {
            std::string file_path = file.path().generic_string();
            file_path.assign(file_path.c_str(), src_path.length(), file_path.length());
            std::cout << "file_path : " << file_path << std::endl;

            const std::string SRC_FILE_PATH = src_path + file_path;

            strncpy_s(header.file_path, file_path.c_str()
                , (sizeof(header.file_path) < file_path.length()) ?
                sizeof(header.file_path) - 1 : file_path.length());

            if (IsExistDirectory(SRC_FILE_PATH))
                header.is_dir  = true;
            else
                header.is_dir = false;
               
            if (SendFileData(SRC_FILE_PATH, socket, header) == false)
            {
                std::cout << "fail to send file data " << file_path << std::endl;
                return false;
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
    std::string InputPath()
    {
        char user_input_path[PACKET_SIZE];
        std::cin.getline(user_input_path,'\n');
        std::cout << "input " << user_input_path  << std::endl;
        return user_input_path;
    }
}

int main()
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == -1)
    {
        std::cout << "WSAStartup errno : " << errno << std::endl;
        return EXIT_FAILURE;
    }

    SOCKET client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1)
    {
        std::cout << "socket errno : " << errno << std::endl;
        return EXIT_FAILURE;
    }

    SOCKADDR_IN client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    client_addr.sin_port = htons(PORT);

    if (connect(client_socket, (SOCKADDR*)&client_addr, sizeof(client_addr)) == -1)
    {
        std::cout << "connect errno : " << errno << std::endl;
        return EXIT_FAILURE;
    }

    while (true) {
        Header header;
        FD_SET read_set;
        FD_SET cpy_read_set;
        TIMEVAL time;

        FD_ZERO(&read_set);
        FD_ZERO(&cpy_read_set);
        FD_SET(client_socket, &read_set);
        //char src_path[PACKET_SIZE] = "C:/Users/mnt/Desktop/dd";
        //char dst_path[PACKET_SIZE] = "C:/Users/mnt/Desktop/empty_directory";

        char src_path[PACKET_SIZE] = "C:/Users/mnt/Desktop/a";
        char dst_path[PACKET_SIZE] = "C:/Users/mnt/Desktop/b";

        //char src_path[PACKET_SIZE] = "C:/Users/mnt/Desktop/copy";
        //char dst_path[PACKET_SIZE] = "C:/Users/mnt/Desktop/copy qwer";

        bool input_check = true;
        while (input_check)
        {
            //std::cout << "src_path input : ";
            //std::string input_path = InputPath();
            //strncpy_s(src_path, input_path.c_str()
            //    , (sizeof(src_path) < input_path.length()) ?
            //    sizeof(src_path) - 1 : input_path.length());

            //std::cout << std::endl;

            //std::cout << "dst_path input : ";
            //input_path.clear();
            //input_path = InputPath();
            //strncpy_s(dst_path, input_path.c_str()
            //    , (sizeof(dst_path) < input_path.length()) ?
            //    sizeof(dst_path) - 1 : input_path.length());

            //std::cout << std::endl;
            //if (!strcmp(src_path, dst_path))
            //{
            //    std::cout << "same src_path and dst_path" << std::endl;
            //    continue;
            //}

            //if (IsExistDirectory(src_path) == false)
            //{
            //    std::cout << "invalid src_path" << std::endl;
            //    continue;
            //}

            header.length = strlen(dst_path);

            strncpy_s(header.file_path, dst_path
                , (sizeof(header.file_path) < sizeof(dst_path)) ?
                sizeof(header.file_path) - 1 : sizeof(dst_path));

            int sendlen = send(client_socket, (char*)&header, sizeof(header), 0);
            if (sendlen != sizeof(header))
            {
                std::cout << "can't send header" << std::endl;
                break;
            }
            input_check = false;
        }

        while (!_kbhit())
        {
            cpy_read_set = read_set;

            time.tv_sec = 1;
            time.tv_usec = 0;
            int req_count = select(client_socket + 1, &cpy_read_set, NULL, NULL, &time);

            if (req_count == -1)
            {
                std::cout << "req_count error : " << errno << std::endl;
                continue;
            }

            if (req_count == 0)
                continue;

            if (FD_ISSET(client_socket, &cpy_read_set))
            {
                int recvlen = recv(client_socket, (char*)&header, sizeof(header), 0);
                if (recvlen != sizeof(header))
                {
                    std::cout << "can't recv header" << std::endl;
                    break;
                }

                if (header.type == Header::TYPE::ERROR_SIG)
                {
                    std::cout << header.file_path << std::endl;
                    break;
                }
                else if (header.type == Header::TYPE::CHECK_DIR)
                {
                    if (DirectoryCopy(src_path, dst_path, client_socket, header) == false)
                    {
                        std::cout << "fail to copy directory" << std::endl;
                        break;
                    }

                    char yes_or_no;
                    std::cout << "do you want copy more?(y/n) : ";
                    std::cin >> yes_or_no;
                    if (yes_or_no == 'y')
                        break;

                    header.type = Header::TYPE::FILE_SEND;
                    int sendlen = send(client_socket, (char*)&header, sizeof(header), 0);
                    if (sendlen != sizeof(header))
                    {
                        std::cout << "can't send header" << std::endl;
                        break;
                    }
                }
                else if (header.type == Header::TYPE::FILE_SEND)
                {
                    closesocket(client_socket);
                    WSACleanup();
                    FD_CLR(client_socket, &read_set);
                    return 1;
                }
            }
        }
    }
    return 0;
}


