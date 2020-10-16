#pragma once
#pragma once
#include <csignal>
#include <iostream>
#include <sys/types.h>
#include <string.h>
#include <thread>
#include <condition_variable>
#include <map>
#include <queue>
#include <fstream>
#include <istream>
#include <mutex>
#include <string>
#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>  
#include <signal.h>  

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <conio.h>
#include <WS2tcpip.h>
#include <filesystem>
#include <direct.h>
#else
#include <sys/time.h> 
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <iconv.h>
#endif // _WIN32

struct Header
{
	enum class TYPE
	{
		CONNECT,
		CHECK_DIR,
		NOT_EXIST_DIR,
		ERROR_SIG
	};

	TYPE type;
	int64_t length;
	bool is_dir;
	char file_path[256];
	//struct stat file_info;

	Header()
		: type(Header::TYPE::CONNECT)
		, length(0)
		, is_dir(false)
		//, file_info{}
	{
		memset(file_path, 0, sizeof(file_path));
	}

};


class ClientStatusManage
{
public:
    struct SocketInfo
    {
        int socket;
        Header header;

        SocketInfo()
            : socket(0)
        {
        }

        SocketInfo(int socket, Header header)
            : socket(socket)
            , header(header)
        {
        }

    };

private:
    std::queue<SocketInfo> client_status;
    std::mutex m;

public:
    void Push(int socket, Header header)
    {
        m.lock();
        client_status.push(SocketInfo(socket, header));
        m.unlock();
    }

    SocketInfo Pop()
    {
        m.lock();
        if (client_status.empty())
            std::cout << "dkjflakjf" << std::endl;

        SocketInfo socket_info = client_status.front();
        client_status.pop();
        m.unlock();
        return socket_info;
    }

    bool Empty()
    {
        if (client_status.empty())
            return true;
        else
            return false;
    }
};

class ClientSocketManage
{
private:
    std::queue<int> client_sockets;
    std::mutex m;

public:
    void Push(int socket)
    {
        m.lock();
        client_sockets.push(socket);
        m.unlock();
    }

    int Pop()
    {
        m.lock();
        int client_socket = client_sockets.front();
        client_sockets.pop();
        m.unlock();
        return client_socket;
    }

    bool Empty()
    {
        if (client_sockets.empty())
            return true;
        else
            return false;
    }
};