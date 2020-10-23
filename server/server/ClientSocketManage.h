#pragma once

#include "header.h"

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
        if (client_sockets.empty())
            return 0;

        int client_socket = client_sockets.front();
        client_sockets.pop();
        m.unlock();
        return client_socket;
    }

    bool Empty()
    {
        m.lock();
        bool result = client_sockets.empty();
        m.unlock();
        return result;
    }
};