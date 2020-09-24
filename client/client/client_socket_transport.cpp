#include <iostream>
#include <string>
#include <sys/types.h>
#include <cstring>
#include <WinSock2.h>
#include <filesystem>
#include <Windows.h>
#include <fstream>
#include <WS2tcpip.h>
#include <istream>
#include <conio.h>
#include <ws2def.h>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555
#define PACKET_SIZE 1400
#define SERVER_IP "127.0.0.1"
#define COUNT 1
#define CPY_DIR_THREAD_SIZE 1

struct Header
{
    enum class TYPE
    {
        CONNECT,
        CHECK_DIR,
        NOT_EXIST_DIR,
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
    namespace fs = std::experimental::filesystem;

    bool IsExistDirectory(const std::string& dir_path)
    {
        if (fs::is_directory(fs::path(dir_path)))
        {
            std::cout << "exists Directory" << std::endl;
            return true;
        }

        std::cout << "not exists Directory" << std::endl;

        return false;
    }

    bool SendFileData(const std::string& src_path, const SOCKET& socket, Header& header)
    {
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

        int sendlen = send(socket, (char*)&header, sizeof(header), 0);
        if (sendlen == -1)
        {
            std::cout << "can't send header" << std::endl;
            return false;
        }

        std::cout << "file_path : " << header.file_path << std::endl;
        std::cout << "file_size : " << header.length << std::endl;
        std::cout << "is_directory : " << header.is_dir << std::endl;

        __int64 left_send_size = header.length;
        int send_size = PACKET_SIZE;
        char send_buf[PACKET_SIZE] = { 0 };
        while (left_send_size != 0)
        {
            memset(send_buf, 0, PACKET_SIZE);
            if (left_send_size < PACKET_SIZE)
                send_size = (int)left_send_size;

            int readlen = fread(send_buf, send_size, COUNT, src_fp);
            if (readlen != COUNT)
            {
                std::cout << "can't read file_data" << std::endl;
                return false;
            }

            int sendlen = send(socket, send_buf, send_size, 0);
            if (sendlen == -1)
            {
                std::cout << "can't send file_data" << std::endl;
                return false;
            }
            left_send_size -= sendlen;
        }

        fclose(src_fp);
        return true;
    }

    bool CopyDirectory(const std::string& src_path, const std::string& dst_path, const SOCKET& socket)
    {
        char send_buf[PACKET_SIZE] = { 0 };

        auto dir = fs::recursive_directory_iterator(fs::path(src_path));

        for (const auto& file : dir)
        {
            std::string file_path = file.path().generic_string();
            file_path.assign(file_path.c_str(), src_path.length(), file_path.length());
            std::cout << "file_path : " << file_path << std::endl;

            const std::string SRC_FILE_PATH = src_path + file_path;
            const std::string DST_FILE_PATH = dst_path + file_path;

            Header header;
            strncpy_s(header.file_path, DST_FILE_PATH.c_str()
                , (sizeof(header.file_path) < DST_FILE_PATH.length()) ?
                sizeof(header.file_path) - 1 : DST_FILE_PATH.length());

            header.type = Header::TYPE::CHECK_DIR;

            if (IsExistDirectory(SRC_FILE_PATH))
            {
                header.is_dir = true;
                int sendlen = send(socket, (char*)&header, sizeof(header), 0);
                if (sendlen != sizeof(header))
                {
                    std::cout << "can't send header" << std::endl;
                    return false;
                }
            }
            else
            {
                header.is_dir = false;
                if (SendFileData(SRC_FILE_PATH, socket, header) == false)
                {
                    std::cout << "fail to send file data " << file_path << std::endl;
                    return false;
                }
            }
        }
        return true;
    }
    std::string InputData()
    {
        std::string user_input_data;
        std::cin >> user_input_data;
        std::cout << "input " << user_input_data << std::endl;
        return user_input_data;
    }

    std::string CheckSrcPath()
    {
        //std::string src_path;
        //while (true)
        //{
        //    std::cout << "input src_path : ";
        //    src_path = InputData();

        //    if (IsExistDirectory(src_path) == false)
        //    {
        //        std::cout << "invalid src_path" << std::endl;
        //        continue;
        //    }
        //    break;
        //}
        const std::string src_path = "C:/Users/kyoud/Desktop/a";

        return src_path;
    }
    bool CheckDstPath(const SOCKET& socket)
    {
        std::string dst_path;

        //std::cout << "input dst_path : ";
        //dst_path = InputData();

        dst_path = "C:/Users/kyoud/Desktop/b";

        Header header;
        strncpy_s(header.file_path, dst_path.c_str()
            , (sizeof(header.file_path) < dst_path.length()) ?
            sizeof(header.file_path) - 1 : dst_path.length());

        int sendlen = send(socket, (char*)&header, sizeof(header), 0);
        if (sendlen == -1)
        {
            std::cout << "can't send header" << std::endl;
            return false;
        }

        return true;
    }

    void CopyDirectoryThread(const SOCKET client_socket)
    {
        bool end_signal = false;
        while (end_signal == false)
        {
            const std::string src_path = CheckSrcPath();
            if (CheckDstPath(client_socket) == 0)
                break;
            FD_SET cpy_read_set;
            FD_SET read_set;
            TIMEVAL time;
            FD_ZERO(&read_set);
            FD_ZERO(&cpy_read_set);
            FD_SET(client_socket, &read_set);

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
                    Header header;
                    int recvlen = recv(client_socket, (char*)&header, sizeof(header), 0);
                    if (recvlen == -1)
                    {
                        std::cout << "server_off" << std::endl;
                        FD_CLR(client_socket, &cpy_read_set);
                        closesocket(client_socket);
                        WSACleanup();


                        end_signal = true;
                        break;
                        // 다시 접속 시도할수 있도록 개선
                        // thread로 분리하고
                    }

                    if (header.type == Header::TYPE::ERROR_SIG)
                    {
                        std::cout << header.file_path << std::endl;
                        break;
                    }
                    else if (header.type == Header::TYPE::NOT_EXIST_DIR)
                    {
                        CheckDstPath(client_socket);
                        continue;
                    }

                    else if (header.type == Header::TYPE::CHECK_DIR)
                    {
                        if (CopyDirectory(src_path, header.file_path, client_socket) == false)
                        {
                            std::cout << "fail to copy directory" << std::endl;
                            
                            std::cout << "server_off" << std::endl;
                            FD_CLR(client_socket, &cpy_read_set);
                            closesocket(client_socket);
                            WSACleanup();
                        }

                        break;

                        char yes_or_no;
                        std::cout << "do you want copy more?(y/n) : ";
                        std::cin >> yes_or_no;
                        if (yes_or_no == 'y')
                            break;
                        else
                        {
                            FD_CLR(client_socket, &cpy_read_set);
                            closesocket(client_socket);
                            WSACleanup();
                        }
                    }
                }
            }
        }
    }
    SOCKET ConnectThread()
    {
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == -1)
        {
            std::cout << "WSAStartup errno : " << errno << std::endl;
            return false;
        }

        SOCKET client_socket;
        client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_socket == -1)
        {
            std::cout << "socket errno : " << errno << std::endl;
            return false;
        }

        SOCKADDR_IN client_addr;
        client_addr.sin_family = AF_INET;
        inet_pton(AF_INET, SERVER_IP, &client_addr.sin_addr.s_addr);
        client_addr.sin_port = htons(PORT);

        if (connect(client_socket, (SOCKADDR*)&client_addr, sizeof(client_addr)) == -1)
        {
            std::cout << "connect errno : " << errno << std::endl;
            return false;
        }
        return client_socket;
    }
}

int main()
{
    SOCKET client_socket = ConnectThread();
    if (client_socket == 0)
    {
        std::cout << "Fail to Connect" << std::endl;
        //처리 어케 해
    }

    std::vector<std::thread> cpy_dir_threads;

    for (size_t i = 0; i < CPY_DIR_THREAD_SIZE; i++)
        cpy_dir_threads.push_back(std::thread(CopyDirectoryThread, client_socket));

    for (size_t i = 0; i < CPY_DIR_THREAD_SIZE; i++)
        cpy_dir_threads[i].join();

    cpy_dir_threads.clear();

    closesocket(client_socket);

    return 0;
}