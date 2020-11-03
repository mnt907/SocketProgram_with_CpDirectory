#pragma once

#include "header.pb.h"
#include "include.h"

class ClientStatusManager
{
public:
    struct SocketInfo
    {
        int socket;
        Header header;

        SocketInfo();
        SocketInfo(int socket, Header header);
    };

private:
    std::queue<SocketInfo> m_client_status;
    std::mutex m_mutex;

public:
    ClientStatusManager();
    void Push(int socket, Header header);
    SocketInfo Pop();
    bool Empty();
};

