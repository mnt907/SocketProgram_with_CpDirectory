#include "ClientSocketManager.h"

ClientSocketManager::ClientSocketManager()
{
}
void ClientSocketManager::Push(int socket)
{
    m_mutex.lock();
    m_client_sockets.push(socket);
    m_mutex.unlock();
}

int ClientSocketManager::Pop()
{
    m_mutex.lock();
    if (m_client_sockets.empty())
    {
        m_mutex.unlock();
        return -1;
    }

    int client_socket = m_client_sockets.front();
    m_client_sockets.pop();
    m_mutex.unlock();
    return client_socket;
}

bool ClientSocketManager::Empty()
{
    m_mutex.lock();
    bool result = m_client_sockets.empty();
    m_mutex.unlock();
    return result;
}