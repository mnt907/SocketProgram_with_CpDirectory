#pragma once

#include "header.h"

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
        assert(!client_status.empty());
        SocketInfo socket_info = client_status.front();
        client_status.pop();
        m.unlock();
        return socket_info;
    }

    bool Empty()
    {
        m.lock();
        bool result = client_status.empty();
        m.unlock();
        return result;
    }

};

