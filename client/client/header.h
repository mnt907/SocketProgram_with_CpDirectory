#pragma once
#include "include.h"

class Header
{
public:
    enum class TYPE
    {
        CONNECT,
        CHECK_DIR,
        NOT_EXIST_DIR,
        ERROR_SIG
    };
    //member변수는 앞
private:
    TYPE type;
    int64_t length;
    bool is_dir;
    std::string file_path;
    char* serial_str;
public:
    Header()
        : type(Header::TYPE::CONNECT)
        , length(0)
        , is_dir(false)
    {
    }
    ~Header()
    {
        delete[] serial_str;
    }

    void SetType(Header::TYPE type);
    void SetLength(int64_t length);
    void SetIsDir(bool is_dir);
    void SetFilePath(std::string file_path);

    Header::TYPE GetType();
    int64_t GetLength();
    bool GetIsDir();
    std::string GetFilePath();

    char* Serialization();
    void Deserialize(char* serial_str);
};

void Header::SetType(Header::TYPE type)
{
    this->type = type;
}
void Header::SetLength(int64_t length)
{
    this->length = length;
}
void Header::SetIsDir(bool is_dir)
{
    this->is_dir = is_dir;
}
void Header::SetFilePath(std::string file_path)
{
    this->file_path = file_path;
}


Header::TYPE Header::GetType()
{
    return type;
}
int64_t Header::GetLength()
{
    return length;
}
bool Header::GetIsDir()
{
    return is_dir;
}
std::string Header::GetFilePath()
{
    return file_path;
}

char* Header::Serialization()
{
    int serial_str_length = sizeof(type) + sizeof(length) + sizeof(is_dir) + file_path.length() + sizeof(int) * 2;
    serial_str = new char[serial_str_length + 1];
    memset(serial_str, 0, serial_str_length + 1);

    char* addr = serial_str;

    memcpy(serial_str, &serial_str_length, sizeof(int));
    serial_str += sizeof(int);

    memcpy(serial_str, &type, sizeof(type));
    serial_str += sizeof(type);

    memcpy(serial_str, &length, sizeof(length));
    serial_str += sizeof(length);

    memcpy(serial_str, &is_dir, sizeof(is_dir));
    serial_str += sizeof(is_dir);

    int path_len = file_path.length();
    memcpy(serial_str, &path_len, sizeof(int));
    serial_str += sizeof(int);

    memcpy(serial_str, file_path.c_str(), file_path.length());

    serial_str = addr;

    return serial_str;
}

void Header::Deserialize(char* serial_str)
{
    char* serial_str_addr = serial_str;

    serial_str += sizeof(int);

    memcpy(&type, serial_str, sizeof(type));
    serial_str += sizeof(type);

    memcpy(&length, serial_str, sizeof(length));
    serial_str += sizeof(length);

    memcpy(&is_dir, serial_str, sizeof(is_dir));
    serial_str += sizeof(is_dir);

    int path_len = 0;
    memcpy(&path_len, serial_str, sizeof(int));
    serial_str += sizeof(int);
    file_path = serial_str;

    serial_str = serial_str_addr;
}