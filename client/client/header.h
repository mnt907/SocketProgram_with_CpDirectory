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

private:
    TYPE m_type;
    int64_t m_length;
    bool m_is_dir;
    std::string m_file_path;
    char* m_serial_str;
public:
    Header();
    ~Header();

    void SetType(Header::TYPE type);
    void SetLength(int64_t length);
    void SetIsDir(bool is_dir);
    void SetFilePath(std::string file_path);

    Header::TYPE GetType() const;
    int64_t GetLength() const;
    bool GetIsDir() const;
    std::string GetFilePath() const;

    char* Serialization();
    void Deserialize(char* serial_str);
};
