/**
 * @file download.c
 * @brief FTP Download 기능 구현 (get)
 * @details 사용자로부터 서버의 파일(경로)을 입력받아 다운로드 받습니다.
 *          로컬의 경로를 추가로 입력하면 다운로드 받을 경로 지정이 가능합니다.
 */

#include <stdio.h>

#include "cmd_control.h"
#include "connect.h"
#include "download.h"
#include "response_code.h"

// FTP Download
int download(FTPClient *spFtpClient, const char *sRemotePath, const char *sLocalPath)
{
    int nResponse;
    int nCode;

    if (setupDataConnection(spFtpClient) < 0)
    {
        printf("Download Connection Setup Failed.\n");
        return -1;
    }

    if (sendCommand(spFtpClient, "RETR", sRemotePath) < 0)
    {
        printf("Download Command Send Failed.\n");
        return -1;
    }

    nCode = receiveResponse(spFtpClient);

    if (spFtpClient->mode == MODE_ACTIVE)
    {
        if (acceptDataConnection(spFtpClient) < 0)
        {
            printf("Download Accept Failed.\n");
            return -1;
        }
    }

    if (nCode != FILE_STATUS_OK)
    {
        printf("Download Failed.\n");
        return -1;
    }

    spFtpClient->file = fopen(sLocalPath, "wb");
    if (spFtpClient->file == NULL)
    {
        printf("Failed to create local file.\n");
        CLOSE_SOCKET(spFtpClient->dataSocket);
        return -1;
    }

    printf("Downloading ...\n");
    while ((nResponse = recv(spFtpClient->dataSocket, spFtpClient->sBuffer, BUFFER_SIZE, 0)) > 0)
    {
        fwrite(spFtpClient->sBuffer, 1, nResponse, spFtpClient->file);
        printf("%d bytes done.\n", nResponse);
    }

    fclose(spFtpClient->file);
    CLOSE_SOCKET(spFtpClient->dataSocket);

    nCode = receiveResponse(spFtpClient);
    if (nCode != FILE_TRANSFER_SUCCESS)
    {
        printf("Failed to complete RETR command \n");
        return -1;
    }

    return 0;
}
