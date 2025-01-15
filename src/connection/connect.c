/**
 * @file connect.c
 * @brief FTP 서버와의 커넥션 관련 기능을 구현합니다.
 * @details
 *  - 서버정보 및 사용자 정보를 입력받습니다.
 *  - Passive / Active 각 모드상태에 따른 실제 연결을 구현합니다.
 *  - Active 모드에서는 클라이언트가 데이터 포트를 열고 대기하며
 *    서버가 해당 포트로 연결을 시도하므로 클라이언트 측에서 accept를 하도록 구현합니다.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd_control.h"
#include "connect.h"
#include "ftp_client.h"
#include "response_code.h"

REUSE_FLAG getReuseFlag()
{
#ifdef _WIN32
    return TRUE;
#else
    return 1;
#endif
}

// 서버 주소를 입력받아 구조체에 저장
int getServerAddress(FTPClient *spFtpClient)
{
    printf("Server IP address: ");
    if (scanf("%s", spFtpClient->sServerIP) != 1)
    {
        printf("Invalud Input Value.\n");
        return -1;
    };
    return 0;
}

// 로그인 정보를 입력받아 구조체에 저장
int getLoginInfo(FTPClient *spFtpClient)
{
    printf("Username: ");
    if (scanf("%s", spFtpClient->sUsername) != 1)
    {
        printf("Invalud Input Value.\n");
        return -1;
    }

    printf("Password: ");
    if (scanf("%s", spFtpClient->sPassword) != 1)
    {
        printf("Invalud Input Value.\n");
        return -1;
    }

    return 0;
}

// FTP 서버 연결 제어
int connection(FTPClient *spFtpClient)
{
    struct sockaddr_in serverAddr;
    int nCode;

    spFtpClient->controlSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (spFtpClient->controlSocket == INVALID_SOCKET)
    {
        printf("Socket create failed: %d\n", GET_SOCKET_ERROR);
        CLEAN_UP;
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_FTP_PORT);
    nCode = inet_pton(AF_INET, spFtpClient->sServerIP, &(serverAddr.sin_addr));
    if (nCode < 0)
    {
        printf("Invalid IP address.\n");
        CLOSE_SOCKET(spFtpClient->controlSocket);
        CLEAN_UP;
        return -1;
    }

    printf("Connection to %s:%d ...\n", spFtpClient->sServerIP, DEFAULT_FTP_PORT);
    nCode = connect(spFtpClient->controlSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (nCode == SOCKET_ERROR)
    {
        printf("Connection failed: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(spFtpClient->controlSocket);
        CLEAN_UP;
        return -1;
    }

    nCode = receiveResponse(spFtpClient);
    if (nCode != CONN_SUCCESS)
    {
        printf("Connection failed: %d\n", nCode);
        return -1;
    }

    return nCode;
}

// FTP 서버 연결 프로세스 실행
int connectFTP(FTPClient *spFtpClient)
{
    int nCode;
    int nFailCount = 0;

    nCode = getServerAddress(spFtpClient);
    if (nCode < 0)
    {
        printf("Failed to get Server Address.\n");
        return -1;
    }

    while (1)
    {
        if (nFailCount >= 3)
        {
            printf("Login failed 3times. Program terminate.\n");
            exit(1);
        }

        nCode = getLoginInfo(spFtpClient);
        if (nCode < 0)
        {
            printf("Failed to get Login Info.\n");
            return -1;
        }

        nCode = connection(spFtpClient);
        if (nCode != CONN_SUCCESS)
        {
            printf("Connection failed.\n");
            return -1;
        }

        nCode = sendCommand(spFtpClient, "USER", spFtpClient->sUsername);
        if (nCode < 0)
        {
            printf("Username send failed.\n");
            return -1;
        }

        nCode = receiveResponse(spFtpClient);
        if (nCode != LOGIN_ID_OK)
        {
            printf("Invalid Value.\n");
            return -1;
        }

        nCode = sendCommand(spFtpClient, "PASS", spFtpClient->sPassword);
        if (nCode < 0)
        {
            printf("Password send failed.\n");
            return -1;
        }

        nCode = receiveResponse(spFtpClient);
        if (nCode == LOGIN_FAIL)
        {
            printf("Login Fail, Please Re try.\n");
            nFailCount++;
            CLOSE_SOCKET(spFtpClient->controlSocket);
            continue;
        }

        return 0;
    }
}

// FTP Passive 연결
int connectPASV(FTPClient *spFtpClient)
{
    struct sockaddr_in dataAddr;
    int nCode;
    int nParsedCount;
    int nIpPart1, nIpPart2, nIpPart3, nIpPart4;
    int nPort1, nPort2;
    char *sPasvResponse;

    nCode = sendCommand(spFtpClient, "PASV", NULL);
    if (nCode < 0)
    {
        printf("PASV Connection failed.\n");
        return -1;
    }
    nCode = receiveResponse(spFtpClient);
    if (nCode != ENTER_PASSIVE)
    {
        printf("PASV Connection failed.\n");
        return -1;
    }

    sPasvResponse = strrchr(spFtpClient->sBuffer, '(');
    nParsedCount = sscanf(sPasvResponse + 1, "%d,%d,%d,%d,%d,%d", &nIpPart1, &nIpPart2, &nIpPart3, &nIpPart4, &nPort1, &nPort2);
    if (nParsedCount != 6)
    {
        printf("Parsing failed.\n");
        return -1;
    }

    sprintf(spFtpClient->sDataIP, "%d.%d.%d.%d", nIpPart1, nIpPart2, nIpPart3, nIpPart4);
    spFtpClient->nDataPort = nPort1 * 256 + nPort2;

    spFtpClient->dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&dataAddr, 0, sizeof(dataAddr));
    dataAddr.sin_family = AF_INET;
    dataAddr.sin_port = htons(spFtpClient->nDataPort);
    nCode = inet_pton(AF_INET, spFtpClient->sDataIP, &dataAddr.sin_addr);
    if (nCode < 0)
    {
        printf("PASV Connection failed.\n");
        CLOSE_SOCKET(spFtpClient->dataSocket);
        return -1;
    }

    nCode = connect(spFtpClient->dataSocket, (struct sockaddr *)&dataAddr, sizeof(dataAddr));
    if (nCode == SOCKET_ERROR)
    {
        printf("PASV Data Socket Connection Failed: %d\n", GET_SOCKET_ERROR);
        return -1;
    }

    return 0;
}

// FTP Active
int connectPORT(FTPClient *spFtpClient)
{
    struct sockaddr_in localAddr, boundAddr;
    SOCKLEN_TYPE length;
    REUSE_FLAG reuse;
    unsigned short nPort = REUSE_PORT;
    unsigned char *sIp;
    char sLocalInfo[COMMAND_BUFFER_SIZE];
    int nCode;

    // 데이터 소켓 생성
    spFtpClient->dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (spFtpClient->dataSocket == INVALID_SOCKET)
    {
        printf("Data Socket Creation Failed: %d\n", GET_SOCKET_ERROR);
        return -1;
    }

    // 컨트롤 소켓의 로컬 주소 정보 가져오기
    length = sizeof(localAddr);
    nCode = getsockname(spFtpClient->controlSocket, (struct sockaddr *)&localAddr, &length);
    if (nCode == SOCKET_ERROR)
    {
        printf("getsockname failed: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(spFtpClient->dataSocket);
        return -1;
    }

    // Port REUSE Setting
    reuse = getReuseFlag();
    if (setsockopt(spFtpClient->dataSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
    {
        printf("setsockipt SO_REUSEADDR Failed.\n");
        return -1;
    }

    // 데이터 소켓 바인딩 준비
    memset(&boundAddr, 0, sizeof(boundAddr));
    boundAddr.sin_family = AF_INET;
    boundAddr.sin_addr = localAddr.sin_addr;
    boundAddr.sin_port = REUSE_PORT;

    // 바인딩
    nCode = bind(spFtpClient->dataSocket, (struct sockaddr *)&boundAddr, sizeof(boundAddr));
    if (nCode == SOCKET_ERROR)
    {
        printf("Bind failed: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(spFtpClient->dataSocket);
        return -1;
    }

    // 할당된 Port 확인
    length = sizeof(boundAddr);
    nCode = getsockname(spFtpClient->dataSocket, (struct sockaddr *)&boundAddr, &length);
    if (nCode == SOCKET_ERROR)
    {
        printf("Get bound port failed: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(spFtpClient->dataSocket);
        return -1;
    }

    nPort = ntohs(boundAddr.sin_port);

    // PORT 명령 생성
    sIp = (unsigned char *)&localAddr.sin_addr;
    snprintf(sLocalInfo, COMMAND_BUFFER_SIZE, "%d,%d,%d,%d,%d,%d", sIp[0], sIp[1], sIp[2], sIp[3], nPort >> 8, nPort & 0xFF);

    // PORT 명령 전송
    nCode = sendCommand(spFtpClient, "PORT", sLocalInfo);
    if (nCode < 0)
    {
        printf("PORT Command Send Failed.\n");
        CLOSE_SOCKET(spFtpClient->dataSocket);
        return -1;
    }

    printf("Conn: %d.%d.%d.%d:%d\n", sIp[0], sIp[1], sIp[2], sIp[3], htons(nPort));

    // 서버 응답 대기
    nCode = receiveResponse(spFtpClient);
    if (nCode < 0)
    {
        printf("PORT Connection Failed.\n");
        CLOSE_SOCKET(spFtpClient->dataSocket);
        return -1;
    }

    nCode = listen(spFtpClient->dataSocket, 1);
    if (nCode == SOCKET_ERROR)
    {
        printf("Listen Failed.\n");
        CLOSE_SOCKET(spFtpClient->dataSocket);
        return -1;
    }

    return 0;
}

int acceptDataConnection(FTPClient *spFtpClient)
{
    SOCKET_TYPE tempSocket;

    tempSocket = spFtpClient->dataSocket;
    spFtpClient->dataSocket = accept(tempSocket, NULL, NULL);
    CLOSE_SOCKET(tempSocket);

    if (spFtpClient->dataSocket == INVALID_SOCKET)
    {
        printf("Accept Failed: %d\n", GET_SOCKET_ERROR);
        return -1;
    }

    return 0;
}

int setupDataConnection(FTPClient *spFtpClient)
{
    if (spFtpClient->mode == MODE_PASSIVE)
        return connectPASV(spFtpClient);
    else
        return connectPORT(spFtpClient);

    return 0;
}