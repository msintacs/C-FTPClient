/**
 * @file main.c
 * @brief 메인파일
 */

#include "cmd_control.h"
#include "connect.h"
#include "ftp_client.h"
#include "socket.h"
#include "socket_utils.h"

/**
 * @brief FTP Client 메인
 * Passive / Active 모드를 모두 지원하는 FTP 클라이언트를 구현한 프로그램입니다.
 * 명령어 입력을 통해 기본적인 FTP 작업을 수행할 수 있습니다.
 *
 * @details
 *  - 윈도우 소켓 초기화
 *  - FTP 서버와의 연결 제어
 *  - Passive / Active 모드 선택 제어
 *  - 사용자 명령어 처리
 *
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int main()
{
    FTPClient spFtpClient = {0};
    int nSelect;
    int nCode;
    int nTemp;

    printf("===========================================\n");
    printf("\t\t%s\n", "FTP Client");
    printf("===========================================\n");

    if (winSocketInit() < 0)
    {
        printf("WSAStartup failed.\n");
        return -1;
    }

    nCode = connectFTP(&spFtpClient);
    if (nCode < 0)
    {
        printf("Failed to Connect to FTP.\n");
        return -1;
    }

    while (1)
    {
        printf("1. Passive / 2. Active / 0.exit\n");
        printf("Select FTP Mode: ");
        if (scanf("%d", &nSelect) != 1)
        {
            printf("Invalid Value, Please enter a number (0, 1 or 2).\n");
            while (((nTemp = getchar()) != '\n') && nTemp != EOF);
            continue;
        }
        while (((nTemp = getchar()) != '\n') && nTemp != EOF);

        if (nSelect < 0 || nSelect > 2)
        {
            printf("Invalid Value, Please enter a number (0, 1 or 2).\n");
            continue;
        }

        switch (nSelect)
        {
            case 1:
                spFtpClient.mode = MODE_PASSIVE;
                commandsHandle(&spFtpClient);
                break;
            case 2:
                spFtpClient.mode = MODE_ACTIVE;
                commandsHandle(&spFtpClient);
                break;
            case 0:
                printf("Goodbye.\n");
                CLOSE_SOCKET(spFtpClient.controlSocket);
                CLEAN_UP;
                return -1;
            default:
                printf("Invalid Value, Please enter a number (0, 1 or 2).\n");
                break;
        }
    }

    return 0;
}