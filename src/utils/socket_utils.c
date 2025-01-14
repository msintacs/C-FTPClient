/**
 * @file socket_utils.c
 * @brief 소켓 관련 유틸 구현
 * @details 소켓 사용에 필요한 각종 Utils 함수를 구현합니다.
 */

#include "socket.h"
#include "socket_utils.h"

int winSocketInit()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return -1;
    }
    return 0;
#else
    return 0;
#endif
}