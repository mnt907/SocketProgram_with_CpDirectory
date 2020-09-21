#include <iostream>
#include <sys/types.h>
#include <WinSock2.h>
#include <filesystem>
#include <conio.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555
#define PACKET_SIZE 1400
#define LISTEN_BACKLOG 50
#define COUNT 1
#define CPY_DIR_THREADS_SIZE 2

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
    bool RecvFileData(const SOCKET& socket, const Header& header)
    {
        errno_t write_fp_err;
        FILE* dst_fp = NULL;

        write_fp_err = fopen_s(&dst_fp, header.file_path, "wb");

        if (write_fp_err != 0)
        {
            std::cout << "can't create write_filepointer" << write_fp_err << std::endl;
            if (write_fp_err == 13)
            {
                Header error_header;
                std::string error_msg = "wrong file or using file";
                error_header.type = Header::TYPE::ERROR_SIG;
                strncpy_s(error_header.file_path, error_msg.c_str()
                    , (sizeof(error_header.file_path) < sizeof(error_msg)) ?
                    sizeof(error_header.file_path) - 1 : sizeof(error_msg));

                int sendlen = send(socket, (char*)&error_header, sizeof(error_header), 0);
                if (sendlen == -1)
                {
                    std::cout << "can't send header" << std::endl;
                    return false;
                }
            }
        }

        char recv_buf[PACKET_SIZE] = { 0 };
        __int64 sum_recv_len = header.length;
        int recv_size = PACKET_SIZE;

        while (sum_recv_len != 0) {
            memset(recv_buf, 0, PACKET_SIZE);
            if (sum_recv_len < recv_size)
                recv_size = (int)sum_recv_len;

            int recvlen = recv(socket, recv_buf, recv_size, 0);
            if (recvlen == -1)
            {
                std::cout << "can't recv file_data" << std::endl;
                fclose(dst_fp);
                return false;
            }

            int writelen = fwrite(recv_buf, recvlen, COUNT, dst_fp);
            if (writelen < COUNT)
            {
                std::cout << "can't write file_data" << std::endl;
                fclose(dst_fp);
                return false;
            }
            sum_recv_len -= recvlen;
        }
        fclose(dst_fp);
        return true;
    }

    bool DirectoryCopy(const SOCKET& socket, const Header& header)
    {
        std::cout << "file_path : " << header.file_path << std::endl;
        std::cout << "file_size : " << header.length << std::endl;
        std::cout << "is_directory : " << header.is_dir << std::endl;

        if (header.is_dir == true)
        {
            std::cout << "make directory" << std::endl;
            namespace fs = std::experimental::filesystem;
            fs::create_directory(header.file_path);
        }
        else
        {
            if (RecvFileData(socket, header) == false)
            {
                std::cout << "can't RecvFileData" << std::endl;
                return false;
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

    bool end_signal = false;
    void CopyDirectoryThread(std::map<SOCKET, Header>* client_status, std::mutex* m, std::condition_variable* cv, FD_SET* read_set)
    {
        std::cout << "make copy directory thread" << std::endl;
        while (true)
        {
            std::cout << "Copy Directory Thread Num : " << std::this_thread::get_id() << std::endl;
            std::unique_lock<std::mutex> lock(*m);
            cv->wait(lock, [&] { return end_signal || !client_status->empty(); });

            if (end_signal)
                break;

            std::map<SOCKET, Header>::iterator iter;
            iter = client_status->begin();
            const SOCKET socket = iter->first;
            Header header = iter->second;
            client_status->erase(iter);
            lock.unlock();

            if (header.type == Header::TYPE::CONNECT)
            {
                if (IsExistDirectory(header.file_path))
                {
                    header.is_dir = true;
                    header.type = Header::TYPE::CHECK_DIR;
                }
                else
                {
                    std::cout << "can't check Directory" << std::endl;
                    header.is_dir = false;
                    header.type = Header::TYPE::NOT_EXIST_DIR;
                }

                std::cout << "dst_path : " << header.file_path << std::endl;

                int sendlen = send(socket, (char*)&header, sizeof(header), 0);
                if (sendlen != sizeof(header))
                {
                    std::cout << "can't send header" << std::endl;
                    closesocket(socket);
                    continue;
                }
            }
            else if (header.type == Header::TYPE::CHECK_DIR)
            {
                if (DirectoryCopy(socket, header) == false)
                {
                    std::cout << "fail to copy directory" << std::endl;
                    std::cout << "--------------------------finish----------------------------" << std::endl;

                    header.type = Header::TYPE::ERROR_SIG;
                    int sendlen = send(socket, (char*)&header, sizeof(header), 0);
                    if (sendlen != sizeof(header))
                    {
                        std::cout << "can't send header" << std::endl;
                        continue;
                    }

                    closesocket(socket);
                    continue;
                }
                std::cout << "end file copy" << std::endl;
            }
            FD_SET(socket, read_set);
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
        std::cout << "end copy directory thread" << std::endl;
    }

    void AcceptThread(const SOCKET& server_socket, std::map<SOCKET, Header>* client_status, std::mutex* m, std::condition_variable* cv, FD_SET* read_set)
    {
        std::cout << "make accept thread" << std::endl;
        FD_SET cpy_read_set;
        TIMEVAL time;
        FD_ZERO(&cpy_read_set);
        FD_SET(server_socket, read_set);

        while (!_kbhit())
        {
            cpy_read_set = *read_set;
            time.tv_sec = 1;
            time.tv_usec = 0;

            unsigned int fd_max = server_socket;
            for (unsigned int i = 0; i < cpy_read_set.fd_count; i++) {
                if (fd_max < cpy_read_set.fd_array[i])
                    fd_max = cpy_read_set.fd_array[i];
            }

            int req_count = select(fd_max + 1, &cpy_read_set, NULL, NULL, &time);

            if (req_count == -1)
            {
                std::cout << "req_count error : " << errno << std::endl;
                continue;
            }

            if (req_count == 0)
                continue;

            for (unsigned int i = 0; i < cpy_read_set.fd_count; i++)
            {
                if (FD_ISSET(server_socket, &cpy_read_set))
                {
                    SOCKADDR_IN client_addr;
                    int sockaddr_size = sizeof(SOCKADDR_IN);
                    SOCKET client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &sockaddr_size);
                    if (client_socket == -1)
                    {
                        std::cout << "can't accept client_socket : " << std::endl;
                        continue;
                    }
                    FD_SET(client_socket, read_set);
                }
                else
                {
                    Header header;
                    int recvlen = recv(cpy_read_set.fd_array[i], (char*)&header, sizeof(header), 0);
                    if (recvlen == -1)
                    {
                        FD_CLR(cpy_read_set.fd_array[i], read_set);
                        closesocket(cpy_read_set.fd_array[i]);
                        std::cout << "--------------------------finish----------------------------" << std::endl;
                        break;
                    }
                    m->lock();
                    client_status->insert(std::pair<SOCKET, Header>(cpy_read_set.fd_array[i], header));
                    m->unlock();

                    FD_CLR(cpy_read_set.fd_array[i], read_set);
                    cv->notify_one();
                }
            }
        }
        std::cout << "end access thread" << std::endl;
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

    std::map<SOCKET, Header> client_status;
    std::mutex m;
    std::condition_variable cv;
    FD_SET read_set;
    FD_ZERO(&read_set);

    std::thread accept_thread(AcceptThread, server_socket, &client_status, &m, &cv, &read_set);

    std::vector<std::thread> cpy_dir_threads;

    for (int i = 0; i < CPY_DIR_THREADS_SIZE; i++)
        cpy_dir_threads.push_back(std::thread(CopyDirectoryThread, &client_status, &m, &cv, &read_set));

    accept_thread.join();

    end_signal = true;
    cv.notify_all();

    for (size_t i = 0; i < cpy_dir_threads.size(); i++)
        cpy_dir_threads[i].join();

    cpy_dir_threads.clear();

    closesocket(server_socket);

    WSACleanup();
    return true;
}