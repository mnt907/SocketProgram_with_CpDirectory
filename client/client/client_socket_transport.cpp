#include "header.h"

const int PORT = 5555;
const int PACKET_SIZE = 1400;
const int COUNT = 1;
const int CPY_DIR_THREAD_SIZE = 1;
const int RECV_HEADER_THREAD_SIZE = 1;
const int CONNECT_THREAD_SIZE = 1;
const int RECONNECT_COUNT = 5;

bool end_signal = false;

namespace
{
#ifdef _WIN32	
    std::string MultiByteToUtf8(const std::string& multibyte_str)
    {
        char *pszIn = new char[multibyte_str.length() + 1];
        strncpy_s(pszIn, multibyte_str.length() + 1, multibyte_str.c_str(), multibyte_str.length());
        std::string resultString;
        int nLenOfUni = 0, nLenOfUTF = 0;
        wchar_t* uni_wchar = NULL;
        char* pszOut = NULL;

        if ((nLenOfUni = MultiByteToWideChar(CP_ACP, 0, pszIn, (int)strlen(pszIn), NULL, 0)) <= 0) return 0;
        uni_wchar = new wchar_t[nLenOfUni + 1];
        memset(uni_wchar, 0x00, sizeof(wchar_t)*(nLenOfUni + 1));

        nLenOfUni = MultiByteToWideChar(CP_ACP, 0, pszIn, (int)strlen(pszIn), uni_wchar, nLenOfUni);

        if ((nLenOfUTF = WideCharToMultiByte(CP_UTF8, 0, uni_wchar, nLenOfUni, NULL, 0, NULL, NULL)) <= 0)
        {
            delete[] uni_wchar;
            return 0;
        }
        pszOut = new char[nLenOfUTF + 1];
        memset(pszOut, 0, sizeof(char)*(nLenOfUTF + 1));

        nLenOfUTF = WideCharToMultiByte(CP_UTF8, 0, uni_wchar, nLenOfUni, pszOut, nLenOfUTF, NULL, NULL);
        pszOut[nLenOfUTF] = 0;
        resultString = pszOut;
        delete[] uni_wchar;
        delete[] pszOut;
        return resultString;
    }

    int64_t CheckFileSize(FILE* fp)
    {
        _fseeki64(fp, (__int64)0, SEEK_END);
        const int64_t FILE_SIZE = _ftelli64(fp);
        if (FILE_SIZE < 0)
        {
            std::cout << "can't get a file's offset" << std::endl;
            return false;
        }
        rewind(fp);

        return FILE_SIZE;
    }
#endif //_WIN32

#ifdef _WIN32
    namespace fs = std::experimental::filesystem;
#endif // _WIN32

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

    bool SendFileData(const std::string& src_path, const int& socket, Header& header)
    {
        FILE* src_fp = NULL;
#ifdef _WIN32
        int read_fp_err = fopen_s(&src_fp, src_path.c_str(), "rb");
        if (read_fp_err != 0)
        {
            std::cout << "can't create read_filepointer" << std::endl;
            return false;
        }

        header.length = CheckFileSize(src_fp);
#else
        src_fp = fopen64(src_path.c_str(), "rb");

        struct stat file_info;
        int stat_result = stat(src_path.c_str(), &file_info);
        if (stat_result != 0)
        {
            if (errno == EACCES)
                std::cout << "can't read file info " << src_path << std::endl;
        }
        header.length = file_info.st_size;

#endif // _WIN32

        int sendlen = send(socket, (char*)&header, sizeof(header), 0);
        if (sendlen == -1)
        {
            std::cout << "can't send header" << std::endl;
            return false;
        }

        std::cout << "file_path : " << header.file_path << std::endl;
        std::cout << "file_size : " << header.length << std::endl;
        std::cout << "is_directory : " << header.is_dir << std::endl;

        int64_t left_send_size = header.length;

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

    bool CopyDirectory(const std::string& src_path, const std::string& dst_path, const int& socket)
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
            const std::string DST_FILE_PATH = dst_path + file_path;

            std::string converted_dst_path = MultiByteToUtf8(DST_FILE_PATH);

            Header header;
            strncpy_s(header.file_path, sizeof(header.file_path), converted_dst_path.c_str(), converted_dst_path.length());
#else
        DIR *src_dirp = NULL;
        if ((src_dirp = opendir(src_path.c_str())) == NULL)
        {
            std::cout << "can't open directory" << src_path << std::endl;
            return false;
        }

        const struct dirent *dp;
        while ((dp = readdir(src_dirp)) != NULL)
        {
            const std::string filename = dp->d_name;
            const std::string SRC_FILE_PATH = src_path + "/" + filename;
            const std::string DST_FILE_PATH = dst_path + "/" + filename;

            Header header;
            strncpy(header.file_path, DST_FILE_PATH.c_str()
                , (sizeof(header.file_path) < DST_FILE_PATH.length()) ?
                sizeof(header.file_path) - 1 : DST_FILE_PATH.length());

            struct stat file_info;
            int result = lstat(SRC_FILE_PATH.c_str(), &file_info);

            if (result != 0)
            {
                if (errno == ENOENT)
                    std::cout << "File " << SRC_FILE_PATH << " not found.\n" << std::endl;
                else if (errno == EINVAL)
                    std::cout << "Invalid parameter to _stat.\n" << std::endl;
                else
                    std::cout << "Unexpected error in _stat.\n" << strerror(errno) << std::endl;

                continue;
            }
#endif //_WIN32

            header.type = Header::TYPE::CHECK_DIR;

#ifdef _WIN32

            if (fs::is_directory(fs::path(SRC_FILE_PATH)))
            {
#else
            if (S_ISDIR(file_info.st_mode))
            {
                if (filename == "." || filename == "..")
                    continue;
                else
                {
#endif //_WIN32
                    header.is_dir = true;
                    std::cout << "[directory name] " << SRC_FILE_PATH << std::endl;

                    int sendlen = send(socket, (char*)&header, sizeof(header), 0);
                    if (sendlen != sizeof(header))
                    {
                        std::cout << "can't send header" << std::endl;
#ifndef _WIN32
                        closedir(src_dirp);
#endif // !_WIN32
                        return false;
                    }
#ifndef _WIN32
                    CopyDirectory(SRC_FILE_PATH, DST_FILE_PATH, socket);
                }
#endif //_WIN32
            }
            else
            {
                header.is_dir = false;
                std::cout << "[file name] " << SRC_FILE_PATH << std::endl;
                if (SendFileData(SRC_FILE_PATH, socket, header) == false)
                {
                    std::cout << "fail to send file data " << SRC_FILE_PATH << std::endl;
#ifndef _WIN32
                    closedir(src_dirp);
#endif // !_WIN32
                    return false;
                }
            }
        }
#ifndef _WIN32
        closedir(src_dirp);
#endif // !_WIN32
        return true;
    }

    int ConnectClient(std::queue<int>* sockets, std::mutex* m)
    {
        int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_socket == -1)
        {
            std::cout << "socket errno : " << errno << std::endl;
            return 0;
        }

        sockaddr_in client_addr;
        client_addr.sin_family = AF_INET;
        std::cout << "input server_ip : ";
        //InputData().c_str() 172.18.62.41   192.168.56.101
#ifdef _WIN32
        inet_pton(AF_INET, "192.168.56.101", &client_addr.sin_addr.s_addr);
#else
        client_addr.sin_addr.s_addr = inet_addr("192.168.56.1");
#endif // _WIN32
        client_addr.sin_port = htons(PORT);

        if (connect(client_socket, (sockaddr*)&client_addr, sizeof(client_addr)) == -1)
        {
            std::cout << "connect errno : " << errno << std::endl;
            return false;
        }
        m->lock();
        sockets->push(client_socket);
        m->unlock();

        return client_socket;
    }

    int AttemptConnect(std::queue<int>* sockets, std::mutex* m)
    {
        int connect_result = 0;
        for (int i = 0; i < RECONNECT_COUNT; ++i)
        {
            connect_result = ConnectClient(sockets, m);
            if (connect_result == 0)
            {
                std::cout << "can't connect socket" << std::endl;
                continue;
            }
            else
                break;
        }
        return connect_result;
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
        std::string src_path;
        while (true)
        {
            //std::cout << "input src_path : ";
            //src_path = InputData();
#ifdef _WIN32
            src_path = "C:\\Users\\hunesion\\Desktop\\a";
#else
            src_path = "/home/mnt907/study/dst";
#endif //_WIN32
            if (IsExistDirectory(src_path) == false)
            {
                std::cout << "invalid src_path" << std::endl;
                continue;
            }
            break;
        }

        return src_path;
    }

    bool CheckDstPath(const int& socket)
    {
        std::string dst_path;

        //std::cout << "input dst_path : ";
        //dst_path = InputData();   /home/mnt907/study/dst  C:\\Users\\hunesion\\Desktop\\a

#ifdef _WIN32
        dst_path = "/home/mnt907/study/dst2";
#else
        dst_path = "C:\\Users\\hunesion\\Desktop\\d";
#endif //_WIN32

        Header header;
#ifdef _WIN32        
        strncpy_s(header.file_path, sizeof(header.file_path), dst_path.c_str(), dst_path.length());
#else
        strncpy(header.file_path, dst_path.c_str()
            , (sizeof(header.file_path) < dst_path.length()) ?
            sizeof(header.file_path) - 1 : dst_path.length());
#endif
        int sendlen = send(socket, (char*)&header, sizeof(header), 0);
        if (sendlen == -1)
        {
            std::cout << "can't send header" << std::endl;
            return false;
        }
        return true;
    }

    std::string CheckContinue(const int socket)
    {
        char yes_or_no;
        std::cout << "do you want copy more?(y/n) : ";
        std::cin >> yes_or_no;
        if (yes_or_no == 'y')
        {
            std::string src_path = CheckSrcPath();
            if (CheckDstPath(socket) == 0)
                std::cout << "Fail to send dst_path" << std::endl;
            return src_path;
        }
        else
            return "n";
    }

    void CopyDirectoryThread(const int& socket, std::queue<int>* sockets, std::mutex* m)
    {
        fd_set cpy_read_set;
        fd_set read_set;
        timeval time;
        FD_ZERO(&read_set);
        FD_ZERO(&cpy_read_set);

        FD_SET(socket, &read_set);
        std::string src_path = CheckSrcPath();
        if (CheckDstPath(socket) == 0)
        {
            std::cout << "Fail to send dst_path" << std::endl;
        }

        while (end_signal == false)
        {
            cpy_read_set = read_set;
            time.tv_sec = 1;
            time.tv_usec = 0;

            int req_count = select(socket + 1, &cpy_read_set, NULL, NULL, &time);

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

            if (FD_ISSET(socket, &cpy_read_set))
            {
                Header header;
                int recvlen = recv(socket, (char*)&header, sizeof(header), 0);
                if ((recvlen == -1 || recvlen == 0))
                {
                    std::cout << "server_off" << std::endl;
                    FD_CLR(socket, &read_set);
#ifdef _WIN32
                    closesocket(socket);
#else
                    close(socket);
#endif // _WIN32
                    int connect_result = AttemptConnect(sockets, m);
                    if (connect_result != 0)
                        FD_SET(connect_result, &read_set);

                    break;
                }

                if (header.type == Header::TYPE::ERROR_SIG)
                {
                    std::cout << header.file_path << std::endl;
                    continue;
                }
                else if (header.type == Header::TYPE::NOT_EXIST_DIR)
                {
                    CheckDstPath(socket);
                    continue;
                }

                else if (header.type == Header::TYPE::CHECK_DIR)
                {
                    if (CopyDirectory(src_path, header.file_path, socket) == false)
                    {
                        std::cout << "fail to copy directory" << std::endl;
                        continue;
                    }

                    src_path = CheckSrcPath();
                    if (CheckDstPath(socket) == 0)
                        std::cout << "Fail to send dst_path" << std::endl;
                    continue;

                    src_path = CheckContinue(socket);
                    if (!src_path.compare("n"))
                    {
                        FD_CLR(socket, &read_set);
#ifdef _WIN32
                        closesocket(socket);
#else
                        close(socket);
#endif // _WIN32
                        std::cout << "end send file data : " << socket << std::endl;
                        break;
                    }
                }
            }
        }
        std::cout << "end Copy Directory Thread" << std::endl;
    }

    void SignalHandler(int signum)
    {
        end_signal = true;
    }
}

int main()
{
    std::signal(SIGINT, SignalHandler);

#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == -1)
    {
        std::cout << "WSAStartup errno : " << errno << std::endl;
    }
#endif // _WIN32

    std::queue<int> sockets;
    std::mutex m;

    if (AttemptConnect(&sockets, &m) == 0)
        return 0;

    std::vector<std::thread> cpy_dir_threads;

    while (!end_signal)
    {
        m.lock();
        if (!sockets.empty())
        {
            cpy_dir_threads.push_back(std::thread(CopyDirectoryThread, sockets.front(), &sockets, &m));
            sockets.pop();
        }
        m.unlock();
    }

    for (unsigned int i = 0; i < cpy_dir_threads.size(); ++i)
        cpy_dir_threads[i].join();

    cpy_dir_threads.clear();

    std::cout << "end  test_client" << std::endl;
#ifdef _WIN32
    WSACleanup();
#endif // _WIN32

    return 0;
}
