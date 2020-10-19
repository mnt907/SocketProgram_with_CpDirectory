#pragma once
#include <csignal>
#include <iostream>
#include <sys/types.h>
#include <string.h>
#include <thread>
#include <condition_variable>
#include <map>
#include <queue>
#include <fstream>
#include <istream>
#include <mutex>
#include <string>
#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>  
#include <signal.h>  

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <conio.h>
#include <WS2tcpip.h>
#include <filesystem>
#include <direct.h>
#else
#include <sys/time.h> 
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <iconv.h>
#endif // _WIN32