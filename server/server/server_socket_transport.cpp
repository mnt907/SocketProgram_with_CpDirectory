#include <iostream>
#include <sys/types.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <filesystem>
#include <conio.h>
#include <cassert>
#include <stdio.h>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#else
#include <sys/socket.h>

#endif

#pragma comment(lib, "ws2_32.lib")

#define PORT 5555
#define PACKET_SIZE 1400
#define LISTEN_BACKLOG 50
#define COUNT 1
#define ACCEPT_THREADS_SIZE 1
#define CPY_DIR_THREADS_SIZE 2

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

//Header::TYPE::PATH

namespace
{
    bool RecvFileData(const std::string& dst_path, SOCKET& socket, Header& header)
    {
        errno_t write_fp_err;
        FILE* dst_fp = NULL;
        write_fp_err = fopen_s(&dst_fp, dst_path.c_str(), "wb");
        if (write_fp_err != 0)
        {
            std::cout << "can't create write_filepointer" << write_fp_err << std::endl;
            if (write_fp_err == 13)
            {
                std::string error_msg = "wrong file or using file";
                header.type = Header::TYPE::ERROR_SIG;
                strncpy_s(header.file_path, error_msg.c_str()
                    , (sizeof(header.file_path) < sizeof(error_msg)) ?
                    sizeof(header.file_path) - 1 : sizeof(error_msg));

                int sendlen = send(socket, (char*)&header, sizeof(header), 0);
                if (sendlen != sizeof(header))
                {
                    std::cout << "can't send header" << std::endl;
                    return false;
                }
            }           
        }

        const __int64 FILE_SIZE = header.length;
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
                fclose(dst_fp);
                return false;
            }
            int writelen = fwrite(recv_buf, PACKET_SIZE, COUNT, dst_fp);
            if (writelen != COUNT)
            {
                std::cout << "can't write file_data" << std::endl;
                fclose(dst_fp);
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
                fclose(dst_fp);
                return false;
            }
            int writelen = fwrite(recv_buf, LAST_FILE_SIZE, COUNT, dst_fp);
            if (writelen != COUNT)
            {
                std::cout << "can't write file_data" << std::endl;
                fclose(dst_fp);
                return false;
            }
            memset(recv_buf, 0, LAST_FILE_SIZE);
        }

        fclose(dst_fp);
        return true;
    }

    bool DirectoryCopy(std::string dst_path, SOCKET socket, Header file_info)
    {
        std::cout << "file_path : " << file_info.file_path << std::endl;
        std::cout << "file_size : " << file_info.length << std::endl;
        std::cout << "is_directory : " << file_info.is_dir << std::endl;

        const std::string DST_FILE_PATH = dst_path + file_info.file_path;
        std::cout << "dst_file_path : " << DST_FILE_PATH << std::endl;

        if (file_info.is_dir == true)
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
    void CopyDirectoryThread(std::queue<SOCKET>* sockets, std::mutex* m, std::condition_variable* cv)
    {
        std::cout << "make copy directory thread" << std::endl;
        while (true)
        {
            std::cout << "Copy Directory Thread Num : " << std::this_thread::get_id() << std::endl;
            std::unique_lock<std::mutex> lock(*m);
            cv->wait(lock, [&] { return end_signal || !sockets->empty(); });
            //!sockets->empty()
            if (end_signal)
                break;

            SOCKET socket = sockets->front();
            sockets->pop();

            lock.unlock();

            FD_SET read_set;
            FD_SET cpy_read_set;
            TIMEVAL time;
            FD_ZERO(&read_set);
            FD_ZERO(&cpy_read_set);
            FD_SET(socket, &read_set);
            std::string dst_path;

            while (true)
            {
                std::cout << "Copy Directory Thread Num : " << std::this_thread::get_id() << std::endl;
                cpy_read_set = read_set;
                time.tv_sec = 1;
                time.tv_usec = 0;

                int req_count = select(socket + 1, &cpy_read_set, NULL, NULL, &time);

                if (req_count == -1)
                {
                    std::cout << "req_count error : " << errno << std::endl;
                    continue;
                }

                if (req_count == 0)
                    continue;

                if (FD_ISSET(socket, &cpy_read_set))
                {
                    Header header;
                    int recvlen = recv(socket, (char*)&header, sizeof(header), 0);
                    if (recvlen != sizeof(header))
                    {
                        std::cout << "can't recv file_data" << std::endl;
                        FD_CLR(socket, &read_set);
                        std::cout << "--------------------------finish----------------------------" << std::endl;
                        closesocket(socket);
                        break;
                    }

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
                            header.type = Header::TYPE::ERROR_SIG;
                            break;
                        }

                        dst_path = header.file_path;
                        std::cout << "dst_path : " << dst_path << std::endl;

                        int sendlen = send(socket, (char*)&header, sizeof(header), 0);
                        if (sendlen != sizeof(header))
                        {
                            std::cout << "can't send header" << std::endl;
                            break;
                        }
                    }
                    else if (header.type == Header::TYPE::CHECK_DIR)
                    {
                        if (DirectoryCopy(dst_path, socket, header) == false)
                        {
                            std::cout << "fail to copy directory" << std::endl;
                            FD_CLR(socket, &read_set);
                            std::cout << "--------------------------finish----------------------------" << std::endl;

                            header.type = Header::TYPE::ERROR_SIG;
                            int sendlen = send(socket, (char*)&header, sizeof(header), 0);
                            if (sendlen != sizeof(header))
                            {
                                std::cout << "can't send header" << std::endl;
                                break;
                            }

                            closesocket(socket);
                            break;
                        }
                        std::cout << "end file copy" << std::endl;
                    }
                    else if (header.type == Header::TYPE::FILE_SEND)
                    {
                        int sendlen = send(socket, (char*)&header, sizeof(header), 0);
                        if (sendlen != sizeof(header))
                        {
                            std::cout << "can't send header" << std::endl;
                            break;
                        }

                        FD_CLR(socket, &cpy_read_set);
                        std::cout << std::this_thread::get_id() << "--------------------------finish----------------------------" << std::endl;
                        closesocket(socket);
                        break;
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }  
        std::cout << "end copy directory thread" << std::endl;
    }

    void AcceptThread(SOCKET server_socket, std::queue<SOCKET>* sockets, std::mutex* m, std::condition_variable* cv)
    {
        FD_SET read_set;
        FD_SET cpy_read_set;
        TIMEVAL time;
        FD_ZERO(&read_set);
        FD_ZERO(&cpy_read_set);
        FD_SET(server_socket, &read_set);

        while (!_kbhit())
        {
            std::cout << "Accept Thread Num : "<< std::this_thread::get_id() << std::endl;
            cpy_read_set = read_set;
            time.tv_sec = 1;
            time.tv_usec = 0;
            int req_count = select(server_socket + 1, &cpy_read_set, NULL, NULL, &time);

            if (req_count == -1)
            {
                std::cout << "req_count error : " << errno << std::endl;
                continue;
            }

            if (req_count == 0)
                continue;

            Header header;

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
                //thread로 만들어야함

                m->lock();
                sockets->push(client_socket);
                m->unlock();

                cv->notify_one();
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

    std::queue<SOCKET> sockets;
    std::mutex m;
    std::condition_variable cv;

    std::vector<std::thread> cpy_dir_threads;
    std::vector<std::thread> accept_threads;

    for(int i=0; i < ACCEPT_THREADS_SIZE; i++)
        accept_threads.push_back(std::thread(AcceptThread, server_socket, &sockets, &m, &cv));

    for(int i=0; i < CPY_DIR_THREADS_SIZE; i++)
        cpy_dir_threads.push_back(std::thread(CopyDirectoryThread, &sockets, &m, &cv));
    
    for (size_t i = 0; i < accept_threads.size(); i++) 
        accept_threads[i].join();
        
    end_signal = true;
    cv.notify_all();

    for (size_t i = 0; i < cpy_dir_threads.size(); i++)
        cpy_dir_threads[i].join();

    cpy_dir_threads.clear();
    accept_threads.clear();

    closesocket(server_socket);

    WSACleanup();
    return true;
}