/**
 * @file socket.h
 * @brief OS 별 SOCKET 정의
 * @details OS 별로 SOCKET 을 정의하고 SOCKET 제어에 필요한 라이브러리 및 변수를 정의한다.
 */

#ifndef SOCKET_H
#define SOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define SOCKET_TYPE SOCKET
#define CLOSE_SOCKET(s) closesocket(s)
#define SOCKLEN_TYPE int
#define CLEAN_UP WSACleanup()
#define GET_SOCKET_ERROR WSAGetLastError()
#define REUSE_FLAG BOOL
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET_TYPE int
#define CLOSE_SOCKET(s) close(s)
#define SOCKLEN_TYPE socklen_t
#define CLEAN_UP
#define GET_SOCKET_ERROR errno
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define REUSE_FLAG int
#endif

#endif  // SOCKET_H