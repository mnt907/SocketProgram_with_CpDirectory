#include "ClientStatusManager.h"

ClientStatusManager::ClientStatusManager()
{
}

ClientStatusManager::SocketInfo::SocketInfo()
    : socket(0)
{
}

ClientStatusManager::SocketInfo::SocketInfo(int socket, Header header)
    : socket(socket)
    , header(header)
{
}

void ClientStatusManager::Push(int socket, Header header)
{
    m_mutex.lock();
    m_client_status.push(SocketInfo(socket, header));
    m_mutex.unlock();
}

ClientStatusManager::SocketInfo ClientStatusManager::Pop()
{
    m_mutex.lock();
    if (m_client_status.empty())
    {
        m_mutex.unlock();
        SocketInfo empty_socket_info;
        return empty_socket_info;
    }

    SocketInfo socket_info = m_client_status.front();
    m_client_status.pop();
    m_mutex.unlock();
    return socket_info;
}

bool ClientStatusManager::Empty()
{
    m_mutex.lock();
    bool result = m_client_status.empty();
    m_mutex.unlock();
    return result;
}
