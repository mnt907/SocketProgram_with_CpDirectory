#pragma once

#include "include.h"

class ClientSocketManager
{
private:
    std::queue<int> m_client_sockets;
    std::mutex m_mutex;

public:
    ClientSocketManager();
    void Push(int socket);
    int Pop();
    bool Empty();
};