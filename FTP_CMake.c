#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_IP_LENGTH 16
#define DEFAULT_FTP_PORT 21

#ifdef _WIN32
#define SOCKET_TYPE SOCKET
#define GET_SOCKET_ERROR WSAGetLastError()
#define CLEAN_UP WSACleanup()
#define CLOSE_SOCKET(s) closesocket(s)
#define SOCKLEN_TYPE int
#else
#define SOCKET_TYPE int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define GET_SOCKET_ERROR errno
#define CLEAN_UP
#define CLOSE_SOCKET(s) close(s)
#define SOCKLEN_TYPE socklen_t
#endif

typedef enum { MODE_PASSIVE, MODE_ACTIVE } FtpMode;

typedef struct FTPClient {
    FtpMode mode;
    SOCKET_TYPE controlSocket;
    SOCKET_TYPE dataSocket;
    FILE *file;
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char dataIP[MAX_IP_LENGTH];
    int dataPort;
    int isConneted;
    int bytesRead;
} FTPClient;

static int sendCommand(FTPClient *obj, const char *command, const char *value) {
    int code;

    memset(obj->command, 0, sizeof(obj->command));

    if (value == NULL) {
        snprintf(obj->command, BUFFER_SIZE, "%s\r\n", command);
    } else {
        snprintf(obj->command, BUFFER_SIZE, "%s %s\r\n", command, value);
    }

    code = send(obj->controlSocket, obj->command, strlen(obj->command), 0);
    if (code == SOCKET_ERROR) {
        printf("Failed to send command: %d\n", GET_SOCKET_ERROR);
        return -1;
    }

    return 0;
}

static int receiveResponse(FTPClient *obj) {
    memset(obj->buffer, 0, sizeof(obj->buffer));
    int response = recv(obj->controlSocket, obj->buffer, BUFFER_SIZE - 1, 0);
    int code;

    printf("%s\n", obj->buffer);
    sscanf(obj->buffer, "%d", &code);

    return code;
}

static int serverLogin(FTPClient *obj) {
    char username[12];
    char password[12];
    int code = 0;
    int count = 0;

    while (1) {
        if (count >= 3) {
            printf("Login failed 3 times. Program teminate.\n");
            exit(1);
        }

        printf("Username: ");
        scanf("%s", username);
        code = sendCommand(obj, "USER", username);
        if (code < 0) {
            return code;
        }

        code = receiveResponse(obj);
        if (code != 331) {
            printf("Invalid username\n");
        }

        printf("Password: ");
        scanf("%s", password);
        code = sendCommand(obj, "PASS", password);
        if (code < 0) {
            return code;
        }

        code = receiveResponse(obj);

        if (code == 230) {
            break;
        } else {
            if (count < 2) {
                printf("Re try.\n");
            }
            count++;
            continue;
        }
    }

    return 0;
}

static int connectToServer(FTPClient *obj) {
    char serverIP[MAX_IP_LENGTH];
    struct sockaddr_in serverAddr;
    int code;

    printf("Server IP address: ");
    scanf("%s", serverIP);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_FTP_PORT);

    code = inet_pton(AF_INET, serverIP, &(serverAddr.sin_addr));
    if (code <= 0) {
        printf("Invalid IP address.\n");
        CLOSE_SOCKET(obj->controlSocket);
        CLEAN_UP;
        return -1;
    }

    printf("Connection to %s:%d ...\n", serverIP, DEFAULT_FTP_PORT);
    code = connect(obj->controlSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (code == SOCKET_ERROR) {
        printf("Connection failed: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(obj->controlSocket);
        CLEAN_UP;
        return -1;
    }

    code = receiveResponse(obj);

    if (code != 220) {
        printf("Connect failed. [%d]", code);
        return -1;
    }

    return 0;
}

static int winSocketInit() {
#ifdef _WIN32
    WSADATA wsaData;
    int code = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (code != 0) {
        return -1;
    }
    return 0;
#else
    return 0;
#endif
}

static void work() {
    FTPClient obj = {0};
    int code;

    // Window OS Socket Init
    if (winSocketInit() < 0) {
        printf("WSAStartup failed.\n");
        return;
    }

    /*
     * AF_INET     : IPv4 사용
     * SOCK_TREAM  : TCP 통신
     * IPPROTO_TCP : TCP 프로토콜
     */
    obj.controlSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (obj.controlSocket == INVALID_SOCKET) {
        printf("Socket create failed: %d\n", GET_SOCKET_ERROR);
        CLEAN_UP;
        return;
    }

    connectToServer(&obj);
    serverLogin(&obj);

    int select;
    while (1) {
        printf("1. Passive / 2.Active / 0.exit\n");
        printf("Select FTP Mode: ");
        scanf("%d", &select);

        switch (select) {
            case 1: {
                printf("Passive\n");
            }
            case 2: {
                printf("Active\n");
            }
            case 3: {
                printf("bye bye.\n");
                return;
            }
            default: {
                printf("Invalid Select Num.\n");
            }
        }
    }
}

int main() {
    printf("===========================================\n");
    printf("\t\t%s\n", "FTP Client");
    printf("===========================================\n");

    work();

    return 0;
}