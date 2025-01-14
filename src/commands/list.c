/**
 * @file list.c
 * @brief FTP List (ls) 기능 구현
 * @details FTP 서버의 디렉토리 리스트를 요청하고 받습니다.
 */

#include <stdio.h>
#include <string.h>

#include "cmd_control.h"
#include "connect.h"
#include "list.h"

// FTP List
int list(FTPClient *spFtpClient)
{
    int nResponse;
    int nCode;

    if (setupDataConnection(spFtpClient) < 0)
    {
        printf("List Command Failed.\n");
        return -1;
    }

    if (sendCommand(spFtpClient, "LIST", NULL) < 0)
    {
        printf("List Command Send Failed.\n");
        return -1;
    }

    nCode = receiveResponse(spFtpClient);
    if (nCode < 0)
    {
        printf("List ReceiveResponse Failed.\n");
        return -1;
    }

    if (spFtpClient->mode == MODE_ACTIVE)
    {
        if (acceptDataConnection(spFtpClient) < 0)
        {
            printf("List accept Failed.\n");
            return -1;
        }
    }

    memset(spFtpClient->sBuffer, 0, BUFFER_SIZE);

    while ((nResponse = recv(spFtpClient->dataSocket, spFtpClient->sBuffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        spFtpClient->sBuffer[nResponse] = '\0';
        printf("%s", spFtpClient->sBuffer);
        memset(spFtpClient->sBuffer, 0, BUFFER_SIZE);
    }

    CLOSE_SOCKET(spFtpClient->dataSocket);
    spFtpClient->dataSocket = INVALID_SOCKET;

    if (receiveResponse(spFtpClient) < 0)
    {
        printf("List ReceiveResponse Failed.\n");
        return -1;
    }

    return 0;
}