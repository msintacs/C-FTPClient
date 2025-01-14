/**
 * @file cmd_control.h
 * @brief 서버와의 명령어 송수신을 제어하는 헤더파일
 * @details
 *  - FTP 명령어를 상수로 정의합니다.
 *  - 사용자 입력을 FTP 명령어로 변환합니다.
 *  - 변환된 명령어를 처리합니다.
 *  - 서버로 명령어를 전송합니다.
 *  - 서버의 응답을 수신합니다.
 */

#ifndef CMD_CONTROL_H
#define CMD_CONTROL_H

#include "ftp_client.h"

/**
 * @brief FTP 명령어 타입 정의
 */
typedef enum
{
    CMD_UNKNOWN = -1,
    CMD_QUIT = 0,
    CMD_LIST,
    CMD_PWD,
    CMD_CD,
    CMD_GET,
    CMD_PUT,
    CMD_HELP
} CommandType;

/**
 * @brief 사용자의 입력을 FTP 명령어로 전환
 * @param cmd 사용자 입력 문자열
 * @return CommandType 변환 된 명령어 타입
 */
CommandType getCommandType(const char *cmd);

/**
 * @brief FTP 명령어를 서버로 전송
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체 (control 소켓 연결 완료상태)
 * @param command FTP 프로토콜 명령어
 * @param arg 명령어 인자
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int sendCommand(FTPClient *spFtpClient, const char *command, const char *args);

/**
 * @brief 서버로부터 응답 수신
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체 (control 소켓 연결 완료상태)
 * @return int 서버 응답 코드
 */
int receiveResponse(FTPClient *spFtpClient);

/**
 * @brief 사용자 입력 명령어 처리
 *
 * @param spFtpClient 클라이언트, 연결소켓정보가 포함된 구조체 (control 소켓 연결 완료상태)
 * @return int 결과성공시 0 반환, 결과실패 시 -1 반환
 */
int commandsHandle(FTPClient *spFtpClient);

#endif  // CMD_CONTROL_H