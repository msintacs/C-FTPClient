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
#define COMMAND_BUFFER_SIZE 64
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

typedef enum { MODE_PASSIVE, MODE_ACTIVE } FTPMode;
typedef enum { CMD_UNKNOWN = -1, CMD_QUIT = 0, CMD_LIST, CMD_PWD, CMD_CD, CMD_GET, CMD_PUT, CMD_HELP } CommandType;

typedef struct FTPClient {
    FTPMode mode;
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

// Active MODE Accept
static int acceptDataConnection(FTPClient *obj) {
    SOCKET_TYPE tempSocket = obj->dataSocket;
    obj->dataSocket = accept(tempSocket, NULL, NULL);
    CLOSE_SOCKET(tempSocket);

    if (obj->dataSocket == INVALID_SOCKET) {
        printf("Accept failed: %d\n", GET_SOCKET_ERROR);
        return -1;
    }
    return 0;
}

// Active MODE
static int connectPORT(FTPClient *obj) {
    struct sockaddr_in localAddr, boundAddr;
    SOCKLEN_TYPE length = sizeof(localAddr);
    unsigned short port;
    unsigned char *ip;

    // 데이터 소켓 생성
    obj->dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (obj->dataSocket == INVALID_SOCKET) {
        printf("Data socket creation failed: %d\n", GET_SOCKET_ERROR);
        return -1;
    }

    // 컨트롤 소켓의 로컬 주소 정보 가져오기
    if (getsockname(obj->controlSocket, (struct sockaddr *)&localAddr, &length) == SOCKET_ERROR) {
        printf("getsockname failed: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(obj->dataSocket);
        return -1;
    }

    // 데이터 소켓 바인딩 준비
    memset(&boundAddr, 0, sizeof(boundAddr));
    boundAddr.sin_family = AF_INET;
    boundAddr.sin_addr = localAddr.sin_addr;
    boundAddr.sin_port = 0;

    // 바인딩
    if (bind(obj->dataSocket, (struct sockaddr *)&boundAddr, sizeof(boundAddr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(obj->dataSocket);
        return -1;
    }

    // 할당된 포트 확인
    length = sizeof(boundAddr);
    if (getsockname(obj->dataSocket, (struct sockaddr *)&boundAddr, &length) == SOCKET_ERROR) {
        printf("Get bound port failed: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(obj->dataSocket);
        return -1;
    }

    port = ntohs(boundAddr.sin_port);

    // PORT 명령 생성
    ip = (unsigned char *)&localAddr.sin_addr;
    snprintf(obj->command, BUFFER_SIZE, "PORT %d,%d,%d,%d,%d,%d\r\n", ip[0], ip[1], ip[2], ip[3], port >> 8, port & 0xFF);

    // PORT 명령 전송
    if (send(obj->controlSocket, obj->command, strlen(obj->command), 0) == SOCKET_ERROR) {
        printf("Failed to send PORT command: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(obj->dataSocket);
        return -1;
    }

    printf("PORT command: %s", obj->command);

    // 서버 응답 대기
    int code = receiveResponse(obj);
    if (code != 200) {
        printf("PORT command failed: %d\n", GET_SOCKET_ERROR);
        CLOSE_SOCKET(obj->dataSocket);
        return -1;
    }

    if (listen(obj->dataSocket, 1) == SOCKET_ERROR) {
        printf("Listen failed: %d", GET_SOCKET_ERROR);
        CLOSE_SOCKET(obj->dataSocket);
        return -1;
    }

    obj->isConneted = 1;

    return 0;
}

// Passive MODE
static int connectPASV(FTPClient *obj) {
    int a, b, c, d, p1, p2;

    if (sendCommand(obj, "PASV", NULL) < 0) {
        return -1;
    }

    receiveResponse(obj);
    char *start = strrchr(obj->buffer, '(');
    int parsed = sscanf(start + 1, "%d,%d,%d,%d,%d,%d", &a, &b, &c, &d, &p1, &p2);

    if (parsed != 6) {
        printf("paring failed\n");
        return -1;
    }

    sprintf(obj->dataIP, "%d.%d.%d.%d", a, b, c, d);
    obj->dataPort = p1 * 256 + p2;
    printf("Parsed IP: %s, Port: %d\n", obj->dataIP, obj->dataPort);

    obj->dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in dataAddr;
    memset(&dataAddr, 0, sizeof(dataAddr));
    dataAddr.sin_family = AF_INET;
    dataAddr.sin_port = htons(obj->dataPort);
    inet_pton(AF_INET, obj->dataIP, &dataAddr.sin_addr);

    if (connect(obj->dataSocket, (struct sockaddr *)&dataAddr, sizeof(dataAddr)) == SOCKET_ERROR) {
        printf("Data socket connection failed: %d\n", GET_SOCKET_ERROR);
        return -1;
    }

    obj->isConneted = 1;

    return 0;
}

static int setupDataConnection(FTPClient *obj) {
    if (obj->mode == MODE_PASSIVE) {
        return connectPASV(obj);
    } else {
        return connectPORT(obj);
    }
}

static CommandType getCommandType(const char *cmd) {
    if (strcmp(cmd, "cd") == 0) return CMD_CD;
    if (strcmp(cmd, "ls") == 0) return CMD_LIST;
    if (strcmp(cmd, "pwd") == 0) return CMD_PWD;
    if (strcmp(cmd, "get") == 0) return CMD_GET;
    if (strcmp(cmd, "put") == 0) return CMD_PUT;
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) return CMD_HELP;
    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) return CMD_QUIT;
    return CMD_UNKNOWN;
}

static int Upload(FTPClient *obj, const char *localPath) {
    int code;

    obj->file = fopen(localPath, "rb");
    if (obj->file == NULL) {
        printf("Cannot open file: %s\n", localPath);
        return -1;
    }

    if (setupDataConnection(obj) < 0) {
        return -1;
    }

    if (sendCommand(obj, "STOR", localPath) < 0) {
        return -1;
    }

    code = receiveResponse(obj);

    if (obj->mode == MODE_ACTIVE) {
        if (acceptDataConnection(obj) < 0) {
            return -1;
        }
    }

    if (code != 150) {
        printf("Failed to start STOR command: %s [errno: %d]\n", strerror(errno), errno);
        CLOSE_SOCKET(obj->dataSocket);
        return -1;
    }

    while ((obj->bytesRead = fread(obj->buffer, 1, BUFFER_SIZE, obj->file)) > 0) {
        if (send(obj->dataSocket, obj->buffer, obj->bytesRead, 0) < 0) {
            printf("Failed to send file data\n");
            fclose(obj->file);
            CLOSE_SOCKET(obj->dataSocket);
            return -1;
        }
        printf("%d bytes uploaded.\n", obj->bytesRead);
    }

    fclose(obj->file);
    CLOSE_SOCKET(obj->dataSocket);

    code = receiveResponse(obj);

    if (code != 226) {
        printf("Failed to complete STOR command\n");
        return -1;
    }

    return 0;
}

static int Download(FTPClient *obj, const char *remotePath, const char *localPath) {
    int code;
    int response;

    if (setupDataConnection(obj) < 0) {
        return -1;
    }

    if (sendCommand(obj, "RETR", remotePath) < 0) {
        return -1;
    }

    code = receiveResponse(obj);

    if (obj->mode == MODE_ACTIVE) {
        if (acceptDataConnection(obj) < 0) {
            return -1;
        }
    }

    if (code != 150) {
        printf("Failed to start RETR command\n");
        CLOSE_SOCKET(obj->dataSocket);
        return -1;
    }

    obj->file = fopen(localPath, "wb");
    if (obj->file == NULL) {
        printf("Failed to create local file: %s (errno: %d)\n", strerror(errno), errno);
        CLOSE_SOCKET(obj->dataSocket);
        return -1;
    }

    printf("downloading ...\n");
    while ((response = recv(obj->dataSocket, obj->buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(obj->buffer, 1, response, obj->file);
        printf("%d bytes done.\n", response);
    }

    fclose(obj->file);
    CLOSE_SOCKET(obj->dataSocket);

    code = receiveResponse(obj);
    if (code != 226) {
        printf("Failed to complete RETR command\n");
        return -1;
    }

    return 0;
}

static int List(FTPClient *obj) {
    int response;

    if (setupDataConnection(obj) < 0) {
        return -1;
    }

    if (sendCommand(obj, "LIST", NULL) < 0) {
        return -1;
    }

    receiveResponse(obj);

    if (obj->mode == MODE_ACTIVE) {
        if (acceptDataConnection(obj) < 0) {
            return -1;
        }
    }

    memset(obj->buffer, 0, BUFFER_SIZE);

    while ((response = recv(obj->dataSocket, obj->buffer, BUFFER_SIZE - 1, 0)) > 0) {
        obj->buffer[response] = '\0';
        printf("%s", obj->buffer);
        memset(obj->buffer, 0, BUFFER_SIZE);
    }

    CLOSE_SOCKET(obj->dataSocket);
    obj->dataSocket = INVALID_SOCKET;

    receiveResponse(obj);

    return 0;
}

static int handleFTPCommands(FTPClient *obj) {
    char input[BUFFER_SIZE];
    char *args[10] = {0};
    int argc = 0;

    printf("%s mode.\n", obj->mode == MODE_PASSIVE ? "Passive" : "Active");

    while (1) {
        printf("ftp> ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            continue;
        }

        input[strcspn(input, "\n")] = 0;

        // 문자열 공백을 기준으로 분리
        char *token = strtok(input, " ");
        argc = 0;

        while (token != NULL && argc < 10) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }

        if (argc == 0) continue;

        CommandType cmd = getCommandType(args[0]);

        switch (cmd) {
            case CMD_QUIT: {
                return 0;
            }
            case CMD_LIST: {
                List(obj);
                break;
            }
            case CMD_PWD: {
                sendCommand(obj, "PWD", NULL);
                receiveResponse(obj);
                break;
            }
            case CMD_CD: {
                if (argc > 1) {
                    sendCommand(obj, "CWD", args[1]);
                    receiveResponse(obj);
                } else {
                    printf("Usage: cd <directory>\n");
                    continue;
                }
                break;
            }
            case CMD_GET: {
                switch (argc) {
                    case 2: {  // get remotefile
                        Download(obj, args[1], args[1]);
                        break;
                    }
                    case 3: {  // get remotefile localfile
                        Download(obj, args[1], args[2]);
                        break;
                    }
                    default: {
                        printf("Usage: get <remoteFile> [localPath]\n");
                        continue;
                    }
                }
                break;
            }
            case CMD_PUT: {
                if (argc < 2) {
                    printf("Usage: put <localPath>\n");
                    continue;
                }
                Upload(obj, args[1]);
                break;
            }
            case CMD_HELP: {
                printf("Available commands:\n");
                printf("\t%s\t-\t%s\n", "ls", "list files");
                printf("\t%s\t-\t%s\n", "pwd", "print working directory");
                printf("\t%s\t-\t%s\n", "cd", "cd <directory> change directory");
                printf("\t%s\t-\t%s\n", "get", "get <filepath> get file");
                printf("\t%s\t-\t%s\n", "put", "put <filepath> put file");
                printf("\t%s\t-\t%s\n", "quit", "Terminate");
                printf("\t%s\t-\t%s\n", "exit", "Terminate");
                break;
            }
            case CMD_UNKNOWN: {
                printf("Unknown command. Type 'help' for commands.\n");
                break;
            }
        }
    }

    return 0;
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
    char temp;
    while (1) {
        printf("1. Passive / 2.Active / 0.exit\n");
        printf("Select FTP Mode: ");
        scanf("%d", &select);

        while ((temp = getchar()) != '\n' && temp != EOF);

        switch (select) {
            case 1: {
                obj.mode = MODE_PASSIVE;
                handleFTPCommands(&obj);
                break;
            }
            case 2: {
                obj.mode = MODE_ACTIVE;
                handleFTPCommands(&obj);
                break;
            }
            case 0: {
                printf("Goodbye.\n");
                CLOSE_SOCKET(obj.controlSocket);
                CLEAN_UP;
                return;
            }
            default: {
                printf("Invalid Select Num.\n");
                break;
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