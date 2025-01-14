/**
 * @file upload.c
 * @brief FTP Upload (put) 기능 구현
 * @details 사용자(로컬) 파일을 서버로 업로드합니다.
 */

#include <stdio.h>

#include "cmd_control.h"
#include "connect.h"
#include "ftp_client.h"
#include "response_code.h"
#include "upload.h"

// FTP Upload
int upload(FTPClient *spFtpClient, const char *sLocalPath)
{
    int nCode;

    spFtpClient->file = fopen(sLocalPath, "rb");
    if (spFtpClient->file == NULL)
    {
        printf("Cannot open file: %s\n", sLocalPath);
        return -1;
    }

    if (setupDataConnection(spFtpClient) < 0)
    {
        printf("Upload Connect Setup Failed.\n");
        return -1;
    }

    if (sendCommand(spFtpClient, "STOR", sLocalPath) < 0)
    {
        printf("Upload Command Send Failed.\n");
        return -1;
    }

    nCode = receiveResponse(spFtpClient);
    if (nCode < 0)
    {
        printf("Upload ReceiveResponse Failed.\n");
        return -1;
    }

    if (spFtpClient->mode == MODE_ACTIVE)
    {
        if (acceptDataConnection(spFtpClient))
        {
            printf("Upload Accept Failed.\n");
            return -1;
        }
    }

    if (nCode != FILE_STATUS_OK)
    {
        printf("Failed to start STOR command.\n");
        return -1;
    }

    while ((spFtpClient->nBytesRead = fread(spFtpClient->sBuffer, 1, BUFFER_SIZE, spFtpClient->file)) > 0)
    {
        if (send(spFtpClient->dataSocket, spFtpClient->sBuffer, spFtpClient->nBytesRead, 0) < 0)
        {
            printf("Failed to send file data.\n");
            fclose(spFtpClient->file);
            CLOSE_SOCKET(spFtpClient->dataSocket);
            return -1;
        }
        printf("%d bytes uploaded.\n", spFtpClient->nBytesRead);
    }

    fclose(spFtpClient->file);
    CLOSE_SOCKET(spFtpClient->dataSocket);

    nCode = receiveResponse(spFtpClient);
    if (nCode != FILE_TRANSFER_SUCCESS)
    {
        printf("Failed to complete STOR command.\n");
        return -1;
    }
    return 0;
}