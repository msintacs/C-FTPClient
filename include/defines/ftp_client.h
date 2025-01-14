/**
 * @file ftp_client.h
 * @brief FTP 클라이언트 구현을 위한 헤더 파일
 * @date 2025-01-08
 * @details FTP 프로토콜을 사용하여 서버와 통신하기 위한 클라이언트 구조체와
 *          관련 상수들을 정의합니다.
 */

#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include <stdio.h>

#include "socket.h"

#define BUFFER_SIZE 1024
#define COMMAND_BUFFER_SIZE 64
#define MAX_IP_LENGTH 16
#define ID_LENGTH 32
#define PWD_LENGTH 32
#define DEFAULT_FTP_PORT 21

/**
 * @brief FTP 데이터 전송 모드 정의
 */
typedef enum
{
    MODE_PASSIVE,
    MODE_ACTIVE
} FTPMode;

/**
 * @brief FTP 클라이언트의 상태와 데이터를 관리하는 구조체
 */
typedef struct FTPClient
{
    FTPMode mode;
    SOCKET_TYPE controlSocket;
    SOCKET_TYPE dataSocket;

    char sServerIP[MAX_IP_LENGTH];
    char sUsername[ID_LENGTH];
    char sPassword[PWD_LENGTH];

    char sDataIP[MAX_IP_LENGTH];
    int nDataPort;

    char sCommand[COMMAND_BUFFER_SIZE];
    char sBuffer[BUFFER_SIZE];

    FILE *file;
    int nBytesRead;

} FTPClient;

#endif  // FTP_CLIENT_H