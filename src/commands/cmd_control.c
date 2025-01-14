/**
 * @file commands.c
 * @brief 서버로 명령어 전송/응답 제어를 위한 구현파일
 * @date 2025-01-08
 * @details commands.h 의 인터페이스를 구현합니다.
 *          입력받은 명령어 치환값 반환 및 서버와의 전송/수신 로직을 포함합니다.
 */
#include <stdio.h>
#include <string.h>

#include "cmd_control.h"
#include "download.h"
#include "ftp_client.h"
#include "list.h"
#include "upload.h"

// 사용자로부터 입력받은 값 변환
CommandType getCommandType(const char *cmd)
{
    if (strcmp(cmd, "cd") == 0) return CMD_CD;
    if (strcmp(cmd, "ls") == 0) return CMD_LIST;
    if (strcmp(cmd, "pwd") == 0) return CMD_PWD;
    if (strcmp(cmd, "get") == 0) return CMD_GET;
    if (strcmp(cmd, "put") == 0) return CMD_PUT;
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) return CMD_HELP;
    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) return CMD_QUIT;
    return CMD_UNKNOWN;
}

// 서버로 명령어 전송
int sendCommand(FTPClient *spFtpClient, const char *command, const char *args)
{
    int nCode;

    memset(spFtpClient->sCommand, 0, sizeof(spFtpClient->sCommand));

    if (args == NULL)
        snprintf(spFtpClient->sCommand, COMMAND_BUFFER_SIZE, "%s\r\n", command);
    else
        snprintf(spFtpClient->sCommand, COMMAND_BUFFER_SIZE, "%s %s\r\n", command, args);

    nCode = send(spFtpClient->controlSocket, spFtpClient->sCommand, strlen(spFtpClient->sCommand), 0);
    if (nCode == SOCKET_ERROR)
    {
        printf("Failed to send command: %d\n", GET_SOCKET_ERROR);
        return -1;
    }

    return 0;
}

// 명령어 전송 후 응답 받음
int receiveResponse(FTPClient *spFtpClient)
{
    int nCode;
    int nResponse;

    memset(spFtpClient->sBuffer, 0, sizeof(spFtpClient->sBuffer));
    nResponse = recv(spFtpClient->controlSocket, spFtpClient->sBuffer, BUFFER_SIZE - 1, 0);
    if (nResponse < 0)
    {
        printf("Failed to receive: %d\n", GET_SOCKET_ERROR);
        return -1;
    }

    printf("%s\n", spFtpClient->sBuffer);
    sscanf(spFtpClient->sBuffer, "%d", &nCode);

    return nCode;
}

// 사용자로부터 입력받은 명령어에 따른 제어
int commandsHandle(FTPClient *spFtpClient)
{
    CommandType cmd;
    char sInput[COMMAND_BUFFER_SIZE];
    char *sArgs[10] = {0};
    char *sParts;
    int nArgc;
    int nCode;

    while (1)
    {
        printf("ftp> ");

        if (fgets(sInput, sizeof(sInput), stdin) == NULL) continue;

        sInput[strcspn(sInput, "\n")] = 0;
        sParts = strtok(sInput, " ");
        nArgc = 0;

        while (sParts != NULL && nArgc < 10)
        {
            sArgs[nArgc++] = sParts;
            sParts = strtok(NULL, " ");
        }

        if (nArgc == 0) continue;

        cmd = getCommandType(sArgs[0]);

        switch (cmd)
        {
            case CMD_QUIT:
                return 0;

            case CMD_LIST:
                list(spFtpClient);
                break;

            case CMD_PWD:
                if (sendCommand(spFtpClient, "PWD", NULL) < 0)
                {
                    printf("PWD Command Send Failed.\n");
                    return -1;
                }

                nCode = receiveResponse(spFtpClient);
                if (nCode < 0)
                {
                    printf("PWD ReceiveResponse Failed.\n");
                    return -1;
                }

                break;

            case CMD_CD:
                if (nArgc > 1)
                {
                    if (sendCommand(spFtpClient, "CWD", sArgs[1]) < 0)
                    {
                        printf("CD Command Send Failed.\n");
                        return -1;
                    }
                    nCode = receiveResponse(spFtpClient);
                    if (nCode < 0)
                    {
                        printf("CD ReceiveResponse Failed.\n");
                        return -1;
                    }
                }
                else
                {
                    printf("Usage: cd <directory>\n");
                    continue;
                }

                break;

            case CMD_GET:
                switch (nArgc)
                {
                    case 2:
                        if (download(spFtpClient, sArgs[1], sArgs[1]) < 0)
                        {
                            printf("Download Failed\n");
                            return -1;
                        }
                        break;

                    case 3:
                        if (download(spFtpClient, sArgs[1], sArgs[2]) < 0)
                        {
                            printf("Download Failed\n");
                            return -1;
                        }
                        break;

                    default:
                        printf("Usage: get <remoteFile> [localPath]\n");
                        continue;
                }
                break;

            case CMD_PUT:
                if (nArgc < 2)
                {
                    printf("Usage: put <localPath>\n");
                    continue;
                }
                if (upload(spFtpClient, sArgs[1]) < 0)
                {
                    printf("Upload Failed.\n");
                    return -1;
                }
                break;

            case CMD_HELP:
                printf("Available commands:\n");
                printf("\t%s\t-\t%s\n", "ls", "list files");
                printf("\t%s\t-\t%s\n", "pwd", "print working directory");
                printf("\t%s\t-\t%s\n", "cd", "cd <directory> change directory");
                printf("\t%s\t-\t%s\n", "get", "get <filepath> get file");
                printf("\t%s\t-\t%s\n", "put", "put <filepath> put file");
                printf("\t%s\t-\t%s\n", "quit", "Terminate");
                printf("\t%s\t-\t%s\n", "exit", "Terminate");
                break;

            case CMD_UNKNOWN:
                printf("Unknown command Type. 'help' for commands.\n");
                break;
        }
    }

    return 0;
}