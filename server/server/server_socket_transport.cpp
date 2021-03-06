#include "header.h"
#include "ClientStatusManager.h"
#include "ClientSocketManager.h"

const int PORT = 5555;
const int PACKET_SIZE = 1300;
const int LISTEN_BACKLOG = 50;
const int COUNT = 1;
const int CPY_DIR_THREADS_SIZE = 2;
const int NUM_OF_RECV_HEADER_THREAD = 1;

namespace
{
#ifdef _WIN32
    std::string Utf8ToMultiByte(const std::string& multibyte_str)
    {
        if (multibyte_str.empty())
            return 0;

        char *pszIn = new char[multibyte_str.length() + 1];
        strncpy_s(pszIn, multibyte_str.length() + 1, multibyte_str.c_str(), multibyte_str.length());

        int nLenOfUni = 0;
        if ((nLenOfUni = MultiByteToWideChar(CP_UTF8, 0, pszIn, (int)strlen(pszIn), NULL, 0)) <= 0)
            return 0;

        wchar_t* uni_wchar = NULL;
        uni_wchar = new wchar_t[nLenOfUni + 1];
        memset(uni_wchar, 0x00, sizeof(wchar_t)*(nLenOfUni + 1));

        nLenOfUni = MultiByteToWideChar(CP_UTF8, 0, pszIn, (int)strlen(pszIn), uni_wchar, nLenOfUni);

        int nLenOfUTF = 0;
        if ((nLenOfUTF = WideCharToMultiByte(CP_ACP, 0, uni_wchar, nLenOfUni, NULL, 0, NULL, NULL)) <= 0)
        {
            delete[] uni_wchar;
            return 0;
        }

        char* pszOut = NULL;
        pszOut = new char[nLenOfUTF + 1];
        memset(pszOut, 0, sizeof(char)*(nLenOfUTF + 1));

        nLenOfUTF = WideCharToMultiByte(CP_ACP, 0, uni_wchar, nLenOfUni, pszOut, nLenOfUTF, NULL, NULL);
        pszOut[nLenOfUTF] = 0;
        std::string resultString = pszOut;
        delete[] uni_wchar;
        delete[] pszOut;
        return resultString;
    }
#endif // _WIN32
    bool SendToClient(const int socket, char* send_data)
    {
        int send_data_size = 0;
        memcpy(&send_data_size, send_data, sizeof(int));

        int byte_send_data_size = htonl(send_data_size);

#ifdef _WIN32
        int sendlen = send(socket, (char*)&byte_send_data_size, sizeof(int), 0);
#else
        int sendlen = send(socket, (char*)&byte_send_data_size, sizeof(int), MSG_NOSIGNAL);
#endif //_WIN32
        if (sendlen == -1)
        {
            std::cout << "can't send serialized header length" << std::endl;
            return false;
        }
        int send_size = PACKET_SIZE;
        while (send_data_size != 0)
        {
            if (send_data_size < send_size)
                send_size = send_data_size;
#ifdef _WIN32
            int sendlen = send(socket, send_data, send_size, 0);
#else 
            int sendlen = send(socket, send_data, send_size, MSG_NOSIGNAL);
#endif //_WIN32
            if (sendlen == -1)
            {
                std::cout << "can't send serialized header" << std::endl;
                return false;
            }
            send_data_size -= sendlen;
            send_data += sendlen;
        }
        return true;
    }
    
    bool RecvFromClient(const int socket, Header& header)
    {
        int byte_size_deserial_str = 0;
        int recvlen = recv(socket, (char*)&byte_size_deserial_str, sizeof(int), 0);
        if (recvlen == -1 || byte_size_deserial_str == 0)
        {
#ifdef _WIN32
            closesocket(socket);
#else
            close(socket);
#endif // _WIN32
            std::cout << "--------------------------finish----------------------------" << std::endl;
            return false;
        }
        int size_deserial_str = ntohl(byte_size_deserial_str);

        char* recv_data = new char[size_deserial_str + 1];
        memset(recv_data, 0, size_deserial_str + 1);
        char* recv_data_addr = recv_data;
        int recv_size = PACKET_SIZE;
        while (size_deserial_str != 0)
        {
            if (size_deserial_str < PACKET_SIZE)
                recv_size = size_deserial_str;

            int recvlen = recv(socket, recv_data, recv_size, 0);
            if (recvlen == -1)
            {
#ifdef _WIN32
                closesocket(socket);
#else
                close(socket);
#endif // _WIN32
                std::cout << "--------------------------finish----------------------------" << std::endl;
                return false;
            }
            recv_data += recvlen;
            size_deserial_str -= recvlen;
        }
        recv_data = recv_data_addr;
        header.Deserialize(recv_data);
        if (header.GetFilePath().empty())
        {
#ifdef _WIN32
            closesocket(socket);
#else
            close(socket);
#endif // _WIN32
            std::cout << "--------------------------finish----------------------------" << std::endl;
            return false;
        }
        delete[] recv_data;

        return true;
    }

    bool RecvFileData(const int socket, const Header& header)
    {
        FILE* dst_fp = NULL;

#ifdef _WIN32
        int write_fp_err = fopen_s(&dst_fp, header.GetFilePath().c_str(), "wb");
        if (write_fp_err != 0)
        {
            std::cout << "can't create write_filepointer" << write_fp_err << std::endl;
            if (write_fp_err == 13)
            {
                Header error_header;
                std::string error_msg = "wrong file or using file";
                error_header.SetType(Header::TYPE::ERROR_SIG);
                error_header.SetFilePath(error_msg);
                char* serial_str = error_header.Serialization();

                if (SendToClient(socket, serial_str) == false)
                    return false;
            }
            return false;
        }
#else 
        dst_fp = fopen(header.GetFilePath().c_str(), "wb");
#endif // _WIN32

        char recv_buf[PACKET_SIZE] = { 0 };

        int64_t sum_recv_len = header.GetLength();

        int recv_size = PACKET_SIZE;

        while (sum_recv_len != 0)
        {
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

    bool CopyDirectory(const int socket, const Header& header)
    {
        std::cout << "file_path : " << header.GetFilePath() << std::endl;
        std::cout << "file_size : " << header.GetLength() << std::endl;
        std::cout << "is_directory : " << header.GetIsDir() << std::endl;

        if (header.GetIsDir() == true)
        {
            std::cout << "make directory" << std::endl;
#ifdef _WIN32
            int result = _mkdir(header.GetFilePath().c_str());
#else
            int result = mkdir(header.GetFilePath().c_str(), 0666);
#endif // _WIN32
            if (result != 0)
            {
                if (errno == EEXIST)
                    std::cout << "already exist " << header.GetFilePath() << std::endl;
                else if (errno == ENOTDIR)
                {
                    std::cout << header.GetFilePath() << " is not directory" << std::endl;
                    return false;
                }
                else
                {
                    std::cout << "mkdir error :  " << errno << "  " << header.GetFilePath() << std::endl;
                    return false;
                }
            }
            std::cout << "success mkdir " << header.GetFilePath() << std::endl;
        }
        else
        {
            if (RecvFileData(socket, header) == false)
            {
                std::cout << "can't RecvFileData" << std::endl;
                return false;
            }
            //Sleep(1);
        }
        return true;
    }

    bool IsExistDirectory(const std::string& dir_path)
    {
#ifdef _WIN32
        namespace fs = std::experimental::filesystem;
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

    bool end_signal = false;
    void CopyDirectoryThread(ClientSocketManager* client_sockets, ClientStatusManager* client_status
        , std::condition_variable* cpy_cv, std::condition_variable* recv_cv, std::mutex* m)
    {
        std::cout << "make copy directory thread" << std::endl;
        while (end_signal == false)
        {
            std::cout << "Copy Directory Thread Num : " << std::this_thread::get_id() << std::endl;
            std::unique_lock<std::mutex> lock(*m);
            cpy_cv->wait(lock, [&] { return end_signal || !client_status->Empty(); });

            if (end_signal)
                break;

            ClientStatusManager::SocketInfo socket_info = client_status->Pop();
            if (socket_info.socket == 0)
                continue;

            lock.unlock();

            const int socket = socket_info.socket;
            Header header = socket_info.header;

            if (header.GetType() == Header::TYPE::CONNECT)
            {
                if (IsExistDirectory(header.GetFilePath()))
                {
                    header.SetIsDir(true);
                    header.SetType(Header::TYPE::CHECK_DIR);
                }
                else
                {
                    std::cout << "can't check Directory" << std::endl;
                    header.SetIsDir(false);
                    header.SetType(Header::TYPE::NOT_EXIST_DIR);
                }
                
                std::cout << "dst_path : " << header.GetFilePath() << std::endl;
                char* serial_str = header.Serialization();
                if (SendToClient(socket, serial_str) == false)
                {
                    std::cout << "can't send header" << std::endl;
#ifdef _WIN32
                    closesocket(socket);
#else
                    close(socket);
#endif // _WIN32
                    std::cout << "--------------------------finish----------------------------" << std::endl;
                    continue;
                }
            }
            else if (header.GetType() == Header::TYPE::CHECK_DIR)
            {
                if (CopyDirectory(socket, header) == false)
                {
                    std::cout << "fail to copy directory" << std::endl;

                    header.SetType(Header::TYPE::ERROR_SIG);
                    char* serial_str = header.Serialization();
                    if (SendToClient(socket, serial_str) == false)
                        std::cout << "can't send header" << std::endl;

#ifdef _WIN32
                    closesocket(socket);
#else
                    close(socket);
#endif // _WIN32
                    std::cout << "--------------------------finish----------------------------" << std::endl;
                    continue;
                }
                std::cout << "end file copy" << std::endl;
            }

            client_sockets->Push(socket);

            recv_cv->notify_one();
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
        std::cout << "end copy directory thread" << std::endl;
    }

    void RecvHeaderThread(ClientSocketManager* client_sockets, ClientStatusManager* client_status
        , std::condition_variable* cpy_cv, std::condition_variable* recv_cv, std::mutex* m)
    {
        std::cout << "make recv header thread" << std::endl;
        fd_set read_set;
        fd_set cpy_read_set;
        timeval time;
        FD_ZERO(&read_set);
        FD_ZERO(&cpy_read_set);

        while (true)
        {
            std::cout << "Recv Header Thread Num : " << std::this_thread::get_id() << std::endl;
            std::unique_lock<std::mutex> lock(*m);
            recv_cv->wait(lock, [&] { return end_signal || !client_sockets->Empty(); });

            lock.unlock();
            if (end_signal)
                break;
#ifndef _WIN32
            int fd_max = 0;
#endif // !_WIN32

            while (end_signal == false)
            {
                if (!client_sockets->Empty())
                {
                    int socket = client_sockets->Pop();
                    if (socket != -1)
                    {
                        FD_SET(socket, &read_set);
#ifndef _WIN32
                        if (fd_max < socket)
                            fd_max = socket;
#endif // !_WIN32
                    }
                }

                cpy_read_set = read_set;
                time.tv_sec = 1;
                time.tv_usec = 0;
#ifdef _WIN32 
                unsigned int fd_max = 0;
                for (unsigned int i = 0; i < cpy_read_set.fd_count; ++i)
                {
                    if (fd_max < cpy_read_set.fd_array[i])
                        fd_max = cpy_read_set.fd_array[i];
                }
#endif // _WIN32

                int req_count = select(fd_max + 1, &cpy_read_set, NULL, NULL, &time);

                if (req_count == -1)
                {
                    if (errno == 0)
                        continue;
                    else
                    {
                        std::cout << "req_count error : " << errno << std::endl;
                        continue;
                    }
                }

                if (req_count == 0)
                    continue;
#ifdef _WIN32
                for (unsigned int i = 0; i < cpy_read_set.fd_count; ++i)
                {
                    int socket = cpy_read_set.fd_array[i];
#else
                for (int i = 0; i < fd_max + 1; ++i)
                {
                    int socket = i;
#endif // _WIN32
                    if (FD_ISSET(socket, &cpy_read_set))
                    {
                        Header header;
                        if (RecvFromClient(socket, header) == false)
                        {
                            FD_CLR(socket, &read_set);
                            continue;
                        }
#ifdef _WIN32
                        const std::string converted_dst_path = Utf8ToMultiByte(header.GetFilePath());
                        header.SetFilePath(converted_dst_path);
#endif //_WIN32
                        client_status->Push(socket, header);

                        FD_CLR(socket, &read_set);
                        cpy_cv->notify_one();
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
        std::cout << "end recv header thread" << std::endl;
    }

    void SignalHandler(int signal)
    {
        end_signal = true;
    }

    void AcceptThread(const int server_socket, ClientSocketManager* client_sockets, std::condition_variable* recv_cv)
    {
        std::signal(SIGINT, SignalHandler);

        std::cout << "make accept thread" << std::endl;
        fd_set cpy_read_set;
        fd_set read_set;
        timeval time;
        FD_ZERO(&cpy_read_set);
        FD_ZERO(&read_set);
        FD_SET(server_socket, &read_set);

        while (end_signal == false)
        {
            cpy_read_set = read_set;
            time.tv_sec = 1;
            time.tv_usec = 0;

            int req_count = select(server_socket + 1, &cpy_read_set, NULL, NULL, &time);

            if (req_count == -1)
            {
                if (errno == 0)
                    continue;
                else
                {
                    std::cout << "req_count error : " << errno << std::endl;
                    continue;
                }
            }

            if (req_count == 0)
                continue;
            
            if (FD_ISSET(server_socket, &cpy_read_set))
            {
                sockaddr_in client_addr;
                socklen_t sockaddr_size = sizeof(sockaddr_in);
                int client_socket = accept(server_socket, (sockaddr*)&client_addr, &sockaddr_size);
                if (client_socket == -1)
                {
                    std::cout << "can't accept client_socket : " << std::endl;
                    continue;
                }

                client_sockets->Push(client_socket);

                recv_cv->notify_one();
            }
        }
        std::cout << "end access thread" << std::endl;
    }

    int BindListenThread()
    {
        int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket == -1)
        {
            std::cout << "socket errno : " << errno << std::endl;
            return -1;
        }
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(PORT);

        int enable = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)) < 0)
            std::cout << "setsockopt(SO_REUSEADDR) failed" << std::endl;

        if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        {
            std::cout << "bind errno : " << errno << std::endl;
            return -1;
        }

        if (listen(server_socket, LISTEN_BACKLOG) == -1)
        {
            std::cout << "listen errno : " << errno << std::endl;
            return -1;
        }

        return server_socket;
    }
}

int main()
{
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == -1)
    {
        std::cout << "WSAStartup errno : " << errno << std::endl;
        return 0;
    }
#endif // _WIN32

    int server_socket = 0;
    for (int i = 0; i < 5; ++i)
    {
        server_socket = BindListenThread();
        if (server_socket == -1)
        {
            std::cout << "Fail to Bind or Listen" << std::endl;
            continue;
        }
        else
            break;
    }
    ClientSocketManager client_sockets;
    ClientStatusManager client_status;

    std::condition_variable recv_cv;
    std::thread accept_thread(AcceptThread, server_socket, &client_sockets, &recv_cv);

    std::mutex m;
    std::vector<std::thread> recv_header_threads;
    std::condition_variable cpy_cv;
    for (int i = 0; i < NUM_OF_RECV_HEADER_THREAD; ++i)
        recv_header_threads.push_back(std::thread(RecvHeaderThread, &client_sockets, &client_status, &cpy_cv, &recv_cv, &m));

    std::vector<std::thread> cpy_dir_threads;
    for (int i = 0; i < CPY_DIR_THREADS_SIZE; ++i)
        cpy_dir_threads.push_back(std::thread(CopyDirectoryThread, &client_sockets, &client_status, &cpy_cv, &recv_cv, &m));

    accept_thread.join();

    end_signal = true;
    recv_cv.notify_all();
    cpy_cv.notify_all();

    for (size_t i = 0; i < recv_header_threads.size(); ++i)
        recv_header_threads[i].join();

    for (size_t i = 0; i < cpy_dir_threads.size(); ++i)
        cpy_dir_threads[i].join();

    recv_header_threads.clear();
    cpy_dir_threads.clear();


#ifdef _WIN32
    closesocket(server_socket);
    WSACleanup();

#else
    close(server_socket);
#endif // _WIN32


    return 1;
}