#include "header.h"

Header::Header()
    : m_type(Header::TYPE::CONNECT)
    , m_length(0)
    , m_is_dir(false)
{
}
Header::~Header()
{
    delete[] m_serial_str;
}

void Header::SetType(Header::TYPE type)
{
    m_type = type;
}
void Header::SetLength(int64_t length)
{
    m_length = length;
}
void Header::SetIsDir(bool is_dir)
{
    m_is_dir = is_dir;
}
void Header::SetFilePath(std::string file_path)
{
    m_file_path = file_path;
}


Header::TYPE Header::GetType() const
{
    return m_type;
}
int64_t Header::GetLength() const
{
    return m_length;
}
bool Header::GetIsDir() const
{
    return m_is_dir;
}
std::string Header::GetFilePath() const
{
    return m_file_path;
}

char* Header::Serialization()
{
    int serial_str_length = sizeof(m_type) + sizeof(m_length) + sizeof(m_is_dir) + m_file_path.length() + sizeof(int) * 2;
    m_serial_str = new char[serial_str_length + 1];
    memset(m_serial_str, 0, serial_str_length + 1);

    char* addr = m_serial_str;

    memcpy(m_serial_str, &serial_str_length, sizeof(int));
    m_serial_str += sizeof(int);

    memcpy(m_serial_str, &m_type, sizeof(m_type));
    m_serial_str += sizeof(m_type);

    memcpy(m_serial_str, &m_length, sizeof(m_length));
    m_serial_str += sizeof(m_length);

    memcpy(m_serial_str, &m_is_dir, sizeof(m_is_dir));
    m_serial_str += sizeof(m_is_dir);

    int path_len = m_file_path.length();
    memcpy(m_serial_str, &path_len, sizeof(int));
    m_serial_str += sizeof(int);

    memcpy(m_serial_str, m_file_path.c_str(), m_file_path.length());

    m_serial_str = addr;

    return m_serial_str;
}

void Header::Deserialize(char* serial_str)
{
    char* serial_str_addr = serial_str;

    serial_str += sizeof(int);

    memcpy(&m_type, serial_str, sizeof(m_type));
    serial_str += sizeof(m_type);

    memcpy(&m_length, serial_str, sizeof(m_length));
    serial_str += sizeof(m_length);

    memcpy(&m_is_dir, serial_str, sizeof(m_is_dir));
    serial_str += sizeof(m_is_dir);

    int path_len = 0;
    memcpy(&path_len, serial_str, sizeof(int));
    serial_str += sizeof(int);
    m_file_path = serial_str;

    serial_str = serial_str_addr;
}