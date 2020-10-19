#pragma once

#include "include.h"

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